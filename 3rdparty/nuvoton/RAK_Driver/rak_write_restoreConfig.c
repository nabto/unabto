//;*****************************************************
//;Company :   rakwireless
//;File Name : rak_write_restoreConfig.c
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
 * @fn           int16 rak_write_restoreConfig(void)
 * @brief        UART, rak_write_restoreConfig
 * @param[in]    none 
 * @param[out]   none
 * @return       errCode
 *               -1 = Module s busy 
 *               0  = SUCCESS
 * @section description  
 * 
 */
 /*=============================================================================*/


int8 rak_write_restoreConfig(void)
{
	int8	retval=0;	
	char    rak_sendCmd[1000]="";
	char    rak_sendLenth[5]="";
	uint16  SendLength=0;
	uint16  SendHeadLen=0;
	uint16  WriteLength=0;
	uint16  AddLength=0;

	/**********模式 参数*************/
	AddLength=sprintf(rak_sendCmd,"%s%s%s",RAK_WLAN_MODE,RAK_AND,RAK_POWER_MODE);
	WriteLength+=AddLength;
	if(strcmp(RAK_WLAN_MODE, "wifi_mode=AP") == 0)
	{
	  /**********AP 参数*************/
	  AddLength=sprintf(&rak_sendCmd[WriteLength],"%s%s%s%s%s%s%s%s%s%s%s%s%s%s",RAK_AND,RAK_AP_SSID,
						                                 RAK_AND,RAK_AP_CHANNEL,
														 RAK_AND,RAK_AP_SECU_EN,
														 RAK_AND,RAK_AP_PSK,
														 RAK_AND,RAK_AP_IPADDR,
														 RAK_AND,RAK_AP_NETMASK,
														 RAK_AND,RAK_AP_BDCAST_EN
									                     );
	 WriteLength+=AddLength;
	}else
	{
	  /**********STA 参数*************/
	  AddLength=sprintf(&rak_sendCmd[WriteLength],"%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",RAK_AND,RAK_STA_SSID,
						                                 RAK_AND,RAK_STA_SECU_EN,
														 RAK_AND,RAK_STA_PSK,
														 RAK_AND,RAK_STA_DHCP_EN,
														 RAK_AND,RAK_STA_IPADDR,
														 RAK_AND,RAK_STA_NETMASK,
														 RAK_AND,RAK_STA_GATEWAY,
														 RAK_AND,RAK_STA_DNSS1,
														 RAK_AND,RAK_STA_DNSS2
									                     );
	 WriteLength+=AddLength;
	}
	/**********UART 参数*************/
	 AddLength=sprintf(&rak_sendCmd[WriteLength],"%s%s%s%s%s%s%s%s%s%s", RAK_AND,RAK_UART_BAUD,
						                                 RAK_AND,RAK_UART_DATALEN,
														 RAK_AND,RAK_UART_PARITYEN,
														 RAK_AND,RAK_UART_STOPLEN,
														 RAK_AND,RAK_UART_RTSCTSEN														
									                     );

	WriteLength+=AddLength;
	/**********Socket 参数*************/
	AddLength=sprintf(&rak_sendCmd[WriteLength],"%s%s",RAK_AND,RAK_SOCKET_BAUD);

	WriteLength+=AddLength;
	AddLength=sprintf(&rak_sendCmd[WriteLength],"%s%s%s%s%s%s%s%s%s%s",RAK_AND,RAK_SOCKETA_TYPE,
						                                 RAK_AND,RAK_SOCKETA_LPORT,
														 RAK_AND,RAK_SOCKETA_DESTIP,
														 RAK_AND,RAK_SOCKETA_DPORT,
														 RAK_AND,RAK_SOCKETA_TCPTIMEOUT														
									                     );
	 WriteLength+=AddLength;
	if(strcmp(RAK_SOCKET_BAUD, "socket_multi_en=1") == 0)
	{
	  AddLength=sprintf(&rak_sendCmd[WriteLength],"%s%s%s%s%s%s%s%s%s%s",RAK_AND,RAK_SOCKETB_TYPE,
						                                 RAK_AND,RAK_SOCKETB_LPORT,
														 RAK_AND,RAK_SOCKETB_DESTIP,
														 RAK_AND,RAK_SOCKETB_DPORT,
														 RAK_AND,RAK_SOCKETB_TCPTIMEOUT														
	 								                     );
	 WriteLength+=AddLength;
	}

	/**********Name 参数*************/
	AddLength=sprintf(&rak_sendCmd[WriteLength],"%s%s%s%s",RAK_AND,RAK_MODULE_NAME,RAK_AND,RAK_MODULE_GROUP);
   	WriteLength+=AddLength;

	/**********WEB 参数*************/
	AddLength=sprintf(&rak_sendCmd[WriteLength],"%s%s%s%s%s%s",RAK_AND,RAK_WEB_USERNAME,
	                                                           RAK_AND,RAK_WEB_PASSWORD,
															   RAK_AND,RAK_WEB_DLANGUAGE);
    WriteLength+=AddLength;

	/**********Function 选择*************/
    AddLength=sprintf(&rak_sendCmd[WriteLength],"%s%s%s%s%s%s",RAK_AND,RAK_WEB_FUNCEN,
	                                                           RAK_AND,RAK_UDP_FUNCEN,
															   RAK_AND,RAK_MODE_PIN
															   );
    WriteLength+=AddLength;

	//填充配置参数长度
	SendLength=WriteLength;
	sprintf(rak_sendLenth,"%d",SendLength);

	sprintf(&rak_sendCmd[WriteLength],"%s",RAK_END);

	SendHeadLen=sprintf((char *)uCmdRspFrame.uCmdRspBuf,"%s%s%s",RAK_WRITE_RCONFIG_CMD,rak_sendLenth,RAK_COMMA);

	sprintf((char *)&uCmdRspFrame.uCmdRspBuf[SendHeadLen],"%s",rak_sendCmd);

    retval=rak_UART_send(uCmdRspFrame.uCmdRspBuf,SendLength+SendHeadLen+2);//发送打开模块命令接口请求

	return retval;
}

