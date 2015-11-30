//;*****************************************************
//;Company :   rakwireless
//;File Name : rak_wps_config.c
//;Author :    Junhua
//;Create Data : 2014-01-12
//;Last Modified : 
//;Description : RAK415 WIFI UART  DRIVER
//;Version :    1.0.5
//;update url:  www.rakwireless.com
//;****************************************************


#include "BSP_Driver.h"

/*=============================================================================*/
/**
 * @fn           int16 rak_wps_config(void)
 * @brief        UART, rak_wps_config 
 * @param[in]    none 
 * @param[out]   none
 * @return       errCode
 *               -1 = busy 
 *               0  = SUCCESS
 * @section description  
 * 
 */
 /*=============================================================================*/
int8 rak_wps_config(void)
{
	int8	     retval=0;
	char rak_sendCmd[20]="";
	strcpy(rak_sendCmd,RAK_WPS_CONFIG_CMD);
	strcat(rak_sendCmd,RAK_END);
    retval=rak_UART_send((uint8_t*)rak_sendCmd,strlen(rak_sendCmd));//发送打开模块命令接口请求
	return retval;
}

