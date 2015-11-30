/**************************************************************************//**
 * @file     clk.h
 * @version  V1.00
 * $Revision: 18 $
 * $Date: 14/11/27 11:50a $
 * @brief    Nano102/112 series CLK driver header file
 *
 * @note
 * Copyright (C) 2013~2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef __CLK_H__
#define __CLK_H__

#ifdef __cplusplus
extern "C"
{
#endif


/** @addtogroup NANO1X2_Device_Driver NANO102/112 Device Driver
  @{
*/

/** @addtogroup NANO1X2_CLK_Driver CLK Driver
  @{
*/

/** @addtogroup NANO1X2_CLK_EXPORTED_CONSTANTS CLK Exported Constants
  @{
*/


#define FREQ_32MHZ       32000000
#define FREQ_16MHZ       16000000

/********************* Bit definition of PWRCTL register **********************/
#define CLK_PWRCTL_HXT_EN         ((uint32_t)0x00000001)      /*!<Enable high speed crystal */
#define CLK_PWRCTL_LXT_EN         ((uint32_t)0x00000002)      /*!<Enable low speed crystal */
#define CLK_PWRCTL_HIRC_EN        ((uint32_t)0x00000004)      /*!<Enable internal high speed oscillator */
#define CLK_PWRCTL_LIRC_EN        ((uint32_t)0x00000008)      /*!<Enable internal low speed oscillator */
#define CLK_PWRCTL_DELY_EN        ((uint32_t)0x00000010)      /*!<Enable the wake-up delay counter */
#define CLK_PWRCTL_WAKEINT_EN     ((uint32_t)0x00000020)      /*!<Enable the wake-up interrupt */
#define CLK_PWRCTL_PWRDOWN_EN     ((uint32_t)0x00000040)      /*!<Power down enable bit */
#define CLK_PWRCTL_HXT_SELXT      ((uint32_t)0x00000100)      /*!<High frequency crystal loop back path Enabled */
#define CLK_PWRCTL_HXT_GAIN       ((uint32_t)0x00000200)      /*!<High frequency crystal Gain control Enabled */
#define CLK_PWRCTL_LXT_SCNT       ((uint32_t)0x00000400)      /*!<Delay 8192 LXT before LXT output */


/********************* Bit definition of AHBCLK register **********************/
#define CLK_AHBCLK_GPIO_EN        ((uint32_t)0x00000001)      /*!<GPIO clock enable */
#define CLK_AHBCLK_DMA_EN         ((uint32_t)0x00000002)      /*!<DMA clock enable */
#define CLK_AHBCLK_ISP_EN         ((uint32_t)0x00000004)      /*!<Flash ISP controller clock enable */
#define CLK_AHBCLK_EBI_EN         ((uint32_t)0x00000008)      /*!<EBI clock enable */
#define CLK_AHBCLK_SRAM_EN        ((uint32_t)0x00000010)      /*!<SRAM Controller Clock Enable */
#define CLK_AHBCLK_TICK_EN        ((uint32_t)0x00000020)      /*!<System Tick Clock Enable */

/********************* Bit definition of APBCLK register **********************/
#define CLK_APBCLK_WDT_EN         ((uint32_t)0x00000001)      /*!<Watchdog clock enable */
#define CLK_APBCLK_RTC_EN         ((uint32_t)0x00000002)      /*!<RTC clock enable */
#define CLK_APBCLK_TMR0_EN        ((uint32_t)0x00000004)      /*!<Timer 0 clock enable */
#define CLK_APBCLK_TMR1_EN        ((uint32_t)0x00000008)      /*!<Timer 1 clock enable */
#define CLK_APBCLK_TMR2_EN        ((uint32_t)0x00000010)      /*!<Timer 2 clock enable */
#define CLK_APBCLK_TMR3_EN        ((uint32_t)0x00000020)      /*!<Timer 3 clock enable */
#define CLK_APBCLK_FDIV_EN        ((uint32_t)0x00000040)      /*!<Frequency Divider Output clock enable */
#define CLK_APBCLK_SC2_EN         ((uint32_t)0x00000080)      /*!<SmartCard 2 Clock Enable Control */
#define CLK_APBCLK_I2C0_EN        ((uint32_t)0x00000100)      /*!<I2C 0 clock enable */
#define CLK_APBCLK_I2C1_EN        ((uint32_t)0x00000200)      /*!<I2C 1 clock enable */
#define CLK_APBCLK_SPI0_EN        ((uint32_t)0x00001000)      /*!<SPI 0 clock enable */
#define CLK_APBCLK_SPI1_EN        ((uint32_t)0x00002000)      /*!<SPI 1 clock enable */
#define CLK_APBCLK_SPI2_EN        ((uint32_t)0x00004000)      /*!<SPI 2 clock enable */
#define CLK_APBCLK_UART0_EN       ((uint32_t)0x00010000)      /*!<UART 0 clock enable */
#define CLK_APBCLK_UART1_EN       ((uint32_t)0x00020000)      /*!<UART 1 clock enable */
#define CLK_APBCLK_PWM0_CH01_EN   ((uint32_t)0x00100000)      /*!<PWM0 Channel 0 and Channel 1 Clock Enable Control */
#define CLK_APBCLK_PWM0_CH23_EN   ((uint32_t)0x00200000)      /*!<PWM0 Channel 2 and Channel 3 Clock Enable Control */
#define CLK_APBCLK_TK_EN          ((uint32_t)0x01000000)      /*!<Touch key Clock Enable Control */
#define CLK_APBCLK_DAC_EN         ((uint32_t)0x02000000)      /*!<DAC Clock Enable Control */
#define CLK_APBCLK_LCD_EN         ((uint32_t)0x04000000)      /*!<LCD controller Clock Enable Control */
#define CLK_APBCLK_USBD_EN        ((uint32_t)0x08000000)      /*!<USB device clock enable */
#define CLK_APBCLK_ADC_EN         ((uint32_t)0x10000000)      /*!<ADC clock enable */
#define CLK_APBCLK_I2S_EN         ((uint32_t)0x20000000)      /*!<I2S clock enable */
#define CLK_APBCLK_SC0_EN         ((uint32_t)0x40000000)      /*!<SmartCard 0 Clock Enable Control */
#define CLK_APBCLK_SC1_EN         ((uint32_t)0x80000000)      /*!<SmartCard 1 Clock Enable Control */

/********************* Bit definition of CLKSTATUS register **********************/
#define CLK_CLKSTATUS_HXT_STB     ((uint32_t)0x00000001)      /*!<External high speed crystal clock source stable flag */
#define CLK_CLKSTATUS_LXT_STB     ((uint32_t)0x00000002)      /*!<External low speed crystal clock source stable flag */
#define CLK_CLKSTATUS_PLL_STB     ((uint32_t)0x00000004)      /*!<Internal PLL clock source stable flag */
#define CLK_CLKSTATUS_LIRC_STB    ((uint32_t)0x00000008)      /*!<Internal low speed oscillator clock source stable flag */
#define CLK_CLKSTATUS_HIRC_STB    ((uint32_t)0x00000010)      /*!<Internal high speed oscillator clock source stable flag */
#define CLK_CLKSTATUS_CLK_SW_FAIL ((uint32_t)0x00000080)      /*!<Clock switch fail flag */


/********************* Bit definition of CLKSEL0 register **********************/
#define CLK_CLKSEL0_HCLK_S_HXT    (0UL<<CLK_CLKSEL0_HCLK_S_Pos)     /*!<Select HCLK clock source from high speed crystal */
#define CLK_CLKSEL0_HCLK_S_LXT    (1UL<<CLK_CLKSEL0_HCLK_S_Pos)     /*!<Select HCLK clock source from low speed crystal */
#define CLK_CLKSEL0_HCLK_S_PLL    (2UL<<CLK_CLKSEL0_HCLK_S_Pos)     /*!<Select HCLK clock source from PLL */
#define CLK_CLKSEL0_HCLK_S_LIRC   (3UL<<CLK_CLKSEL0_HCLK_S_Pos)     /*!<Select HCLK clock source from low speed oscillator */
#define CLK_CLKSEL0_HCLK_S_HIRC   (7UL<<CLK_CLKSEL0_HCLK_S_Pos)     /*!<Select HCLK clock source from high speed oscillator */

/********************* Bit definition of CLKSEL1 register **********************/
#define CLK_CLKSEL1_ADC_S_HXT     (0x0UL<<CLK_CLKSEL1_ADC_S_Pos)      /*!<Select ADC clock source from high speed crystal */
#define CLK_CLKSEL1_ADC_S_LXT     (0x1UL<<CLK_CLKSEL1_ADC_S_Pos)      /*!<Select ADC clock source from low speed crystal */
#define CLK_CLKSEL1_ADC_S_PLL     (0x2UL<<CLK_CLKSEL1_ADC_S_Pos)      /*!<Select ADC clock source from PLL */
#define CLK_CLKSEL1_ADC_S_HIRC    (0x3UL<<CLK_CLKSEL1_ADC_S_Pos)      /*!<Select ADC clock source from high speed oscillator */
#define CLK_CLKSEL1_ADC_S_HCLK    (0x4UL<<CLK_CLKSEL1_ADC_S_Pos)      /*!<Select ADC clock source from HCLK */

#define CLK_CLKSEL1_LCD_S_LXT     (0x0UL<<CLK_CLKSEL1_LCD_S_Pos)      /*!<Select LCD clock source from low speed crystal */

#define CLK_CLKSEL1_TMR1_S_HXT    (0x0UL<<CLK_CLKSEL1_TMR1_S_Pos)     /*!<Select TMR1 clock source from high speed crystal */
#define CLK_CLKSEL1_TMR1_S_LXT    (0x1UL<<CLK_CLKSEL1_TMR1_S_Pos)     /*!<Select TMR1 clock source from low speed crystal */
#define CLK_CLKSEL1_TMR1_S_LIRC   (0x2UL<<CLK_CLKSEL1_TMR1_S_Pos)     /*!<Select TMR1 clock source from low speed oscillator  */
#define CLK_CLKSEL1_TMR1_S_EXT    (0x3UL<<CLK_CLKSEL1_TMR1_S_Pos)     /*!<Select TMR1 clock source from external trigger */
#define CLK_CLKSEL1_TMR1_S_HIRC   (0x4UL<<CLK_CLKSEL1_TMR1_S_Pos)     /*!<Select TMR1 clock source from high speed oscillator */
#define CLK_CLKSEL1_TMR1_S_HCLK   (0x5UL<<CLK_CLKSEL1_TMR1_S_Pos)     /*!<Select TMR1 clock source from HCLK */

#define CLK_CLKSEL1_TMR0_S_HXT    (0x0UL<<CLK_CLKSEL1_TMR0_S_Pos)     /*!<Select TMR0 clock source from high speed crystal */
#define CLK_CLKSEL1_TMR0_S_LXT    (0x1UL<<CLK_CLKSEL1_TMR0_S_Pos)     /*!<Select TMR0 clock source from low speed crystal */
#define CLK_CLKSEL1_TMR0_S_LIRC   (0x2UL<<CLK_CLKSEL1_TMR0_S_Pos)     /*!<Select TMR0 clock source from low speed oscillator */
#define CLK_CLKSEL1_TMR0_S_EXT    (0x3UL<<CLK_CLKSEL1_TMR0_S_Pos)     /*!<Select TMR0 clock source from external trigger */
#define CLK_CLKSEL1_TMR0_S_HIRC   (0x4UL<<CLK_CLKSEL1_TMR0_S_Pos)     /*!<Select TMR0 clock source from high speed oscillator */
#define CLK_CLKSEL1_TMR0_S_HCLK   (0x5UL<<CLK_CLKSEL1_TMR0_S_Pos)     /*!<Select TMR0 clock source from HCLK */

#define CLK_CLKSEL1_PWM0_CH01_S_HXT   (0x0UL<<CLK_CLKSEL1_PWM0_CH01_S_Pos)  /*!<Select PWM0_CH01 clock source from high speed crystal */
#define CLK_CLKSEL1_PWM0_CH01_S_LXT   (0x1UL<<CLK_CLKSEL1_PWM0_CH01_S_Pos)  /*!<Select PWM0_CH01 clock source from low speed crystal */
#define CLK_CLKSEL1_PWM0_CH01_S_HCLK  (0x2UL<<CLK_CLKSEL1_PWM0_CH01_S_Pos)  /*!<Select PWM0_CH01 clock source from HCLK */
#define CLK_CLKSEL1_PWM0_CH01_S_HIRC  (0x3UL<<CLK_CLKSEL1_PWM0_CH01_S_Pos)  /*!<Select PWM0_CH01 clock source from high speed oscillator */

#define CLK_CLKSEL1_PWM0_CH23_S_HXT   (0x0UL<<CLK_CLKSEL1_PWM0_CH23_S_Pos)  /*!<Select PWM0_CH23 clock source from high speed crystal */
#define CLK_CLKSEL1_PWM0_CH23_S_LXT   (0x1UL<<CLK_CLKSEL1_PWM0_CH23_S_Pos)  /*!<Select PWM0_CH23 clock source from low speed crystal */
#define CLK_CLKSEL1_PWM0_CH23_S_HCLK  (0x2UL<<CLK_CLKSEL1_PWM0_CH23_S_Pos)  /*!<Select PWM0_CH23 clock source from HCLK */
#define CLK_CLKSEL1_PWM0_CH23_S_HIRC  (0x3UL<<CLK_CLKSEL1_PWM0_CH23_S_Pos)  /*!<Select PWM0_CH23 clock source from high speed oscillator */

#define CLK_CLKSEL1_UART_S_HXT    (0x0UL<<CLK_CLKSEL1_UART_S_Pos)     /*!<Select UART clock source from high speed crystal */
#define CLK_CLKSEL1_UART_S_LXT    (0x1UL<<CLK_CLKSEL1_UART_S_Pos)     /*!<Select UART clock source from low speed crystal */
#define CLK_CLKSEL1_UART_S_PLL    (0x2UL<<CLK_CLKSEL1_UART_S_Pos)     /*!<Select UART clock source from PLL */
#define CLK_CLKSEL1_UART_S_HIRC   (0x3UL<<CLK_CLKSEL1_UART_S_Pos)     /*!<Select UART clock source from high speed oscillator */

/********************* Bit definition of CLKSEL2 register **********************/
#define CLK_CLKSEL2_SPI1_S_PLL    (0x0UL<<CLK_CLKSEL2_SPI1_S_Pos)             /*!<Select SPI 1 clock source from PLL */
#define CLK_CLKSEL2_SPI1_S_HCLK   (0x1UL<<CLK_CLKSEL2_SPI1_S_Pos)             /*!<Select SPI 1 clock source from HCLK */

#define CLK_CLKSEL2_SPI0_S_PLL    (0x0UL<<CLK_CLKSEL2_SPI0_S_Pos)             /*!<Select SPI 0 clock source from PLL */
#define CLK_CLKSEL2_SPI0_S_HCLK   (0x1UL<<CLK_CLKSEL2_SPI0_S_Pos)             /*!<Select SPI 0 clock source from HCLK */

#define CLK_CLKSEL2_SC_S_HXT      (0x0UL<<CLK_CLKSEL2_SC_S_Pos)               /*!<Select SC clock source from high speed crystal */
#define CLK_CLKSEL2_SC_S_PLL      (0x1UL<<CLK_CLKSEL2_SC_S_Pos)               /*!<Select SC clock source from PLL */
#define CLK_CLKSEL2_SC_S_HIRC     (0x2UL<<CLK_CLKSEL2_SC_S_Pos)               /*!<Select SC clock source from high speed oscillator */
#define CLK_CLKSEL2_SC_S_HCLK     (0x3UL<<CLK_CLKSEL2_SC_S_Pos)               /*!<Select SC clock source from HCLK */

#define CLK_CLKSEL2_TMR2_S_HXT    (0x0UL<<CLK_CLKSEL2_TMR2_S_Pos)             /*!<Select TMR2 clock source from high speed crystal */
#define CLK_CLKSEL2_TMR2_S_LXT    (0x1UL<<CLK_CLKSEL2_TMR2_S_Pos)             /*!<Select TMR2 clock source from low speed crystal */
#define CLK_CLKSEL2_TMR2_S_LIRC   (0x2UL<<CLK_CLKSEL2_TMR2_S_Pos)             /*!<Select TMR2 clock source from low speed oscillator */
#define CLK_CLKSEL2_TMR2_S_EXT    (0x3UL<<CLK_CLKSEL2_TMR2_S_Pos)             /*!<Select TMR2 clock source from external trigger */
#define CLK_CLKSEL2_TMR2_S_HIRC   (0x4UL<<CLK_CLKSEL2_TMR2_S_Pos)             /*!<Select TMR2 clock source from high speed oscillator */
#define CLK_CLKSEL2_TMR2_S_HCLK   (0x5UL<<CLK_CLKSEL2_TMR2_S_Pos)             /*!<Select TMR2 clock source from HCLK */

#define CLK_CLKSEL2_TMR3_S_HXT    (0x0UL<<CLK_CLKSEL2_TMR3_S_Pos)             /*!<Select TMR3 clock source from high speed crystal */
#define CLK_CLKSEL2_TMR3_S_LXT    (0x1UL<<CLK_CLKSEL2_TMR3_S_Pos)             /*!<Select TMR3 clock source from low speed crystal */
#define CLK_CLKSEL2_TMR3_S_LIRC   (0x2UL<<CLK_CLKSEL2_TMR3_S_Pos)             /*!<Select TMR3 clock source from low speed oscillator  */
#define CLK_CLKSEL2_TMR3_S_EXT    (0x3UL<<CLK_CLKSEL2_TMR3_S_Pos)             /*!<Select TMR3 clock source from external trigger */
#define CLK_CLKSEL2_TMR3_S_HIRC   (0x4UL<<CLK_CLKSEL2_TMR3_S_Pos)             /*!<Select TMR3 clock source from high speed oscillator */
#define CLK_CLKSEL2_TMR3_S_HCLK   (0x5UL<<CLK_CLKSEL2_TMR3_S_Pos)             /*!<Select TMR3 clock source from HCLK */


#define CLK_CLKSEL2_FRQDIV0_S_HXT      (0x0UL<<CLK_CLKSEL2_FRQDIV0_S_Pos)       /*!<Select FRQDIV0 clock source from high speed crystal */
#define CLK_CLKSEL2_FRQDIV0_S_LXT      (0x1UL<<CLK_CLKSEL2_FRQDIV0_S_Pos)       /*!<Select FRQDIV0 clock source from low speed crystal */
#define CLK_CLKSEL2_FRQDIV0_S_HCLK     (0x2UL<<CLK_CLKSEL2_FRQDIV0_S_Pos)       /*!<Select FRQDIV0 clock source from HCLK */
#define CLK_CLKSEL2_FRQDIV0_S_HIRC     (0x3UL<<CLK_CLKSEL2_FRQDIV0_S_Pos)       /*!<Select FRQDIV0 clock source from high speed oscillator */

#define CLK_CLKSEL2_FRQDIV_S_HXT      CLK_CLKSEL2_FRQDIV0_S_HXT               /*!<Select FRQDIV0 clock source from high speed crystal */
#define CLK_CLKSEL2_FRQDIV_S_LXT      CLK_CLKSEL2_FRQDIV0_S_LXT               /*!<Select FRQDIV0 clock source from low speed crystal */
#define CLK_CLKSEL2_FRQDIV_S_HCLK     CLK_CLKSEL2_FRQDIV0_S_HCLK              /*!<Select FRQDIV0 clock source from HCLK */
#define CLK_CLKSEL2_FRQDIV_S_HIRC     CLK_CLKSEL2_FRQDIV0_S_HIRC              /*!<Select FRQDIV0 clock source from high speed oscillator */

#define CLK_CLKSEL2_FRQDIV1_S_HXT     (0x0UL<<CLK_CLKSEL2_FRQDIV1_S_Pos)      /*!<Select FRQDIV1 clock source from high speed crystal */
#define CLK_CLKSEL2_FRQDIV1_S_LXT     (0x1UL<<CLK_CLKSEL2_FRQDIV1_S_Pos)      /*!<Select FRQDIV1 clock source from low speed crystal */
#define CLK_CLKSEL2_FRQDIV1_S_HCLK    (0x2UL<<CLK_CLKSEL2_FRQDIV1_S_Pos)      /*!<Select FRQDIV1 clock source from HCLK */
#define CLK_CLKSEL2_FRQDIV1_S_HIRC    (0x3UL<<CLK_CLKSEL2_FRQDIV1_S_Pos)      /*!<Select FRQDIV1 clock source from high speed oscillator */

/********************* Bit definition of CLKDIV0 register **********************/
#define CLK_HCLK_CLK_DIVIDER(x)     (((x-1)<< CLK_CLKDIV0_HCLK_N_Pos) & CLK_CLKDIV0_HCLK_N_Msk)  /*!< CLKDIV0 Setting for HCLK clock divider. It could be 1~16 */
#define CLK_UART_CLK_DIVIDER(x)     (((x-1)<< CLK_CLKDIV0_UART_N_Pos) & CLK_CLKDIV0_UART_N_Msk)  /*!< CLKDIV0 Setting for UART clock divider. It could be 1~16 */
#define CLK_ADC_CLK_DIVIDER(x)      (((x-1)<< CLK_CLKDIV0_ADC_N_Pos)  & CLK_CLKDIV0_ADC_N_Msk)    /*!< CLKDIV0 Setting for ADC clock divider. It could be 1~256 */
#define CLK_SC0_CLK_DIVIDER(x)      (((x-1)<< CLK_CLKDIV0_SC0_N_Pos)  & CLK_CLKDIV0_SC0_N_Msk)    /*!< CLKDIV0 Setting for SmartCard0 clock divider. It could be 1~16 */

/********************* Bit definition of CLKDIV1 register **********************/
#define CLK_SC1_CLK_DIVIDER(x)      (((x-1)<< CLK_CLKDIV1_SC1_N_Pos ) & CLK_CLKDIV1_SC1_N_Msk)   /*!< CLKDIV1 Setting for SmartCard1 clock divider. It could be 1~16 */
#define CLK_TMR3_CLK_DIVIDER(x)     (((x-1)<< CLK_CLKDIV1_TMR3_N_Pos) & CLK_CLKDIV1_TMR3_N_Msk)  /*!< CLKDIV1 Setting for Timer3 clock divider. It could be 1~16 */
#define CLK_TMR2_CLK_DIVIDER(x)     (((x-1)<< CLK_CLKDIV1_TMR2_N_Pos) & CLK_CLKDIV1_TMR2_N_Msk)  /*!< CLKDIV1 Setting for Timer2 clock divider. It could be 1~16 */
#define CLK_TMR1_CLK_DIVIDER(x)     (((x-1)<< CLK_CLKDIV1_TMR1_N_Pos) & CLK_CLKDIV1_TMR1_N_Msk)  /*!< CLKDIV1 Setting for Timer1 clock divider. It could be 1~16 */
#define CLK_TMR0_CLK_DIVIDER(x)     (((x-1)<< CLK_CLKDIV1_TMR0_N_Pos) & CLK_CLKDIV1_TMR0_N_Msk)  /*!< CLKDIV1 Setting for Timer0 clock divider. It could be 1~16 */

/********************* Bit definition of PLLCTL register **********************/
#define CLK_PLLCTL_OUT_DV         ((uint32_t)0x00001000)    /*!<PLL Output Divider Control */
#define CLK_PLLCTL_PD             ((uint32_t)0x00010000)    /*!<PLL Power down mode */
#define CLK_PLLCTL_PLL_SRC_HIRC   ((uint32_t)0x00020000)    /*!<PLL clock source from high speed oscillator */
#define CLK_PLLCTL_PLL_SRC_HXT    ((uint32_t)0x00000000)    /*!<PLL clock source from high speed crystal */

/********************* Bit definition of FRQDIV register **********************/
#define CLK_FRQDIV_EN         ((uint32_t)0x00000010)    /*!<Frequency divider enable bit */

/********************* Bit definition of WK_INTSTS register **********************/
#define CLK_WK_INTSTS_IS      ((uint32_t)0x00000001)    /*!<Wake-up Interrupt Status in chip Power-down Mode */


/*---------------------------------------------------------------------------------------------------------*/
/*  MODULE constant definitions.                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
#define MODULE_APBCLK(x)                   ((x >>31) & 0x1)    /*!< Calculate APBCLK offset on MODULE index */
#define MODULE_CLKSEL(x)                   ((x >>29) & 0x3)    /*!< Calculate CLKSEL offset on MODULE index */
#define MODULE_CLKSEL_Msk(x)               ((x >>25) & 0xf)    /*!< Calculate CLKSEL mask offset on MODULE index */
#define MODULE_CLKSEL_Pos(x)               ((x >>20) & 0x1f)   /*!< Calculate CLKSEL position offset on MODULE index */
#define MODULE_CLKDIV(x)                   ((x >>18) & 0x3)    /*!< Calculate APBCLK CLKDIV on MODULE index */
#define MODULE_CLKDIV_Msk(x)               ((x >>10) & 0xff)   /*!< Calculate CLKDIV mask offset on MODULE index */
#define MODULE_CLKDIV_Pos(x)               ((x >>5 ) & 0x1f)   /*!< Calculate CLKDIV position offset on MODULE index */
#define MODULE_IP_EN_Pos(x)                ((x >>0 ) & 0x1f)   /*!< Calculate APBCLK offset on MODULE index */
#define MODULE_NoMsk                       0x0                 /*!< Not mask on MODULE index */
/*-------------------------------------------------------------------------------------------------------------------------------*/
/*   AHBCLK/APBCLK(1) | CLKSEL(2) | CLKSEL_Msk(4) |  CLKSEL_Pos(5) | CLKDIV(2) | CLKDIV_Msk(8) |  CLKDIV_Pos(5)  |  IP_EN_Pos(5)        */
/*-------------------------------------------------------------------------------------------------------------------------------*/
#define TICK_MODULE      ((0UL<<31)|(3<<29)|(MODULE_NoMsk<<25)|( 0<<20)|(1<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_AHBCLK_TICK_EN_Pos     ) /*!< TICK Module */
#define SRAM_MODULE      ((0UL<<31)|(3<<29)|(MODULE_NoMsk<<25)|( 0<<20)|(1<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_AHBCLK_SRAM_EN_Pos     ) /*!< SRAM Module */
#define EBI_MODULE       ((0UL<<31)|(3<<29)|(MODULE_NoMsk<<25)|( 0<<20)|(1<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_AHBCLK_EBI_EN_Pos      ) /*!< EBI Module */
#define ISP_MODULE       ((0UL<<31)|(3<<29)|(MODULE_NoMsk<<25)|( 0<<20)|(1<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_AHBCLK_ISP_EN_Pos      ) /*!< ISP Module */
#define DMA_MODULE       ((0UL<<31)|(3<<29)|(MODULE_NoMsk<<25)|( 0<<20)|(0<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_AHBCLK_DMA_EN_Pos      ) /*!< DMA Module */
#define GPIO_MODULE      ((0UL<<31)|(3<<29)|(MODULE_NoMsk<<25)|( 0<<20)|(0<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_AHBCLK_GPIO_EN_Pos     ) /*!< GPIO Module */

#define SC1_MODULE       ((1UL<<31)|(2<<29)|(3<<25)           |(18<<20)|(1<<18)|(0xF<<10)         |(28<<5)|CLK_APBCLK_SC1_EN_Pos      ) /*!< SmartCard1 Module */
#define SC0_MODULE       ((1UL<<31)|(2<<29)|(3<<25)           |(18<<20)|(0<<18)|(0xF<<10)         |( 0<<5)|CLK_APBCLK_SC0_EN_Pos      ) /*!< SmartCard0 Module */
#define ADC_MODULE       ((1UL<<31)|(1<<29)|(7<<25)           |(19<<20)|(0<<18)|(0xFF<<10)        |(16<<5)|CLK_APBCLK_ADC_EN_Pos      ) /*!< ADC Module */
#define LCD_MODULE       ((1UL<<31)|(1<<29)|(1<<25)           |(18<<20)|(0<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_APBCLK_LCD_EN_Pos      ) /*!< LCD Module */
#define PWM0_CH23_MODULE ((1UL<<31)|(1<<29)|(3<<25)           |( 6<<20)|(0<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_APBCLK_PWM0_CH23_EN_Pos) /*!< PWM0 Channel2 and Channel3 Module */
#define PWM0_CH01_MODULE ((1UL<<31)|(1<<29)|(3<<25)           |( 4<<20)|(0<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_APBCLK_PWM0_CH01_EN_Pos) /*!< PWM0 Channel0 and Channel1 Module */
#define UART1_MODULE     ((1UL<<31)|(1<<29)|(3<<25)           |( 0<<20)|(0<<18)|(0xF<<10)         |( 8<<5)|CLK_APBCLK_UART1_EN_Pos    ) /*!< UART1 Module */
#define UART0_MODULE     ((1UL<<31)|(1<<29)|(3<<25)           |( 0<<20)|(0<<18)|(0xF<<10)         |( 8<<5)|CLK_APBCLK_UART0_EN_Pos    ) /*!< UART0 Module */
#define SPI1_MODULE      ((1UL<<31)|(2<<29)|(1<<25)           |(21<<20)|(0<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_APBCLK_SPI1_EN_Pos     ) /*!< SPI1 Module */
#define SPI0_MODULE      ((1UL<<31)|(2<<29)|(1<<25)           |(20<<20)|(0<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_APBCLK_SPI0_EN_Pos     ) /*!< SPI0 Module */
#define ACMP_MODULE      ((1UL<<31)|(0<<29)|(MODULE_NoMsk<<25)|( 0<<20)|(0<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_APBCLK_ACMP_EN_Pos     ) /*!< ACMP Module */
#define I2C1_MODULE      ((1UL<<31)|(0<<29)|(MODULE_NoMsk<<25)|( 0<<20)|(0<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_APBCLK_I2C1_EN_Pos     ) /*!< I2C1 Module */
#define I2C0_MODULE      ((1UL<<31)|(0<<29)|(MODULE_NoMsk<<25)|( 0<<20)|(0<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_APBCLK_I2C0_EN_Pos     ) /*!< I2C0 Module */
#define FDIV1_MODULE     ((1UL<<31)|(2<<29)|(3<25)            |( 0<<20)|(0<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_APBCLK_FDIV1_EN_Pos    ) /*!< Frequency Divider1 Output Module */
#define FDIV0_MODULE     ((1UL<<31)|(2<<29)|(3<<25)           |( 2<<20)|(0<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_APBCLK_FDIV_EN_Pos     ) /*!< Frequency Divider0 Output Module */
#define TMR3_MODULE      ((1UL<<31)|(2<<29)|(7<<25)           |(12<<20)|(1<<18)|(0xF<<10)         |(20<<5)|CLK_APBCLK_TMR3_EN_Pos     ) /*!< Timer3 Module */
#define TMR2_MODULE      ((1UL<<31)|(2<<29)|(7<<25)           |( 8<<20)|(1<<18)|(0xF<<10)         |(16<<5)|CLK_APBCLK_TMR2_EN_Pos     ) /*!< Timer2 Module */
#define TMR1_MODULE      ((1UL<<31)|(1<<29)|(7<<25)           |(12<<20)|(1<<18)|(0xF<<10)         |(12<<5)|CLK_APBCLK_TMR1_EN_Pos     ) /*!< Timer1 Module */
#define TMR0_MODULE      ((1UL<<31)|(1<<29)|(7<<25)           |( 8<<20)|(1<<18)|(0xF<<10)         |( 8<<5)|CLK_APBCLK_TMR0_EN_Pos     ) /*!< Timer0 Module */
#define RTC_MODULE       ((1UL<<31)|(3<<29)|(MODULE_NoMsk<<25)|( 0<<20)|(0<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_APBCLK_RTC_EN_Pos      ) /*!< Real-Time-Clock Module */
#define WDT_MODULE       ((1UL<<31)|(3<<29)|(MODULE_NoMsk<<25)|( 0<<20)|(0<<18)|(MODULE_NoMsk<<10)|( 0<<5)|CLK_APBCLK_WDT_EN_Pos      ) /*!< Watchdog Timer Module */

#define FDIV_MODULE     FDIV0_MODULE /*!< Frequency Divider Output Module */

/*@}*/ /* end of group NANO1X2_CLK_EXPORTED_CONSTANTS */


/** @addtogroup NANO1X2_CLK_EXPORTED_FUNCTIONS CLK Exported Functions
  @{
*/
void CLK_DisableCKO(void);
void CLK_DisableCKO0(void);
void CLK_DisableCKO1(void);
void CLK_EnableCKO(uint32_t u32ClkSrc, uint32_t u32ClkDiv, uint32_t u32ClkDivBy1En);
void CLK_EnableCKO0(uint32_t u32ClkSrc, uint32_t u32ClkDiv, uint32_t u32ClkDivBy1En);
void CLK_EnableCKO1(uint32_t u32ClkSrc, uint32_t u32ClkDiv, uint32_t u32ClkDivBy1En);

void CLK_PowerDown(void);
void CLK_Idle(void);
uint32_t CLK_GetHXTFreq(void);
uint32_t CLK_GetLXTFreq(void);
uint32_t CLK_GetHCLKFreq(void);
uint32_t CLK_GetPCLKFreq(void);
uint32_t CLK_GetCPUFreq(void);
uint32_t CLK_GetPLLClockFreq(void);
uint32_t CLK_SetCoreClock(uint32_t u32Hclk);
void CLK_SetHCLK(uint32_t u32ClkSrc, uint32_t u32ClkDiv);
void CLK_SetModuleClock(uint32_t u32ModuleIdx, uint32_t u32ClkSrc, uint32_t u32ClkDiv);
void CLK_SetSysTickClockSrc(uint32_t u32ClkSrc);
void CLK_EnableXtalRC(uint32_t u32ClkMask);
void CLK_DisableXtalRC(uint32_t u32ClkMask);
void CLK_EnableModuleClock(uint32_t u32ModuleIdx);
void CLK_DisableModuleClock(uint32_t u32ModuleIdx);
uint32_t CLK_EnablePLL(uint32_t u32PllClkSrc, uint32_t u32PllFreq);
void CLK_DisablePLL(void);
void CLK_SysTickDelay(uint32_t us);
uint32_t CLK_WaitClockReady(uint32_t u32ClkMask);

/*@}*/ /* end of group NANO1X2_CLK_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NANO1X2_CLK_Driver */

/*@}*/ /* end of group NANO1X2_Device_Driver */

#ifdef __cplusplus
}
#endif

#endif //__CLK_H__

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
