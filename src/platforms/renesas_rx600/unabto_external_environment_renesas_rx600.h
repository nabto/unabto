/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#ifndef _UNABTO_EXTERNAL_ENVIRONMENT_RENESAS_RX600_H_
#define _UNABTO_EXTERNAL_ENVIRONMENT_RENESAS_RX600_H_

#ifdef __cplusplus
extern "C" {
#endif

#define UIP_APPCALL app_dispatcher
#define UIP_UDP_APPCALL app_dispatcher

void unabto_app();

void app_dispatcher();


// typedef enum {APP_DHCP, APP_DNS, APP_UNABTO} app_t;

typedef struct  uip_udp_appstate_t
{
	union {
		int unabto;
		int resolv;
	};
	void (*callBack)(void);
} uip_udp_appstate_t;


#ifdef __cplusplus
} //extern "C"
#endif

#endif
