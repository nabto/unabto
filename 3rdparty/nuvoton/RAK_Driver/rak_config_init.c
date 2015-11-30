//;*****************************************************
//;Company :  rakwireless
//;File Name : rak_config_init.c
//;Author :    Junhua
//;Create Data : 2014-01-12
//;Last Modified : 
//;Description : RAK415 WIFI UART  DRIVER
//;Version :    1.0.5
//;update url:  www.rakwireless.com
//;****************************************************

#include "rak_global.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX_ARGV                     (4)

int parse_args(char* str, char* argv[])
{
    int i = 0;
    char* ch = str;  
    
    while(*ch != '\0') {
        i++;
        /*Check if length exceeds*/
        if(i > MAX_ARGV) {
            return 0;
        }
        
        argv[i-1] = ch;
        
        while(*ch != ',' && *ch != '\0' && *ch != '\r') {
            if(*ch == '=' && i == 1) {
                break;
            }
            else
                ch++;           
        }
        if (*ch == '\r')
            break;
        if (*ch != '\0') {
            *ch = '\0';
            ch++;
            while(*ch == ',') {
                ch++;
            }
        }
    }
    return i;
}

int set_handler(int argc, char* argv[], rak_cfg_t* config)
{ 
//  int retval = 0;
    int len;
    
    
    if(argc == 1) {
        return 0;
    }   
    
    if (strcmp(argv[0], "wifi_mode") == 0) {
        if (strcmp(argv[1],"AP") == 0 || strcmp(argv[1],"STA") == 0) {
            strcpy(config->mode.wlan_mode,argv[1]) ;
        }        
        else {
            return -1;            
        }
    }
    else if (strcmp(argv[0], "power_mode") == 0) {
        
        if(strcmp(argv[1],"full") == 0 || strcmp(argv[1],"save") == 0) {
            strcpy(config->power_mode,argv[1]);
        }        
        else {
            return -1;            
        }
    } 
    else if (strcmp(argv[0], "ap_channel") == 0) {
         strcpy(config->channel,argv[1]);
    }
    else if (strcmp(argv[0], "ap_ssid") == 0) {
        if (strlen(argv[1]) > 32 || *argv[1] == '\0') {
            return -1;
        }
        strcpy(config->ap_params.ssid, argv[1]);
    }
    else if (strcmp(argv[0], "ap_secu_en") == 0) {
        if(strcmp(argv[1],"0") == 0) {
            config->ap_params.enable ='0';
        }        
        else if(strcmp(argv[1],"1") == 0) {
            config->ap_params.enable ='1';
        }
        else 
            return -1;
    }
    else if (strcmp(argv[0], "ap_psk") == 0) {
	    if(strcmp(argv[1],"0") == 0){
            strcpy(config->ap_params.psk, argv[1]);
        }
        else
            memset(config->ap_params.psk,0,MAX_PASSPHRASE_SIZE);
    }
    else if (strcmp(argv[0], "ap_bdcast_en") == 0) {
        if (strcmp(argv[1],"0") == 0) {
            config->ap_config.ap_bdcast_en = '0';
        }        
        else if (strcmp(argv[1],"1") == 0) {
            config->ap_config.ap_bdcast_en = '1';
        }
        else  
            return -1;
    }
    else if (strcmp(argv[0], "ap_ipaddr") == 0) {
        
          strcpy(config->ap_params.ip_param.addr, argv[1]);
		  strcpy(config->ap_params.ip_param.gw, argv[1]);
		  strcpy(config->ap_params.ip_param.dnsrv1, argv[1]);
		  strcpy(config->ap_params.ip_param.dnsrv2, argv[1]);
    }
    else if (strcmp(argv[0], "ap_netmask") == 0) {
         strcpy(config->ap_params.ip_param.mask, argv[1]);

    }    
    else if (strcmp(argv[0], "ap_dhcp_en") == 0) {
    }
    /* Station Config */    
    else if (strcmp(argv[0], "sta_ssid") == 0) {
        if (strlen(argv[1]) > 32 || *argv[1] == '\0') {
            return -1;
        }
        strcpy(config->sta_params.ssid, argv[1]);
    }
    else if (strcmp(argv[0], "sta_secu_en") == 0) {
        if(strcmp(argv[1],"0") == 0) {
            config->sta_params.enable = '0';
        }        
        else if(strcmp(argv[1],"1") == 0) {
            config->sta_params.enable ='1';
        }
        else 
            return -1;
    }
    else if (strcmp(argv[0], "sta_psk") == 0) {
        if (config->sta_params.enable =='1') {
            if (strlen(argv[1]) < 8 || strlen(argv[1]) > 32){
                return -1;
            }
            strcpy(config->sta_params.psk, argv[1]);
        }
        else
            config->sta_params.psk[0] = '\0';
    }
    else if (strcmp(argv[0], "sta_dhcp_en") == 0) {
        if (strcmp(argv[1],"0") == 0) {
            config->sta_params.enable = '0';
        }        
        else if (strcmp(argv[1],"1") == 0) {
            config->sta_params.enable = '1';
        }
        else   
            return -1;
    }     
    else if (strcmp(argv[0], "sta_ipaddr") == 0) {
         strcpy(config->sta_params.ip_param.addr, argv[1]);
    }
    else if (strcmp(argv[0], "sta_netmask") == 0) {
        strcpy(config->sta_params.ip_param.mask, argv[1]);
    }    
    else if (strcmp(argv[0], "sta_gateway") == 0) {
        strcpy(config->sta_params.ip_param.gw, argv[1]);
    }      
    else if (strcmp(argv[0], "sta_dnssever1") == 0) {
        strcpy(config->sta_params.ip_param.dnsrv1, argv[1]);
    }     
    else if (strcmp(argv[0], "sta_dnssever2") == 0) {
         strcpy(config->sta_params.ip_param.dnsrv2, argv[1]);   
    }     
    /* End Station Config */
    
    /* UART Config */
    else if (strcmp(argv[0], "uart_baudrate") == 0) {        
		    strcpy(config->uart_cfg.BaudRate, argv[1]);  
    }
    else if (strcmp(argv[0], "uart_datalen") == 0) {
    	   strcpy(config->uart_cfg.DataBits, argv[1]);
    }      
    else if (strcmp(argv[0], "uart_parity_en") == 0) {
           strcpy(config->uart_cfg.Parity, argv[1]);
    }      
    else if (strcmp(argv[0], "uart_stoplen") == 0) {    
	     strcpy(config->uart_cfg.StopBits, argv[1]);
    }
    else if (strcmp(argv[0], "uart_rtscts_en") == 0) {
		 strcpy(config->uart_cfg.Enable485, argv[1]);
    }
    else if (strcmp(argv[0], "uart_timeout") == 0) {

    }
    else if (strcmp(argv[0], "uart_recvlenout") == 0) {

    }
    else if (strcmp(argv[0], "web_switch") == 0) {
        
        if (strcmp(argv[1], "0") == 0) {
            config->web_switch = '0';
        }
        else if (strcmp(argv[1], "1") == 0) {
            config->web_switch = '1';
        }
        else 
            return -1;
    }
    else if (strcmp(argv[0], "web_en") == 0) {
        
        if (strcmp(argv[1], "0") == 0) {
            config->web_en = '0';
        }
        else if (strcmp(argv[1], "1") == 0) {
            config->web_en = '1';
        }
        else 
            return -1;
    }
    /* End UART Config */
    
    /* Socket Config */
    else if (strcmp(argv[0], "socket_multi_en") == 0) {

        if (strcmp(argv[1],"0") == 0) {
            config->conn_multi = '0';
        }        
        else if(strcmp(argv[1],"1") == 0) {
            config->conn_multi = '1';
        }
        else 
            return -1;
    }
    else if (strcmp (argv[0], "socketA_type") == 0) {
          strcpy(config->tcp_udp[0].protocol, argv[1]);   
    }
    else if (strcmp (argv[0], "socketA_destip") == 0) {
		 strcpy(config->tcp_udp[0].foreign_addr.sin_addr, argv[1]);  
    }
    else if (strcmp (argv[0], "socketA_destport") == 0) {
        strcpy(config->tcp_udp[0].foreign_addr.sin_port, argv[1]);          
    }
    else if (strcmp (argv[0], "socketA_localport") == 0) {
       strcpy(config->tcp_udp[0].local_port, argv[1]); 
    }
    else if (strcmp (argv[0], "socketA_tcp_timeout") == 0) {
        strcpy(config->tcp_udp[0].time_out, argv[1]); 
    }
    else if (strcmp (argv[0], "socketB_type") == 0) {
        strcpy(config->tcp_udp[1].protocol, argv[1]);  
    }
    else if (strcmp (argv[0], "socketB_destip") == 0) {
        strcpy(config->tcp_udp[1].foreign_addr.sin_addr, argv[1]); 
    }
    else if (strcmp (argv[0], "socketB_destport") == 0) {
        strcpy(config->tcp_udp[1].foreign_addr.sin_port, argv[1]); 
    }
    else if (strcmp (argv[0], "socketB_localport") == 0) {
       strcpy(config->tcp_udp[1].local_port, argv[1]); 
    }
    else if (strcmp (argv[0], "socketB_tcp_timeout") == 0) {
       strcpy(config->tcp_udp[1].time_out, argv[1]); 
    }
    else if (strcmp (argv[0], "user_name") == 0) {
      
        strcpy(config->module_info.user_name, argv[1]);
    }
    else if (strcmp (argv[0], "user_password") == 0) {
        config->module_info.user_psk[16] = '\0';
        len = strlen(argv[1]);
        if (len > 16 || len < 1)
            return -1;
        /* merge psk and name */
        strcpy(config->module_info.user_psk, argv[1]);
        
    }
    else if (strcmp (argv[0], "module_name") == 0) {
        config->module_info.module_name[16] = '\0';
        len = strlen(argv[1]);
        if (len > 16 || len < 1)
            return -1;
        strcpy(config->module_info.module_name, argv[1]);
    }

    else if (strcmp (argv[0], "module_group") == 0) {
        config->module_info.module_group[16] = '\0';
        len = strlen(argv[1]);
        if (len > 16 || len < 1)
            return -1;
        strcpy(config->module_info.module_group, argv[1]);
    }
    else {
        return -1;
    }
    return 0;
} /* Endbody */


int32_t parse_config(char* str , rak_cfg_t* config)
{
   	 int      argc;
     char*    argv[MAX_ARGV];
    char*    buf;
    uint8_t  flag = 1; 

    do{
        argc = 0;
        if ((buf = strchr(str, '&')) != NULL) {
            *buf = '\0';
        }
        else {
            flag = 0;
        }
        argc = parse_args(str, argv);
        if (argc > 0) {
            if (set_handler(argc, argv, config) != 0) {
                return -1;
            }
        }
        else {
            return -1;
        }
        str = ++buf;
        
    } while(flag);
    
    return 0;
    	   
}


uint8 rak_init_config(char *buff,rak_cfg_t *ptrStrCfg)
{
   int8_t  retval;
   retval= parse_config(buff,ptrStrCfg);
   return retval;
}
