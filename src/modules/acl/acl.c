/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#define NABTO_LOG_MODULE_CURRENT NABTO_LOG_MODULE_ACCESS_CONTROL

#include <string.h>
#include "acl.h"

#if NABTO_ACL_ENABLE

#include <modules/configuration_store/configuration_store.h>

enum
{
  QUERY_ACCESS_CONTROL_GET_USERS = 0x80000014,
  QUERY_ACCESS_CONTROL_ADD_USER = 0x80000015,
  QUERY_ACCESS_CONTROL_REMOVE_USER = 0x80000016,
  QUERY_ACCESS_CONTROL_GET_ALLOW_REMOTE_ACCESS = 0x80000017,
  QUERY_ACCESS_CONTROL_SET_ALLOW_REMOTE_ACCESS = 0x80000018,
  QUERY_ACCESS_CONTROL_GET_DEFAULT_LOCAL_PERMISSIONS = 0x80000019,
  QUERY_ACCESS_CONTROL_SET_DEFAULT_LOCAL_PERMISSIONS = 0x8000001a,
  QUERY_ACCESS_CONTROL_GET_DEFAULT_REMOTE_PERMISSIONS = 0x8000001b,
  QUERY_ACCESS_CONTROL_SET_DEFAULT_REMOTE_PERMISSIONS = 0x8000001c
};

#define OFFSET_OF_ACL                                                         (offsetof(application_configuration, acl))
#define OFFSET_OF_ACL_USER(index)                                             (OFFSET_OF_ACL + ((index) * sizeof(acl_user)))
#define OFFSET_OF_ACL_USER_NAME(index)                                        (OFFSET_OF_ACL_USER(index) + offsetof(acl_user, name))
#define OFFSET_OF_ACL_USER_PERMISSIONS(index)                                 (OFFSET_OF_ACL_USER(index) + offsetof(acl_user, permissions))

#ifndef NABTO_GENERAL_PURPOSE_BUFFER_SIZE
#define NABTO_GENERAL_PURPOSE_BUFFER_SIZE sizeof(acl_user)
#endif

extern char nabtoGeneralPurposeBuffer[NABTO_GENERAL_PURPOSE_BUFFER_SIZE];

static uint8_t numberOfUsers;

acl_status acl_initialize(void)
{
  uint8_t i;

  numberOfUsers = 0;

  for(i = 0; i < NABTO_ACL_SIZE; i++)
  {
    uint8_t firstCharacterOfName;
    if(configuration_store_read(OFFSET_OF_ACL_USER_NAME(i), &firstCharacterOfName, 1) == false)
    {
      return ACL_STATUS_CONFIGURATION_STORE_FAILURE;
    }

    if(firstCharacterOfName != 0)
    {
      numberOfUsers++;
    }
  }

  NABTO_LOG_TRACE(("Initialized ACL (number of users: %u).", (int) numberOfUsers));

  return ACL_STATUS_OK;
}

acl_status acl_get_number_of_users(uint8_t* count)
{
  *count = numberOfUsers;

  return ACL_STATUS_OK;
}

acl_status acl_look_up_user(const char* name, uint32_t* permissions)
{
  uint8_t i;
  uint16_t nameLength = strlen(name);

  for(i = 0; i < NABTO_ACL_SIZE; i++)
  {
    bool match;

    if(configuration_store_compare(OFFSET_OF_ACL_USER_NAME(i), name, nameLength + 1, &match) == false)
    {
      NABTO_LOG_ERROR(("ACL: Lookup failed (underlying store failed)!"));
      return ACL_STATUS_CONFIGURATION_STORE_FAILURE;
    }

    if(match)
    {
      if(configuration_store_read(OFFSET_OF_ACL_USER_PERMISSIONS(i), permissions, sizeof (uint32_t)) == false)
      {
        NABTO_LOG_ERROR(("ACL: Lookup failed (underlying store failed)!"));
        return ACL_STATUS_CONFIGURATION_STORE_FAILURE;
      }

      NABTO_LOG_TRACE(("ACL: Look up succeeded for '%s' [%08" PRIx32 "].", name, *permissions));

      return ACL_STATUS_OK;
    }
  }

  NABTO_LOG_TRACE(("ACL: '%s' was not found.", name));

  return ACL_STATUS_USER_NOT_FOUND;
}

acl_status acl_get_next_user(uint8_t* physicalIndex, acl_user* user)
{
  while(*physicalIndex < NABTO_ACL_SIZE)
  {
    if(configuration_store_read(OFFSET_OF_ACL_USER(*physicalIndex), user, sizeof (acl_user)) == false)
    {
      NABTO_LOG_ERROR(("ACL: Lookup failed (underlying store failed)!"));
      return ACL_STATUS_CONFIGURATION_STORE_FAILURE;
    }

    (*physicalIndex)++;

    if(user->name[0] != 0)
    {
      return ACL_STATUS_OK;
    }
  };

  return ACL_STATUS_USER_NOT_FOUND;
}

acl_status acl_add_user(const acl_user* user)
{
  uint8_t i;
  uint16_t nameLength = strlen(user->name);

  // if user is already in the ACL just update permissions
  for(i = 0; i < NABTO_ACL_SIZE; i++)
  {
    bool match;
    if(configuration_store_compare(OFFSET_OF_ACL_USER_NAME(i), &user->name, nameLength + 1, &match) == false)
    {
      NABTO_LOG_ERROR(("ACL: Add failed (underlying store failed)!"));
      return ACL_STATUS_CONFIGURATION_STORE_FAILURE;
    }

    if(match) // user name matches
    {
      if(configuration_store_compare(OFFSET_OF_ACL_USER_PERMISSIONS(i), (const void*) &user->permissions, sizeof (uint32_t), &match) == false)
      {
        NABTO_LOG_ERROR(("ACL: Add failed (underlying store failed)!"));
        return ACL_STATUS_CONFIGURATION_STORE_FAILURE;
      }

      if(match == false)
      {
        if(configuration_store_write(OFFSET_OF_ACL_USER_PERMISSIONS(i), (const void*) &user->permissions, sizeof (uint32_t)) == false)
        {
          NABTO_LOG_TRACE(("ACL: Add failed for '%s' (already in list but permissions could not be updated).", user->name));
          return ACL_STATUS_CONFIGURATION_STORE_FAILURE;
        }
        else
        {
          NABTO_LOG_TRACE(("ACL: Add succeeded for '%s' (already in list but permissions were updated).", user->name));
        }
      }
      else
      {
        NABTO_LOG_TRACE(("ACL: Add succeeded for '%s' (already in list).", user->name));
      }

      return ACL_STATUS_OK;
    }
  }

  // look for an empty entry
  for(i = 0; i < NABTO_ACL_SIZE; i++)
  {
    uint8_t firstCharacterOfName;
    if(configuration_store_read(OFFSET_OF_ACL_USER_NAME(i), &firstCharacterOfName, 1) == false)
    {
      NABTO_LOG_ERROR(("ACL: Lookup failed (underlying store failed)!"));
      return ACL_STATUS_CONFIGURATION_STORE_FAILURE;
    }

    if(firstCharacterOfName == 0) // is the entry empty?
    {
      if(configuration_store_write(OFFSET_OF_ACL_USER(i), user, sizeof (acl_user)) == false)
      {
        NABTO_LOG_ERROR(("ACL: Add failed for '%s' (underlying store failed).", user->name));
        return ACL_STATUS_CONFIGURATION_STORE_FAILURE;
      }

      numberOfUsers++;

      NABTO_LOG_TRACE(("ACL: Add succeeded for '%s'.", user->name));

      return ACL_STATUS_OK;
    }
  }

  NABTO_LOG_TRACE(("ACL: Unable to add '%s' (ACL full).", user->name));

  return ACL_STATUS_NO_ROOM;
}

acl_status acl_remove_user(const char* name)
{
  uint8_t i;
  uint16_t nameLength = strlen(name);

  // if user is already in the ACL just update permissions
  for(i = 0; i < NABTO_ACL_SIZE; i++)
  {
    bool match;
    if(configuration_store_compare(OFFSET_OF_ACL_USER_NAME(i), name, nameLength + 1, &match) == false)
    {
      NABTO_LOG_ERROR(("ACL: Add failed (underlying store failed)!"));
      return ACL_STATUS_CONFIGURATION_STORE_FAILURE;
    }

    if(match) // user name matches
    {
      if(configuration_store_set(OFFSET_OF_ACL_USER(i), 0, sizeof (acl_user)) == false)
      {
        NABTO_LOG_ERROR(("ACL: Remove failed for '%s' (underlying store failed).", name));
        return ACL_STATUS_CONFIGURATION_STORE_FAILURE;
      }

      numberOfUsers--;

      NABTO_LOG_TRACE(("ACL: Remove succeeded for '%s'.", name));

      return ACL_STATUS_OK;
    }
  }

  NABTO_LOG_TRACE(("ACL: Remove failed for '%s' (user not found).", name));

  return ACL_STATUS_USER_NOT_FOUND;
}

// <editor-fold defaultstate="collapsed" desc="Application level access checkers">

#if NABTO_ENABLE_ACL_FULL

static bool access_control_check(bool isConnection, bool isLocal, const char* clientId, uint32_t requiredPermissions);

// connection establishment access control check

bool allow_client_access(nabto_connect* connection)
{
  return access_control_check(true, connection->isLocal, connection->clientId, ACL_PERMISSION_NONE);
}

// application event access control check

bool acl_is_request_allowed(application_request* request, uint32_t requiredPermissions)
{
  return access_control_check(false, request->isLocal, request->clientId, requiredPermissions);
}

// stream access control check
#if NABTO_ENABLE_STREAM

bool acl_is_stream_allowed(unabto_stream* streamHandle, uint32_t requiredPermissions)
{
  nabto_connect* connection = unabto_stream_connection(streamHandle);
  return access_control_check(false, connection->isLocal, connection->clientId, requiredPermissions);
}
#endif

static bool access_control_check(bool isConnection, bool isLocal, const char* clientId, uint32_t requiredPermissions)
{
  bool configurationAllowRemoteAccess;
  uint32_t configurationDefaultLocalPermissions;
  uint32_t configurationDefaultRemotePermissions;
  uint32_t effectivePermissions;
  bool defaultUser = false;
  bool accessGranted = true;

  // read configurations

  if(configuration_store_read(offsetof(application_configuration, allowRemoteAccess), &configurationAllowRemoteAccess, sizeof (configurationAllowRemoteAccess)) == false ||
     configuration_store_read(offsetof(application_configuration, defaultLocalPermissions), &configurationDefaultLocalPermissions, sizeof (configurationDefaultLocalPermissions)) == false ||
     configuration_store_read(offsetof(application_configuration, defaultRemotePermissions), &configurationDefaultRemotePermissions, sizeof (configurationDefaultRemotePermissions)) == false)
  {
    NABTO_LOG_ERROR(("Unable to read from configuration store!"));
    return false;
  }

  // do access control checks

  if(isLocal)
  {
    requiredPermissions |= ACL_PERMISSION_LOCAL_ACCESS; // ...also require local access permission
    effectivePermissions = configurationDefaultLocalPermissions; // load default remote permissions until we access the ACL
  }
  else
  {
    requiredPermissions |= ACL_PERMISSION_REMOTE_ACCESS; // ...also require remote access permission
    effectivePermissions = configurationDefaultRemotePermissions; // load default remote permissions until we access the ACL

    if(configurationAllowRemoteAccess == false) // if allow_remote_access flag is not set deny access regardless of permissions
    {
      accessGranted = false;
    }
  }

  switch(acl_look_up_user(clientId, &effectivePermissions)) // look up user in ACL
  {
    case ACL_STATUS_OK: // user was found
      break;

    case ACL_STATUS_USER_NOT_FOUND:
      defaultUser = true; // user wasn't in ACL so use default permissions
      break;

    default:
      NABTO_LOG_ERROR(("Lookup failed (underlying store failed)!"));
      return false;
  }

  if((effectivePermissions & requiredPermissions) != requiredPermissions)
  {
    accessGranted = false;
  }

  NABTO_LOG_TRACE(("Access control check: granted=%u local=%u connection=%u default=%u user='%s' permis. req/eff=%08" PRIx32 "/%08" PRIx32, (int) accessGranted, (int) isLocal, (int) isConnection, (int) defaultUser, clientId, requiredPermissions, effectivePermissions));

  return accessGranted;
}

// </editor-fold>

application_event_result acl_application_event(application_request* request, unabto_query_request* readBuffer, unabto_query_response* writeBuffer)
{
  switch(request->queryId)
  {
    case QUERY_ACCESS_CONTROL_GET_USERS:
    {
      uint8_t startingIndex;
      uint8_t maximumNumberOfUsers;
      uint8_t totalNumberOfUsers;
      uint8_t numberOfUsersToSend;
      uint8_t numberOfUsersFound = 0;
      uint8_t physicalIndex = 0;
      acl_user* user = (acl_user*) nabtoGeneralPurposeBuffer;

      if(acl_is_request_allowed(request, ACL_PERMISSION_ACCESS_CONTROL) == false)
      {
        return AER_REQ_NO_ACCESS;
      }

      if(unabto_query_read_uint8(readBuffer, &startingIndex) == false || unabto_query_read_uint8(readBuffer, &maximumNumberOfUsers) == false)
      {
        return AER_REQ_TOO_SMALL;
      }

      if(acl_get_number_of_users(&totalNumberOfUsers) != ACL_STATUS_OK)
      {
        return AER_REQ_SYSTEM_ERROR;
      }

      if(startingIndex < totalNumberOfUsers)
      {
        numberOfUsersToSend = totalNumberOfUsers - startingIndex;
      }
      else
      {
        numberOfUsersToSend = 0;
      }

      // limit the number of users to send to 1) no more than requested by the client, 2) no more than the device is capable of sending (due to comm. buffer size)
      numberOfUsersToSend = MIN3(numberOfUsersToSend, maximumNumberOfUsers, MAXIMUM_NUMBER_OF_ACL_USERS_PER_REQUEST);

      if(unabto_query_write_uint16(writeBuffer, numberOfUsersToSend) == false)
      {
        return AER_REQ_RSP_TOO_LARGE;
      }

      while(numberOfUsersToSend > 0)
      {
        acl_get_next_user(&physicalIndex, user); // no need to check if a user was found as previous calculations guarantees this (except for ACL_STATUS_CONFIGURATION_STORE_FAILURE!)

        if(numberOfUsersFound >= startingIndex)
        {
          if(unabto_query_write_uint8_list(writeBuffer, (uint8_t*) user->name, strlen(user->name)) == false || unabto_query_write_uint32(writeBuffer, user->permissions) == false)
          {
            return AER_REQ_RSP_TOO_LARGE;
          }

          numberOfUsersToSend--;
        }

        numberOfUsersFound++;
      }

      if(unabto_query_write_uint8(writeBuffer, totalNumberOfUsers) == false)
      {
        return AER_REQ_RSP_TOO_LARGE;
      }

      return AER_REQ_RESPONSE_READY;
    }

    case QUERY_ACCESS_CONTROL_ADD_USER:
    {
      unabto_buffer nameBuffer;
      acl_user* aclUser = (acl_user*) nabtoGeneralPurposeBuffer;

      if(acl_is_request_allowed(request, ACL_PERMISSION_ACCESS_CONTROL) == false)
      {
        return AER_REQ_NO_ACCESS;
      }

      // read request parameters
      if(unabto_query_read_uint8_list_to_buffer_nc(readBuffer, &nameBuffer) == false || unabto_query_read_uint32(readBuffer, &aclUser->permissions) == false)
      {
        return AER_REQ_TOO_SMALL;
      }

      if(nameBuffer.size < (1 + 1)) // must contain at least one character and zero-termination
      {
        return AER_REQ_TOO_SMALL;
      }

      if(nameBuffer.size > NABTO_CLIENT_ID_MAX_SIZE)
      {
        return AER_REQ_TOO_LARGE;
      }

      memset((void*) &aclUser->name, 0, NABTO_CLIENT_ID_MAX_SIZE);
      memcpy((void*) &aclUser->name, (const void*) nameBuffer.data, nameBuffer.size - 1); // don't copy zero-termination (which might be any non-visible character

      NABTO_LOG_TRACE(("Add user '%s'.", aclUser->name));

      switch(acl_add_user(aclUser))
      {
        case ACL_STATUS_CONFIGURATION_STORE_FAILURE:
          return AER_REQ_SYSTEM_ERROR;

        case ACL_STATUS_NO_ROOM:
          return AER_REQ_OUT_OF_RESOURCES;
      }

      return AER_REQ_RESPONSE_READY;
    }

    case QUERY_ACCESS_CONTROL_REMOVE_USER:
    {
      unabto_buffer nameBuffer;

      if(acl_is_request_allowed(request, ACL_PERMISSION_ACCESS_CONTROL) == false)
      {
        return AER_REQ_NO_ACCESS;
      }

      if(unabto_query_read_uint8_list_to_buffer_nc(readBuffer, &nameBuffer) == false)
      {
        return AER_REQ_TOO_SMALL;
      }

      if(nameBuffer.size < (1 + 1)) // must contain at least one character and zero-termination
      {
        return AER_REQ_TOO_SMALL;
      }

      nameBuffer.data[nameBuffer.size - 1] = 0; // ensure last character is zero-termination

      switch(acl_remove_user((const char*) nameBuffer.data))
      {
        case ACL_STATUS_CONFIGURATION_STORE_FAILURE:
          return AER_REQ_SYSTEM_ERROR;
      }

      return AER_REQ_RESPONSE_READY;
    }

      // <editor-fold defaultstate="collapsed" desc="get/set allow remote access">

    case QUERY_ACCESS_CONTROL_GET_ALLOW_REMOTE_ACCESS:
    {
      uint8_t value;

      if(acl_is_request_allowed(request, ACL_PERMISSION_ACCESS_CONTROL) == false)
      {
        return AER_REQ_NO_ACCESS;
      }

      if(configuration_store_read(offsetof(application_configuration, allowRemoteAccess), (void*) &value, sizeof (value)) == false)
      {
        return AER_REQ_SYSTEM_ERROR;
      }

      if(unabto_query_write_uint8(writeBuffer, value) == false)
      {
        return AER_REQ_RSP_TOO_LARGE;
      }

      return AER_REQ_RESPONSE_READY;
    }

    case QUERY_ACCESS_CONTROL_SET_ALLOW_REMOTE_ACCESS:
    {
      uint8_t value;

      if(acl_is_request_allowed(request, ACL_PERMISSION_ACCESS_CONTROL) == false)
      {
        return AER_REQ_NO_ACCESS;
      }

      if(unabto_query_read_uint8(readBuffer, &value) == false)
      {
        return AER_REQ_TOO_SMALL;
      }

      if(configuration_store_write(offsetof(application_configuration, allowRemoteAccess), (const void*) &value, sizeof (value)) == false)
      {
        return AER_REQ_SYSTEM_ERROR;
      }

      return AER_REQ_RESPONSE_READY;
    }

      // </editor-fold>

      // <editor-fold defaultstate="collapsed" desc="get/set default local permissions">

    case QUERY_ACCESS_CONTROL_GET_DEFAULT_LOCAL_PERMISSIONS:
    {
      uint32_t value;

      if(acl_is_request_allowed(request, ACL_PERMISSION_ACCESS_CONTROL) == false)
      {
        return AER_REQ_NO_ACCESS;
      }

      if(configuration_store_read(offsetof(application_configuration, defaultLocalPermissions), (void*) &value, sizeof (value)) == false)
      {
        return AER_REQ_SYSTEM_ERROR;
      }

      if(uanbto_query_write_uint32(writeBuffer, value) == false)
      {
        return AER_REQ_RSP_TOO_LARGE;
      }

      return AER_REQ_RESPONSE_READY;
    }

    case QUERY_ACCESS_CONTROL_SET_DEFAULT_LOCAL_PERMISSIONS:
    {
      uint32_t value;

      if(acl_is_request_allowed(request, ACL_PERMISSION_ACCESS_CONTROL) == false)
      {
        return AER_REQ_NO_ACCESS;
      }

      if(unabto_query_read_uint32(readBuffer, &value) == false)
      {
        return AER_REQ_TOO_SMALL;
      }

      if(configuration_store_write(offsetof(application_configuration, defaultLocalPermissions), (const void*) &value, sizeof (value)) == false)
      {
        return AER_REQ_SYSTEM_ERROR;
      }

      return AER_REQ_RESPONSE_READY;
    }

      // </editor-fold>

      // <editor-fold defaultstate="collapsed" desc="get/set default remote permissions">

    case QUERY_ACCESS_CONTROL_GET_DEFAULT_REMOTE_PERMISSIONS:
    {
      uint32_t value;

      if(acl_is_request_allowed(request, ACL_PERMISSION_ACCESS_CONTROL) == false)
      {
        return AER_REQ_NO_ACCESS;
      }

      if(configuration_store_read(offsetof(application_configuration, defaultRemotePermissions), (void*) &value, sizeof (value)) == false)
      {
        return AER_REQ_SYSTEM_ERROR;
      }

      if(unabto_query_write_uint32(writeBuffer, value) == false)
      {
        return AER_REQ_RSP_TOO_LARGE;
      }

      return AER_REQ_RESPONSE_READY;
    }

    case QUERY_ACCESS_CONTROL_SET_DEFAULT_REMOTE_PERMISSIONS:
    {
      uint32_t value;

      if(acl_is_request_allowed(request, ACL_PERMISSION_ACCESS_CONTROL) == false)
      {
        return AER_REQ_NO_ACCESS;
      }

      if(unabto_query_read_uint32(readBuffer, &value) == false)
      {
        return AER_REQ_TOO_SMALL;
      }

      if(configuration_store_write(offsetof(application_configuration, defaultRemotePermissions), (const void*) &value, sizeof (value)) == false)
      {
        return AER_REQ_SYSTEM_ERROR;
      }

      return AER_REQ_RESPONSE_READY;
    }

      // </editor-fold>

    default:
      return AER_REQ_INV_QUERY_ID;
  }
}

#endif

#endif
