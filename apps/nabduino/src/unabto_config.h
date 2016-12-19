#ifndef _UNABTO_CONFIG_H_
#define _UNABTO_CONFIG_H_

#define STACK_CHECK_SIZE                                            4 // Size of the pattern area used for software stack overflow check (a system reset is performed when an overflow is detected). Comment out this line to disable the check.

#define NABTO_ACL_ENABLE                                            0 // Future functionality
#define NABTO_ENABLE_CONFIGURATION_STORE                            0 // Future functionality

// EEPROM mounted on Nabduino 1.2 and onwards (32 kB)
#define EEPROM_24xx256                                              0
// EEPROM mounted on Nabduino 0.4 beta (1 kB)
#define EEPROM_24xx08                                               0

#define NABTO_ENABLE_REMOTE_CONNECTION                              1
#define NABTO_ENABLE_LOCAL_CONNECTION                               1
#define NABTO_ENABLE_LOCAL_ACCESS_LEGACY_PROTOCOL                   0
#define NABTO_REQUEST_MAX_SIZE                                      32
#define NABTO_RESPONSE_MAX_SIZE                                     32
#define NABTO_CONNECTIONS_SIZE                                      3 // maximum number of simultaneous connections.
#define NABTO_SET_TIME_FROM_ALIVE                                   0 // if set to 1 an event will be generated periodically telling the current time.
#define NABTO_ENABLE_EVENTCHANNEL                                   0
#define NABTO_ENABLE_STREAM                                         0

#if NABTO_ACL_ENABLE
#define NABTO_ENABLE_CLIENT_ID                                      1
#define NABTO_ENABLE_CONNECTION_ESTABLISHMENT_ACL_CHECK             1
#define NABTO_ACL_SIZE                                              10
#define NABTO_ENABLE_FLASH_IO_DRIVER                                1
#else
#define NABTO_ENABLE_CLIENT_ID                                      0
#define NABTO_ENABLE_CONNECTION_ESTABLISHMENT_ACL_CHECK             0
#endif

#if __CRYPTO__
#define NABTO_ENABLE_UCRYPTO                                        1
#else
#define NABTO_ENABLE_UCRYPTO                                        0
#endif

#if __DEBUG__
#define NABTO_ENABLE_LOGGING                                        1
#define NABTO_USE_DEBUG_CHANNEL                                     1
#define NABTO_LOG_SEVERITY_MODULE                                   NABTO_LOG_MODULE_ALL
#define NABTO_LOG_SEVERITY_FILTER                                   NABTO_LOG_SEVERITY_TRACE | NABTO_LOG_SEVERITY_INFO
#define NABTO_ENABLE_LOG_TIMESTAMP                                  1
#define NABTO_ENABLE_LOG_FILENAME                                   0
#define NABTO_ENABLE_LOG_MESSAGE                                    1
#define NABTO_LOG_ALL                                               1
#else
#define NABTO_ENABLE_LOGGING                                        0
#endif

#define NABTO_DEVICE_VERSION_MAX_SIZE                               20

#endif
