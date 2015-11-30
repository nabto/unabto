#include <string.h>
#include "Nano1X2Series.h"
#include "rak_config.h"
#include "rak_global.h"
#include "RAK415.h"
#include "systimer.h"
#include "ringbuff.h"

int rakmgr_module_work_mode = EASY_TXRX_TYPE;

rak_uCmdRsp uCmdRspFrame;
uint16_t uCmdRspFrame_len = 0;

// Implement UART buffer as ring buffer. 
// The buffer is filled in UART1_IRQHandler() and copied to user buffer through rakmgr_sockdata_poll();
static uint8_t uartbuf_raw[RXBUFSIZE + 1];
static struct RingBuff uartbuf = {
    uartbuf_raw,
    0,
    sizeof (uartbuf_raw),
    0,
    0
};

rak_cfg_t         rak_userCfgstr;      
rak_cfg_t         rak_restoreCfgstr;
uint32_t          rak_Timer=0;

/**
  * @brief          Lock UART buffer.
  */
void rakmgr_uartbuf_lock(void)
{
    NVIC_DisableIRQ(UART1_IRQn);
#ifdef TIMER1_PUNC_SOCK
    NVIC_DisableIRQ(TMR1_IRQn);
#endif
}

/**
  * @brief          Unlock UART buffer.
  */
void  rakmgr_uartbuf_unlock(void)
{
#ifdef TIMER1_PUNC_SOCK
    NVIC_EnableIRQ(TMR1_IRQn);
#endif
    NVIC_EnableIRQ(UART1_IRQn);
}

/**
  * @brief          Write one byte into UART buffer.
  */
void rakmgr_uartbuf_putc(uint8_t bydata)
{
    uint8_t *next_wrt_p = NULL;
    unsigned next_wrt_cap = 0;
    
    rakmgr_uartbuf_lock();
    rb_next_write(&uartbuf, (void **) &next_wrt_p, &next_wrt_cap);
    if (next_wrt_cap) {
        *next_wrt_p = bydata;
        rb_write_done(&uartbuf, 1);
    }
    rakmgr_uartbuf_unlock();
}

/*
uint8_t rakmgr_uartbuf_getc(void)
{
}
*/

/*
int rakmgr_uartbuf_empty(void)
{
    int empty;
    
    rakmgr_uartbuf_lock();
    empty = rb_empty(&uartbuf);
    rakmgr_uartbuf_unlock();
    
    return empty;
}
*/

/**
  * @brief          Test if UART buffer if full.
  */
int rakmgr_uartbuf_full(void)
{
    int full;
    
    rakmgr_uartbuf_lock();
    full = rb_full(&uartbuf);
    rakmgr_uartbuf_unlock();
    
    return full;
}

/**
  * @brief          Initialize UART buffer.
  */
void rakmgr_uartbuf_init(void)
{
    rakmgr_uartbuf_lock();
    rb_init(&uartbuf, uartbuf_raw, sizeof (uartbuf_raw));
    rakmgr_uartbuf_unlock();
}

/**
  * @brief          Copy socket data from UART buffer to user buffer.
  */
void rakmgr_sockdata_poll(void)
{
    uint8_t *next_rd_p = NULL;
    unsigned next_rd_cap = 0;
    unsigned totran = sizeof (uCmdRspFrame.uCmdRspBuf) - uCmdRspFrame_len;
    
    rakmgr_uartbuf_lock();
    rb_next_read(&uartbuf, (void **) &next_rd_p, &next_rd_cap);
    totran = (totran <= next_rd_cap) ? totran : next_rd_cap;
    if (totran) {
        memcpy(uCmdRspFrame.uCmdRspBuf + uCmdRspFrame_len, next_rd_p, totran);
        uCmdRspFrame_len += totran;
    }
    rb_read_done(&uartbuf, totran);
    rakmgr_uartbuf_unlock();
}

static const uint8_t *find_pat(const uint8_t *src, const uint8_t *src_end, const uint8_t *pat, const uint8_t * pat_end)
{
    const uint8_t *src_ind = src;
    unsigned pat_len = pat_end - pat;
    
    for (; (src_ind + pat_len) <= src_end; src_ind ++) {
        const uint8_t *src_ind2 = src_ind;
        const uint8_t *pat_ind = pat;
        
        for (; pat_ind != pat_end; src_ind2 ++, pat_ind ++) {
            if (*src_ind2 != *pat_ind) {
                break;
            }
        }
        
        if (pat_ind == pat_end) {
            return src_ind;
        }
    }
    
    return NULL;
}

/**
  * @brief          Poll AT command response.
  * @return         Size of AT command response.
  * @note           RAK_RECIEVE_DATA_CMD is excluded in the polling.
  */
int rakmgr_sockdata_poll_at_resp(void)
{
    rakmgr_sockdata_poll();
    
    do {
        const uint8_t *pat_in_src = NULL;
        const uint8_t *src = uCmdRspFrame.uCmdRspBuf;
        const uint8_t *src_end = uCmdRspFrame.uCmdRspBuf + uCmdRspFrame_len;
        const uint8_t *pat = RAK_END;
        const uint8_t *pat_end = pat + strlen(RAK_END);
        unsigned disp = 0;
        
        if (uCmdRspFrame_len > 2 && strncmp((const char *) uCmdRspFrame.uCmdRspBuf, "OK", 2) == 0) {
            disp = 2;
        }
        else if (uCmdRspFrame_len > 5 && strncmp((const char *) uCmdRspFrame.uCmdRspBuf, "ERROR", 5) == 0) {
            disp = 5;
        }
        
        // Exclude RAK_RECIEVE_DATA_CMD
        if (disp) {
            pat_in_src = find_pat(uCmdRspFrame.uCmdRspBuf + disp, src_end, pat, pat_end);
            if (pat_in_src) {
                return pat_in_src - src + (pat_end - pat);
            }
        }
    }
    while (0);
    
    return 0;
}

/**
  * @brief          Poll RAK_RECIEVE_DATA_CMD.
  * @return         Size of complete RAK_RECIEVE_DATA_CMD.
  */
int rakmgr_sockdata_poll_at_recvdata(void)
{
    rakmgr_sockdata_poll();
    
    if (uCmdRspFrame_len >= 24 && 
        strncmp((char *)uCmdRspFrame.recvdataFrame.recvdata.recvheader, RAK_RECIEVE_DATA_CMD, strlen(RAK_RECIEVE_DATA_CMD)) == 0) {
        uint16_t recv_len = uCmdRspFrame.recvdataFrame.recvdata.recDataLen[0] + uCmdRspFrame.recvdataFrame.recvdata.recDataLen[1]*256;
        if (uCmdRspFrame_len >= (24 + recv_len)) {
            return (24 + recv_len);
        }
        else {
            return 0;
        }
    }
    else {
        return 0;
    }
}

/**
  * @brief          Poll unframed socket data.
  * @return         Size of socket data.
  */
int rakmgr_sockdata_poll_trn(void)
{
    rakmgr_sockdata_poll();
    
    return uCmdRspFrame_len;
}

int rakmgr_sockdata_timeout(uint32_t timeOut, int (*poll_func)(void))
{
    int retval = RAK_SUCCESS;

    RAK_RESET_TIMER; 
    while (! poll_func() && (RAK_INC_TIMER <= timeOut));                                                                            
    if (RAK_INC_TIMER > timeOut) {
        retval = RAK_FAIL;                                
    }
    
    return retval;
}

void rakmgr_sockdata_prg(int n_prg)
{   
    if (n_prg > 0) {
        uCmdRspFrame_len -= n_prg;
        memmove(uCmdRspFrame.uCmdRspBuf, uCmdRspFrame.uCmdRspBuf + n_prg, uCmdRspFrame_len);
    }
    else if (n_prg < 0) {
        uCmdRspFrame_len = 0;
    }
}

/* Required by user program and RAK415 library */
uint8_t rak_UART_send(uint8_t *tx_buf, uint16_t buflen)
{
    UART_Write(UART1, tx_buf, buflen);
    return 0;   
}

/* Reset RAK415 */
void rakmgr_reset_rak(void)
{
    RESET_PORT_PIN = 0;
    delay_ms(10);
    RESET_PORT_PIN = 1; 
    
    rakmgr_module_work_mode = EASY_TXRX_TYPE;
}

int rakmgr_rst_facty_conf(void)
{
    return rakmgr_conf_n_rst(rak_restore);
}

int rakmgr_wrt_facty_conf(void)
{
    return rakmgr_conf_n_rst(rak_write_restoreConfig);
}

int rakmgr_wrt_user_conf(void)
{
    return rakmgr_conf_n_rst(rak_write_userConfig);
}


int rakmgr_conf_n_rst(int8 (*conf_func)(void))
{
    int retval = RAK_SUCCESS;

    // FIXME: Replace rak_open_cmdMode() with rakmgr_open_cmdMode() due to modified UART buffer management.
    //retval = rak_open_cmdMode();
    retval = rakmgr_open_cmdMode();
    if (retval<0) {
       return retval;
    }
    //delay_ms(1000);       

    retval = conf_func();
    if (retval < 0) {
       return retval;
    }

    rakmgr_sockdata_prg(-1);
    retval = rakmgr_sockdata_timeout(RAK_WRCFGTIMEOUT, rakmgr_sockdata_poll_at_resp);
    if (retval < 0) {
        return retval;
    }
    
    if (strncmp((char *) uCmdRspFrame.uCmdRspBuf, "OK", 2)) {
        retval = RAK_CFG_ERROR;     
    }
    
    rakmgr_reset_rak();

    //delay_ms(1000);    //等待模块加载参数  Not Necessary

    return retval; 
}

int rakmgr_enter_AT_mode(void)
{
    int retval = RAK_SUCCESS;

    // FIXME: Replace rak_open_cmdMode() with rakmgr_open_cmdMode() due to modified UART buffer management.
    //retval = rak_open_cmdMode();
    retval = rakmgr_open_cmdMode();
    if (retval < 0) {
        return retval;
    }
//  delay_ms(1000);

    retval = rak_read_userConfig();
    if (retval < 0) {
       return retval;
    }
      
    rakmgr_sockdata_prg(-1);
    retval = rakmgr_sockdata_timeout(RAK_RDCFGTIMEOUT, rakmgr_sockdata_poll_at_resp);
    if (retval < 0) {
        return retval;
    }
    
    if (strncmp((char *) uCmdRspFrame.uCmdRspBuf, "OK", 2) == 0) {
        rak_init_config((char *) &uCmdRspFrame.uCmdRspBuf[2], &rak_userCfgstr);
    }
    else {
        retval = RAK_CFG_ERROR;
        return retval;
    }
 
   do {
        retval = rak_query_constatus();
        if (retval<0) {
            return retval;
        }
        
        rakmgr_sockdata_prg(-1);
        retval = rakmgr_sockdata_timeout(RAK_QCONSATUSTIMEOUT, rakmgr_sockdata_poll_at_resp);
        if (retval<0) {
            return retval;
        }
        
        if (strncmp((char *) uCmdRspFrame.uCmdRspBuf, "OK", 2) == 0 &&
            (uCmdRspFrame.qryconstatusResponse.qryconstatusframe.status == 0x01)) {
            retval= RAK_SUCCESS; 
        }
        else {
            retval= RAK_FAIL;
        }
         
        delay_ms(200);
    }
    while (retval != RAK_SUCCESS);

    do {  
       retval = rak_query_ipconifg();
        if (retval < 0) {
            return retval;
        }
        
        rakmgr_sockdata_prg(-1);
        retval=rakmgr_sockdata_timeout(RAK_QIPCFGTIMEOUT, rakmgr_sockdata_poll_at_resp);
        if (retval < 0) {
            return retval;
        }
        
        if (strncmp((char *) uCmdRspFrame.uCmdRspBuf, "OK", 2) == 0) {
            if (uCmdRspFrame.qryipconfigResponse.qryipconfigframe.ipAddr[0] == 0x01 &&
                uCmdRspFrame.qryipconfigResponse.qryipconfigframe.ipAddr[1] == 0x00 &&
                uCmdRspFrame.qryipconfigResponse.qryipconfigframe.ipAddr[2] == 0x00 &&
                uCmdRspFrame.qryipconfigResponse.qryipconfigframe.ipAddr[3] == 0x7F 
            ) {
                retval= RAK_FAIL ;
            }
            else {
                retval= RAK_SUCCESS;
            }
        }

        delay_ms(200);
    }
    while (retval != RAK_SUCCESS);

    return retval;
}

int rakmgr_read_userConfig(void)
{
    int retval = RAK_SUCCESS;

    // FIXME: Replace rak_open_cmdMode() with rakmgr_open_cmdMode() due to modified UART buffer management.
    //retval = rak_open_cmdMode();
    retval = rakmgr_open_cmdMode();
    if (retval < 0) {
        return retval;
    }
//  delay_ms(1000);

    retval = rak_read_userConfig();
    if (retval < 0) {
       return retval;
    }
      
    rakmgr_sockdata_prg(-1);
    retval = rakmgr_sockdata_timeout(RAK_RDCFGTIMEOUT, rakmgr_sockdata_poll_at_resp);
    if (retval < 0) {
        return retval;
    }
    
    if (strncmp((char *) uCmdRspFrame.uCmdRspBuf, "OK", 2) == 0) {
        rak_init_config((char *) &uCmdRspFrame.uCmdRspBuf[2], &rak_userCfgstr);
    }
    else {
        retval = RAK_CFG_ERROR;
        return retval;
    }
    
    return retval;
}

int rakmgr_send_WPS_cmd(void)
{
    int retval = RAK_SUCCESS;
    
    retval=rak_wps_config();
    if (retval < 0) {
        return retval;
    }
    
    rakmgr_sockdata_prg(-1);
    retval = rakmgr_sockdata_timeout(RAK_WPSTIMEOUT, rakmgr_sockdata_poll_at_resp);
    if (retval < 0) {
        return retval;
    }
    
    if (strncmp((char *) uCmdRspFrame.uCmdRspBuf, "OK", 2)) {
        retval= RAK_FAIL;
        return retval;
    }

    return retval;
}


int rakmgr_send_EasyConfig_cmd(void)
{
    int retval = RAK_SUCCESS;
    
    retval = rak_easy_config();
    if (retval<0) {
        return retval;
    }
    
    rakmgr_sockdata_prg(-1);
    retval = rakmgr_sockdata_timeout(RAK_EASYCFGTIMEOUT, rakmgr_sockdata_poll_at_resp);
    if (retval < 0) {
        return retval;
    }
    
    if (strncmp((char *) uCmdRspFrame.uCmdRspBuf, "OK",2)) {
        retval= RAK_FAIL;
        return retval;
    }

    return retval;
}

int rakmgr_open_cmdMode(void)
{
    int retval = RAK_SUCCESS;
    int retry_times = 0;
//  RAK_DEBUG_PORT_PIN=~RAK_DEBUG_PORT_PIN;

    if (rakmgr_module_work_mode == ASSIST_CMD_TYPE) {
        return retval;
    }
    
    while (1) {
        if (++ retry_times > 10) {
           retval = RAK_FAIL;
           break;
        }
        
        // Opening AT mode may get failed because unwanted network transmission can break in the process.
        // In every retry, purge all buffer data which should not appear in the process.
        rakmgr_uartbuf_init();
        rakmgr_sockdata_prg(-1);
        
        rak_UART_send((uint8_t *) RAK_OPEN_CMD_REQUEST, strlen(RAK_OPEN_CMD_REQUEST));
        rakmgr_sockdata_prg(-1);
        retval = rakmgr_sockdata_timeout(RAK_200MS_TIMEOUT, rakmgr_sockdata_poll_trn);
        if (retval < 0) {
            retval = RAK_SUCCESS;
            continue;
        }
        
        if (uCmdRspFrame.uCmdRspBuf[0] == RAK_ACK_FOR_REQUEST) {
            rak_UART_send((uint8_t *) RAK_CONFIRM_CMD_MODE, strlen(RAK_CONFIRM_CMD_MODE));
            rakmgr_sockdata_prg(-1);
            retval = rakmgr_sockdata_timeout(RAK_200MS_TIMEOUT, rakmgr_sockdata_poll_at_resp);
            if (retval < 0) {
                retval= RAK_SUCCESS;
                continue;
            }
            
            if (strncmp((char *) uCmdRspFrame.uCmdRspBuf, "OK", 2) == 0) {
                retval = RAK_SUCCESS;
                // FIXME
                rakmgr_module_work_mode = ASSIST_CMD_TYPE;
                break;
            }
            else {
                retval = RAK_SUCCESS;
                continue;
            }
        }
        else {
            retval = RAK_SUCCESS;
            continue;
        }
    }
//  RAK_DEBUG_PORT_PIN=~RAK_DEBUG_PORT_PIN;
    return retval;
}

int rakmgr_easy_txrxMode(void)
{
    int retval = RAK_SUCCESS;
    
    if (rakmgr_module_work_mode == EASY_TXRX_TYPE) {
        return retval;
    }
    
    retval = rakmgr_open_cmdMode();
    if (retval<0) {
       return retval;
    }
    //delay_ms(1000);       
    
    retval = rak_easy_txrxMode();
    if (retval<0) {
        return retval;
    }
    
    rakmgr_sockdata_prg(-1);
    retval = rakmgr_sockdata_timeout(RAK_200MS_TIMEOUT, rakmgr_sockdata_poll_at_resp);
    if (retval < 0) {
        return retval;
    }
    
    if (strncmp((char *) uCmdRspFrame.uCmdRspBuf, "OK",2)) {
        retval= RAK_FAIL;
        return retval;
    }
    rakmgr_module_work_mode = EASY_TXRX_TYPE;
    
    return retval;
}

void delay_ms(uint32_t ms)
{
    //while (ms --) {
    //    CLK_SysTickDelay(1000);
    //}
    systimer_delay_ms(ms);
}
