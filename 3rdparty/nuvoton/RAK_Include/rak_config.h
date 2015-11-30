#ifndef _RAKCONFIG_H_
#define _RAKCONFIG_H_

/**/
/**
 * Global Defines
 */
#define EASY_TXRX_TYPE      0
#define ASSIST_CMD_TYPE     1

//ģ����������ģʽѡ��  ͸������ģʽ �� ��������ģʽ

#define  RAK_MODULE_WORK_MODE   ASSIST_CMD_TYPE          //ASSIST_CMD_TYPE/EASY_TXRX_TYPE

/***************ģ��ģʽѡ������ ********************/

#define  RAK_WLAN_MODE                                "wifi_mode=AP"	   //@Param:  AP , STA
#define  RAK_POWER_MODE                               "power_mode=full"    //@Param:  full , save


/***************APģʽ�������� ********************/
#define  RAK_AP_SSID                                  "ap_ssid=RAK_AP_SSID"
#define  RAK_AP_CHANNEL                               "ap_channel=6"	       //@Param: 1-13
#define  RAK_AP_SECU_EN                               "ap_secu_en=0"	       //@Param: 0:������ 1������
#define  RAK_AP_PSK                                   "ap_psk=1234567890"
#define  RAK_AP_IPADDR                                "ap_ipaddr=192.168.7.1"
#define  RAK_AP_NETMASK                               "ap_netmask=255.255.255.0"
#define  RAK_AP_BDCAST_EN                             "ap_bdcast_en=1"
//#define  RAK_AP_DHCP_EN                             "ap_dhcp_en=1"	        //Ĭ��ʹ�� �ݲ�֧���޸�


/***************STAģʽ�������� ********************/
#define  RAK_STA_SSID                                 "sta_ssid=TP-LINK_2.4GHz"
#define  RAK_STA_SECU_EN                              "sta_secu_en=1"		     //@Param: 0:������ 1������
#define  RAK_STA_PSK                                  "sta_psk=1234567890"
#define  RAK_STA_DHCP_EN                              "sta_dhcp_en=1"		  	 //@Param: 0����̬����  1����̬��·�ɻ�ȡIP (DHCP��ʽ)
#define  RAK_STA_IPADDR                               "sta_ipaddr=192.168.1.100"
#define  RAK_STA_NETMASK                              "sta_netmask=255.255.255.0"
#define  RAK_STA_GATEWAY                              "sta_gateway=192.168.1.1"
#define  RAK_STA_DNSS1                                "sta_dnssever1=192.68.1.1"
#define  RAK_STA_DNSS2                                "sta_dnssever2=0.0.0.0"

/***************UARTͨ�Ų������� ********************/

#define  RAK_UART_BAUD                                 "uart_baudrate=115200"   //@Param: autobaud���ݲ�֧�֣���9600,19200,38400,57600,115200,230400,380400,460800,921600
#define  RAK_UART_DATALEN                              "uart_datalen=8"		    //@Param: 5-8
#define  RAK_UART_PARITYEN                             "uart_parity_en=0"	    //@Param: 0��None 1��Odd  3:Even
#define  RAK_UART_STOPLEN                              "uart_stoplen=1"		    //@Param: 1-2
//#define  RAK_UART_TIMEOUT                            "uart_timeout=20"		  //�ݲ�֧��
//#define  RAK_UART_RECVLENOUT                         "uart_recvlenout=500"
#define  RAK_UART_RTSCTSEN                             "uart_rtscts_en=0"	    //@Param: 0:Disable  1:Enable  2:RS485/RTS (RTS��Ϊ�շ��л��� �ߵ�ƽ���ͣ�


/***************Socketͨ�Ų������� ********************/
#define  RAK_SOCKET_BAUD							    "socket_multi_en=1"		      //@Param: 0:��Socketģʽ  1��˫Socketģʽ

#define  RAK_SOCKETA_TYPE                               "socketA_type=ludp"		       //@Param: tcp,ltcp,udp,ludp (TCP�ͻ��ˣ�TCP��������UDP�ͻ��ˣ�UDP������)
#define  RAK_SOCKETA_LPORT                              "socketA_localport=5570"	   //@Param: 1-65535
#define  RAK_SOCKETA_DESTIP                             "socketA_destip=192.168.1.101"
#define  RAK_SOCKETA_DPORT                              "socketA_destport=5570"	   //@Param: 1-65535
#define  RAK_SOCKETA_TCPTIMEOUT                         "socketA_tcp_timeout=0"		   //@Param: 1-600 S	 0������

#define  RAK_SOCKETB_TYPE                               "socketB_type=ludp"			    //@Param: tcp,ltcp,udp,ludp (TCP�ͻ��ˣ�TCP��������UDP�ͻ��ˣ�UDP������)
#define  RAK_SOCKETB_LPORT                              "socketB_localport=25001"		//@Param: 1-65535
#define  RAK_SOCKETB_DESTIP                             "socketB_destip=192.168.1.101"
#define  RAK_SOCKETB_DPORT                              "socketB_destport=25001"	    //@Param: 1-65535
#define  RAK_SOCKETB_TCPTIMEOUT                         "socketB_tcp_timeout=0"		    //@Param: 1-600 S	 0������


/***************ģ���������� ***********************/
#define  RAK_MODULE_NAME                                "module_name=rak415"			//@Param: ����16�ֽ�
#define  RAK_MODULE_GROUP                               "module_group=rak415"			//@Param: ����16�ֽ�


/***************����WEB���������� ***********************/
#define  RAK_WEB_USERNAME                               "user_name=admin"				//@Param: ����16�ֽ�
#define  RAK_WEB_PASSWORD                               "user_password=admin"			//@Param: ����16�ֽ�
//#define  RAK_WEB_SWITCH                               "web_switch=0"					//@Param: 0:ԭ��������ҳ   1���û�������ҳ
#define  RAK_WEB_DLANGUAGE                              "web_en=1"					    //@Param: 0��Ӣ��WEB   1������WEB  ��һ�δ���WEBʱ��ʾ����





/***************���ֹ��ܿ�ѡ���� ***********************/
#define  RAK_WEB_FUNCEN                                "web_func_en=1"	                // 0���ر�WEB����   1������WEB���ܽӿ�   Ĭ�Ͽ���
#define  RAK_UDP_FUNCEN                                "udp_func_en=1"                  // 0���ر�UDP����   2������UDP���ܽӿ�   Ĭ�Ͽ���
#define  RAK_MODE_PIN                                  "mode_pin=wps"                   // wps:�л�ģ��mode����ΪWPS����   easy���л�Ϊ������������

//���๦���������ӡ�����

#endif
