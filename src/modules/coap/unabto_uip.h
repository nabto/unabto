#ifndef __UNABTO_UIP_H__
#define __UNABTO_UIP_H__
#include <stdint.h>
#include <stdbool.h>
#include <unabto/unabto.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UIP_CONF_BUFFER_SIZE UNABTO_COMMUNICATION_BUFFER_SIZE

typedef uint32_t uip_ipaddr_t;

struct uip_ip_hdr_t 
{
    uip_ipaddr_t srcipaddr;
};

struct uip_udp_hdr_t
{
    uint16_t srcport;
    uint16_t datalen;
};

struct uip_udp_conn
{
    ssize_t sstatus;
    uip_ipaddr_t ripaddr;
    nabto_socket_t socket;
    uint16_t lport,rport;
};

extern struct uip_ip_hdr_t *uip_ip_hdr;

extern struct uip_udp_hdr_t *uip_udp_hdr;

extern uint8_t uip_appdata[];

struct uip_udp_conn *udp_new(const uip_ipaddr_t *ripaddr, uint16_t rport, void *appstate);

void uip_udp_bind(struct uip_udp_conn *conn, uint16_t port);

void uip_udp_packet_send(struct uip_udp_conn *c, const void *data, int len);

bool uip_newdata( void );

void uip_ipaddr_copy(uip_ipaddr_t *ta, uip_ipaddr_t *sa);

uint16_t uip_datalen( void );


#define uip_ipaddr_copy(t, s) do{ *t = *s; }while(0)
#define uip_ipaddr_cmp(a1, a2) (*a1 == *a2)



#ifdef __cplusplus
} //extern "C"
#endif

#endif /* __UNABTO_UIP_H__ */
