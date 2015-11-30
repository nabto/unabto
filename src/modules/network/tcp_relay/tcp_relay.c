/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NETWORK

#include <unabto/unabto_env_base.h>
#include <unabto/unabto_util.h>
#include <modules/network/dns/dns_client.h>
#include "tcp_provider.h"
#include <modules/crc/Ipv4Checksum.h>

#ifndef TCP_RELAY_SERVICE_NAME
#define TCP_RELAY_SERVICE_NAME                      "testbs.nabto.com"
#endif

#ifndef TCP_RELAY_SERVICE_PORT
#define TCP_RELAY_SERVICE_PORT                      5581
#endif

#define MAXIMUM_PACKET_SIZE_WITH_HEADER             1300
#define MAXIMUM_PACKET_SIZE                         (MAXIMUM_PACKET_SIZE_WITH_HEADER - 4)
#define TCP_RELAY_MAXIMUM_NUMBER_OF_SOCKETS         4
#define SERVICE_ACTIVITY_TIMEOUT_PERIOD             22000 // if nothing is received from the TCP relay service within this period assume that the TCP connection is lost.

enum Command
{
	Command_OpenUdpSocket,
	Command_OpenUdpSocketConfirmation,
    Command_CloseUdpSocket,
    Command_CloseUdpSocketConfirmation,
    Command_ReadUdpSocket,
    Command_ReadUdpSocketConfirmation,
    Command_WriteUdpSocket,
    Command_WriteUdpSocketConfirmation,
    Command_Disconnect
};

enum
{
    SERVICE_CONNECTION_STATE_UNCONNECTED,
    SERVICE_CONNECTION_STATE_CONNECTING,
    SERVICE_CONNECTION_STATE_CONNECTED
};

enum
{
    RELAY_SOCKET_STATE_FREE,
    RELAY_SOCKET_STATE_PENDING_OPEN,
    RELAY_SOCKET_STATE_OPENING,
    RELAY_SOCKET_STATE_OPEN,
    RELAY_SOCKET_STATE_PENDING_CLOSE,
    RELAY_SOCKET_STATE_CLOSING
};

typedef struct
{
    uint8_t state;
    uint32_t localAddress;
    uint16_t localPort;
    uint8_t receivedPacket[MAXIMUM_PACKET_SIZE];
    uint16_t receivedPacketLength;
    uint32_t sourceAddress;
    uint16_t sourcePort;
} relaySocket;

static void handle_packet_from_relay_service(uint8_t* packet, uint16_t packetLength);
static void send_to_relay_service(uint8_t* packet, uint16_t length);

static tcp_socket serviceSocket = TCP_PROVIDER_INVALID_SOCKET;
static uint8_t serviceState = SERVICE_CONNECTION_STATE_UNCONNECTED;
static nabto_stamp_t serviceConnectionTimeout;
static relaySocket relaySockets[TCP_RELAY_MAXIMUM_NUMBER_OF_SOCKETS];

void tcp_relay_initialize(void)
{
    tcp_provider_initialize();
    memset(relaySockets, 0, sizeof(relaySockets));
}

void tcp_relay_tick(void)
{
    static uint8_t currentPacket[MAXIMUM_PACKET_SIZE_WITH_HEADER];
    static uint16_t receivedPacketLength;
    static uint16_t packetLength;
    
    switch(serviceState)
    {
    case SERVICE_CONNECTION_STATE_UNCONNECTED:
        serviceState = SERVICE_CONNECTION_STATE_CONNECTING;
        break;
        
        // ensure that there is an open connection to the TCP relay service
    case SERVICE_CONNECTION_STATE_CONNECTING:
        if(tcp_provider_connect(&serviceSocket, TCP_RELAY_SERVICE_NAME, TCP_RELAY_SERVICE_PORT))
        {
            receivedPacketLength = 0;
            nabtoSetFutureStamp(&serviceConnectionTimeout, SERVICE_ACTIVITY_TIMEOUT_PERIOD);
            serviceState = SERVICE_CONNECTION_STATE_CONNECTED;
        }
        break;
        
    case SERVICE_CONNECTION_STATE_CONNECTED:
        {
            const uint16_t HEADER_SIZE = 2 + 2;
            uint8_t i;
            
            // receive packets
            //   receive packet header (payload length and checksum)
            if(receivedPacketLength < HEADER_SIZE)
            {
                receivedPacketLength += tcp_provider_read(&serviceSocket, &currentPacket[receivedPacketLength], HEADER_SIZE - receivedPacketLength); // receive chunk of the header

                if(receivedPacketLength >= HEADER_SIZE)
                {
                    READ_U16(packetLength, &currentPacket[0]);
                }
            }

            //   receive actual packet
            if(receivedPacketLength >= HEADER_SIZE)
            {
                receivedPacketLength += tcp_provider_read(&serviceSocket, &currentPacket[receivedPacketLength], packetLength - (receivedPacketLength - HEADER_SIZE)); // receive chunk of the payload
                
                // has the whole packet been received?
                if((receivedPacketLength - HEADER_SIZE) == packetLength)
                {
                    if(packetLength >= 1) // packet must contain atleast command to be valid
                    {
                        if(Ipv4Checksum_Calculate(currentPacket, 2, packetLength + 2) == 0)
                        {
                            handle_packet_from_relay_service(&currentPacket[4], packetLength);
                        }
                    }

                    receivedPacketLength = 0;
                }
            }

            if(nabtoIsStampPassed(&serviceConnectionTimeout))
            {
                NABTO_LOG_ERROR(("Lost connection to TCP relay service!"));

                tcp_provider_disconnect(&serviceSocket);
                serviceState = SERVICE_CONNECTION_STATE_CONNECTING;

                // reopen sockets
                for(i = 0; i < TCP_RELAY_MAXIMUM_NUMBER_OF_SOCKETS; i++)
                {
                    relaySocket* rs = &relaySockets[i];
                    switch(rs->state)
                    {
                    case RELAY_SOCKET_STATE_OPENING:
                        rs->state = RELAY_SOCKET_STATE_PENDING_OPEN;
                        NABTO_LOG_TRACE(("Restarting socket open process for socket %u.", (int)i));
                        break;

                    case RELAY_SOCKET_STATE_OPEN:
                        rs->state = RELAY_SOCKET_STATE_PENDING_OPEN;
                        NABTO_LOG_TRACE(("Reopening socket %u.", (int)i));
                        break;

                    case RELAY_SOCKET_STATE_CLOSING:
                    case RELAY_SOCKET_STATE_PENDING_CLOSE:
                        memset(rs, 0, sizeof(relaySocket));
                        rs->state = RELAY_SOCKET_STATE_FREE;
                        NABTO_LOG_TRACE(("Force-closed socket %u.", (int)i));
                        break;
                    }
                }
            }
            else
            {
                // maintain relay channels
                for(i = 0; i < TCP_RELAY_MAXIMUM_NUMBER_OF_SOCKETS; i++)
                {
                    relaySocket* rs = &relaySockets[i];
                    switch(rs->state)
                    {
                    case RELAY_SOCKET_STATE_PENDING_OPEN:
                        {
                            uint8_t packet[MAXIMUM_PACKET_SIZE_WITH_HEADER];
                            uint8_t* p = &packet[4];
                            WRITE_FORWARD_U8(p, Command_OpenUdpSocket);
                            WRITE_FORWARD_U8(p, i);
                            send_to_relay_service(packet, 2);

                            rs->state = RELAY_SOCKET_STATE_OPENING;
                            
                            NABTO_LOG_TRACE(("Socket %u is waiting for open confirmation...", (int)i));
                        }
                        break;

                    case RELAY_SOCKET_STATE_PENDING_CLOSE:
                        {
                            uint8_t packet[MAXIMUM_PACKET_SIZE_WITH_HEADER];
                            uint8_t* p = &packet[4];
                            WRITE_FORWARD_U8(p, Command_CloseUdpSocket);
                            WRITE_FORWARD_U8(p, i);
                            send_to_relay_service(packet, 2);

                            rs->state = RELAY_SOCKET_STATE_CLOSING;
                            NABTO_LOG_TRACE(("Socket %u is waiting for close confirmation...", (int)i));
                        }
                        break;
                    }
                }
            }
        }
        break;
    }
}

static void handle_packet_from_relay_service(uint8_t* packet, uint16_t packetLength)
{
    bool success = false;
    uint8_t command;

    READ_FORWARD_U8(command, packet);
    
    switch(command)
    {
    case Command_OpenUdpSocketConfirmation:
        {
            relaySocket* rs;
            uint8_t index;
            
            READ_FORWARD_U8(index, packet);
            
            if(index >= TCP_RELAY_MAXIMUM_NUMBER_OF_SOCKETS) // invalid index?
            {
                NABTO_LOG_TRACE(("Dumped packet for invalid socket %u!", (int)index));
                break;
            }

            rs = &relaySockets[index];

            if(rs->state != RELAY_SOCKET_STATE_OPENING) // only accepted when opening the socket
            {
                NABTO_LOG_TRACE(("Dumped packet for socket %u not in opening state.", (int)index));
                break;
            }

            READ_FORWARD_U32(rs->localAddress, packet);
            READ_FORWARD_U16(rs->localPort, packet);

            rs->state = RELAY_SOCKET_STATE_OPEN;
    
            success = true;

            NABTO_LOG_TRACE(("Socket %u bound to " PRIip ":%u", (int)index, MAKE_IP_PRINTABLE(rs->localAddress), rs->localPort));
        }
        break;
        
    case Command_CloseUdpSocketConfirmation:
        {
            relaySocket* rs;
            uint8_t index;
            
            READ_FORWARD_U8(index, packet);
                    
            if(index >= TCP_RELAY_MAXIMUM_NUMBER_OF_SOCKETS) // invalid index?
            {
                NABTO_LOG_TRACE(("Dumped packet for invalid socket %u!", (int)index));
                break;
            }

            rs = &relaySockets[index];

            if(rs->state != RELAY_SOCKET_STATE_CLOSING) // only accepted when closing the socket
            {
                NABTO_LOG_TRACE(("Dumped packet for socket %u not in closing state.", (int)index));
                break;
            }

            memset(rs, 0, sizeof(relaySocket));
    
            success = true;
    
            NABTO_LOG_TRACE(("Socket %u released.", (int)index));
        }
        break;

    case Command_ReadUdpSocket:
        {
            relaySocket* rs;
            uint8_t index;
            uint32_t address;
            uint16_t port;
            uint16_t payloadLength;
            
            READ_FORWARD_U8(index, packet);
                    
            if(index >= TCP_RELAY_MAXIMUM_NUMBER_OF_SOCKETS) // invalid index?
            {
                NABTO_LOG_TRACE(("Dumped packet for invalid socket %u!", (int)index));
                break;
            }

            rs = &relaySockets[index];

            if(rs->state != RELAY_SOCKET_STATE_OPEN) // unused index?
            {
                NABTO_LOG_TRACE(("Dumped packet for unopened socket %u.", (int)index));
                break;
            }

            if(rs->receivedPacketLength != 0) // is the one-level queue for that socket full?
            {
                NABTO_LOG_TRACE(("Dumped packet for socket %u - queue full!", (int)index));
                break;
            }

            READ_FORWARD_U32(address, packet);
            READ_FORWARD_U16(port, packet);
            READ_FORWARD_U16(payloadLength, packet);

            if(payloadLength > MAXIMUM_PACKET_SIZE) // dump oversize packets
            {
                NABTO_LOG_TRACE(("Dumped packet for socket %u - oversize packet!", (int)index));
                break;
            }
                    
            rs->sourceAddress = address;
            rs->sourcePort = port;
            rs->receivedPacketLength = payloadLength;
            memset(rs->receivedPacket, 0, MAXIMUM_PACKET_SIZE);
            memcpy(rs->receivedPacket, packet, payloadLength);
    
            success = true;
    
            {
                uint8_t replyPacket[MAXIMUM_PACKET_SIZE_WITH_HEADER];
                uint8_t* p = &replyPacket[4];
    
                WRITE_FORWARD_U8(p, Command_ReadUdpSocketConfirmation);
                WRITE_FORWARD_U8(p, index);
                WRITE_FORWARD_U8(p, 0); // dummy sequence number

                send_to_relay_service(replyPacket, p - replyPacket - 2 - 2);
            }

            //NABTO_LOG_TRACE(("Queued packet for socket %u. source: " PRIip ":%u", (int)index, MAKE_IP_PRINTABLE(rc->sourceAddress), rc->sourcePort));
        }
        break;

    case Command_WriteUdpSocketConfirmation:
        {
            relaySocket* rs;
            uint8_t index;
            uint8_t sequenceNumber;
            
            READ_FORWARD_U8(index, packet);
                    
            if(index >= TCP_RELAY_MAXIMUM_NUMBER_OF_SOCKETS) // invalid index?
            {
                NABTO_LOG_TRACE(("Dumped packet for invalid socket %u!", (int)index));
                break;
            }

            rs = &relaySockets[index];

            if(rs->state != RELAY_SOCKET_STATE_OPEN) // unused index?
            {
                NABTO_LOG_TRACE(("Dumped packet for unopened socket %u.", (int)index));
                break;
            }

            READ_FORWARD_U8(sequenceNumber, packet);
    
            // no check is performed on the sequence number - it is only used to keep the TCP connection alive.

            success = true;
    
            NABTO_LOG_TRACE(("Received confirmation of write packet %u on socket %u.", (int)sequenceNumber, (int)index));
            break;
        }
    }

    if(success)
    {
        // reset timeout when a valid packet has been received.
        nabtoSetFutureStamp(&serviceConnectionTimeout, SERVICE_ACTIVITY_TIMEOUT_PERIOD);
    }
}

static void send_to_relay_service(uint8_t* packet, uint16_t length)
{
    WRITE_U16(&packet[0], length);
    WRITE_U16(&packet[2], Ipv4Checksum_Calculate(packet, 4, length));
    
    tcp_provider_write(&serviceSocket, packet, 2 + 2 + length);
}

bool nabto_init_socket(uint32_t localAddr, uint16_t* localPort, nabto_socket_t* socketIdentifier)
{
    uint8_t packet[4];
    uint8_t* p = packet;
    uint8_t i;

    if(localAddr != INADDR_ANY) // local IP is determined by the TCP relay service and can not be specified by the client
    {
        NABTO_LOG_FATAL(("TCP relay does not support binding to a specific IP address!"));
        return false;
    }
    
    if(*localPort != 0) // local UDP port is determined by the TCP relay service and can not be specified by the client
    {
        NABTO_LOG_FATAL(("TCP relay does not support binding to a specific local port!"));
        return false;
    }

    for(i = 0; i < TCP_RELAY_MAXIMUM_NUMBER_OF_SOCKETS; i++)
    {
        relaySocket* rs = &relaySockets[i];
        if(rs->state == RELAY_SOCKET_STATE_FREE)
        {
            rs->state = RELAY_SOCKET_STATE_PENDING_OPEN;
            rs->receivedPacketLength = 0;

            *socketIdentifier = i;
            
            NABTO_LOG_TRACE(("Initiated socket open of socket %u", (int)i));

            return true;
        }
    }

    *socketIdentifier = NABTO_INVALID_SOCKET;

    NABTO_LOG_ERROR(("Failed to open socket - no free connection slots!"));

    return false;
}

void nabto_close_socket(nabto_socket_t* socketIdentifier)
{
    relaySocket* rs = &relaySockets[*socketIdentifier];
    uint8_t packet[4];
    uint8_t* p = packet;
    
    if(*socketIdentifier >= TCP_RELAY_MAXIMUM_NUMBER_OF_SOCKETS)
    {
        NABTO_LOG_ERROR(("Tried to close invalid socket %u!", (int)socketIdentifier));
        return;
    }
    
    if(rs->state == RELAY_SOCKET_STATE_FREE)
    {
        NABTO_LOG_ERROR(("Tried to close unopened socket %u!", (int)socketIdentifier));
        return;
    }
    
    rs->state = RELAY_SOCKET_STATE_PENDING_CLOSE;
    
    NABTO_LOG_TRACE(("Closed socket %u.", (int)*socketIdentifier));
}

ssize_t nabto_read(nabto_socket_t socketIdentifier, uint8_t* buf, size_t len, uint32_t* addr, uint16_t* port)
{
    relaySocket* rs = &relaySockets[socketIdentifier];
    uint16_t length = 0;

    if(socketIdentifier >= TCP_RELAY_MAXIMUM_NUMBER_OF_SOCKETS)
    {
        NABTO_LOG_ERROR(("nabto_read: Invalid socket (%u)!", (int)socketIdentifier));
        return 0;
    }
    
    if(rs->state == RELAY_SOCKET_STATE_FREE)
    {
        NABTO_LOG_ERROR(("nabto_read: Unopened socket (%u)!", (int)socketIdentifier));
        return 0;
    }
    
    if(rs->state != RELAY_SOCKET_STATE_OPEN)
    {
        NABTO_LOG_TRACE(("nabto_read: Socket not fully opened (%u)!", (int)socketIdentifier));
        return 0;
    }
    
    if(rs->receivedPacketLength == 0)
    {
        return 0;
    }
    
    // copy the packet if the user-provided buffer is large enough
    if(rs->receivedPacketLength <= len)
    {
        *addr = rs->sourceAddress;
        *port = rs->sourcePort;
        length = rs->receivedPacketLength;
        memcpy(buf, rs->receivedPacket, length);
    }
    
    // remove packet from the sockets queue
    rs->sourceAddress = 0;
    rs->sourcePort = 0;
    rs->receivedPacketLength = 0;
 
    NABTO_LOG_TRACE(("nabto_read: socket=%u, length=%u, source=" PRIip ":%u", (int)socketIdentifier, length, MAKE_IP_PRINTABLE(*addr), *port));

    return length;
}

ssize_t nabto_write(nabto_socket_t socketIdentifier, const uint8_t* buf, size_t length, uint32_t addr, uint16_t port)
{
    relaySocket* rs = &relaySockets[socketIdentifier];
    uint8_t packet[MAXIMUM_PACKET_SIZE_WITH_HEADER];
    uint8_t* p = &packet[4];
    
    if(socketIdentifier >= TCP_RELAY_MAXIMUM_NUMBER_OF_SOCKETS)
    {
        NABTO_LOG_ERROR(("nabto_write: Invalid socket (%u)!", (int)socketIdentifier));
        return 0;
    }
    
    if(rs->state == RELAY_SOCKET_STATE_FREE)
    {
        NABTO_LOG_ERROR(("nabto_write: Unopened socket (%u)!", (int)socketIdentifier));
        return 0;
    }
    
    if(rs->state != RELAY_SOCKET_STATE_OPEN)
    {
        NABTO_LOG_TRACE(("nabto_write: Socket not fully opened (%u)!", (int)socketIdentifier));
        return 0;
    }
    
    if(MAXIMUM_PACKET_SIZE < (1 + 1 + 1 + 4 + 2 + 2 + length))
    {
        NABTO_LOG_ERROR(("nabto_write: Oversize packet (%u)!", (int)socketIdentifier));
        return 0;
    }

    WRITE_FORWARD_U8(p, Command_WriteUdpSocket);
    WRITE_FORWARD_U8(p, socketIdentifier);
    WRITE_FORWARD_U8(p, 0); // dummy sequence number
    WRITE_FORWARD_U32(p, addr);
    WRITE_FORWARD_U16(p, port);
    WRITE_FORWARD_U16(p, length);
    memcpy(p, buf, length);
    p += length;

    send_to_relay_service(packet, p - packet - 2 - 2);

    NABTO_LOG_TRACE(("nabto_write: socket=%u, length=%u, destination=" PRIip ":%u", (int)socketIdentifier, length, MAKE_IP_PRINTABLE(addr), port));

    return length;
}


// when using TCP relay the dns client module is used for dns resolution.
void nabto_dns_resolve(const char* id)
{ 
    dns_client_nabto_dns_resolve(id);
}

nabto_dns_status_t nabto_dns_is_resolved(const char *id, uint32_t* v4addr)
{
    return dns_client_nabto_dns_is_resolved(id, v4addr);
}
