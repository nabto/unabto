/****************************************************************************
* Program/file: Main.c
*
* Copyright (C) by RTX A/S, Denmark.
* These computer program listings and specifications, are the property of
* RTX A/S, Denmark and shall not be reproduced or copied or used in
* whole or in part without written permission from RTX A/S, Denmark.
*
* DESCRIPTION: Co-Located Application (COLA).
*
****************************************************************************/

/****************************************************************************
*                                  PVCS info
*****************************************************************************

$Author:   lka  $
$Date:   07 Mar 2013 16:43:44  $
$Revision:   1.5  $
$Modtime:   07 Mar 2013 16:24:28  $
$Archive:   J:/sw/Projects/Amelie/COLApps/Apps/Nabto/vcs/Main.c_v  $

*/

/****************************************************************************
*                               Include files
****************************************************************************/
#include <Core/RtxCore.h>
#include <Ros/RosCfg.h>
#include <PortDef.h>
#include <Api/Api.h>
#include <Cola/Cola.h>
#include <Nvs/NvsDef.h>
#include <Protothreads/Protothreads.h>
#include <SwClock/SwClock.h>
#include <BuildInfo/BuildInfo.h>
#include <NetUtils/NetUtils.h>

//#include <Nabto/AppNabto.h>     // Use library
#include "AppNabto.h"         // Use Source

#include <PtApps/AppCommon.h>
#include <PtApps/AppShell.h>
#include <PtApps/AppLed.h>
#include <PtApps/AppWifi.h>
#include <PtApps/AppDhcpd.h>
#include <PtApps/AppWebConfig.h>

#include <Drivers/DrvButtons.h>
#include <Drivers/DrvIntTemp.h>
#include <Drivers/DrvNtcTemp.h>
#include <Drivers/DrvI2cIntf.h>
#include <Drivers/DrvApds990x.h>
#include <Drivers/DrvHIH613x.h>
#include <Drivers/DrvLps331ap.h>
#include <Drivers/DrvLsm303dlhc.h>

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/****************************************************************************
*                              Macro definitions
****************************************************************************/
// Program defines
#define ID_STR_LENGTH           32
#define TMP_STR_LENGTH          50

#define CONNECT_FAILED_TIMEOUT  (30*RS_T1SEC)
#define SENSOR_READ_INTERVAL    (60*RS_T1SEC)

#define APP_NVS_OFFSET(x)       (NVS_OFFSET(RtxAppData) + RSOFFSETOF(NvsDataType, x))
#define NVS_APP_NAME_LENGTH     5

#define DELAY(x)                RosTimerStart(APP_DELAY_TIMER, x, &DelayTimer); PT_YIELD_UNTIL(Pt, IS_RECEIVED(APP_DELAY_TIMEOUT))

#define SENSOR_OFFSET           2000

// Soft Access Point defines
#define SOFT_AP_STATIC_IP       "192.168.1.1"
#define SOFT_AP_SUBNET_MASK     "255.255.255.0"
#define SOFT_AP_GATEWAY         "192.168.1.1"
#define SOFT_AP_DHCP            TRUE

#define SOFT_AP_COUNTRY_CODE    "DK"
#define SOFT_AP_CHANNEL         2432  // Channel 5 center frequency
#define SOFT_AP_INACT           5
#define SOFT_AP_BEACON          100

// Acess Point security
#define AP_SECURITY             0
  
#if AP_SECURITY == 1
#define SOFT_AP_SECURITY_TYPE   AWST_WPA2
#define SOFT_AP_KEY             "password"
#else
#define SOFT_AP_SECURITY_TYPE   AWST_NONE
#define SOFT_AP_KEY             ""
#endif

/****************************************************************************
*                     Enumerations/Type definitions/Structs
****************************************************************************/
typedef struct
{
  struct pt ChildPt;
  rsbool ap_mode;
} AppDataType;

/****************************************************************************
*                            Global variables/const
****************************************************************************/
RsListEntryType PtList; // Do not rename/move

/****************************************************************************
*                            Local variables/const
****************************************************************************/
static rschar IdStr[ID_STR_LENGTH];
static rschar TmpStr[TMP_STR_LENGTH];
static AppDataType AppData;

static const RosTimerConfigType DelayTimer = ROSTIMER(COLA_TASK, APP_DELAY_TIMEOUT, APP_DELAY_TIMER);

static AccAxesRaw_t accReading;
static MagAxesRaw_t magReading;
static rsint16      tempReading;

/****************************************************************************
*                          Local Function prototypes
****************************************************************************/

/****************************************************************************
*                                Implementation
****************************************************************************/
static rsuint8 GetRedLed() 
{
  return GPIO_PinOutGet(RED_LED_PORT, RED_LED_PIN);
}

static rsuint8 GetGreenLed() 
{
  return GPIO_PinOutGet(GREEN_LED_PORT, GREEN_LED_PIN);
}

static void SetRedLed(rsbool on)
{
  if (on) {
    GPIO_PinOutSet(RED_LED_PORT, RED_LED_PIN);
  }
  else {
    GPIO_PinOutClear(RED_LED_PORT, RED_LED_PIN);
  }
}

static void SetGreenLed(rsbool on) 
{
  if (on) {
    GPIO_PinOutSet(GREEN_LED_PORT, GREEN_LED_PIN);
  }
  else {
    GPIO_PinOutClear(GREEN_LED_PORT, GREEN_LED_PIN);
  }
}

static rsuint8 GetButton1() 
{
  return !(GPIO_PinInGet(MFB_KEY_PORT, MFB_KEY_PIN));
}

static rsuint8 GetButton2() 
{
  return !(GPIO_PinInGet(WPS_KEY_PORT, WPS_KEY_PIN));
}

/** The Nabto application callback */
AppNabtoEventResultType AppNabtoEventCb(rsuint32 QueryId, void *ReadBufferPtr, void *WriteBufferPtr)
{
  switch(QueryId) {
    /** Case 1: State Query */
    case 1:
      if (TRUE) {
        AppNabtoWriteUint8(WriteBufferPtr, GetButton1());
        AppNabtoWriteUint8(WriteBufferPtr, GetButton2());
        AppNabtoWriteUint8(WriteBufferPtr, GetRedLed());
        AppNabtoWriteUint8(WriteBufferPtr, GetGreenLed());
      }
      return REQ_RESPONSE_READY;
    /** Case 2: Set Query */
    case 2:
      if (TRUE) {
        rsuint8 ledId, ledState;
        
        if (AppNabtoReadUint8(ReadBufferPtr, &ledId) == FALSE) {
          return REQ_TOO_SMALL;
        }
        if (AppNabtoReadUint8(ReadBufferPtr, &ledState) == FALSE) {
          return REQ_TOO_SMALL;
        }

        switch(ledId) {
          case 0:
            SetRedLed(ledState);
            break;
          case 1:
            SetGreenLed(ledState);
            break;
          default:
            return REQ_INV_QUERY_ID;
        }
        AppNabtoWriteUint8(WriteBufferPtr, ledState);
      }
      return REQ_RESPONSE_READY;
    /** Case 3: Temperature Query */
    case 3:
      if (GetTempRaw(&tempReading) != MEMS_SUCCESS) {
        return REQ_SYSTEM_ERROR;
      }
      else {
        uint16_t temp = (uint16_t)(tempReading + SENSOR_OFFSET);
        
        //snprintf(TmpStr, TMP_STR_LENGTH, "Temp: %d \n", AppData.Temperature);
        //AppShellPrint(TmpStr);
        
        AppNabtoWriteUint16(WriteBufferPtr, temp);
      }
      return REQ_RESPONSE_READY;
    /** Case 4: Accelerometer Query */
    case 4:
      if (GetAccAxesRaw(&accReading) != MEMS_SUCCESS) {
        return REQ_SYSTEM_ERROR;
      }
      else {
        uint16_t acc_x, acc_y, acc_z;
        
        acc_x = (uint16_t)(accReading.AXIS_X + SENSOR_OFFSET);
        acc_y = (uint16_t)(accReading.AXIS_Y + SENSOR_OFFSET);
        acc_z = (uint16_t)(accReading.AXIS_Z + SENSOR_OFFSET);
        
        //snprintf(TmpStr, TMP_STR_LENGTH, "Acc: %d %d %d\n", accReading.AXIS_X, accReading.AXIS_Y, accReading.AXIS_Z);
        //AppShellPrint(TmpStr);
        
        AppNabtoWriteUint16(WriteBufferPtr, acc_x);
        AppNabtoWriteUint16(WriteBufferPtr, acc_y);
        AppNabtoWriteUint16(WriteBufferPtr, acc_z);
      }
      return REQ_RESPONSE_READY;
    /** Case 5: Magnetometer Query */
    case 5:
      if (GetMagAxesRaw(&magReading) != MEMS_SUCCESS) {
        return REQ_SYSTEM_ERROR;
      }
      else {
        uint16_t mag_x, mag_y, mag_z;
        
        mag_x = (uint16_t)(magReading.AXIS_X + SENSOR_OFFSET);
        mag_y = (uint16_t)(magReading.AXIS_Y + SENSOR_OFFSET);
        mag_z = (uint16_t)(magReading.AXIS_Z + SENSOR_OFFSET);
        
        //snprintf(TmpStr, TMP_STR_LENGTH, "Mag: %d %d %d\n", magReading.AXIS_X, magReading.AXIS_Y, magReading.AXIS_Z);
        //AppShellPrint(TmpStr);
        
        AppNabtoWriteUint16(WriteBufferPtr, mag_x);
        AppNabtoWriteUint16(WriteBufferPtr, mag_y);
        AppNabtoWriteUint16(WriteBufferPtr, mag_z);
      }
      return REQ_RESPONSE_READY;
  }
  return REQ_NO_QUERY_ID;
}

/*****************************************************************************
*                                AppShell                                    *
*****************************************************************************/
#if APP_SHELL_ENABLED == 1
rsbool AppShellPrintWelcomeCallback(void)
{
  snprintf(TmpStr, TMP_STR_LENGTH, "\n\nNabto v%lu.%lu\n", UnabtoVersionMajor, UnabtoVersionMinor);
  AppShellPrint(TmpStr);
  snprintf(TmpStr, TMP_STR_LENGTH, "Build: 20%02X-%02X-%02X %02X:%02X\n", LinkDate[0], LinkDate[1], LinkDate[2], LinkDate[3], LinkDate[4]);
  AppShellPrint(TmpStr);

  AppShellPrint("ID: ");
  AppShellPrint(IdStr);
  AppShellPrint("\n");

  if (AppWifiIpConfigIsStaticIp()) {
    inet_ntoa(AppWifiIpv4GetAddress(), TmpStr);
    AppShellPrint("Static IP: ");
    AppShellPrint(TmpStr);
    AppShellPrint("\n");
  }
  else {
    AppShellPrint("DHCP enabled\n");
  }

  if (AppData.ap_mode) {
    AppShellPrint("AP MODE");
  }
  else {
    AppShellPrint("SSID: ");
    AppShellPrint((char*)AppWifiGetSsid(0));
  }
  AppShellPrint("\n\n");

  AppShellShowCursor();
  return TRUE;
}

void AppShellPrintHelpCallback(void)
{
}

// Project specific handling of shell commands
static PT_THREAD(PtOnCommand(struct pt *Pt, const RosMailType *Mail, rsuint16 Argc, char *Argv[1]))
{
  PT_BEGIN(Pt);

  if(Argc) {
    AppShellPrint("unknown cmd\n");
  }

  PT_END(Pt);
}
#endif

/*****************************************************************************
*                     Main application loop - PtMain                         *
*****************************************************************************/
static PT_THREAD(PtMain(struct pt *Pt, const RosMailType *Mail))
{  
  PT_BEGIN(Pt);

  // Power on/reset the Wifi chip
  PT_SPAWN(Pt, &AppData.ChildPt, PtAppPrologueNoConnect(&AppData.ChildPt, Mail));

  // Build ID string
  const ApiWifiMacAddrType* mac_addr = AppWifiGetMacAddr();
  snprintf(IdStr, sizeof(IdStr), "%02x%02x%02x.rtx.u.nabto.net", (*mac_addr)[3], (*mac_addr)[4], (*mac_addr)[5]);
    
  // Start in WEB config (SoftAp) mode?
  if (!GPIO_PinInGet(WPS_KEY_PORT, WPS_KEY_PIN))
  {
    // Switch on the GREEN led to signal to the user that WEB config mode is started
    AppLedSetLedState(LED_STATE_ACTIVE);
    
    // Setup Web Config
    AppWebConfigSetSsid((rsuint8 *)IdStr);
    AppWebConfigSetSecType(SOFT_AP_SECURITY_TYPE);
    //AppWebConfigSetTitle((rsuint8 *)TitleStr);
    //AppWebConfigSetInfoText((rsuint8 *)InfoStr);
    
    // Do WEB config (the module must be reset to exit WEB config mode)
    PT_SPAWN(Pt, &AppData.ChildPt, PtAppWebConfig(&AppData.ChildPt, Mail, &PtList));
  }
  
  // Check if we should go to Soft AP mode
  if (GetButton1()) {
    AppData.ap_mode = TRUE;

    // Setup the relevant values for the Soft AP
    AppWifiApSetSoftApInfo((rsuint8 *)IdStr, SOFT_AP_SECURITY_TYPE, FALSE, strlen((char*)SOFT_AP_KEY), (rsuint8 *)SOFT_AP_KEY, 
                           SOFT_AP_CHANNEL, SOFT_AP_INACT, (rsuint8 *)SOFT_AP_COUNTRY_CODE, SOFT_AP_BEACON);
  
    // Now start the Soft AP
    PT_SPAWN(Pt, &AppData.ChildPt, PtAppWifiStartSoftAp(&AppData.ChildPt, Mail));
  }
  
  // Set Wifi power management params
  AppWifiSetPowerSaveProfile(POWER_SAVE_HIGH_IDLE);
  AppWifiSetListenInterval(100);
  
#if APP_SHELL_ENABLED == 1
  // Init AppShell - common shell handler
  AppShellInit(&PtList, PtOnCommand);
  PT_YIELD_UNTIL(Pt, FALSE == AppShellIsActive());
#endif
  
  // Init the Nabto application
  AppNabtoInit(&PtList, IdStr);
  
  // Start Soft DHCP Server
  if (AppData.ap_mode && SOFT_AP_DHCP) {
    AppDhcpdStart(&PtList);
  }
  
  while (1)
  {
    // Don't do anything if the AppShell is active
#if APP_SHELL_ENABLED == 1
    while (AppShellIsActive())
    {
      PT_YIELD(Pt);
    }
#endif

    if (AppData.ap_mode) {
      PT_YIELD(Pt);
    }
    else {
      // Connect to the AP if not connected
      while (!AppWifiIsConnected()) {
        // Try to connect to the AP
        AppShellPrint("\nConnecting...");
        AppLedSetLedState(LED_STATE_CONNECTING);
        PT_SPAWN(Pt, &AppData.ChildPt, PtAppWifiConnect(&AppData.ChildPt, Mail));
        
        if (AppWifiIsConnected()) {
          // Connected!
          AppLedSetLedState(LED_STATE_CONNECTED);
          AppShellPrint("\nConnected\n");
        }
        else {
          // Not connected to AP!
          AppLedSetLedState(LED_STATE_ERROR_NO_AP);
          AppShellPrint("\nConnect failed\n");

          // Power off wifi
          PT_SPAWN(Pt, &AppData.ChildPt, PtAppWifiPowerOff(&AppData.ChildPt, Mail));

          // Start timer and try again when it expires or button is pressed
          RosTimerStart(APP_DELAY_TIMER, CONNECT_FAILED_TIMEOUT, &DelayTimer);
          PT_WAIT_UNTIL(Pt, IS_RECEIVED(APP_DELAY_TIMEOUT) || 
                        IS_RECEIVED(KEY_MESSAGE));
          if (IS_RECEIVED(KEY_MESSAGE) && Mail->P1.P1 == KEY_WPS) {
            // Do WPS!
            AppLedSetLedState(LED_STATE_WPS_ONGOING);
            PT_SPAWN(Pt, &AppData.ChildPt, PtAppWifiDoWps(&AppData.ChildPt, Mail));
            AppLedSetLedState(LED_STATE_IDLE);
          }
          else {
            // Power on wifi again
            PT_SPAWN(Pt, &AppData.ChildPt, PtAppWifiPowerOn(&AppData.ChildPt, Mail));
          }
        }
      }

      // Connected to AP!
      while (AppWifiIsConnected()) {
        PT_YIELD(Pt);
      }
    }
  }

  PT_END(Pt);
}

/*****************************************************************************
*                                CoLA TASK                                   *
*****************************************************************************/
static void Init(void)
{
  // Init the Protothreads lib
  PtInit(&PtList);

  // Init the Buttons driver
  DrvButtonsInit();

  // Init the LED application
  AppLedInit(&PtList);

  // Init the WiFi management application
  AppWifiInit(&PtList);
  
  // Init sensors
  InitLsm303dlhc();

  // Start the Main protothread
  PtStart(&PtList, PtMain, NULL, NULL);
}

void ColaTask(const RosMailType *Mail)
{
  // Pre-dispatch mail handling
  switch (Mail->Primitive) {
    case INITTASK:
      Init();
      break;

    case TERMINATETASK:
      RosTaskTerminated(ColaIf->ColaTaskId);
      break;

    case API_GPIO_INTERRUPT_IND:
      // Dispatch API_GPIO_INTERRUPT_IND to the button driver and return
      DrvButtonsOnMail(Mail);
      return;
  }

  // Dispatch mail to all protothreads started
  PtDispatchMail(&PtList, Mail);
}

// End of file.

