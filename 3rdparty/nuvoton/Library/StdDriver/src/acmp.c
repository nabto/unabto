/**************************************************************************//**
 * @file     acmp.c
 * @version  V1.00
 * $Revision: 18 $
 * $Date: 14/12/17 7:16p $
 * @brief    Nano 102/112 series Analog Comparator(ACMP) driver source file
 *
 * @note
 * Copyright (C) 2013~2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include "Nano1X2Series.h"

#ifdef __cplusplus
extern "C"
{
#endif

/** @addtogroup NANO1X2_Device_Driver NANO102/112 Device Driver
  @{
*/

/** @addtogroup NANO1X2_ACMP_Driver ACMP Driver
  @{
*/


/** @addtogroup NANO1X2_ACMP_EXPORTED_FUNCTIONS ACMP Exported Functions
  @{
*/


/**
  * @brief  This function open and configure comparator parameters
  *
  * @param[in]  Acmp The base address of ACMP module
  * @param[in]  u32ChNum comparator number.
  * @param[in]  u32NegSrc is comparator0 negative input selection.  Including :
  *                  - \ref ACMP_CR_CN_PIN
  *                  - \ref ACMP_CR_CN_CRV
  *                  - \ref ACMP_CR_CN_VREFI
  *                  - \ref ACMP_CR_CN_AGND
  *             if select \ref ACMP_CR_CN_CRV can add comparator reference voltage setting.
  *             The formula is:
  *                       comparator reference voltage = AVDD x (1/6 +CRV/24)
  *             The range of CRV  is 0 ~ 15.
  *             Example : u32NegSrc = ( \ref ACMP_CR_CN_CRV | 10)
  *
  * @param[in]  u32HysteresisEn is charge or discharge pin selection. Including :
  *                  - \ref ACMP_CR_ACMP_HYSTERSIS_ENABLE
  *                  - \ref ACMP_CR_ACMP_HYSTERSIS_DISABLE
  * @return None
  */
void ACMP_Open(ACMP_T *Acmp, uint32_t u32ChNum, uint32_t u32NegSrc, uint32_t u32HysteresisEn)
{
    Acmp->CR[u32ChNum] = (Acmp->CR[u32ChNum] & ~ACMP_CR_CN_Msk) | (u32NegSrc>>24)<<ACMP_CR_CN_Pos;

    if((u32NegSrc&(0xFFUL<<24))==ACMP_CR_CN_CRV)
        Acmp->RVCR = (Acmp->RVCR & ~ACMP_RVCR_CRVS_Msk) | (u32NegSrc & 0x00FFFFFF) | ACMP_RVCR_CRV_EN_Msk;

    Acmp->CR[u32ChNum] = (Acmp->CR[u32ChNum] & ~ACMP_CR_ACMP_HYSEN_Msk) | u32HysteresisEn;
    Acmp->CR[u32ChNum] |= ACMP_CR_ACMPEN_Msk;
}

/**
  * @brief  This function close comparator
  * @param[in]  Acmp The base address of ACMP module
  * @param[in]  u32ChNum comparator number.
  */
void ACMP_Close(ACMP_T *Acmp, uint32_t u32ChNum)
{
    Acmp->CR[u32ChNum] &= ~ACMP_CR_ACMPEN_Msk;
}

/**
  * @brief  This function configure ACMP to sigma-delta mode.
  *
  * @param[in]  u32TimerNum is set to 0 to use timer0 and timer1, set to 1 to use timer0 and timer1. Including :
  *                  - \ref ACMP_TIMER01
  *                  - \ref ACMP_TIMER23
  * @param[in]  u32TriggerPolarity is set rising trigger or falling trigger. Including :
  *                  - \ref ACMP_MODCR0_TMR_TRI_LV_RISING
  *                  - \ref ACMP_MODCR0_TMR_TRI_LV_FALLING
  * @param[in]  u32PosPin is comparator0 positive input selection.  Including :
  *                  - \ref ACMP_CR_CPP0SEL_PA1
  *                  - \ref ACMP_CR_CPP0SEL_PA2
  *                  - \ref ACMP_CR_CPP0SEL_PA3
  *                  - \ref ACMP_CR_CPP0SEL_PA4
  * @param[in]  u32ChargePin is charge or discharge pin selection. Including :
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PA1
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PA2
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PA3
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PA4
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PA5
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PA6
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PA14
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PF5
  *
  * @return None
  */
void ACMP_SetSigmaDeltaConv(uint32_t u32TimerNum, uint32_t u32TriggerPolarity, uint32_t u32PosPin, uint32_t u32ChargePin)
{
    ACMP->CR[0] &= ~ACMP_CR_CPO0_SEL_Msk;
    ACMP->CR[0] &= ~ACMP_CR_ACMP0_INV_Msk;
    ACMP->CR[0] |=  ACMP_CR_ACOMP0_PN_AutoEx_Msk; //Auto Exchange Enabled
    ACMP->MODCR0 = (ACMP->MODCR0 & ~ACMP_MODCR0_TMR_SEL_Msk) | (u32TimerNum<<ACMP_MODCR0_TMR_SEL_Pos);
    ACMP->MODCR0 = (ACMP->MODCR0 & ~ACMP_MODCR0_MOD_SEL_Msk)| ACMP_MODCR0_MOD_SEL_SIGAMA_DELTA;
    ACMP->MODCR0 = (ACMP->MODCR0 & ~ACMP_MODCR0_TMR_TRI_LV_Msk) | u32TriggerPolarity;
    ACMP->MODCR0 = (ACMP->MODCR0 & ~ACMP_MODCR0_CH_DIS_PIN_SEL_Msk)  | u32ChargePin ;
}


/**
  * @brief  This function configure ACMP to slope mode.
  *
  * @param[in]  u32TimerNum is set to 0 to use timer0 and timer1, set to 1 to use timer0 and timer1. Including :
  *                  - \ref ACMP_TIMER01
  *                  - \ref ACMP_TIMER23
  * @param[in]  u32TriggerPolarity is set rising trigger or falling trigger. Including :
  *                  - \ref ACMP_MODCR0_TMR_TRI_LV_RISING
  *                  - \ref ACMP_MODCR0_TMR_TRI_LV_FALLING
  * @param[in]  u32PosPin is comparator0 positive input selection.  Including :
  *                  - \ref ACMP_CR_CPP0SEL_PA1
  *                  - \ref ACMP_CR_CPP0SEL_PA2
  *                  - \ref ACMP_CR_CPP0SEL_PA3
  *                  - \ref ACMP_CR_CPP0SEL_PA4
  * @param[in]  u32ChargePin is charge or discharge pin selection. Including :
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PA1
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PA2
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PA3
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PA4
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PA5
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PA6
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PA14
  *                  - \ref ACMP_MODCR0_CH_DIS_PINSEL_PF5
  *
  * @return None
  */
void ACMP_SetSlopeConv(uint32_t u32TimerNum, uint32_t u32TriggerPolarity, uint32_t u32PosPin, uint32_t u32ChargePin)
{
    ACMP->CR[0] =(ACMP->CR[0] & ~ACMP_CR_CPP0SEL_Msk) | u32PosPin;
    ACMP->CR[0] &= ~ACMP_CR_CPO0_SEL_Msk ; /* Comparator output is from internal path */
    ACMP->MODCR0 = (ACMP->MODCR0 & ~ACMP_MODCR0_MOD_SEL_Msk)| ACMP_MODCR0_MOD_SEL_SLOPE;
    ACMP->MODCR0 = (ACMP->MODCR0 & ~ACMP_MODCR0_TMR_TRI_LV_Msk) | u32TriggerPolarity;
    ACMP->MODCR0 = (ACMP->MODCR0 & ~ACMP_MODCR0_CH_DIS_PIN_SEL_Msk)  | u32ChargePin ; /* A Pin Can Only Do Discharging Work */
    ACMP->MODCR0 = (ACMP->MODCR0 & ~ACMP_MODCR0_TMR_SEL_Msk) | (u32TimerNum<<ACMP_MODCR0_TMR_SEL_Pos);
}


/*@}*/ /* end of group NANO1X2_ACMP_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NANO1X2_ACMP_Driver */

/*@}*/ /* end of group NANO1X2_Device_Driver */

#ifdef __cplusplus
}
#endif

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/

