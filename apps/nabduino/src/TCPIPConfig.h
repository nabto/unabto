#ifndef _TCPIPCONFIG_H_
#define _TCPIPCONFIG_H_

#define STACK_USE_ICMP_SERVER			// Ping query and response capability
#define STACK_USE_DHCP_CLIENT			// Dynamic Host Configuration Protocol client for obtaining IP address and o
#define STACK_USE_DNS
#define STACK_USE_UDP

#define MY_DEFAULT_IP_ADDR_BYTE1        (169ul) // 169.254.1.1
#define MY_DEFAULT_IP_ADDR_BYTE2        (254ul)
#define MY_DEFAULT_IP_ADDR_BYTE3        (1ul)
#define MY_DEFAULT_IP_ADDR_BYTE4        (1ul)

#define MY_DEFAULT_MASK_BYTE1           (255ul)
#define MY_DEFAULT_MASK_BYTE2           (255ul)
#define MY_DEFAULT_MASK_BYTE3           (0ul)
#define MY_DEFAULT_MASK_BYTE4           (0ul)

#define MY_DEFAULT_GATE_BYTE1           (169ul)
#define MY_DEFAULT_GATE_BYTE2           (254ul)
#define MY_DEFAULT_GATE_BYTE3           (1ul)
#define MY_DEFAULT_GATE_BYTE4           (1ul)

#define MY_DEFAULT_PRIMARY_DNS_BYTE1	(169ul)
#define MY_DEFAULT_PRIMARY_DNS_BYTE2	(254ul)
#define MY_DEFAULT_PRIMARY_DNS_BYTE3	(1ul)
#define MY_DEFAULT_PRIMARY_DNS_BYTE4	(1ul)

#define MY_DEFAULT_SECONDARY_DNS_BYTE1	(0ul)
#define MY_DEFAULT_SECONDARY_DNS_BYTE2	(0ul)
#define MY_DEFAULT_SECONDARY_DNS_BYTE3	(0ul)
#define MY_DEFAULT_SECONDARY_DNS_BYTE4	(0ul)

#define MAX_HTTP_CONNECTIONS            1 // Microchips TCP/IP stack requires this define even tough HTTP is not used.

#define STACK_CLIENT_MODE                 // Set the TCP/IP stack to operate in client mode

#define MAX_UDP_SOCKETS     (3u)          // Maximum number of UDP sockets available to the application.

#define UDP_USE_TX_CHECKSUM               // This slows UDP TX performance by nearly 50%, except when using the ENCX24J600, which has a super fast DMA and incurs virtually no speed pentalty.

#endif
