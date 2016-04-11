/*
 * FreeRTOS+UDP V1.0.0 (C) 2013 Real Time Engineers ltd.
 *
 * This file is part of the FreeRTOS+UDP distribution.  The FreeRTOS+UDP license
 * terms are different to the FreeRTOS license terms.
 *
 * FreeRTOS+UDP uses a dual license model that allows the software to be used 
 * under a standard GPL open source license, or a commercial license.  The 
 * standard GPL license (unlike the modified GPL license under which FreeRTOS 
 * itself is distributed) requires that all software statically linked with 
 * FreeRTOS+UDP is also distributed under the same GPL V2 license terms.  
 * Details of both license options follow:
 *
 * - Open source licensing -
 * FreeRTOS+UDP is a free download and may be used, modified, evaluated and
 * distributed without charge provided the user adheres to version two of the
 * GNU General Public License (GPL) and does not remove the copyright notice or
 * this text.  The GPL V2 text is available on the gnu.org web site, and on the
 * following URL: http://www.FreeRTOS.org/gpl-2.0.txt.
 *
 * - Commercial licensing -
 * Businesses and individuals that for commercial or other reasons cannot comply
 * with the terms of the GPL V2 license must obtain a commercial license before 
 * incorporating FreeRTOS+UDP into proprietary software for distribution in any 
 * form.  Commercial licenses can be purchased from http://shop.freertos.org/udp 
 * and do not require any source files to be changed.
 *
 * FreeRTOS+UDP is distributed in the hope that it will be useful.  You cannot
 * use FreeRTOS+UDP unless you agree that you use the software 'as is'.
 * FreeRTOS+UDP is provided WITHOUT ANY WARRANTY; without even the implied
 * warranties of NON-INFRINGEMENT, MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. Real Time Engineers Ltd. disclaims all conditions and terms, be they
 * implied, expressed, or statutory.
 *
 * 1 tab == 4 spaces!
 *
 * http://www.FreeRTOS.org
 * http://www.FreeRTOS.org/udp
 *
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

#ifdef MANAGED_EMBEDDED_PLATFORM
#include "os_interface.h"
#include "system_watchdog.h"
static int swdHandle;
static uint32_t swdCount=0;
#endif

/* FreeRTOS+UDP includes. */
#include "FreeRTOS_UDP_IP.h"

/* Only attempt to build the rest of the code if Nabto is actually being used. */
#if ipconfigFREERTOS_PLUS_NABTO

#if configUSE_16_BIT_TICKS != 0
    #error FreeRTOS+Nabto requires portTickType to be 32-bits.  Set configUSE_16_BIT_TICKS to 0 in FreeRTOSConfig.h
#endif

/* Nabto includes. */
#include "unabto/unabto.h"

/* Always expect the calculated delay period to be less than this.  If it is
above this value then it is likely that the tick value has wrapped before the
Nabto time stamp (indicating that the next event that requires processing is
actually in the past.*/
#define nabtoWRAP_ERROR_VALUE    0x7fffffffUL

/*
 * Implements the Nabto service task.
 */
static void prvNabtoTask( void *pvParameters );

/*
 * The function that must be supplied by the application to return the URL
 * used by the device.
 */
extern const char *pcApplicationNabtoDeviceURL( void );

/*
* The function that must be supplied by the application to return the preshared
* crypto key used by the device.
*/
extern const char *pcApplicationNabtoDeviceKey(void);

/*-----------------------------------------------------------*/

/* The socket set used to block on the local and remote sockets 
simultaneously. */
static xSocketSet_t xFD_Set = NULL;

/* The Nabto main context from the Nabto code. */

#define MIN_POLL_INTERVAL 50 // ms
#define MAX_NABTO_TASK_BLOCKING_TIME 2 // ample time
/*-----------------------------------------------------------*/

#if NABTO_ENABLE_REMOTE_ACCESS

    static void prvNabtoTask( void *pvParameters )
    {
        uint32_t ipAddress;
//        static uint32_t bt;
    portTickType xTimeNow, xNextEventTime, xBlockTime;

        /* Remove compiler warnings as the parameter is not used. */
        ( void ) pvParameters;
#ifdef MANAGED_EMBEDDED_PLATFORM
        swdHandle = swdRegisterThread(MAX_NABTO_TASK_BLOCKING_TIME);
#endif
        for( ;; )
        {
            unabto_next_event( &xNextEventTime );
            xTimeNow = nabtoGetStamp();

            /* This if() will prevent errors when the next event time has
            wrapped    but the time now hasn't, or during start up when the next
            event time is calculated to be in the past. */
            xBlockTime = 0;
            if( xTimeNow < xNextEventTime )
            {
                /* This if() will prevent errors when the tick count wraps
                before the Nabto time stamp - indicating that the due time for
                the next event is actually in the past. */
                if( ( xNextEventTime - xTimeNow ) < nabtoWRAP_ERROR_VALUE )
                {
                    /* Nabto does not require immediate service, so may as well
                    block until either it does or a network packet is
                    received. */
                    xBlockTime = ( xNextEventTime - xTimeNow );
                }
            }
#if NABTO_APPLICATION_EVENT_MODEL_ASYNC
            if (nabtoMsec2Stamp(MIN_POLL_INTERVAL) < xBlockTime)
            {
                xBlockTime = nabtoMsec2Stamp(MIN_POLL_INTERVAL);
            }
#endif
//bt = xBlockTime;
            FreeRTOS_select( xFD_Set, xBlockTime );
            unabto_tick();
            FreeRTOS_GetAddressConfiguration(&ipAddress, NULL, NULL, NULL);
            ipAddress = FreeRTOS_htonl(ipAddress);
            if (unabto_get_main_context()->nabtoMainSetup.ipAddress != ipAddress)
                unabto_notify_ip_changed(ipAddress);
#ifdef MANAGED_EMBEDDED_PLATFORM
            swdThreadAlive(swdHandle, swdCount++);
#endif
        }
    }

#else /* NABTO_ENABLE_REMOTE_ACCESS */

    /* 
     * When remote access is disabled the Nabto task only has to respond to 
     * local traffic as it arrives, and can therefore block indefinitely to 
     * wait for traffic.
     */
    static void prvNabtoTask( void *pvParameters )
    {
        /* Remove compiler warnings as the parameter is not used. */
        ( void ) pvParameters;

        for( ;; )
        {
            /* There is no base station to contact so just delay until local
            network traffic arrives. */
            FreeRTOS_select( xFD_Set, portMAX_DELAY );
            unabto_tick();
        }
    }

#endif /* NABTO_ENABLE_REMOTE_ACCESS */

/*-----------------------------------------------------------*/

portBASE_TYPE xStartNabtoTask( void )
{
static portBASE_TYPE xNabtoTaskStarted = pdFALSE;
portBASE_TYPE xReturn = pdPASS;
    nabto_main_setup *nms;
    /* Ensure the Nabto task is not created more than once. */
    if( xNabtoTaskStarted == pdFALSE )
    {
        uint32_t ipAddress;
        xNabtoTaskStarted = pdTRUE;

        /* Set default values. */
        nms = unabto_init_context();

        /* Overwrite the default ID with the ID supplied by the application. */
        nms->id = pcApplicationNabtoDeviceURL();
        FreeRTOS_GetAddressConfiguration(&ipAddress, NULL, NULL, NULL);
        nms->ipAddress = FreeRTOS_htonl(ipAddress);

        /* setup crypto */
        nms->secureAttach = true;
        nms->secureData = true;
        nms->cryptoSuite = CRYPT_W_AES_CBC_HMAC_SHA256;
        memcpy(nms->presharedKey, pcApplicationNabtoDeviceKey(), PRE_SHARED_KEY_SIZE);

        if( unabto_init() == false )
        {
            xReturn = pdFAIL;
        }

        /* Create the socket set used to block on the local and remote 
        sockets. */
        xFD_Set = FreeRTOS_CreateSocketSet( 20 );
        configASSERT( xFD_Set );

        if( xFD_Set == NULL )
        {
            xReturn = pdFAIL;
        }
        else
        {
            /* Add the sockets that are actually being used to the set. */
            uint16_t nSockets;
            nabto_socket_t *sockets = getActiveSockets(&nSockets);
            if (NULL == sockets)
            {
                xReturn = pdFAIL;
            }
            else
            {/*
                for (i=0; i < nSockets; i++)
                    FreeRTOS_FD_SET( sockets[i], xFD_Set );
                    */
                /* nSockets > 0 */
                do
                {
                    FreeRTOS_FD_SET( sockets[--nSockets], xFD_Set );
                } while (nSockets > 0);
            }

        }

        if( xReturn == pdPASS )
        {

            xReturn = xTaskCreate(  prvNabtoTask,                       /* The function that implements the task. */
                                    ( const char * const ) "Nabto",     /* Text name for the task - for debug only. */
                                    ipconfigNABTO_TASK_STACK_SIZE,      /* Stack size set in FreeRTOSIPConfig.h. */
                                    NULL,                               /* No parameters. */
                                    ipconfigNABTO_TASK_PRIORITY,        /* Stack size set in FreeRTOSIPConfig.h. */
                                    ( xTaskHandle * ) NULL );           /* Don't need the task handle. */
        }
        else
        {
            /* No attempt is made to clean up any sockets that might have been
            created as it is assumed the application is not going anywhere
            anyway.  Also, this code can only execute once. */
        }
    }

    configASSERT( xReturn );
    return xReturn;
}
/*-----------------------------------------------------------*/



#endif /* ipconfigFREERTOS_PLUS_NABTO */



