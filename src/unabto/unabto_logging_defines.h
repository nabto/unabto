#ifndef _UNABTO_LOGGING_DEFINES_H_
#define _UNABTO_LOGGING_DEFINES_H_

// Modules

// Module bit masks.
#define NABTO_LOG_MODULE_NONE                                       0x00000000ul
#define NABTO_LOG_MODULE_ALL                                        0xfffffffful
#define NABTO_LOG_MODULE_DEFAULT                                    0x00000001ul
#define NABTO_LOG_MODULE_UNABTO                                     0x00000002ul
#define NABTO_LOG_MODULE_UNABTO_APPLICATION                         0x00000004ul
#define NABTO_LOG_MODULE_ENCRYPTION                                 0x00000008ul
#define NABTO_LOG_MODULE_STREAM                                     0x00000010ul
#define NABTO_LOG_MODULE_ACCESS_CONTROL                             0x00000020ul
#define NABTO_LOG_MODULE_PLATFORM                                   0x00000040ul
#define NABTO_LOG_MODULE_MEMORY                                     0x00000080ul
#define NABTO_LOG_MODULE_NETWORK                                    0x00000100ul
#define NABTO_LOG_MODULE_APPLICATION                                0x00000200ul
#define NABTO_LOG_MODULE_STACK                                      0x00000400ul
#define NABTO_LOG_MODULE_NSLP                                       0x00000800ul
#define NABTO_LOG_MODULE_PERIPHERAL                                 0x00001000ul
#define NABTO_LOG_MODULE_DEVICE_DRIVER                              0x00002000ul
#define NABTO_LOG_MODULE_STORAGE                                    0x00004000ul
#define NABTO_LOG_MODULE_SERIAL_PORT                                0x00008000ul
#define NABTO_LOG_MODULE_MODBUS                                     0x00010000ul
#define NABTO_LOG_MODULE_TEST                                       0x00020000ul


// Severity definitions

#define NABTO_LOG_SEVERITY_NONE                                     0x00000000ul
#define NABTO_LOG_SEVERITY_ALL                                      0xfffffffful
// Individual bit masks
#define NABTO_LOG_SEVERITY_FATAL                                    0x00000001ul
#define NABTO_LOG_SEVERITY_ERROR                                    0x00000002ul
#define NABTO_LOG_SEVERITY_WARN                                     0x00000004ul
#define NABTO_LOG_SEVERITY_INFO                                     0x00000008ul
#define NABTO_LOG_SEVERITY_DEBUG                                    0x00000010ul
#define NABTO_LOG_SEVERITY_TRACE                                    0x00000020ul
#define NABTO_LOG_SEVERITY_BUFFERS                                  0x00000040ul
#define NABTO_LOG_SEVERITY_USER1                                    0x00000080ul
#define NABTO_LOG_SEVERITY_STATISTICS                               0x00000100ul
#define NABTO_LOG_SEVERITY_STATE                                    0x00000200ul
// Level bit masks
#define NABTO_LOG_SEVERITY_LEVEL_NONE                               NABTO_LOG_SEVERITY_NONE
#define NABTO_LOG_SEVERITY_LEVEL_FATAL                              NABTO_LOG_SEVERITY_FATAL
#define NABTO_LOG_SEVERITY_LEVEL_ERROR                              (NABTO_LOG_SEVERITY_ERROR | NABTO_LOG_SEVERITY_LEVEL_FATAL)
#define NABTO_LOG_SEVERITY_LEVEL_WARN                               (NABTO_LOG_SEVERITY_WARN  | NABTO_LOG_SEVERITY_LEVEL_ERROR)
#define NABTO_LOG_SEVERITY_LEVEL_INFO                               (NABTO_LOG_SEVERITY_INFO  | NABTO_LOG_SEVERITY_LEVEL_WARN )
#define NABTO_LOG_SEVERITY_LEVEL_DEBUG                              (NABTO_LOG_SEVERITY_DEBUG | NABTO_LOG_SEVERITY_LEVEL_INFO )
#define NABTO_LOG_SEVERITY_LEVEL_TRACE                              (NABTO_LOG_SEVERITY_TRACE | NABTO_LOG_SEVERITY_LEVEL_DEBUG)



#endif
