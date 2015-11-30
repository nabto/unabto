#include <stdbool.h>
#include <unabto/unabto.h>
#include "unabto_uip.h"



uint32_t addr;
static struct uip_ip_hdr_t _uip_ip_hdr;
static struct uip_udp_hdr_t _uip_udp_hdr;

struct uip_ip_hdr_t *uip_ip_hdr = &_uip_ip_hdr;
struct uip_udp_hdr_t *uip_udp_hdr = &_uip_udp_hdr;

uint8_t uip_appdata[UNABTO_COMMUNICATION_BUFFER_SIZE];

static struct uip_udp_conn udpConn;

struct uip_udp_conn *udp_new(const uip_ipaddr_t *ripaddr, uint16_t rport, void *appstate)
{
    if (NULL != ripaddr)
        udpConn.ripaddr = *ripaddr;
    else
        udpConn.ripaddr = 0;
    return &udpConn;
}

void udp_bind(struct uip_udp_conn *conn, uint16_t port)
{
    if (nabto_init_socket(udpConn.ripaddr, &port, &udpConn.socket))
        udpConn.lport = port;
}

bool uip_newdata( void )
{
    ssize_t datalen;
    datalen = nabto_read(udpConn.socket, uip_appdata, UNABTO_COMMUNICATION_BUFFER_SIZE, &_uip_ip_hdr.srcipaddr, &_uip_udp_hdr.srcport);
    if (0 < datalen)
        _uip_udp_hdr.datalen = datalen;
    return (0 < datalen);
}


void uip_udp_packet_send(struct uip_udp_conn *c, const void *data, int len)
{
    c->sstatus = nabto_write(c->socket, data, len, c->ripaddr, c->rport);
}

/*

void uip_ipaddr_copy(uip_ipaddr_t *ta, uip_ipaddr_t *sa)
{
    *ta = *sa;
}
*/

uint16_t uip_datalen()
{
    return _uip_udp_hdr.datalen;
}

uint16_t uip_nthos(uint16_t s)
{
    uint16_t t;
    READ_U16(t , &s);
    return t;
}
