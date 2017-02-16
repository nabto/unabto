/****************************************************************************
*  Program/file: AppNabto.c
*   
*  Copyright (C) by RTX A/S, Denmark.
*  These computer program listings and specifications, are the property of 
*  RTX A/S, Denmark and shall not be reproduced or copied or used in 
*  whole or in part without written permission from RTX A/S, Denmark.
*
*  Programmer: LKA
*
*  MODULE:
*  CONTROLLING DOCUMENT:
*  SYSTEM DEPENDENCIES:
*   
*   
*  DESCRIPTION: Nabto application
*   
****************************************************************************/

/****************************************************************************
*                                  PVCS info                                  
*****************************************************************************

$Author:   lka  $
$Date:   29 Nov 2012 14:09:18  $
$Revision:   1.0  $
$Modtime:   29 Nov 2012 14:09:12  $
$Archive:   J:/sw/Projects/Amelie/Components/Nabto/vcs/AppNabto.c_v  $

*/


/****************************************************************************
*                               Include files                                 
****************************************************************************/
//Type definitions
#include <Core/RtxCore.h> // Mandatory, Must be the first include

//Framework/Kernel
#include <Ros/RosCfg.h>

//Interfaces
#include <Api/Api.h>
#include <Cola/Cola.h>
#include <Protothreads/Protothreads.h>
#include <PtApps/AppSocket.h>
#include <PtApps/AppShell.h>
#include <PtApps/AppWifi.h>
#include <stdio.h>

//Configurations

//Private
//#include <Nabto/AppNabto.h>     // Use library
#include "AppNabto.h"         // Use Source
#include <unabto/unabto.h>

/****************************************************************************
*                              Macro definitions                              
****************************************************************************/


/****************************************************************************
*                     Enumerations/Type definitions/Structs                   
****************************************************************************/
typedef struct
{
  struct pt ChildPt;
  PtEntryType *PtEntryPtr;
  nabto_main_context nmc;
} AppNabtoDataType;

/****************************************************************************
*                            Global variables/const                           
****************************************************************************/
const rsuint16 UnabtoVersionMajor = VERSION_MAJOR; 
const rsuint16 UnabtoVersionMinor = VERSION_MINOR; 

/****************************************************************************
*                            Local variables/const                            
****************************************************************************/
AppNabtoDataType AppNabtoData; // All static data stored by AppNabto

// Timer info
static const RosTimerConfigType AppNabtoTickTimer = ROSTIMER(COLA_TASK, APP_NABTO_TICK_TIMEOUT, APP_NABTO_TICK_TIMER);

/****************************************************************************
*                          Local Function prototypes                          
****************************************************************************/


/****************************************************************************
*                                Implementation                               
****************************************************************************/
// This PT function is called each time a mail is received
static PT_THREAD(PtAppNabtoOnMail(struct pt *Pt, const RosMailType* Mail))
{
  PT_BEGIN(Pt);

  PT_WAIT_UNTIL(Pt, (AppWifiIsConnected() && (0 != AppWifiIpv4GetAddress())));

  // Init uNabto
  if(!unabto_init())
  {
    PT_EXIT(Pt);
  }

  // Start tick timer
  RosTimerStart(APP_NABTO_TICK_TIMER, RS_T100MS, &AppNabtoTickTimer);
  
  while(1) // This function should never end
  {
    PT_YIELD_UNTIL(Pt, IS_RECEIVED(APP_NABTO_TICK_TIMEOUT));
    unabto_tick_timer_tick(100);
    RosTimerStart(APP_NABTO_TICK_TIMER, RS_T100MS, &AppNabtoTickTimer);

    if(AppWifiIsConnected() && !AppWifiIsSuspended())
    {
      unabto_tick();
    }
  }

  unabto_close();
  PT_END(Pt);
}

// This PT function is called if PtAppNabtoOnMail() ends
static PT_THREAD(PtAppNabtoOnExit(struct pt *Pt, const RosMailType* Mail))
{
  PT_BEGIN(Pt);

  // Clean up all resouces allocated/used
  AppNabtoData.PtEntryPtr = NULL;
  //nabto_main_close(&AppNabtoData.nmc);
  PT_END(Pt);
}

// Application event notification call back from unabto
application_event_result application_event(application_request* req, uanbto_query_request* r_b, unabto_query_response* w_b)
{
  RSASSERTSTATIC((int)REQ_RESPONSE_READY   == (int)AER_REQ_RESPONSE_READY);
  RSASSERTSTATIC((int)REQ_NOT_READY        == (int)AER_REQ_NOT_READY);
  RSASSERTSTATIC((int)REQ_NO_ACCESS        == (int)AER_REQ_NO_ACCESS);
  RSASSERTSTATIC((int)REQ_TOO_SMALL        == (int)AER_REQ_TOO_SMALL);
  RSASSERTSTATIC((int)REQ_TOO_LARGE        == (int)AER_REQ_TOO_LARGE);
  RSASSERTSTATIC((int)REQ_INV_QUERY_ID     == (int)AER_REQ_INV_QUERY_ID);
  RSASSERTSTATIC((int)REQ_RSP_TOO_LARGE    == (int)AER_REQ_RSP_TOO_LARGE);
  RSASSERTSTATIC((int)REQ_OUT_OF_RESOURCES == (int)AER_REQ_OUT_OF_RESOURCES);
  RSASSERTSTATIC((int)REQ_SYSTEM_ERROR     == (int)AER_REQ_SYSTEM_ERROR);
  RSASSERTSTATIC((int)REQ_NO_QUERY_ID      == (int)AER_REQ_NO_QUERY_ID);
  return (application_event_result)AppNabtoEventCb(req->queryId, (void*)r_b, (void*)w_b); 
}

/*****************************************************************************
*                      AppNabto interface functions                        *
*****************************************************************************/
PtEntryType* AppNabtoInit(RsListEntryType *PtList, rschar *IdPtr)
{
  platform_initialize();

  if(AppNabtoData.PtEntryPtr == NULL)
  {
    nabto_main_setup* nms = unabto_init_context();
    nms->id = (const char*)IdPtr;
    
    AppNabtoData.nmc.nabtoMainSetup = *nms;
    AppNabtoData.PtEntryPtr = PtStart(PtList, PtAppNabtoOnMail, PtAppNabtoOnExit, NULL);
  }
  return AppNabtoData.PtEntryPtr;
}

rsbool AppNabtoRead(void *ReadBufferPtr, rsuint32 Length, rsuint8 *DataPtr)
{
  rsbool result = TRUE; 

  while(Length-- && result == TRUE)
  {
    result = unabto_query_read_uint8((unabto_query_request*)ReadBufferPtr, DataPtr++);
  }
  return result;
}

rsbool AppNabtoReadUint8(void *ReadBufferPtr, rsuint8 *DataPtr)
{
  return unabto_query_read_uint8((unabto_query_request*)ReadBufferPtr, DataPtr);
}

rsbool AppNabtoReadUint16(void *ReadBufferPtr, rsuint16 *DataPtr)
{
  return unabto_query_read_uint16((unabto_query_request*)ReadBufferPtr, DataPtr);
}

rsbool AppNabtoReadUint32(void *ReadBufferPtr, rsuint32 *DataPtr)
{
  return unabto_query_read_uint32((unabto_query_request*)ReadBufferPtr, DataPtr);
}


rsbool AppNabtoWrite(void *WriteBufferPtr, rsuint32 Length, rsuint8 *DataPtr)
{
  rsbool result = TRUE; 

  while(Length-- && result == TRUE)
  {
    result = unabto_query_write_uint8((unabto_query_response*)WriteBufferPtr, *DataPtr++);
  }
  return result;
}

rsbool AppNabtoWriteUint8(void *WriteBufferPtr, rsuint8 Data)
{
  return unabto_query_write_uint8((unabto_query_response*)WriteBufferPtr, Data);
}

rsbool AppNabtoWriteUint16(void *WriteBufferPtr, rsuint16 Data)
{
  return unabto_query_write_uint16((unabto_query_response*)WriteBufferPtr, Data);
}

rsbool AppNabtoWriteUint32(void *WriteBufferPtr, rsuint32 Data)
{
  return unabto_query_write_uint32((unabto_query_response*)WriteBufferPtr, Data);
}
