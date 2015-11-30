/**************************************************************************//**
 * @file     wdt.c
 * @version  V1.00
 * $Revision: 6 $
 * $Date: 14/11/27 11:50a $
 * @brief    Nano 102/112 series WDT driver source file
 *
 * @note
 * Copyright (C) 2013~2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "Nano1x2Series.h"

/** @addtogroup NANO1X2_Device_Driver NANO102/112 Device Driver
  @{
*/

/** @addtogroup NANO1X2_WDT_Driver WDT Driver
  @{
*/


/** @addtogroup NANO1X2_WDT_EXPORTED_FUNCTIONS WDT Exported Functions
  @{
*/

/**
 * @brief This function make WDT module start counting with different time-out interval
 * @param[in] u32TimeoutInterval  Time-out interval period of WDT module. Valid values are:
 *                - \ref WDT_TIMEOUT_2POW4
 *                - \ref WDT_TIMEOUT_2POW6
 *                - \ref WDT_TIMEOUT_2POW8
 *                - \ref WDT_TIMEOUT_2POW10
 *                - \ref WDT_TIMEOUT_2POW12
 *                - \ref WDT_TIMEOUT_2POW14
 *                - \ref WDT_TIMEOUT_2POW16
 *                - \ref WDT_TIMEOUT_2POW18
 * @param[in] u32ResetDelay Reset delay period while WDT time-out happened. Valid values are:
 *                - \ref WDT_RESET_DELAY_3CLK
 *                - \ref WDT_RESET_DELAY_18CLK
 *                - \ref WDT_RESET_DELAY_130CLK
 *                - \ref WDT_RESET_DELAY_1026CLK
 * @param[in] u32EnableReset Enable WDT rest system function. Valid values are TRUE and FALSE
 * @param[in] u32EnableWakeup Enable WDT wake-up system function. Valid values are TRUE and FALSE
 * @return None
 */
void  WDT_Open(uint32_t u32TimeoutInterval,
               uint32_t u32ResetDelay,
               uint32_t u32EnableReset,
               uint32_t u32EnableWakeup)
{

    WDT->CTL = u32TimeoutInterval | u32ResetDelay | WDT_CTL_WTE_Msk |
               (u32EnableReset << WDT_CTL_WTRE_Pos) |
               (u32EnableWakeup << WDT_CTL_WTWKE_Pos);
    return;
}


/*@}*/ /* end of group NANO1X2_WDT_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NANO1X2_WDT_Driver */

/*@}*/ /* end of group NANO1X2_Device_Driver */

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
