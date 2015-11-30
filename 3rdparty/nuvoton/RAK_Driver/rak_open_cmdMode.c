//;*****************************************************
//;Company :   rakwireless
//;File Name : rak_open_cmdMode.c
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
 * @fn           int16 rak_open_cmdMode(void)
 * @brief        UART, open cmd interface 
 * @param[in]    none 
 * @param[out]   none
 * @return       errCode
 *               -1 = busy 
 *               0  = SUCCESS
 * @section description  
 * 
 */
 /*=============================================================================*/
int8 rak_open_cmdMode(void)
{
	int8	     retval=0;
    uint8_t	     retry_times=0;	   //重试次数设置
//  RAK_DEBUG_PORT_PIN=~RAK_DEBUG_PORT_PIN;	   	
	while(1)
	{	    
		
		if(++retry_times>10)
		{
		   retval= RAK_FAIL;
	       break;
		}
		RAK_RESET_TIMER;
		rak_UART_send((uint8_t*)RAK_OPEN_CMD_REQUEST,strlen(RAK_OPEN_CMD_REQUEST));//发送打开模块命令接口请求
	  	while((UART_RecieveDataFlag == FALSE) && (RAK_INC_TIMER<=RAK_200MS_TIMEOUT));//等待模块ACK 并置超时
	
		if(UART_RecieveDataFlag == TRUE)
		{
		  UART_RecieveDataFlag = FALSE;
		  if(uCmdRspFrame.uCmdRspBuf[0]==RAK_ACK_FOR_REQUEST)			//接收到模块对请求的应答
		  {
		    RAK_RESET_TIMER;
			rak_UART_send((uint8_t*)RAK_CONFIRM_CMD_MODE,strlen(RAK_CONFIRM_CMD_MODE));//3S 内回复应答确认
			while((UART_RecieveDataFlag == FALSE) && (RAK_INC_TIMER<=RAK_200MS_TIMEOUT));//等待模块ACK 并置超时

			if(UART_RecieveDataFlag == TRUE)
		    {
			   UART_RecieveDataFlag = FALSE;
			   if(strncmp((char *)uCmdRspFrame.uCmdRspBuf,"OK",2) == 0)
			   {
			   	 	retval= RAK_SUCCESS;
					break;
			   }else
			   {
			   	    retval= RAK_FAIL;
					break;
			   }
			}else
			   {
			   	    retval= RAK_FAIL;
					break;
			   } 
		  }else
		   {
			 retval= RAK_FAIL;
		   	 break;
	       }
		}
	   else
	   {
		 retval= RAK_FAIL;
	   }

	}
//	RAK_DEBUG_PORT_PIN=~RAK_DEBUG_PORT_PIN;
	return retval;
}

