/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_MODBUS

#include "modbus_tcp_slave.h"
#include "modbus_rtu_master.h"
#include <modules/util/list_malloc.h>
#include <modules/util/memory_allocation.h>

#if !WIN32
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#endif

#define MODBUS_TCP_MAXIMUM_CLIENTS              4
#define MODBUS_TCP_SERVER_PORT                  502
#define MODBUS_TCP_HEADER_SIZE                  6
// TODO Determine correct buffer size
#define MODBUS_TCP_MAXIMUM_PAYLOAD_SIZE         300

#if !WIN32
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#endif

typedef enum
{
    CLIENT_STATE_FREE,
    CLIENT_STATE_CONNECTED,
    CLIENT_STATE_DISCONNECTING
} client_state;

typedef struct
{
    uint16_t transactionIdentifier;
    modbus_message* message;
} modbus_tcp_message;

typedef struct
{
    client_state state;
    SOCKET socket;
    struct sockaddr_in endpoint;
    // TODO Determine correct buffer size
    uint8_t communicationBuffer[300];
    uint16_t communicationBufferSize;
    uint16_t currentTransactionIdentifier;
    uint16_t currentProtocolIdentifier;
    uint16_t currentPduLength;
    uint8_t currentUnitIdentifier;
    uint16_t maximumReadSize;
    list messages;
} modbus_tcp_client;

uint8_t determine_bus(uint8_t address);

static bool non_blocking_accept(SOCKET* listeningSocket, SOCKET* clientSocket, struct sockaddr_in* clientEndpoint);

static struct sockaddr_in localEndpoint;
static SOCKET serverSocket;

static modbus_tcp_client clients[MODBUS_TCP_MAXIMUM_CLIENTS];

bool modbus_tcp_slave_initialize(void)
{
    uint8_t i;

    // reset all client connections
    memset(clients, 0, sizeof(clients));
    for(i = 0; i < MODBUS_TCP_MAXIMUM_CLIENTS; i++)
    {
        modbus_tcp_client* client = &clients[i];

        list_initialize(&client->messages);
    }

    // open server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket == INVALID_SOCKET)
    {
        NABTO_LOG_FATAL(("Unable to create Modbus TCP server socket: (%i) %s", errno, strerror(errno)));
        return false;
    }

    localEndpoint.sin_family = AF_INET;
    localEndpoint.sin_addr.s_addr = INADDR_ANY;
    localEndpoint.sin_port = htons(MODBUS_TCP_SERVER_PORT);

    if (INVALID_SOCKET == bind(serverSocket, (struct sockaddr*)&localEndpoint, sizeof(localEndpoint)))
    {
        NABTO_LOG_FATAL(("Unable to bind Modbus TCP server socket: (%i) %s", errno, strerror(errno)));
        return false;
    }

    if(INVALID_SOCKET == listen(serverSocket, 5))
    {
        NABTO_LOG_FATAL(("Unable to listen on Modbus TCP server socket: (%i) %s", errno, strerror(errno)));
        return false;
    }

    NABTO_LOG_INFO(("Initialized."));

    return true;
}

void modbus_tcp_slave_tick(void)
{
    uint8_t i;
    
    for(i = 0; i < MODBUS_TCP_MAXIMUM_CLIENTS; i++)
    {
        modbus_tcp_client* client = &clients[i];

        bool tickAgain = false;
        client_state oldState = client->state;

        switch(client->state)
        {
        case CLIENT_STATE_FREE:
            if(non_blocking_accept(&serverSocket, &client->socket, &client->endpoint))
            {
                if(-1 == client->socket)
                {
                    NABTO_LOG_ERROR(("Bad TCP socket returned."));
                }
                else
                {
                    NABTO_LOG_TRACE(("Client connected [%u].", (int)i));
                    
                    // make the new socket non-blocking
                    // disable Nagles algorithm on the new socket to avoid huge delay on Modbus/TCP
                    //// set DONTLINGER flag to avoid slow disconnects
#if WIN32
                    {
                        u_long mode = 1;
                        ioctlsocket(client->socket, FIONBIO, &mode);
                    }
                    
                    {
                        /*int setsockopt_mode = 1;
                        if(setsockopt(c->socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&setsockopt_mode, sizeof(setsockopt_mode)) != 0)
                        {
                            NABTO_LOG_FATAL(("Unable to set TCPNODELAY! (%i)", WSAGetLastError()));
                        }*/
                    }
                    
                    {
                        int flag;
                        
                        flag = 1;
                        if (setsockopt(client->socket, IPPROTO_TCP, TCP_NODELAY, (const char*)&flag, sizeof(flag)) == SOCKET_ERROR)
                        {
                            NABTO_LOG_FATAL(("setsockopt(TCP_NODELAY=1)"));
                        }
                        
                        flag = 1;
                        if (setsockopt(client->socket, SOL_SOCKET, SO_DONTLINGER, (const char*)&flag, sizeof(flag)) == SOCKET_ERROR)
                        {
                            NABTO_LOG_FATAL(("setsockopt(SO_DONTLINGER=1)"));
                        }
                    }
#else
                    fcntl(client->socket, F_SETFL, O_NONBLOCK);

                    {
                        int flag = 1;
                        if (setsockopt(client->socket, IPPROTO_TCP, TCP_NODELAY, (const void*)&flag, sizeof(flag)) == -1)
                        {
                            NABTO_LOG_FATAL(("setsockopt(TCP_NODELAY=1)"));
                        }

                    }
#endif
                    
                    client->communicationBufferSize = 0;
                    client->maximumReadSize = MODBUS_TCP_HEADER_SIZE;
                    client->state = CLIENT_STATE_CONNECTED;
                }
            }
            break;

        case CLIENT_STATE_CONNECTED:
            {
                int length = recv(client->socket, (char*)client->communicationBuffer + client->communicationBufferSize, client->maximumReadSize, 0);

                if(length > 0)
                {
                    NABTO_LOG_TRACE(("Received data."));

                    client->communicationBufferSize += length;

                    NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_TRACE,("Communications buffer now contains:"), client->communicationBuffer, client->communicationBufferSize);

                    if(client->communicationBufferSize == MODBUS_TCP_HEADER_SIZE) // has the Modbus/TCP header just been received?
                    {
                        client->currentTransactionIdentifier = client->communicationBuffer[0];
                        client->currentTransactionIdentifier <<= 8;
                        client->currentTransactionIdentifier |= client->communicationBuffer[1];

                        client->currentProtocolIdentifier = client->communicationBuffer[2];
                        client->currentProtocolIdentifier <<= 8;
                        client->currentProtocolIdentifier |= client->communicationBuffer[3];

                        client->currentPduLength = client->communicationBuffer[4];
                        client->currentPduLength <<= 8;
                        client->currentPduLength |= client->communicationBuffer[5];

                        if(client->currentProtocolIdentifier == 0 && client->currentPduLength <= MODBUS_TCP_MAXIMUM_PAYLOAD_SIZE)
                        {
                            client->maximumReadSize = client->currentPduLength; // valid header received -> go on to receive payload
                            tickAgain = true; // tick again as it is very likely that the Modbus/TCP payload is ready to be read out of the TCP stream
                        }
                        else
                        {
                            client->state = CLIENT_STATE_DISCONNECTING;
                        }
                    }
                    else if(client->communicationBufferSize == (MODBUS_TCP_HEADER_SIZE + client->currentPduLength))  // has the Modbus/TCP payloyad just been received?
                    {
                        modbus_tcp_message* tcpMessage;
                        modbus_message* message;
                        
                        client->currentUnitIdentifier = client->communicationBuffer[6];

                        if(client->currentUnitIdentifier <= 247) // verify frame
                        {
                            NABTO_LOG_TRACE(("Received Modbus TCP frame from client: tid=%i, length=%i, uid=%i", (int)client->currentTransactionIdentifier, (int)client->currentPduLength, (int)client->currentUnitIdentifier));

                            tcpMessage = (modbus_tcp_message*)checked_malloc(sizeof(modbus_tcp_message));

                            if(tcpMessage != NULL)
                            {
                                message = modbus_rtu_master_allocate_message();
                                if(message != NULL)
                                {
                                    message->bus = determine_bus(client->currentUnitIdentifier); // determine which bus to send out the message on by using the application level device cache.
                                    message->address = client->currentUnitIdentifier;
                                    memcpy(message->frame, client->communicationBuffer + MODBUS_TCP_HEADER_SIZE, client->currentPduLength); // copy frame to message...
                                    message->frameSize = client->currentPduLength; // ...

                                    // enqueue message for transfer
                                    if(modbus_rtu_master_transfer_message(message))
                                    {
                                        NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_TRACE, ("Modbus/TCP request:"), message->frame, message->frameSize);
                                        tcpMessage->transactionIdentifier = client->currentTransactionIdentifier;
                                        tcpMessage->message = message;
                                        list_append(&client->messages, tcpMessage);
                                    } 
                                    else
                                    {
                                        NABTO_LOG_ERROR(("Unable to transfer Modbus message!"));

                                        checked_free(tcpMessage);
                                        modbus_rtu_master_release_message(message);
                                    }
                                }
                                else
                                {
                                    NABTO_LOG_ERROR(("Failed to allocate Modbus message!"));

                                    checked_free(tcpMessage);
                                    // error signaled to client by timeout
                                }
                            }
                        }
                        else
                        {
                            NABTO_LOG_TRACE(("Discarded invalid Modbus TCP frame from client: tid=%i pid=%i length=%i uid=%i", client->currentTransactionIdentifier, client->currentProtocolIdentifier, client->currentPduLength, client->currentUnitIdentifier));
                            // error signaled to client by timeout
                        }
                        
                        client->communicationBufferSize = 0;
                        client->maximumReadSize = MODBUS_TCP_HEADER_SIZE;
                    }
                    else if(client->communicationBufferSize > (MODBUS_TCP_HEADER_SIZE + client->currentPduLength)) // has an overrun occured?
                    {
                        NABTO_LOG_ERROR(("Oversize Modbus TCP frame received from client - disconnecting!"));
                        client->state = CLIENT_STATE_DISCONNECTING;
                    }
                }
                // detect client disconnect
#if WIN32
                else if(WSAGetLastError() != WSAEWOULDBLOCK) // if an actual error occurred
#else
                else if(length == 0 || errno != EWOULDBLOCK)
#endif
                {
                    NABTO_LOG_TRACE(("Client appears to be disconnecting."));
                    client->state = CLIENT_STATE_DISCONNECTING;
                }

                // check if any in-progress-messages have completed
                if(list_count(&client->messages) > 0)
                {
                    modbus_tcp_message* tcpMessage;

                    list_peek_first(&client->messages, (void**)&tcpMessage);

                    if(tcpMessage->message->state == MODBUS_MESSAGE_STATE_COMPLETED || tcpMessage->message->state == MODBUS_MESSAGE_STATE_FAILED)
                    {
                        if(tcpMessage->message->state == MODBUS_MESSAGE_STATE_COMPLETED)
                        {
                            uint8_t response[300]; // TODO Determine correct buffer size
                            uint16_t responseLength = MODBUS_TCP_HEADER_SIZE + tcpMessage->message->frameSize;

                            // create response
                            //   write original transaction identifier
                            response[0] = tcpMessage->transactionIdentifier >> 8;
                            response[1] = (uint8_t)tcpMessage->transactionIdentifier;
                            //   write original protocol identifier (always 0)
                            response[2] = 0;
                            response[3] = 0;
                            //   write response data length (slave address/unit identifier, function code and payload)
                            response[4] = tcpMessage->message->frameSize >> 8;
                            response[5] = (uint8_t)tcpMessage->message->frameSize;
                            //   write actual response data
                            memcpy(response + MODBUS_TCP_HEADER_SIZE, tcpMessage->message->frame, tcpMessage->message->frameSize);

                            NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_TRACE, ("Modbus/TCP response:"), response, responseLength);

                            send(client->socket, (const char*)response, responseLength, 0);
                        }

                        modbus_rtu_master_release_message(tcpMessage->message);

                        list_remove_first(&client->messages, NULL);

                        checked_free(tcpMessage);
                    }
                }
            }
            break;

        case CLIENT_STATE_DISCONNECTING:
            {
                list_element* e;

#if WIN32
                closesocket(client->socket);
#else
                close(client->socket);
#endif
                
                list_foreach(&client->messages, e)
                {
                    modbus_tcp_message* tcpMessage = (modbus_tcp_message*)e->data;

                    modbus_rtu_master_release_message(tcpMessage->message);
                    checked_free(tcpMessage);
                }

                list_remove_all(&client->messages);

                client->state = CLIENT_STATE_FREE;

                if(client->communicationBufferSize > 0)
                {
                    NABTO_LOG_TRACE(("Client disconnected - data left behind!"));
                }
                else
                {
                    NABTO_LOG_TRACE(("Client disconnected."));
                }
            }
            break;
        }

        if(oldState != client->state || tickAgain)
        {
            i--;
        }
    }
}

static bool non_blocking_accept(SOCKET* listeningSocket, SOCKET* clientSocket, struct sockaddr_in* clientEndpoint)
{
    struct timeval tv;
    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET(*listeningSocket, &rfds);

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    if(select(*listeningSocket + 1, &rfds, (fd_set *) 0, (fd_set *) 0, &tv) > 0)
    {
        int endpointLength;

        NABTO_LOG_TRACE(("There appears to be a client."));

        endpointLength = sizeof(struct sockaddr_in);
        *clientSocket = accept(*listeningSocket, (struct sockaddr*)clientEndpoint, &endpointLength);

        return true;
    }
    else
    {
        return false;
    }
}
