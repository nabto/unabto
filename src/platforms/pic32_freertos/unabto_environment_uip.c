/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#include "../../unabto_external_environment.h"

#include "uip.h"

void nabto_random(uint8_t* buf, size_t len) {
    if (buf) { /* Warning removal */ }
    if (len) { /* Warning removal */ }
    // uninplemented.
}

void nabto_socket_set_invalid(nabto_socket_t* socket)
{
    socket = NABTO_INVALID_SOCKET;
}

bool nabto_socket_init(uint16_t* localPort, nabto_socket_t* socket) {

    /**
     * uip_udp_new returns a pointer to the array of connections in uip.
     * We could have used a integer index in this array as well.
     */
    uip_ipaddr_t addr;
    
    uip_ipaddr(&addr, 0,0,0,0);

    *socket = uip_udp_new(&addr,0);
    if (*socket == NULL) return false;
    if (*localPort == 0) {
        *localPort = htons((*socket)->lport);
    } else {
        uip_udp_bind(*socket, (htons(*localPort)));
    }
    return true;
}

bool nabto_socket_is_equal(const nabto_socket_t* s1, const nabto_socket_t* s2)
{
    return *s1==*s2;
}

void nabto_socket_close(nabto_socket_t* socket) {
    uip_udp_remove(*socket);
    *socket = NULL;
}

typedef struct {
    nabto_socket_t socket;
    uint32_t addr;
    uint16_t port;
    uint8_t buffer[500];
    size_t len;
} packet_buffer_t;

/**
 * store an input and output packet
 */
static packet_buffer_t ipacket, opacket;

#define BUF     ( ( struct uip_tcpip_hdr * ) &uip_buf[UIP_LLH_LEN] )

void in_out_packet() {
    bool incoming = false;
    if (uip_len > 0) {
        NABTO_LOG_INFO(("incoming nabto traffic"));

        ipacket.socket = uip_udp_conn;
        ipacket.addr = 
            ((uint32_t)uip_ipaddr1(BUF->srcipaddr) << 24) +
            ((uint32_t)uip_ipaddr2(BUF->srcipaddr) << 16) +
            ((uint32_t)uip_ipaddr3(BUF->srcipaddr) << 8) +
            (uint32_t)uip_ipaddr4(BUF->srcipaddr);
        ipacket.port = htons(BUF->srcport);
        ipacket.len = uip_len;
        memcpy(ipacket.buffer, uip_appdata, uip_len);
        incoming = true;
    }

    if (opacket.socket == uip_udp_conn) { // we send on the current connection
        NABTO_LOG_INFO(("outgoing nabto traffic"));
        opacket.socket=NULL;
        uip_ipaddr(&uip_udp_conn->ripaddr, 
                   opacket.addr>>24 & 255, 
                   opacket.addr>>16 & 255, 
                   opacket.addr>>8 & 255, 
                   opacket.addr & 255);
        uip_udp_conn->rport = htons(opacket.port);
        memcpy(uip_appdata, opacket.buffer, opacket.len);
        uip_udp_send(opacket.len);
    }
}

/**
 * this is satisfying the uIP when tcp packets are received
 */
void dummy_packet_handler() {
}

#define MIN(a,b) ((a) <= (b) ? (a) : (b))
ssize_t nabto_read(nabto_socket_t socket,
                   uint8_t*       buf,
                   size_t         len,
                   struct nabto_ip_address*      addr,
                   uint16_t*      port) {
    // will be handled when  newdata is called.
    if (ipacket.socket == socket) {
        ipacket.socket = NULL;
        memmove(buf, ipacket.buffer, MIN(ipacket.len, len));
        addr.type = NABTO_IP_V4;
        addr.addr.ipv4 = ipacket.addr;
        *port = ipacket.port;
        return ipacket.len;
    }
    return 0;
}

ssize_t nabto_write(nabto_socket_t socket,
                    const uint8_t* buf,
                    size_t         len,
                    struct nabto_ip_address*       addr,
                    uint16_t       port) {
    // Set the socket in uip to be this one.
    if (addr->type != NABTO_IP_V4) {
        return 0;
    }
    opacket.socket = socket;
    memcpy(opacket.buffer, buf, len);
    opacket.len = len;
    opacket.addr = addr->addr.ipv4;
    opacket.port = port;
    return len;
}


bool nabto_init_platform() {
    // not implemented
    return true;
}

void nabto_close_platform() {
    // not implemented
}


nabto_stamp_t nabtoGetStamp() {
    // TODO implement
    return 0;
}

bool nabtoIsStampPassed() {
    return true;
}
