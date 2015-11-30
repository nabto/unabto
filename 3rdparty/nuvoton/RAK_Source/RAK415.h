#ifndef __RAK415_H__
#define __RAK415_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rak_config.h"
#include "rak_global.h"

/* Use TIMER1 to punctuate socket data */
/* NOTE: Don't enable it. It is flawless and its implementation is incomplete. */
//#define TIMER1_PUNC_SOCK

extern int rakmgr_module_work_mode;

extern uint16_t uCmdRspFrame_len;

extern uint32_t rak_Timer;

void rakmgr_uartbuf_lock(void);
void rakmgr_uartbuf_unlock(void);
void rakmgr_uartbuf_putc(uint8_t data);
//uint8_t rakmgr_uartbuf_getc(void);
//int rakmgr_uartbuf_empty(void);
int rakmgr_uartbuf_full(void);
void rakmgr_uartbuf_init(void);

void rakmgr_sockdata_poll(void);
int rakmgr_sockdata_poll_at_resp(void);
int rakmgr_sockdata_poll_at_recvdata(void);
int rakmgr_sockdata_poll_trn(void);
int rakmgr_sockdata_timeout(uint32_t timeOut, int (*poll_func)(void));
void rakmgr_sockdata_prg(int n_prg);

void rakmgr_reset_rak(void);
int rakmgr_rst_facty_conf(void);
int rakmgr_wrt_facty_conf(void);
int rakmgr_wrt_user_conf(void);
int rakmgr_conf_n_rst(int8 (*conf_func)(void));
int rakmgr_enter_AT_mode(void);
int rakmgr_read_userConfig(void);
int rakmgr_send_WPS_cmd(void);
int rakmgr_send_EasyConfig_cmd(void);
int rakmgr_open_cmdMode(void);
int rakmgr_easy_txrxMode(void);

void delay_ms(uint32_t ms);

#endif
