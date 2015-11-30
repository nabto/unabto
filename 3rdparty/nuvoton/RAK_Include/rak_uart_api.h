#ifndef _RAK_UART_API_H_
#define _RAK_UART_API_H_

/*******************  correlation function ******************/

int8 WIFI_MODULE_INIT(void);
int8 rak_open_cmdMode(void);
int8 rak_easy_txrxMode(void);
int8 rak_query_fwversion(void);
int8 rak_query_macaddr(void);
int8 rak_read_userConfig(void);
int8 rak_write_userConfig(void);
int8 rak_read_restoreConfig(void);
int8 rak_write_restoreConfig(void);
int8 rak_copy_uConfig(void);
int8 rak_query_constatus(void);
int8 rak_query_ipconifg(void);
int8 rak_query_tcpstatus(void);
int8 rak_query_rssi(void);
int8 rak_scan_ap(void);
int8 rak_get_scan(void);

int8 rak_easy_config(void);
int8 rak_wps_config(void);
int8 rak_reset(void);
int8 rak_restore(void);
int8 rak_send_Data(uint8_t socket_id,uint16_t destport,uint32_t destip,uint8_t *send_Data,uint16_t send_DataLen);

#endif

