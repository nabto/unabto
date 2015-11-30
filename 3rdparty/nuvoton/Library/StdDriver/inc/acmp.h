/**************************************************************************//**
 * @file     acmp.h
 * @version  V1.00
 * $Revision: 17 $
 * $Date: 14/11/28 11:12a $
 * @brief    Nano102/112 series Analog Comparator(ACMP) driver header file
 *
 * @note
 * Copyright (C) 2013~2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef __ACMP_H__
#define __ACMP_H__

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

/** @addtogroup NANO1X2_ACMP_EXPORTED_CONSTANTS ACMP Exported Constants
  @{
*/

#define ACMP_CR_CN_PIN    (0<<24)  ///< The comparator reference pin CPN0/1 is selected
#define ACMP_CR_CN_CRV    (1<<24)  ///< The internal comparator reference voltage (CRV) is selected
#define ACMP_CR_CN_VREFI  (2<<24)  ///< The internal reference voltage (VREFI) is selected
#define ACMP_CR_CN_AGND   (3<<24)  ///< The AGND is selected */

#define ACMP_CR_ACMP_HYSTERSIS_ENABLE    ACMP_CR_ACMP_HYSEN_Msk  ///< ACMP hysteresis enable
#define ACMP_CR_ACMP_HYSTERSIS_DISABLE   0                           ///< ACMP hysteresis disable

#define ACMP_CR_CPP0SEL_PA1 (3UL<<ACMP_CR_CPP0SEL_Pos)  ///< The comparator positive input select PA1
#define ACMP_CR_CPP0SEL_PA2 (2UL<<ACMP_CR_CPP0SEL_Pos)  ///< The comparator positive input select PA2
#define ACMP_CR_CPP0SEL_PA3 (1UL<<ACMP_CR_CPP0SEL_Pos)  ///< The comparator positive input select PA3
#define ACMP_CR_CPP0SEL_PA4 (0UL<<ACMP_CR_CPP0SEL_Pos)  ///< The comparator positive input select PA4


#define ACMP_MODCR0_TMR_TRI_LV_RISING  (0UL<<ACMP_MODCR0_TMR_TRI_LV_Pos) ///< The comparator output low to high to enable timer
#define ACMP_MODCR0_TMR_TRI_LV_FALLING (1UL<<ACMP_MODCR0_TMR_TRI_LV_Pos) ///< The comparator output high to low to enable timer

#define ACMP_MODCR0_CH_DIS_PINSEL_PA1  (0UL<<ACMP_MODCR0_CH_DIS_PIN_SEL_Pos) ///< The charge/discharge pin select PA1
#define ACMP_MODCR0_CH_DIS_PINSEL_PA2  (1UL<<ACMP_MODCR0_CH_DIS_PIN_SEL_Pos) ///< The charge/discharge pin select PA2
#define ACMP_MODCR0_CH_DIS_PINSEL_PA3  (2UL<<ACMP_MODCR0_CH_DIS_PIN_SEL_Pos) ///< The charge/discharge pin select PA3
#define ACMP_MODCR0_CH_DIS_PINSEL_PA4  (3UL<<ACMP_MODCR0_CH_DIS_PIN_SEL_Pos) ///< The charge/discharge pin select PA4
#define ACMP_MODCR0_CH_DIS_PINSEL_PA5  (4UL<<ACMP_MODCR0_CH_DIS_PIN_SEL_Pos) ///< The charge/discharge pin select PA5
#define ACMP_MODCR0_CH_DIS_PINSEL_PA6  (5UL<<ACMP_MODCR0_CH_DIS_PIN_SEL_Pos) ///< The charge/discharge pin select PA6
#define ACMP_MODCR0_CH_DIS_PINSEL_PA14 (6UL<<ACMP_MODCR0_CH_DIS_PIN_SEL_Pos) ///< The charge/discharge pin select PA14
#define ACMP_MODCR0_CH_DIS_PINSEL_PF5  (7UL<<ACMP_MODCR0_CH_DIS_PIN_SEL_Pos) ///< The charge/discharge pin select PF5


#define ACMP_MODCR0_MOD_SEL_NORMAL       (0UL<<ACMP_MODCR0_MOD_SEL_Pos) ///< The comparator mode select normal mode
#define ACMP_MODCR0_MOD_SEL_SIGAMA_DELTA (1UL<<ACMP_MODCR0_MOD_SEL_Pos) ///< The comparator mode select sigma-delta mode
#define ACMP_MODCR0_MOD_SEL_SLOPE        (2UL<<ACMP_MODCR0_MOD_SEL_Pos) ///< The comparator mode select slope mode

#define ACMP_TIMER01 0  ///< ACMP use timer0 and timer1
#define ACMP_TIMER23 1  ///< ACMP use timer2 and timer3

/*@}*/ /* end of group NANO1X2_ACMP_EXPORTED_CONSTANTS */


/** @addtogroup NANO1X2_ACMP_EXPORTED_FUNCTIONS ACMP Exported Functions
  @{
*/

/**
  * @brief This macro is used to enable output inverse
  * @param[in] acmp The base address of ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * \hideinitializer
  */
#define ACMP_ENABLE_OUTPUT_INVERSE(acmp,u32ChNum) (acmp->CMPCR[u32ChNum] |= ACMP_CR_ACMP0_INV_Msk)

/**
  * @brief This macro is used to disable output inverse
  * @param[in] acmp The base address of ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * \hideinitializer
  */
#define ACMP_DISABLE_OUTPUT_INVERSE(acmp,u32ChNum) (acmp->CR[u32ChNum] &= ~ACMP_CR_ACMP0_INV_Msk)

/**
  * @brief This macro is used to enable output inverse
  * @param[in] acmp The base address of ACMP module
  * @param[in] u32ChNum The ACMP number
  * @param[in] u32Src is comparator0 negative input selection.  Including :
  *                  - \ref ACMP_CR_CN_PIN
  *                  - \ref ACMP_CR_CN_CRV
  *                  - \ref ACMP_CR_CN_VREFI
  *                  - \ref ACMP_CR_CN_AGND
  * @return None
  * \hideinitializer
  */
#define ACMP_SET_NEG_SRC(acmp,u32ChNum,u32Src) (acmp->CR[u32ChNum] = (acmp->CR[u32ChNum] & ~ACMP_CR_CN_Msk) | u32Src)

/**
  * @brief This macro is used to enable hysteresis
  * @param[in] acmp The base address of ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * \hideinitializer
  */
#define ACMP_ENABLE_HYSTERESIS(acmp,u32ChNum) (acmp->CMPCR[u32ChNum] |= ACMP_CR_ACMP_HYSEN_Msk)

/**
  * @brief This macro is used to disable hysteresis
  * @param[in] acmp The base address of ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * \hideinitializer
  */
#define ACMP_DISABLE_HYSTERESIS(acmp,u32ChNum) (acmp->CMPCR[u32ChNum] &= ~ACMP_CR_ACMP_HYSEN_Msk)

/**
  * @brief This macro is used to enable interrupt
  * @param[in] acmp The base address of ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * \hideinitializer
  */
#define ACMP_ENABLE_INT(acmp,u32ChNum) (acmp->CMPCR[u32ChNum] |= ACMP_CR_ACMPIE_Msk)

/**
  * @brief This macro is used to disable interrupt
  * @param[in] acmp The base address of ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * \hideinitializer
  */
#define ACMP_DISABLE_INT(acmp,u32ChNum) (acmp->CMPCR[u32ChNum] &= ~ACMP_CR_ACMPIE_Msk)


/**
  * @brief This macro is used to enable ACMP
  * @param[in] acmp The base address of ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * \hideinitializer
  */
#define ACMP_ENABLE(acmp,u32ChNum) (acmp->CR[u32ChNum] |= ACMP_CR_ACMPEN_Msk)

/**
  * @brief This macro is used to disable ACMP
  * @param[in] acmp The base address of ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return None
  * \hideinitializer
  */
#define ACMP_DISABLE(acmp,u32ChNum) (acmp->CR[u32ChNum] &= ~ACMP_CR_ACMPEN_Msk)

/**
  * @brief This macro is used to get ACMP output value
  * @param[in] acmp The base address of ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return  1 or 0
  * \hideinitializer
  */
#define ACMP_GET_OUTPUT(acmp,u32ChNum) ((acmp->SR & ACMP_SR_CO0_Msk<<u32ChNum)?1:0)

/**
  * @brief This macro is used to get ACMP interrupt flag
  * @param[in] acmp The base address of ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return   ACMP interrupt occurred or not
  * \hideinitializer
  */
#define ACMP_GET_INT_FLAG(acmp,u32ChNum) ((acmp->SR & ACMP_SR_ACMPF0_Msk<<u32ChNum)?1:0)

/**
  * @brief This macro is used to clear ACMP interrupt flag
  * @param[in] acmp The base address of ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return   None
  * \hideinitializer
  */
#define ACMP_CLR_INT_FLAG(acmp,u32ChNum) (acmp->SR |= (ACMP_SR_ACMPF0_Msk<<u32ChNum))

/**
  * @brief This macro is used to enable ACMP wake-up
  * @param[in] acmp The base address of ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return   None
  * \hideinitializer
  */
#define ACMP_ENABLE_WAKEUP(acmp,u32ChNum) (acmp->CR[u32ChNum] |= ACMP_CR_ACMP_WKEUP_EN_Msk)

/**
  * @brief This macro is used to disable ACMP wake-up
  * @param[in] acmp The base address of ACMP module
  * @param[in] u32ChNum The ACMP number
  * @return   None
  * \hideinitializer
  */
#define ACMP_DISABLE_WAKEUP(acmp,u32ChNum) (acmp->CR[u32ChNum] &= ~ACMP_CR_ACMP_WKEUP_EN_Msk)

/**
  * @brief This macro is used to enable ACMP wake-up
  * @param[in] u32Level  comparator reference voltage setting.
  *             The formula is:
  *                       comparator reference voltage = AVDD x (1/6 +u32Level/24)
  *             The range of u32Level  is 0 ~ 15.
  * @return   None
  * \hideinitializer
  */
#define ACMP_CRV_SEL(u32Level) (acmp->RVCR = (ACMP->RVCR & ~ACMP_RVCR_CRVS_Msk)| u32Level)

/**
  * @brief This macro is used to enable CRV(comparator reference voltage)
  * @param[in] acmp The base address of ACMP module
  * @return   None
  * \hideinitializer
  */
#define ACMP_ENABLE_CRV(acmp) (acmp->RVCR |= ACMP_RVCR_CRV_EN_Msk)

/**
  * @brief This macro is used to disable CRV(comparator reference voltage)
  * @param[in] acmp The base address of ACMP module
  * @return   None
  * \hideinitializer
  */
#define ACMP_DISABLE_CRV(acmp) (acmp->RVCR &= ~ACMP_RVCR_CRV_EN_Msk)

/**
  * @brief This macro is used to start ACMP on sigma-delta mode or slope mode
  * @param[in] acmp The base address of ACMP module
  * @return   None
  * \hideinitializer
  */
#define ACMP_START_CONV(acmp) \
do{ \
    acmp->MODCR0 &= ~ACMP_MODCR0_START_Msk; \
    acmp->MODCR0 |=  ACMP_MODCR0_START_Msk; \
}while(0);

void ACMP_Open(ACMP_T *, uint32_t u32ChNum, uint32_t u32NegSrc, uint32_t u32HysteresisEn);
void ACMP_Close(ACMP_T *, uint32_t u32ChNum);
void ACMP_SetSigmaDeltaConv(uint32_t u32TimerNum, uint32_t u32TriggerPolarity, uint32_t u32PosPin, uint32_t u32ChargePin);
void ACMP_SetSlopeConv(uint32_t u32TimerNum, uint32_t u32TriggerPolarity, uint32_t u32PosPin, uint32_t u32ChargePin);

/*@}*/ /* end of group NANO1X2_ACMP_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NANO1X2_ACMP_Driver */

/*@}*/ /* end of group NANO1X2_Device_Driver */

#ifdef __cplusplus
}
#endif

#endif //__ACMP_H__

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
