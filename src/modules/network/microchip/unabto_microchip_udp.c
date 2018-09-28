/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_NETWORK

#include "unabto_microchip_udp.h"
#include "unabto_microchip_arp.h"
#include <unabto/unabto_util.h>


ssize_t microchip_udp_write(UDP_SOCKET socket, const uint8_t* buf, size_t len, uint32_t addr, uint16_t port) {
    NODE_INFO remoteInfo;
    ssize_t size;
    uint32_t addrNetworkOrder;
#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_TRACE)
    UDP_SOCKET_INFO* debug;
    uint32_t sourceIp;
#endif

    if (addr == 0 || port == 0) {
        return 0;
    }
    
    WRITE_U32(&addrNetworkOrder, addr);

    if (unabto_microchip_arp_resolve(addrNetworkOrder, &remoteInfo.MACAddr)) {
        remoteInfo.IPAddr.Val = addrNetworkOrder;
        memcpy((void*) &UDPSocketInfo[socket].remote.remoteNode, (void*) &remoteInfo, sizeof (NODE_INFO));
    } else {
        if (UDPSocketInfo[socket].remote.remoteNode.IPAddr.Val != addrNetworkOrder) {
            return 0;
        }
        // the arp resolve is not finished but the socket already knows
        // a mac address, lets just use it.
    }
    if (UDPIsPutReady(socket) >= len) {
#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_TRACE)
        debug = &UDPSocketInfo[socket];
        READ_U32(sourceIp, &debug->remote.remoteNode.IPAddr.Val);
#endif

        UDPSocketInfo[socket].remotePort = port;
        size = (ssize_t) UDPPutArray((BYTE*) buf, len);
        UDPFlush();
        NABTO_LOG_TRACE(("UDP write length: %i, %" PRIu16 " -> " PRIip ":%" PRIu16, size, debug->localPort, MAKE_IP_PRINTABLE(sourceIp), debug->remotePort));

        NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_TRACE, ("UDP out datagram"), buf, len);

        return size;
    } else {
        NABTO_LOG_TRACE(("Socket was not put ready %i", socket));
    }

    return 0;
}

ssize_t microchip_udp_read(UDP_SOCKET socket, uint8_t* buf, size_t len, uint32_t* addr, uint16_t* port) {
    ssize_t rlen;
#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_TRACE)
    UDP_SOCKET_INFO* debug;
    uint32_t sourceIp;
#endif
    if (UDPIsGetReady(socket)) {
#if NABTO_LOG_CHECK(NABTO_LOG_SEVERITY_TRACE)
        debug = &UDPSocketInfo[socket];
        READ_U32(sourceIp, &debug->remote.remoteNode.IPAddr.Val);
#endif

        unabto_microchip_arp_add_resolved(&(UDPSocketInfo[socket].remote.remoteNode));
        //        *addr = ntohl(UDPSocketInfo[socket].remote.remoteNode.IPAddr.Val);
        WRITE_U32(addr, UDPSocketInfo[socket].remote.remoteNode.IPAddr.Val);
        *port = UDPSocketInfo[socket].remotePort;
        rlen = (ssize_t) UDPGetArray(buf, len);
        NABTO_LOG_TRACE(("UDP read length: %i, %" PRIu16 " <- " PRIip ":%" PRIu16, rlen, debug->localPort, MAKE_IP_PRINTABLE(sourceIp), debug->remotePort));

        NABTO_LOG_BUFFER(NABTO_LOG_SEVERITY_TRACE, ("UDP in datagram"), buf, rlen);

        return rlen;
    }

    return 0;
}

void microchip_udp_close(UDP_SOCKET* socketDescriptor) {
    if (socketDescriptor && *socketDescriptor != NABTO_INVALID_SOCKET) {
        UDPClose(*socketDescriptor);
        *socketDescriptor = NABTO_INVALID_SOCKET;
    }
}

bool microchip_udp_open(uint16_t* localPort, UDP_SOCKET* socketDescriptor) {
    UDP_SOCKET sd;
    sd = UDPOpenEx(0, UDP_OPEN_SERVER, *localPort, 0);

        NABTO_LOG_INFO(("create socket..."));

    if (sd == INVALID_UDP_SOCKET) {
        NABTO_LOG_INFO(("failed to create socket"));
        return false;
    }
    *localPort = UDPSocketInfo[sd].localPort;
    *socketDescriptor = sd;
    //    NABTO_LOG_INFO(("init socket %i port %i", *socketDescriptor, *localPort));

    return true;
}
