/* FreeRTOS Config file*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION                    1
#define configTICK_RATE_HZ                      ( ( portTickType ) 1000 )
#define configPERIPHERAL_CLOCK_HZ               (80000000UL)
#define configCPU_CLOCK_HZ                      (80000000UL)

//#define configPERIPHERAL_CLOCK_HZ               (20000000UL)
//#define configCPU_CLOCK_HZ                      (40000000UL)

//#define configUSE_TIMER5                        1

#define configUSE_16_BIT_TICKS                  0
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_TRACE_FACILITY                0
#define configUSE_COUNTING_SEMAPHORES           0
#define configUSE_MUTEXES                       1

#define configMINIMAL_STACK_SIZE                ( 240 )
#define configISR_STACK_SIZE                    ( 400 )
#define configKERNEL_INTERRUPT_PRIORITY         0x01
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    0x04

#define configMAX_PRIORITIES                    ( ( unsigned portBASE_TYPE ) 6 )
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) 0 ) /* Not used when heap_3 is included in the build. */
#define configMAX_TASK_NAME_LEN                 ( 8 )
#define configIDLE_SHOULD_YIELD                 1
#define configCHECK_FOR_STACK_OVERFLOW          0

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         ( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet                0
#define INCLUDE_uxTaskPriorityGet               0
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskCleanUpResources           0
#define INCLUDE_vTaskSuspend                    0
#define INCLUDE_vTaskDelayUntil                 0
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_uxTaskGetStackHighWaterMark     0

/* Defines the MAC address to be used. */
#define configMAC_0 0x00
#define configMAC_1 0x04
#define configMAC_2 0x9F
#define configMAC_3 0x00
#define configMAC_4 0xAB
#define configMAC_5 0x2B

/* Defines the IP address to be used. */
#define configIP_ADDR0  169
#define configIP_ADDR1  254
#define configIP_ADDR2  1
#define configIP_ADDR3  1

/* Defines the gateway address to be used. */
#define configGW_ADDR0  169
#define configGW_ADDR1  254
#define configGW_ADDR2  1
#define configGW_ADDR3  1

/* Defins the net mask. */
#define configNET_MASK0 255
#define configNET_MASK1 255
#define configNET_MASK2 0
#define configNET_MASK3 0


#endif /* FREERTOS_CONFIG_H */


