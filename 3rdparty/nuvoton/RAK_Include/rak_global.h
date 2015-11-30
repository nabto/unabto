#ifndef _RAKGLOBAL_H_
#define _RAKGLOBAL_H_

#include <stdint.h>
/*
 * @ Type Definitions
 */
typedef unsigned char	uint8;
typedef unsigned short	uint16;
typedef unsigned long	uint32;
typedef signed char	    int8;
typedef signed short	int16;
typedef signed long	     int32;

typedef enum Socket{
  SOCKETA_ID  =0 ,
  SOCKETB_ID
}Socket_ID;

#define    RAK_SUCCESS              0
#define    RAK_FAIL                -1
#define    RAK_CFG_ERROR           -2

/*********************  Interface Definition  ***********************************/
#define    RXBUFSIZE 1100

// FIXME: Match NuMaker TRIO
#define    RESET_PORT               PC
#define    RESET_PIN                6
#define    RESET_PORT_PIN           PC6	   //Module RESET pin
// FIXME: No such pin in NuMaker TRIO
//#define    RAK_DEBUG_PORT             P2
//#define    RAK_DEBUG_PIN              5
//#define    RAK_DEBUG_PORT_PIN         P25	   //Module DEBUG pin

/********************** The AT command definition **********************************/

#define    RAK_OPEN_CMD_REQUEST                   "+++"
#define    RAK_ACK_FOR_REQUEST                    'U'
#define    RAK_CONFIRM_CMD_MODE                   "U"

#define    RAK_AND					  		       "&"
#define    RAK_COMMA					  		   ","
#define    RAK_END						           "\r\n"

/***************ģ���������� ********************/
//#define    RAK_OPEN_ASCII_CMD                   "at+ascii=1"
#define    RAK_QUERY_MACADDR_CMD                  "at+mac"
#define    RAK_ENTER_EASYTXRX_CMD                 "at+easy_txrx"
#define    RAK_QUERY_VERSION_CMD                  "at+version"
#define    RAK_RESET_CMD                          "at+reset"
#define    RAK_RESTORE_CMD                        "at+restore"


/***************һ�������������� ****************/
#define    RAK_READ_UCONFIG_CMD                   "at+read_config"
#define    RAK_WRITE_UCONFIG_CMD                  "at+write_config="
#define    RAK_READ_RCONFIG_CMD                   "at+read_restoreconfig"
#define    RAK_WRITE_RCONFIG_CMD                  "at+write_restoreconfig="
#define    RAK_COPY_UCONFIG_CMD                   "at+copy_cfg"				 //������ǰ�û��������򳧲���

/***************AP STA��ѯ�������� ****************/
#define    RAK_QUERY_CONSTATUS_CMD                "at+con_status"
#define    RAK_QUERY_IPCONFIG_CMD                 "at+ipconfig"
#define    RAK_QUERY_TCPSTATUS_CMD                "at+tcp_status=0"			//��ѯ��ǰTCP���ӣ���socket��SocketA��
#define    RAK_QUERY_RSSI_CMD                     "at+rssi"
#define    RAK_SCAN_CMD                           "at+scan=0"
#define    RAK_GET_SCAN_CMD                       "at+get_scan=8"
//#define    RAK_PING_CMD                         "at+ping=192.168.1.1"
//#define    RAK_AP_CONFIG_CMD                    "at+apconfig=CN,1000,100,1"
#define    RAK_EASY_CONFIG_CMD                    "at+easy_config"	 	     //����һ������
#define    RAK_WPS_CONFIG_CMD                     "at+wps"			         //����WPS����




/****************AP STA��ѯ�������� ***************/
#define    RAK_SEND_DATA_CMD         	    "at+send_data="		  // at+send_data=0,10,1234567890 ͨ����socket��SocketA ����10���ֽڳ�������
																  // at+send_data=1,10,1234567890 ͨ��SocketB ����10���ֽڳ�������
#define    RAK_RECIEVE_DATA_CMD             "at+recv_data="		  // ģ���󶯷��ظ�����
                                                                  // at+recv_data=1B(socketID)2B(�Է��˿�)4B(�Է�IP)2B(���ݳ���)nB()���ݲ���
																  //				0            25000    192.168.1.101  10     1234567890
																  // ģ��SocketA���յ���IP��ַ192.168.1.100�Ͷ˿ں�12345�������ֽڳ���Ϊ10�����ݣ�1234567890
#define    SOCKETA_HDER			    "S0"
#define    SOCKETB_HDER			    "S1"
//ģ������͸��ģʽ��  ˫Socket����ʱ  ���ݲ��ֿ�ͷ����S0��S1  ��ʾ��SocketA��SocketB  ���ͻ�����


/********************************��ʱ����*********************************/
// FIXME
#define RAK_TICKS_PER_MISECOND      100000 					 //@MCU-M0  12MHz  100ms
#define RAK_INC_TIMER               rak_Timer++
#define RAK_RESET_TIMER             rak_Timer=0


#define RAK_200MS_TIMEOUT           2 * RAK_TICKS_PER_MISECOND
#define RAK_QFWVTIMEOUT			    1 * RAK_TICKS_PER_MISECOND
#define RAK_QMACTIMEOUT			    1 * RAK_TICKS_PER_MISECOND
#define RAK_WRCFGTIMEOUT			5 * RAK_TICKS_PER_MISECOND
#define RAK_RDCFGTIMEOUT			5 * RAK_TICKS_PER_MISECOND
#define RAK_QCONSATUSTIMEOUT        2 * RAK_TICKS_PER_MISECOND
#define RAK_QIPCFGTIMEOUT           2 * RAK_TICKS_PER_MISECOND
#define RAK_WPSTIMEOUT              1 * RAK_TICKS_PER_MISECOND
#define RAK_EASYCFGTIMEOUT          1 * RAK_TICKS_PER_MISECOND

/**
 * Device Parameters
 */
#define RAK_FRAME_CMD_RSP_LEN           10
#define RAK_MAX_PAYLOAD_SIZE			1200      // maximum recieve data payload size
#define RAK_MAX_PAYLOAD_SEND_SIZE		800       // maximum recieve data payload size
#define RAK_AP_SCANNED_MAX		        8	       // maximum number of scanned acces points

#define RAK_MAX_DOMAIN_NAME_LEN 		42
#define RAK_SSID_LEN			        33	     // maximum length of SSID
#define RAK_BSSID_LEN			        6	     // maximum length of SSID
#define RAK_IP_ADD_LEN                  4
#define RAK_MAC_ADD_LEN                 6

#define RAK_PSK_LEN						33


typedef struct {
	uint8					ssid[RAK_SSID_LEN];			// 33-byte ssid of scanned access point
	uint8					bssid[RAK_BSSID_LEN];			// 32-byte bssid of scanned access point
	uint8					rfChannel;				// rf channel to us, 0=scan for all
	uint8					rssiVal;				// absolute value of RSSI
	uint8					securityMode;				// security mode
	uint8	 				end[2];
} rak_getscanInfo;

typedef  union {
	struct {
		uint8                       rspCode[2];  			   //0= success	   !0= Failure
		uint8				    	scanCount;				// uint8, number of access points found
		uint8	 					end[2];
	} scanOkframe;
	struct {
		uint8                     	rspCode[5];                    		// command code
		uint8                     	ErrorCode;
		uint8	 					end[2];
	} scanErrorframe;
	uint8				      scanFrameRcv[8]  ;			// uint8, socket descriptor, like a file handle, usually 0x00
} rak_scanResponse;

typedef  union {
	struct {
		uint8                   rspCode[2];
		rak_getscanInfo		    strScanInfo[RAK_AP_SCANNED_MAX];	// 32 maximum responses from scan command
		uint8	 				end[2];
	} getOkframe;
	struct {
		uint8                     	rspCode[5];                    		// command code
		uint8                     	ErrorCode;
		uint8	 					end[2];
	} getErrorframe;
	uint8				      getscanFrameRcv[RAK_AP_SCANNED_MAX*42+4] ;			// uint8, socket descriptor, like a file handle, usually 0x00
} rak_getscanResponse;


typedef struct {
	uint8                   rspCode[2];  	 //0=connected  -2=no connect
} rak_qryconResponse;

/*typedef struct {
	uint8                   rspCode; 			        //0= success  -3,-4,-5,-6,-7,-8=error
	uint8					ssid[RAK_SSID_LEN];			//33-byte ssid of wps connect access point
	uint8					securityMode;				//security mode
	uint8					psk[RAK_PSK_LEN	];			    //65Byte
	uint8                   end[2];						//\r\n
} rak_wpsconnectResponse; */


typedef  union {

	struct {
		uint8                   rspCode[2];  	 //0=connected  -2=no connect
		uint8                   rssi;
		uint8	 			    end[2];
	} qryrssiOkframe;
	uint8				       qryrssiFrameRcv[5]  ;			// uint8, socket descriptor, like a file handle, usually 0x00
} rak_qryrssiResponse;

typedef  union {

	struct {
		uint8                   rspCode[2];  	 //0=connected  -2=no connect
		uint8                   status;
		uint8	 			    end[2];
	} qryconstatusframe;
	uint8				       qryconstatusFrameRcv[5]  ;			// uint8, socket descriptor, like a file handle, usually 0x00
} rak_qryconstatusResponse;

typedef  union {

	struct {
		uint8                       rspCode[2];  			   //0= success	   !0= Failure
		uint8				        ipAddr[4];				// Configured IP address
		uint8				        netMask[4];				// Configured netmask
		uint8				        gateWay[4];				// Configured default gateway
		uint8				        dns1[4];				// dns1
		uint8				        dns2[4];				// dns2
		uint8	 					end[2];
	} qryipconfigframe;

	uint8				      qryipconfigFrameRcv[24];			// uint8, socket descriptor, like a file handle, usually 0x00
} rak_qryipconfigResponse;



typedef struct {
	uint8                   rspCode[2]; 			   //0= success	   !0= Failure
} rak_pingResponse;


typedef  union {

	struct {
		uint8                       recvheader[13]; 		   // at+recv_data=
		uint8				        socketID;
		uint8					    destPort[2];
		uint8					    destIp[4];
		uint8					    recDataLen[2];
		uint8                       recvdataBuf[RAK_MAX_PAYLOAD_SIZE];
		uint8	 					end[2];
	} recvdata;
	   uint8				        socketFrameRcv[RAK_MAX_PAYLOAD_SIZE+24]  ;			// uint8, socket descriptor, like a file handle, usually 0x00
} rak_recvdataFrame;


typedef  union {

	struct {
		uint8                   rspCode[2];
		uint8					hostFwversion[8];				// uint8[10], firmware version text string, 1.0.6.0
	} qryframe;
	uint8				      qryFwversionFrameRcv[10]  ;			// uint8, socket descriptor, like a file handle, usually 0x00
} rak_qryFwversionFrameRcv;



typedef union {

	rak_scanResponse          		scanResponse;
	rak_getscanResponse          	getscanResponse;
//	rak_wpsconnectResponse          wpsconnectResponse;
    rak_qryconstatusResponse		qryconstatusResponse;
	rak_qryrssiResponse             qryrssiResponse;
	rak_qryipconfigResponse         qryipconfigResponse;
	rak_pingResponse                pingResponse;
	rak_recvdataFrame          		recvdataFrame;
//	rak_resetResponse               resetResponse;
	rak_qryFwversionFrameRcv  		qryFwversionFrameRcv;
	uint8					        uCmdRspBuf[RAK_FRAME_CMD_RSP_LEN + RAK_MAX_PAYLOAD_SIZE];

} rak_uCmdRsp;



#define MAX_PASSPHRASE_SIZE             (32+1)//64
#define MAX_SSID_LENGTH 		        (32+1)


typedef struct mode {
    char wlan_mode[4];
}mode_t;

typedef struct DRVUART_STRUCT
{
    char        BaudRate[9]; //921600 autobaud
	char        DataBits[2]; //8
    char        StopBits[2]; //2
   	char          Parity[2]; //1
    char	   Enable485[2];
	char         padding[1];
}STR_UART_T;


typedef  struct sockaddr {
	 char   sin_port[16];	   //Port number
 	 char   sin_addr[43];	   //IPv4 Address or domain
} SOCKADDR_T;


typedef struct {
     char        protocol[5];	//ltcp
     char        local_port[6];   //65535
     SOCKADDR_T  foreign_addr;
     char        time_out[4]; 	//600
}tcpip_info_t;

typedef struct {

    uint8_t  ap_bdcast_en;
/*  uint8_t  dtimval;
    uint8_t  countryCode[3];
    uint8_t  ps_val[2];
    uint16_t beaconInterval;
    uint32_t period;*/
}ap_config_t;

typedef struct {
    char     addr[16];   // 192.168.255.255
    char     mask[16];
    char     gw[16];
    char     dnsrv1[16];
    char     dnsrv2[16];
}ip_param_t;


typedef struct {
    uint32_t   enable;
    char       psk[MAX_PASSPHRASE_SIZE];
    char       ssid[MAX_SSID_LENGTH];
    ip_param_t ip_param;
}ap_sta_cfg_t;

typedef struct {
    char     user_name[17];
    char     user_psk[17];
    char     module_name[17];
    char     module_group[17];
}module_t;

typedef struct {
    /* ap/station */ /* power */
    mode_t          mode;
    char            power_mode[5];

	/* ap */
    char            channel[4];
    ap_config_t     ap_config;
    ap_sta_cfg_t    ap_params;
    /* sta */
    ap_sta_cfg_t    sta_params;

	 /* socket */
    uint32_t        conn_multi;
    tcpip_info_t    tcp_udp[2];

    /* uart */
    STR_UART_T      uart_cfg;

    /* module */
    module_t        module_info;
	/* web */
    uint32_t        web_switch;
    uint32_t        web_en;

}rak_cfg_t;




extern volatile uint8 read_flag ;
extern rak_uCmdRsp	uCmdRspFrame;


int8 * rak_bytes4ToAsciiDotAddr(uint8 *hexAddr,uint8 *strBuf);
void rak_asciiDotAddressTo4Bytes(uint8 *hexAddr, int8 *asciiDotAddress,  uint8 length);

extern rak_cfg_t    rak_userCfgstr;
extern rak_cfg_t    rak_restoreCfgstr;
uint8  rak_init_config(char *buff,rak_cfg_t *ptrStrCfg);

#include "rak_uart_api.h"
#include "rak_config.h"
#endif
