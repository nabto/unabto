//;*****************************************************
//;Company :   rakwireless
//;File Name : rak_send_data.c
//;Author :    Junhua
//;Create Data : 2014-01-12
//;Last Modified : 
//;Description : RAK415 WIFI UART  DRIVER
//;Version :    1.0.6
//;update url:  www.rakwireless.com
//;****************************************************

#include "BSP_Driver.h"


/*=============================================================================*/
/**
 * @fn           int16 rak_send_data
 * @brief        UART
 * @param[in]    socket_id, destpor,destip,Send_Data,Send_DataLen
 * @param[out]   none
 * @return       errCode
 *               -1 = busy 
 *               0  = SUCCESS
 * @section description  
 * 
 */
 /*=============================================================================*/

int8_t rak_send_Data(uint8_t socket_id,uint16_t destport,uint32_t destip,uint8_t *send_Data,uint16_t send_DataLen)
{
	 int8	     retval=0;
	 char rak_sendCmd[50]="";
	 char sock[2]="";
	 char sendlen[5]="";
	 char send_dport[6]="";
	 char send_dip[25]="";

	 sock[0]=socket_id+0x30;
	 strcpy(rak_sendCmd,RAK_SEND_DATA_CMD);
	 strcat(rak_sendCmd,sock);
	 strcat(rak_sendCmd,RAK_COMMA);

	 /****添加 发送目标端口 UDPSever下有效****/
	 sprintf(send_dport,"%d",destport);
	 strcat(rak_sendCmd,send_dport);
	 strcat(rak_sendCmd,RAK_COMMA);

	 /****添加 发送目标IP   UDPSever下有效****/
	 sprintf(send_dip,"%d.%d.%d.%d",(destip>>24)&0xFF,(destip>>16)&0xFF,(destip>>8)&0xFF,destip&0xFF);
	 strcat(rak_sendCmd,send_dip);
	 strcat(rak_sendCmd,RAK_COMMA);

	 sprintf(sendlen,"%d",send_DataLen);
	 strcat(rak_sendCmd,sendlen);
	 strcat(rak_sendCmd,RAK_COMMA);

	 retval=rak_UART_send((uint8_t*)rak_sendCmd,strlen(rak_sendCmd));
     retval=rak_UART_send(send_Data,send_DataLen);
	 retval=rak_UART_send(RAK_END,strlen(RAK_END));
	 return retval;
}
