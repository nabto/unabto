/**************************************************************************//**
 * @file     clk.c
 * @version  V1.00
 * $Revision: 34 $
 * $Date: 15/01/28 10:55a $
 * @brief    Nano 102/112 series CLK driver source file
 *
 * @note
 * Copyright (C) 2013~2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include "Nano1X2Series.h"
/** @addtogroup NANO1X2_Device_Driver NANO102/112 Device Driver
  @{
*/

/** @addtogroup NANO1X2_CLK_Driver CLK Driver
  @{
*/


/** @addtogroup NANO1X2_CLK_EXPORTED_FUNCTIONS CLK Exported Functions
  @{
*/

/**
  * @brief  This function disable frequency output function.
  * @param  None
  * @return None
  */
void CLK_DisableCKO(void)
{
    CLK_DisableCKO0();
}
/**
  * @brief  This function disable frequency output function.
  * @return None
  */
void CLK_DisableCKO0(void)
{
    /* Disable CKO0 clock source */
    CLK->APBCLK &= (~CLK_APBCLK_FDIV0_EN_Msk);
}

/**
  * @brief  This function disable frequency output function(1).
  * @return None
  */
void CLK_DisableCKO1(void)
{
    /* Disable CKO clock source */
    CLK->APBCLK &= (~CLK_APBCLK_FDIV1_EN_Msk);
}

/**
  * @brief  This function enable frequency divider module clock,
  *         enable frequency divider clock function and configure frequency divider.
  * @param[in]  u32ClkSrc is frequency divider function clock source
  *         - \ref CLK_CLKSEL2_FRQDIV_S_HXT
  *         - \ref CLK_CLKSEL2_FRQDIV_S_LXT
  *         - \ref CLK_CLKSEL2_FRQDIV_S_HCLK
  *         - \ref CLK_CLKSEL2_FRQDIV_S_HIRC
  * @param[in]  u32ClkDiv is system reset source
  * @param[in]  u32ClkDivBy1En is frequency divided by one enable.
  * @return None
  *
  * @details    Output selected clock to CKO. The output clock frequency is divided by u32ClkDiv.
  *             The formula is:
  *                 CKO frequency = (Clock source frequency) / 2^(u32ClkDiv + 1)
  *             This function is just used to set CKO clock.
  *             User must enable I/O for CKO clock output pin by themselves.
  */
void CLK_EnableCKO(uint32_t u32ClkSrc, uint32_t u32ClkDiv, uint32_t u32ClkDivBy1En)
{
    CLK_EnableCKO0(u32ClkSrc, u32ClkDiv, u32ClkDivBy1En);
}
/**
  * @brief  This function enable frequency divider module clock,
  *         enable frequency divider clock function and configure frequency divider.
  * @param  u32ClkSrc is frequency divider function clock source
  *         - \ref CLK_CLKSEL2_FRQDIV0_S_HXT
  *         - \ref CLK_CLKSEL2_FRQDIV0_S_LXT
  *         - \ref CLK_CLKSEL2_FRQDIV0_S_HCLK
  *         - \ref CLK_CLKSEL2_FRQDIV0_S_HIRC
  * @param  u32ClkDiv is system reset source
  * @param  u32ClkDivBy1En is frequency divided by one enable.
  * @return None
  *
  * @details    Output selected clock to CKO. The output clock frequency is divided by u32ClkDiv.
  *             The formula is:
  *                 CKO frequency = (Clock source frequency) / 2^(u32ClkDiv + 1)
  *             This function is just used to set CKO clock.
  *             User must enable I/O for CKO clock output pin by themselves.
  */
void CLK_EnableCKO0(uint32_t u32ClkSrc, uint32_t u32ClkDiv, uint32_t u32ClkDivBy1En)
{
    /* Select CKO clock source */
    CLK->CLKSEL2 = (CLK->CLKSEL2 & (~CLK_CLKSEL2_FRQDIV0_S_Msk)) | u32ClkSrc;

    /* CKO = clock source / 2^(u32ClkDiv + 1) */
    CLK->FRQDIV0 = CLK_FRQDIV0_FDIV_EN_Msk | u32ClkDiv | u32ClkDivBy1En<<CLK_FRQDIV0_DIV1_Pos;

    /* Enable CKO clock source */
    CLK->APBCLK |= CLK_APBCLK_FDIV0_EN_Msk;
}

/**
  * @brief  This function enable frequency divider module clock,
  *         enable frequency divider clock function and configure frequency divider. (1)
  * @param[in]  u32ClkSrc is frequency divider function clock source
  *         - \ref CLK_CLKSEL2_FRQDIV1_S_HXT
  *         - \ref CLK_CLKSEL2_FRQDIV1_S_LXT
  *         - \ref CLK_CLKSEL2_FRQDIV1_S_HCLK
  *         - \ref CLK_CLKSEL2_FRQDIV1_S_HIRC
  * @param[in]  u32ClkDiv is system reset source
  * @param[in]  u32ClkDivBy1En is frequency divided by one enable.
  * @return None
  *
  * @details    Output selected clock to CKO. The output clock frequency is divided by u32ClkDiv.
  *             The formula is:
  *                 CKO frequency = (Clock source frequency) / 2^(u32ClkDiv + 1)
  *             This function is just used to set CKO clock.
  *             User must enable I/O for CKO clock output pin by themselves.
  */
void CLK_EnableCKO1(uint32_t u32ClkSrc, uint32_t u32ClkDiv, uint32_t u32ClkDivBy1En)
{
    /* Select CKO clock source */
    CLK->CLKSEL2 = (CLK->CLKSEL2 & (~CLK_CLKSEL2_FRQDIV1_S_Msk)) | u32ClkSrc;

    /* CKO = clock source / 2^(u32ClkDiv + 1) */
    CLK->FRQDIV1 = CLK_FRQDIV1_FDIV_EN_Msk | u32ClkDiv | u32ClkDivBy1En<<CLK_FRQDIV1_DIV1_Pos;

    /* Enable CKO clock source */
    CLK->APBCLK |= CLK_APBCLK_FDIV1_EN_Msk;
}

/**
  * @brief  This function let system enter to Power-down mode.
  * @param  None
  * @return None
  */
void CLK_PowerDown(void)
{
    SCB->SCR = SCB_SCR_SLEEPDEEP_Msk;
    CLK->PWRCTL |= (CLK_PWRCTL_PD_EN_Msk | CLK_PWRCTL_WK_DLY_Msk );
    __WFI();
}

/**
  * @brief  This function let system enter to Idle mode
  * @param  None
  * @return None
  */
void CLK_Idle(void)
{
    CLK->PWRCTL |= (CLK_PWRCTL_PD_EN_Msk );
    __WFI();
}

/**
  * @brief  This function get external high frequency crystal frequency. The frequency unit is Hz.
  * @param  None
  * @return None
  */
uint32_t CLK_GetHXTFreq(void)
{
    if(CLK->PWRCTL & CLK_PWRCTL_HXT_EN )
        return __HXT;
    else
        return 0;
}

/**
  * @brief  This function get external low frequency crystal frequency. The frequency unit is Hz.
  * @param  None
  * @return LXT frequency
  */
uint32_t CLK_GetLXTFreq(void)
{
    if(CLK->PWRCTL & CLK_PWRCTL_LXT_EN )
        return __LXT;
    else
        return 0;
}

/**
  * @brief  This function get HCLK frequency. The frequency unit is Hz.
  * @param  None
  * @return HCLK frequency
  */
uint32_t CLK_GetHCLKFreq(void)
{
    SystemCoreClockUpdate();
    return SystemCoreClock;
}

/**
  * @brief  This function get PCLK frequency. The frequency unit is Hz.
  * @param  None
  * @return PCLK frequency
  */
uint32_t CLK_GetPCLKFreq(void)
{
    uint32_t Div[]= {1,2,4,8,16,1,1,1};
    uint32_t PCLK_Div;
    PCLK_Div = CLK->APB_DIV & CLK_APB_DIV_APBDIV_Msk;
    SystemCoreClockUpdate();
    return SystemCoreClock/Div[PCLK_Div];
}

/**
  * @brief  This function get CPU frequency. The frequency unit is Hz.
  * @param  None
  * @return CPU frequency
  */
uint32_t CLK_GetCPUFreq(void)
{
    SystemCoreClockUpdate();
    return SystemCoreClock;
}

/**
  * @brief  This function get PLL frequency. The frequency unit is Hz.
  * @param  None
  * @return PLL frequency
  */
uint32_t CLK_GetPLLClockFreq(void)
{
    uint32_t u32Freq =0, u32PLLSrc;
    uint32_t u32SRC_N,u32PLL_M,u32PllReg;

    u32PllReg = CLK->PLLCTL;

    if (u32PllReg & CLK_PLLCTL_PD)
        return 0;    /* PLL is in power down mode */

    if (u32PllReg & CLK_PLLCTL_PLL_SRC_HIRC) {
        if(CLK->PLLCTL & CLK_PWRCTL_HIRC_FSEL_Msk)
            u32PLLSrc =__HIRC16M;
        else
            u32PLLSrc =__HIRC12M;
    } else
        u32PLLSrc = __HXT;

    u32SRC_N = (u32PllReg & CLK_PLLCTL_PLL_SRC_N_Msk) >> CLK_PLLCTL_PLL_SRC_N_Pos;
    u32PLL_M = (u32PllReg & CLK_PLLCTL_PLL_MLP_Msk) >> CLK_PLLCTL_PLL_MLP_Pos;

    u32Freq = u32PLLSrc * u32PLL_M / (u32SRC_N+1);

    return u32Freq;
}

/**
  * @brief  This function set HCLK frequency. The frequency unit is Hz. The range of u32Hclk is 16 ~ 32 MHz
  * @param[in]  u32Hclk is HCLK frequency
  * @return None
  */
uint32_t CLK_SetCoreClock(uint32_t u32Hclk)
{
    if(CLK->PWRCTL & CLK_PWRCTL_HXT_EN) {
        CLK_EnablePLL( CLK_PLLCTL_PLL_SRC_HXT,u32Hclk);
    } else {
        CLK_EnablePLL( CLK_PLLCTL_PLL_SRC_HIRC,u32Hclk);
    }
    CLK_WaitClockReady(CLK_CLKSTATUS_PLL_STB_Msk);
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_PLL,CLK_HCLK_CLK_DIVIDER(1));
    return SystemCoreClock;
}

/**
  * @brief  This function set HCLK clock source and HCLK clock divider
  * @param[in]  u32ClkSrc is HCLK clock source. Including :
  *         - \ref CLK_CLKSEL0_HCLK_S_HXT
  *         - \ref CLK_CLKSEL0_HCLK_S_LXT
  *         - \ref CLK_CLKSEL0_HCLK_S_PLL
  *         - \ref CLK_CLKSEL0_HCLK_S_LIRC
  *         - \ref CLK_CLKSEL0_HCLK_S_HIRC
  * @param[in]  u32ClkDiv is HCLK clock divider. Including :
  *         - \ref CLK_HCLK_CLK_DIVIDER(x)
  * @return None
  */
void CLK_SetHCLK(uint32_t u32ClkSrc, uint32_t u32ClkDiv)
{
    CLK->CLKDIV0 = (CLK->CLKDIV0 & ~CLK_CLKDIV0_HCLK_N_Msk) | u32ClkDiv;
    CLK->CLKSEL0 = (CLK->CLKSEL0 & ~CLK_CLKSEL0_HCLK_S_Msk) | u32ClkSrc;
    SystemCoreClockUpdate();
}

/**
  * @brief  This function set selected module clock source and module clock divider
  * @param[in]  u32ModuleIdx is module index.
  * @param[in]  u32ClkSrc is module clock source.
  * @param[in]  u32ClkDiv is module clock divider.
  * @return None
  * @details Valid parameter combinations listed in following table:
  *
  * |Module index          |Clock source                       |Divider                      |
  * | :------------------- | :-------------------------------  | :-------------------------  |
  * |\ref SC1_MODULE       |\ref  CLK_CLKSEL2_SC_S_HXT         |\ref CLK_SC1_CLK_DIVIDER(x)  |
  * |\ref SC1_MODULE       |\ref CLK_CLKSEL2_SC_S_PLL          |\ref CLK_SC1_CLK_DIVIDER(x)  |
  * |\ref SC1_MODULE       |\ref CLK_CLKSEL2_SC_S_HIRC         |\ref CLK_SC1_CLK_DIVIDER(x)  |
  * |\ref SC1_MODULE       |\ref CLK_CLKSEL2_SC_S_HCLK         |\ref CLK_SC1_CLK_DIVIDER(x)  |
  * |\ref SC0_MODULE       |\ref CLK_CLKSEL2_SC_S_HXT          |\ref CLK_SC0_CLK_DIVIDER(x)  |
  * |\ref SC0_MODULE       |\ref CLK_CLKSEL2_SC_S_PLL          |\ref CLK_SC0_CLK_DIVIDER(x)  |
  * |\ref SC0_MODULE       |\ref CLK_CLKSEL2_SC_S_HIRC         |\ref CLK_SC0_CLK_DIVIDER(x)  |
  * |\ref SC0_MODULE       |\ref CLK_CLKSEL2_SC_S_HCLK         |\ref CLK_SC0_CLK_DIVIDER(x)  |
  * |\ref ADC_MODULE       |\ref CLK_CLKSEL1_ADC_S_HXT         |\ref CLK_ADC_CLK_DIVIDER(x)  |
  * |\ref ADC_MODULE       |\ref CLK_CLKSEL1_ADC_S_LXT         |\ref CLK_ADC_CLK_DIVIDER(x)  |
  * |\ref ADC_MODULE       |\ref CLK_CLKSEL1_ADC_S_PLL         |\ref CLK_ADC_CLK_DIVIDER(x)  |
  * |\ref ADC_MODULE       |\ref CLK_CLKSEL1_ADC_S_HIRC        |\ref CLK_ADC_CLK_DIVIDER(x)  |
  * |\ref ADC_MODULE       |\ref CLK_CLKSEL1_ADC_S_HCLK        |\ref CLK_ADC_CLK_DIVIDER(x)  |
  * |\ref LCD_MODULE       |\ref CLK_CLKSEL1_LCD_S_LXT         | x                           |
  * |\ref PWM0_CH23_MODULE |\ref CLK_CLKSEL1_PWM0_CH23_S_HXT   | x                           |
  * |\ref PWM0_CH23_MODULE |\ref CLK_CLKSEL1_PWM0_CH23_S_LXT   | x                           |
  * |\ref PWM0_CH23_MODULE |\ref CLK_CLKSEL1_PWM0_CH23_S_HCLK  | x                           |
  * |\ref PWM0_CH23_MODULE |\ref CLK_CLKSEL1_PWM0_CH23_S_HIRC  | x                           |
  * |\ref PWM0_CH01_MODULE |\ref CLK_CLKSEL1_PWM0_CH01_S_HXT   | x                           |
  * |\ref PWM0_CH01_MODULE |\ref CLK_CLKSEL1_PWM0_CH01_S_LXT   | x                           |
  * |\ref PWM0_CH01_MODULE |\ref CLK_CLKSEL1_PWM0_CH01_S_HCLK  | x                           |
  * |\ref PWM0_CH01_MODULE |\ref CLK_CLKSEL1_PWM0_CH01_S_HIRC  | x                           |
  * |\ref UART1_MODULE     |\ref CLK_CLKSEL1_UART_S_HXT        |\ref CLK_UART_CLK_DIVIDER(x) |
  * |\ref UART1_MODULE     |\ref CLK_CLKSEL1_UART_S_LXT        |\ref CLK_UART_CLK_DIVIDER(x) |
  * |\ref UART1_MODULE     |\ref CLK_CLKSEL1_UART_S_PLL        |\ref CLK_UART_CLK_DIVIDER(x) |
  * |\ref UART1_MODULE     |\ref CLK_CLKSEL1_UART_S_HIRC       |\ref CLK_UART_CLK_DIVIDER(x) |
  * |\ref UART0_MODULE     |\ref CLK_CLKSEL1_UART_S_HXT        |\ref CLK_UART_CLK_DIVIDER(x) |
  * |\ref UART0_MODULE     |\ref CLK_CLKSEL1_UART_S_LXT        |\ref CLK_UART_CLK_DIVIDER(x) |
  * |\ref UART0_MODULE     |\ref CLK_CLKSEL1_UART_S_PLL        |\ref CLK_UART_CLK_DIVIDER(x) |
  * |\ref UART0_MODULE     |\ref CLK_CLKSEL1_UART_S_HIRC       |\ref CLK_UART_CLK_DIVIDER(x) |
  * |\ref SPI1_MODULE      |\ref CLK_CLKSEL2_SPI1_S_PLL        | x                           |
  * |\ref SPI1_MODULE      |\ref CLK_CLKSEL2_SPI1_S_HCLK       | x                           |
  * |\ref SPI0_MODULE      |\ref CLK_CLKSEL2_SPI0_S_PLL        | x                           |
  * |\ref SPI0_MODULE      |\ref CLK_CLKSEL2_SPI0_S_HCLK       | x                           |
  * |\ref ACMP_MODULE      | x                                 | x                           |
  * |\ref I2C1_MODULE      | x                                 | x                           |
  * |\ref I2C0_MODULE      | x                                 | x                           |
  * |\ref FDIV1_MODULE     |\ref CLK_CLKSEL2_FRQDIV1_S_HXT     | x                          |
  * |\ref FDIV1_MODULE     |\ref CLK_CLKSEL2_FRQDIV1_S_LXT     | x                           |
  * |\ref FDIV1_MODULE     |\ref CLK_CLKSEL2_FRQDIV1_S_HCLK    | x                           |
  * |\ref FDIV1_MODULE     |\ref CLK_CLKSEL2_FRQDIV1_S_HIRC    | x                           |
  * |\ref FDIV0_MODULE     |\ref CLK_CLKSEL2_FRQDIV0_S_HXT     | x                           |
  * |\ref FDIV0_MODULE     |\ref CLK_CLKSEL2_FRQDIV0_S_LXT     | x                           |
  * |\ref FDIV0_MODULE     |\ref CLK_CLKSEL2_FRQDIV0_S_HCLK    | x                           |
  * |\ref FDIV0_MODULE     |\ref CLK_CLKSEL2_FRQDIV0_S_HIRC    | x                           |
  * |\ref FDIV_MODULE      |\ref CLK_CLKSEL2_FRQDIV_S_HXT      | x                           |
  * |\ref FDIV_MODULE      |\ref CLK_CLKSEL2_FRQDIV_S_LXT      | x                           |
  * |\ref FDIV_MODULE      |\ref CLK_CLKSEL2_FRQDIV_S_HCLK     | x                           |
  * |\ref FDIV_MODULE      |\ref CLK_CLKSEL2_FRQDIV_S_HIRC     | x                           |
  * |\ref TMR3_MODULE      |\ref CLK_CLKSEL2_TMR3_S_HXT        |\ref CLK_TMR3_CLK_DIVIDER(x) |
  * |\ref TMR3_MODULE      |\ref CLK_CLKSEL2_TMR3_S_LXT        |\ref CLK_TMR3_CLK_DIVIDER(x) |
  * |\ref TMR3_MODULE      |\ref CLK_CLKSEL2_TMR3_S_LIRC       |\ref CLK_TMR3_CLK_DIVIDER(x) |
  * |\ref TMR3_MODULE      |\ref CLK_CLKSEL2_TMR3_S_EXT        |\ref CLK_TMR3_CLK_DIVIDER(x) |
  * |\ref TMR3_MODULE      |\ref CLK_CLKSEL2_TMR3_S_HIRC       |\ref CLK_TMR3_CLK_DIVIDER(x) |
  * |\ref TMR3_MODULE      |\ref CLK_CLKSEL2_TMR3_S_HCLK       |\ref CLK_TMR3_CLK_DIVIDER(x) |
  * |\ref TMR2_MODULE      |\ref CLK_CLKSEL2_TMR2_S_HXT        |\ref CLK_TMR2_CLK_DIVIDER(x) |
  * |\ref TMR2_MODULE      |\ref CLK_CLKSEL2_TMR2_S_LXT        |\ref CLK_TMR2_CLK_DIVIDER(x) |
  * |\ref TMR2_MODULE      |\ref CLK_CLKSEL2_TMR2_S_LIRC       |\ref CLK_TMR2_CLK_DIVIDER(x) |
  * |\ref TMR2_MODULE      |\ref CLK_CLKSEL2_TMR2_S_EXT        |\ref CLK_TMR2_CLK_DIVIDER(x) |
  * |\ref TMR2_MODULE      |\ref CLK_CLKSEL2_TMR2_S_HIRC       |\ref CLK_TMR2_CLK_DIVIDER(x) |
  * |\ref TMR2_MODULE      |\ref CLK_CLKSEL2_TMR2_S_HCLK       |\ref CLK_TMR2_CLK_DIVIDER(x) |
  * |\ref TMR1_MODULE      |\ref CLK_CLKSEL1_TMR1_S_HXT        |\ref CLK_TMR1_CLK_DIVIDER(x) |
  * |\ref TMR1_MODULE      |\ref CLK_CLKSEL1_TMR1_S_LXT        |\ref CLK_TMR1_CLK_DIVIDER(x) |
  * |\ref TMR1_MODULE      |\ref CLK_CLKSEL1_TMR1_S_LIRC       |\ref CLK_TMR1_CLK_DIVIDER(x) |
  * |\ref TMR1_MODULE      |\ref CLK_CLKSEL1_TMR1_S_EXT        |\ref CLK_TMR1_CLK_DIVIDER(x) |
  * |\ref TMR1_MODULE      |\ref CLK_CLKSEL1_TMR1_S_HIRC       |\ref CLK_TMR1_CLK_DIVIDER(x) |
  * |\ref TMR1_MODULE      |\ref CLK_CLKSEL1_TMR1_S_HCLK       |\ref CLK_TMR1_CLK_DIVIDER(x) |
  * |\ref TMR0_MODULE      |\ref CLK_CLKSEL1_TMR0_S_HXT        |\ref CLK_TMR0_CLK_DIVIDER(x) |
  * |\ref TMR0_MODULE      |\ref CLK_CLKSEL1_TMR0_S_LXT        |\ref CLK_TMR0_CLK_DIVIDER(x) |
  * |\ref TMR0_MODULE      |\ref CLK_CLKSEL1_TMR0_S_LIRC       |\ref CLK_TMR0_CLK_DIVIDER(x) |
  * |\ref TMR0_MODULE      |\ref CLK_CLKSEL1_TMR0_S_EXT        |\ref CLK_TMR0_CLK_DIVIDER(x) |
  * |\ref TMR0_MODULE      |\ref CLK_CLKSEL1_TMR0_S_HIRC       |\ref CLK_TMR0_CLK_DIVIDER(x) |
  * |\ref TMR0_MODULE      |\ref CLK_CLKSEL1_TMR0_S_HCLK       |\ref CLK_TMR0_CLK_DIVIDER(x) |
  * |\ref RTC_MODULE       | x                                 | x                           |
  * |\ref WDT_MODULE       | x                                 | x                           |
  *                                                                                          |
  */

void CLK_SetModuleClock(uint32_t u32ModuleIdx, uint32_t u32ClkSrc, uint32_t u32ClkDiv)
{
    uint32_t u32tmp=0,u32sel=0,u32div=0;

    if(MODULE_CLKDIV_Msk(u32ModuleIdx)!=MODULE_NoMsk) {
        u32div =(uint32_t)&CLK->CLKDIV0+((MODULE_CLKDIV(u32ModuleIdx))*4);
        u32tmp = *(volatile uint32_t *)(u32div);
        u32tmp = ( u32tmp & ~(MODULE_CLKDIV_Msk(u32ModuleIdx)<<MODULE_CLKDIV_Pos(u32ModuleIdx)) ) | u32ClkDiv;
        *(volatile uint32_t *)(u32div) = u32tmp;
    }

    if(MODULE_CLKSEL_Msk(u32ModuleIdx)!=MODULE_NoMsk) {
        u32sel = (uint32_t)&CLK->CLKSEL0+((MODULE_CLKSEL(u32ModuleIdx))*4);
        u32tmp = *(volatile uint32_t *)(u32sel);
        u32tmp = ( u32tmp & ~(MODULE_CLKSEL_Msk(u32ModuleIdx)<<MODULE_CLKSEL_Pos(u32ModuleIdx)) ) | u32ClkSrc;
        *(volatile uint32_t *)(u32sel) = u32tmp;
    }
}

/**
  * @brief  This function enable clock source
  * @param[in]  u32ClkMask is clock source mask. Including:
  *         - \ref CLK_PWRCTL_HXT_EN_Msk
  *         - \ref CLK_PWRCTL_LXT_EN_Msk
  *         - \ref CLK_PWRCTL_HIRC_EN_Msk
  *         - \ref CLK_PWRCTL_LIRC_EN_Msk
  * @return None
  */
void CLK_EnableXtalRC(uint32_t u32ClkMask)
{
    CLK->PWRCTL |= u32ClkMask;
}

/**
  * @brief  This function disable clock source
  * @param  u32ClkMask is clock source mask. Including:
  *         - \ref CLK_PWRCTL_HXT_EN_Msk
  *         - \ref CLK_PWRCTL_LXT_EN_Msk
  *         - \ref CLK_PWRCTL_HIRC_EN_Msk
  *         - \ref CLK_PWRCTL_LIRC_EN_Msk
  * @return None
  */
void CLK_DisableXtalRC(uint32_t u32ClkMask)
{
    CLK->PWRCTL &= ~u32ClkMask;
}

/**
  * @brief  This function enable module clock
  * @param[in]  u32ModuleIdx is module index. Including :
  *         - \ref GPIO_MODULE
  *         - \ref DMA_MODULE
  *         - \ref ISP_MODULE
  *         - \ref EBI_MODULE
  *         - \ref SRAM_MODULE
  *         - \ref TICK_MODULE
  *         - \ref SC1_MODULE
  *         - \ref SC0_MODULE
  *         - \ref ADC_MODULE
  *         - \ref LCD_MODULE
  *         - \ref PWM0_CH23_MODULE
  *         - \ref PWM0_CH01_MODULE
  *         - \ref UART1_MODULE
  *         - \ref UART0_MODULE
  *         - \ref SPI1_MODULE
  *         - \ref SPI0_MODULE
  *         - \ref ACMP_MODULE
  *         - \ref I2C1_MODULE
  *         - \ref I2C0_MODULE
  *         - \ref FDIV1_MODULE
  *         - \ref FDIV0_MODULE
  *         - \ref FDIV_MODULE
  *         - \ref TMR3_MODULE
  *         - \ref TMR2_MODULE
  *         - \ref TMR1_MODULE
  *         - \ref TMR0_MODULE
  *         - \ref RTC_MODULE
  *         - \ref WDT_MODULE
  * @return None
  */
void CLK_EnableModuleClock(uint32_t u32ModuleIdx)
{
    *(volatile uint32_t *)((uint32_t)&CLK->AHBCLK+(MODULE_APBCLK(u32ModuleIdx)*4))  |= 1<<MODULE_IP_EN_Pos(u32ModuleIdx);
}

/**
  * @brief  This function disable module clock
  * @param[in]  u32ModuleIdx is module index. Including :
  *         - \ref GPIO_MODULE
  *         - \ref DMA_MODULE
  *         - \ref ISP_MODULE
  *         - \ref EBI_MODULE
  *         - \ref SRAM_MODULE
  *         - \ref TICK_MODULE
  *         - \ref SC1_MODULE
  *         - \ref SC0_MODULE
  *         - \ref ADC_MODULE
  *         - \ref LCD_MODULE
  *         - \ref PWM0_CH23_MODULE
  *         - \ref PWM0_CH01_MODULE
  *         - \ref UART1_MODULE
  *         - \ref UART0_MODULE
  *         - \ref SPI1_MODULE
  *         - \ref SPI0_MODULE
  *         - \ref ACMP_MODULE
  *         - \ref I2C1_MODULE
  *         - \ref I2C0_MODULE
  *         - \ref FDIV1_MODULE
  *         - \ref FDIV0_MODULE
  *         - \ref FDIV_MODULE
  *         - \ref TMR3_MODULE
  *         - \ref TMR2_MODULE
  *         - \ref TMR1_MODULE
  *         - \ref TMR0_MODULE
  *         - \ref RTC_MODULE
  *         - \ref WDT_MODULE
  * @return None
  */
void CLK_DisableModuleClock(uint32_t u32ModuleIdx)
{
    *(volatile uint32_t *)((uint32_t)&CLK->AHBCLK+(MODULE_APBCLK(u32ModuleIdx)*4))  &= ~(1<<MODULE_IP_EN_Pos(u32ModuleIdx));
}

/**
  * @brief  This function set PLL frequency
  * @param[in]  u32PllClkSrc is PLL clock source. Including :
  *         - \ref CLK_PLLCTL_PLL_SRC_HIRC
  *         - \ref CLK_PLLCTL_PLL_SRC_HXT
  * @param[in]  u32PllFreq is PLL frequency
  * @return None
  */
uint32_t CLK_EnablePLL(uint32_t u32PllClkSrc, uint32_t u32PllFreq)
{
    uint32_t u32PllCr,u32PLL_N,u32PLL_M,u32PLLReg;
    if ( u32PllFreq < FREQ_16MHZ)
        u32PllFreq=FREQ_16MHZ;
    else if(u32PllFreq > FREQ_32MHZ)
        u32PllFreq=FREQ_32MHZ;

    if(u32PllClkSrc == CLK_PLLCTL_PLL_SRC_HXT) {
        /* PLL source clock from HXT */
        CLK->PLLCTL &= ~CLK_PLLCTL_PLL_SRC_HIRC;
        u32PllCr = __HXT;
    } else {
        /* PLL source clock from HIRC */
        CLK->PLLCTL |= CLK_PLLCTL_PLL_SRC_HIRC;
        if(CLK->PWRCTL & CLK_PWRCTL_HIRC_FSEL_Msk)
            u32PllCr =__HIRC16M;
        else
            u32PllCr =__HIRC12M;
    }


    u32PLL_N=u32PllCr/1000000;
    u32PLL_M=u32PllFreq/1000000;
    while(1) {
        if(u32PLL_M<=32 && u32PLL_N<=16 ) break;
        u32PLL_M >>=1;
        u32PLL_N >>=1;
    }
    u32PLLReg = (u32PLL_M<<CLK_PLLCTL_PLL_MLP_Pos) | ((u32PLL_N-1)<<CLK_PLLCTL_PLL_SRC_N_Pos);
    CLK->PLLCTL = ( CLK->PLLCTL & ~(CLK_PLLCTL_PLL_MLP_Msk | CLK_PLLCTL_PLL_SRC_N_Msk ) )| u32PLLReg;

    if(u32PllClkSrc==CLK_PLLCTL_PLL_SRC_HIRC)
        CLK->PLLCTL = (CLK->PLLCTL & ~CLK_PLLCTL_PLL_SRC_HIRC) | (CLK_PLLCTL_PLL_SRC_HIRC);
    else
        CLK->PLLCTL = (CLK->PLLCTL & ~CLK_PLLCTL_PLL_SRC_HIRC);

    CLK->PLLCTL &= ~CLK_PLLCTL_PD_Msk;

    return CLK_GetPLLClockFreq();
}

/**
  * @brief  This function disable PLL
  * @param  None
  * @return None
  */
void CLK_DisablePLL(void)
{
    CLK->PLLCTL |= CLK_PLLCTL_PD_Msk;
}

/**
  * @brief  This function execute delay function.
  * @param[in]  us  Delay time. The Max value is 2^24 / CPU Clock(MHz). Ex:
  *                             50MHz => 335544us, 48MHz => 349525us, 28MHz => 699050us ...
  * @return None
  * @details    Use the SysTick to generate the delay time and the UNIT is in us.
  *             The SysTick clock source is from HCLK, i.e the same as system core clock.
  */
void CLK_SysTickDelay(uint32_t us)
{
    SysTick->LOAD = us * CyclesPerUs;
    SysTick->VAL  =  (0x00);
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    /* Waiting for down-count to zero */
    while((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
    SysTick->CTRL = 0;
}

/**
  * @brief  This function check selected clock source status
  * @param[in]  u32ClkMask is selected clock source. Including
  *           - \ref CLK_CLKSTATUS_CLK_SW_FAIL_Msk
  *           - \ref CLK_CLKSTATUS_HIRC_STB_Msk
  *           - \ref CLK_CLKSTATUS_LIRC_STB_Msk
  *           - \ref CLK_CLKSTATUS_PLL_STB_Msk
  *           - \ref CLK_CLKSTATUS_LXT_STB_Msk
  *           - \ref CLK_CLKSTATUS_HXT_STB_Msk
  * @return   0  clock is not stable
  *           1  clock is stable
  *
  * @details  To wait for clock ready by specified CLKSTATUS bit or timeout (~5ms)
  */
uint32_t CLK_WaitClockReady(uint32_t u32ClkMask)
{
    int32_t i32TimeOutCnt=2160000;    

    while((CLK->CLKSTATUS & u32ClkMask) != u32ClkMask) {
        if(i32TimeOutCnt-- <= 0)
            return 0;
    }
    return 1;
}


/*@}*/ /* end of group NANO1X2_CLK_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NANO1X2_CLK_Driver */

/*@}*/ /* end of group NANO1X2_Device_Driver */

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/

