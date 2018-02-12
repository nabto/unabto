/*
 * Copyright (C) Nabto - All Rights Reserved.
 */
#ifndef _ACL_H_
#define _ACL_H_

#include <unabto/unabto_env_base.h>
#include <unabto/unabto.h>

#ifdef __cplusplus
extern "C" {
#endif

#if NABTO_ACL_ENABLE

#define ACL_PERMISSION_NONE                                         0x00000000ul
#define ACL_PERMISSION_ALL                                          0xfffffffful

#define ACL_PERMISSION_LOCAL_ACCESS                                 0x80000000ul
#define ACL_PERMISSION_REMOTE_ACCESS                                0x40000000ul
#define ACL_PERMISSION_ACCESS_CONTROL                               0x20000000ul

typedef enum
{
  ACL_STATUS_OK,
  ACL_STATUS_CONFIGURATION_STORE_FAILURE,
  ACL_STATUS_USER_NOT_FOUND,
  ACL_STATUS_INVALID_ENTRY,
  ACL_STATUS_NOT_EMPTY,
  ACL_STATUS_NO_ROOM
} acl_status;

#if !UNABTO_PLATFORM_PIC18
#pragma pack(1)
#endif

typedef struct
{
  char name[NABTO_CLIENT_ID_MAX_SIZE];
  uint32_t permissions;
} acl_user;

#if !UNABTO_PLATFORM_PIC18
#pragma pack()
#endif

acl_status acl_initialize(void);
acl_status acl_get_number_of_users(uint8_t* count);
acl_status acl_look_up_user(const char* name, uint32_t* permissions);
acl_status acl_get_next_user(uint8_t* physicalIndex, acl_user* user);
acl_status acl_add_user(const acl_user* user);
acl_status acl_remove_user(const char* name);

bool acl_is_request_allowed(application_request* request, uint32_t requiredPermissions);

#if NABTO_ENABLE_STREAM
bool acl_is_stream_allowed(unabto_stream* streamHandle, uint32_t requiredPermissions);
#else
#define acl_is_stream_allowed(streamHandle, requiredPermissions) (false)
#endif

application_event_result acl_application_event(application_request* request, unabto_query_request* readBuffer, unabto_query_response* writeBuffer);

#else

#define acl_is_request_allowed(request, permissions) (true)
#define acl_is_stream_allowed(streamHandle, permissions) (true)

#define acl_application_event(request, readBuffer, writeBuffer) (AER_REQ_INV_QUERY_ID)

#endif

#ifdef __cplusplus
} //extern "C"
#endif

#endif
