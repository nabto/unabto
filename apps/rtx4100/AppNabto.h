/****************************************************************************
*  Program/file: AppNabto.h
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
*  DESCRIPTION: Protothread based Nabto application.
*   
****************************************************************************/

/****************************************************************************
*                                  PVCS info                                 
*****************************************************************************

$Author:   lka  $
$Date:   29 Nov 2012 14:09:18  $
$Revision:   1.0  $
$Modtime:   29 Nov 2012 14:08:22  $
$Archive:   J:/sw/Projects/Amelie/Components/Nabto/vcs/AppNabto.h_v  $

*/

#ifndef APPNABTO_H
#define APPNABTO_H

/****************************************************************************
*                               Include files                                 
****************************************************************************/
//Type definitions

//Framework/Kernel

//Interfaces

//Configurations

//Private 

/****************************************************************************
*                              Macro definitions                             
****************************************************************************/


/****************************************************************************
*                     Enumerations/Type definitions/Structs                  
****************************************************************************/
typedef enum 
{
  REQ_RESPONSE_READY,   ///< the response is ready and written into the supplied buffer
  na_REQ_ACCEPTED,      ///< the request has been accepted by the application, the response wil be delivered
                        ///  using #application_poll_query() and #application_poll()
  na_REQ_BUSY,          ///< the new request must be queued by the caller/framework because the application has no ressources at the moment

  // The next values are sent to the ClientPeer in exception packets,
  // i.e. the values MUST NOT be changed!
  REQ_NOT_READY,        ///< the application isn't ready yet (still initialising)
  REQ_NO_ACCESS,        ///< the application doesn't allow this request to be answered
  REQ_TOO_SMALL,        ///< the request is too small, i.e. required parameters aren't present
  REQ_TOO_LARGE,        ///< the request is too large
  REQ_INV_QUERY_ID,     ///< the request queryId is invalid
  REQ_RSP_TOO_LARGE,    ///< the response is larger than the space allocated
  REQ_OUT_OF_RESOURCES, ///< no ressources available, to be sent to the Client
  REQ_SYSTEM_ERROR,     ///< internal error
  REQ_NO_QUERY_ID,      ///< the request is so small that no queryId is present
  // End of values sent to a ClientPeer

  ///////////////////////////////////////////////////////////////////////////////////////////
  // The remaining values are for internal use
  REQ_FIRST_NOT_USED    ///< for internal usage (must be largest value)
} AppNabtoEventResultType;

/****************************************************************************
*                           Global variables/const                           
****************************************************************************/
extern const rsuint16 UnabtoVersionMajor; 
extern const rsuint16 UnabtoVersionMinor; 

/****************************************************************************
*                             Function prototypes                            
****************************************************************************/

/**
 * This function is used to init the Nabto Application and the unabto implementation.
 * 
 * @param PtList Pointer to PtList
 * @param IdPtr  Pointer to string holding the device ID. This string must 
 *               be avaialable as long as the Nabto application is running.
 * 
 * @return PtList entry pointer
 */
PtEntryType* AppNabtoInit(RsListEntryType *PtList, rschar *IdPtr);

/**
 * This function is called by the Nabto lib when a request is received. This 
 * function must be implemented by the main application.
 * 
 * @param QueryId         Id of nabto quiry received
 * @param ReadBufferPtr   Pointer to buffer holding the data received
 * @param WriteBufferPtr  Pointer to buffer where to response data is written
 * 
 * @return REQ_RESPONSE_READY if success and REQ_NOT_READY... 
 *         REQ_NO_QUERY_ID if an error is detected.
 */
AppNabtoEventResultType AppNabtoEventCb(rsuint32 QueryId, 
                                        void *ReadBufferPtr, 
                                        void *WriteBufferPtr);

/**
 * Read data from read buffer. This function does not convert 
 * the data from network byte order to host byte order!
 * 
 * @param ReadBufferPtr Pointer to theread buffer
 * @param Length        Number of bytes to read
 * @param DataPtr       Pointer to output buffer
 * 
 * @return TRUE on success; FALSE if data could not be read
 */
rsbool AppNabtoRead(void *ReadBufferPtr, rsuint32 Length, rsuint8 *DataPtr);

/**
 * Read uint8 from the read buffer and increment the read 
 * pointer. 
 * 
 * @param ReadBufferPtr Pointer to theread buffer
 * @param DataPtr       Pointer to output buffer
 * 
 * @return TRUE on success; FALSE if data could not be read
 */
rsbool AppNabtoReadUint8(void *ReadBufferPtr, rsuint8 *DataPtr);

/**
 * 
 * Read uint16 from the read buffer and increment the read 
 * pointer. This function converts the data from network to host
 * byte order. 
 * 
 * @param ReadBufferPtr Pointer to theread buffer
 * @param DataPtr       Pointer to output buffer
 * 
 * @return TRUE on success; FALSE if data could not be read
 */
rsbool AppNabtoReadUint16(void *ReadBufferPtr, rsuint16 *DataPtr);

/**
 * 
 * Read uint32 from the read buffer and increment the read 
 * pointer. This function converts the data from network to host
 * byte order. 
 * 
 * @param ReadBufferPtr Pointer to theread buffer
 * @param DataPtr       Pointer to output buffer
 * 
 * @return TRUE on success; FALSE if data could not be read
 */
rsbool AppNabtoReadUint32(void *ReadBufferPtr, rsuint32 *DataPtr);

/**
 * Append data to write buffer. This function does not convert 
 * the data to network byte order! 
 * 
 * @param WriteBufferPtr Pointer to write buffer
 * @param Length         Number of bytes to write
 * @param DataPtr        Pointer to data to write. Data MUST be 
 *                       in network byte order (big endian)
 * 
 * @return FALSE if the operation would have resulted in a
 *         buffer overflow
 */
rsbool AppNabtoWrite(void *WriteBufferPtr, rsuint32 Length, rsuint8 *DataPtr);

/**
 * Append uint8 to write buffer
 * 
 * @param WriteBufferPtr Pointer to write buffer
 * @param Data           Data to write
 * 
 * @return FALSE if the operation would have resulted in a
 *         buffer overflow
 */
rsbool AppNabtoWriteUint8(void *WriteBufferPtr, rsuint8 Data);

/**
 * Append uint16 to write buffer. Data is converted to network 
 * byte order by this function. 
 * 
 * @param WriteBufferPtr Pointer to write buffer
 * @param Data           Data to write
 * 
 * @return FALSE if the operation would have resulted in a
 *         buffer overflow
 */
rsbool AppNabtoWriteUint16(void *WriteBufferPtr, rsuint16 Data);

/**
 * Append uint32 to write buffer. Data is converted to network 
 * byte order by this function. 
 * 
 * @param WriteBufferPtr Pointer to write buffer
 * @param Data           Data to write
 * 
 * @return FALSE if the operation would have resulted in a
 *         buffer overflow
 */
rsbool AppNabtoWriteUint32(void *WriteBufferPtr, rsuint32 Data);

#endif

