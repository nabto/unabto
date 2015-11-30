/**************************************************************************//**
* @file     sys.h
* @version  V1.00
* $Revision: 47 $
* $Date: 14/12/25 3:34p $
* @brief    Nano102/112 Series system control header file.
*
* @note
* Copyright (C) 2013~2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef __SYS_H__
#define __SYS_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** @addtogroup NANO1X2_Device_Driver NANO102/112 Device Driver
  @{
*/

/** @addtogroup NANO1X2_SYS_Driver SYS Driver
  @{
*/

/** @addtogroup NANO1X2_SYS_EXPORTED_CONSTANTS SYS Exported Constants
  @{
*/

/*---------------------------------------------------------------------------------------------------------*/
/*  Module Reset Control Resister constant definitions.                                                    */
/*---------------------------------------------------------------------------------------------------------*/
#define CHIP_RST  ((0x0<<24) | SYS_IPRST_CTL1_CPU_RST_Pos   ) /*!< CPU reset is one of the SYS_ResetModule parameter */
#define CPU_RST   ((0x0<<24) | SYS_IPRST_CTL1_CHIP_RST_Pos  ) /*!< CHIP reset is one of the SYS_ResetModule parameter */
#define DMA_RST   ((0x0<<24) | SYS_IPRST_CTL1_DMA_RST_Pos   ) /*!< DMA reset is one of the SYS_ResetModule parameter */
#define SC1_RST   ((0x4<<24) | SYS_IPRST_CTL2_SC1_RST_Pos   ) /*!< SmartCard1 reset is one of the SYS_ResetModule parameter */
#define SC0_RST   ((0x4<<24) | SYS_IPRST_CTL2_SC0_RST_Pos   ) /*!< SmartCard0 reset is one of the SYS_ResetModule parameter */
#define ADC_RST   ((0x4<<24) | SYS_IPRST_CTL2_ADC_RST_Pos   ) /*!< ADC reset is one of the SYS_ResetModule parameter */
#define LCD_RST   ((0x4<<24) | SYS_IPRST_CTL2_LCD_RST_Pos   ) /*!< LCD reset is one of the SYS_ResetModule parameter */
#define CMP01_RST ((0x4<<24) | SYS_IPRST_CTL2_CMP01_RST_Pos ) /*!< CMP01 reset is one of the SYS_ResetModule parameter */
#define PWM0_RST  ((0x4<<24) | SYS_IPRST_CTL2_PWM0_RST_Pos  ) /*!< PWM0 reset is one of the SYS_ResetModule parameter */
#define UART1_RST ((0x4<<24) | SYS_IPRST_CTL2_UART1_RST_Pos ) /*!< UART1 reset is one of the SYS_ResetModule parameter */
#define UART0_RST ((0x4<<24) | SYS_IPRST_CTL2_UART0_RST_Pos ) /*!< UART0 reset is one of the SYS_ResetModule parameter */
#define SPI1_RST  ((0x4<<24) | SYS_IPRST_CTL2_SPI1_RST_Pos  ) /*!< SPI1 reset is one of the SYS_ResetModule parameter */
#define SPI0_RST  ((0x4<<24) | SYS_IPRST_CTL2_SPI0_RST_Pos  ) /*!< SPI0 reset is one of the SYS_ResetModule parameter */
#define I2C1_RST  ((0x4<<24) | SYS_IPRST_CTL2_I2C1_RST_Pos  ) /*!< I2C1 reset is one of the SYS_ResetModule parameter */
#define I2C0_RST  ((0x4<<24) | SYS_IPRST_CTL2_I2C0_RST_Pos  ) /*!< I2C0 reset is one of the SYS_ResetModule parameter */
#define TMR3_RST  ((0x4<<24) | SYS_IPRST_CTL2_TMR3_RST_Pos  ) /*!< Timer3 reset is one of the SYS_ResetModule parameter */
#define TMR2_RST  ((0x4<<24) | SYS_IPRST_CTL2_TMR2_RST_Pos  ) /*!< Timer2 reset is one of the SYS_ResetModule parameter */
#define TMR1_RST  ((0x4<<24) | SYS_IPRST_CTL2_TMR1_RST_Pos  ) /*!< Timer1 reset is one of the SYS_ResetModule parameter */
#define TMR0_RST  ((0x4<<24) | SYS_IPRST_CTL2_TMR0_RST_Pos  ) /*!< Timer0 reset is one of the SYS_ResetModule parameter */
#define GPIO_RST  ((0x4<<24) | SYS_IPRST_CTL2_GPIO_RST_Pos  ) /*!< GPIO reset is one of the SYS_ResetModule parameter */

/*---------------------------------------------------------------------------------------------------------*/
/*  Multi-Function constant definitions.                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
/********************* Bit definition of RSTSRC register **********************/
#define SYS_RSTSRC_POR        ((uint32_t)0x00000001)      /*!<Power on reset flag */
#define SYS_RSTSRC_RESET      ((uint32_t)0x00000002)      /*!<RESET pin reset flag */
#define SYS_RSTSRC_WDT        ((uint32_t)0x00000004)      /*!<Watchdog timer reset flag */
#define SYS_RSTSRC_BOD        ((uint32_t)0x00000010)      /*!<BOD reset flag */
#define SYS_RSTSRC_SYS        ((uint32_t)0x00000020)      /*!<System reset flag */
#define SYS_RSTSRC_CPU        ((uint32_t)0x00000080)      /*!<CPU reset flag */

/********************* Bit definition of IPRSTCTL1 register **********************/
#define SYS_IPRSTCTL1_CHIP    ((uint32_t)0x00000001)      /*!<Chip oneshot reset */
#define SYS_IPRSTCTL1_CPU     ((uint32_t)0x00000002)      /*!<CPU oneshot reset */
#define SYS_IPRSTCTL1_DMA     ((uint32_t)0x00000004)      /*!<DMA reset */
#define SYS_IPRSTCTL1_EBI     ((uint32_t)0x00000008)      /*!<EBI reset */

/********************* Bit definition of IPRSTCTL2 register **********************/
#define SYS_IPRSTCTL2_GPIO      ((uint32_t)0x00000002)      /*!<GPIO reset */
#define SYS_IPRSTCTL2_TMR0      ((uint32_t)0x00000004)      /*!<Timer 0 reset */
#define SYS_IPRSTCTL2_TMR1      ((uint32_t)0x00000008)      /*!<Timer 1 reset */
#define SYS_IPRSTCTL2_TMR2      ((uint32_t)0x00000010)      /*!<Timer 2 reset */
#define SYS_IPRSTCTL2_TMR3      ((uint32_t)0x00000020)      /*!<Timer 3 reset */
#define SYS_IPRSTCTL2_SC2       ((uint32_t)0x00000080)      /*!<SC 2 reset */
#define SYS_IPRSTCTL2_I2C0      ((uint32_t)0x00000100)      /*!<I2C 0 reset */
#define SYS_IPRSTCTL2_I2C1      ((uint32_t)0x00000200)      /*!<I2C 1 reset */
#define SYS_IPRSTCTL2_SPI0      ((uint32_t)0x00001000)      /*!<SPI 0 reset */
#define SYS_IPRSTCTL2_SPI1      ((uint32_t)0x00002000)      /*!<SPI 1 reset */
#define SYS_IPRSTCTL2_SPI2      ((uint32_t)0x00004000)      /*!<SPI 2 reset */
#define SYS_IPRSTCTL2_UART0     ((uint32_t)0x00010000)      /*!<UART 0 reset */
#define SYS_IPRSTCTL2_UART1     ((uint32_t)0x00020000)      /*!<UART 1 reset */
#define SYS_IPRSTCTL2_PWM0      ((uint32_t)0x00100000)      /*!<PWM 0 reset */
#define SYS_IPRSTCTL2_TK        ((uint32_t)0x01000000)      /*!<TK reset */
#define SYS_IPRSTCTL2_DAC       ((uint32_t)0x02000000)      /*!<DAC reset */
#define SYS_IPRSTCTL2_LCD       ((uint32_t)0x04000000)      /*!<LCD reset */
#define SYS_IPRSTCTL2_USBD      ((uint32_t)0x08000000)      /*!<USB device reset */
#define SYS_IPRSTCTL2_ADC       ((uint32_t)0x10000000)      /*!<ADC reset */
#define SYS_IPRSTCTL2_I2S       ((uint32_t)0x20000000)      /*!<I2S reset */
#define SYS_IPRSTCTL2_SC0       ((uint32_t)0x40000000)      /*!<SC0 reset */
#define SYS_IPRSTCTL2_SC1       ((uint32_t)0x80000000)      /*!<SC1 reset */

/********************* Bit definition of BODCTL register **********************/
#define SYS_BODCTL_17EN         ((uint32_t)0x00000001)      /*!<Detector 1.7V Function Enable */
#define SYS_BODCTL_20EN         ((uint32_t)0x00000002)      /*!<Detector 2.0V Function Enable */
#define SYS_BODCTL_25EN         ((uint32_t)0x00000004)      /*!<Detector 2.5V Function Enable */
#define SYS_BODCTL_17RSTEN      ((uint32_t)0x00000010)      /*!<1.7V Reset Enable */
#define SYS_BODCTL_20RSTEN      ((uint32_t)0x00000020)      /*!<2.0V Reset Enable */
#define SYS_BODCTL_25RSTEN      ((uint32_t)0x00000040)      /*!<2.5V Reset Enable */
#define SYS_BODCTL_17INTEN      ((uint32_t)0x00000100)      /*!<1.7V Interrupt Enable */
#define SYS_BODCTL_20INTEN      ((uint32_t)0x00000200)      /*!<2.0V Interrupt Enable */
#define SYS_BODCTL_25INTEN      ((uint32_t)0x00000400)      /*!<2.5V Interrupt Enable */

/********************* Bit definition of VREFCTL register **********************/
#define SYS_VREFCTL_BGP_EN      ((uint32_t)0x00000001)      /*!<Band-gap Enable */
#define SYS_VREFCTL_REG_EN      ((uint32_t)0x00000002)      /*!<Regulator Enable */
#define SYS_VREFCTL_SEL25       ((uint32_t)0x00000004)      /*!<Regulator Output Voltage 2.5V */
#define SYS_VREFCTL_SEL18       ((uint32_t)0x00000000)      /*!<Regulator Output Voltage 1.8V */
#define SYS_VREFCTL_EXTMODE     ((uint32_t)0x00000008)      /*!<Regulator External Mode */


/********************* Bit definition of LDOCTL register **********************/
#define SYS_LDOCTL_LDO_PD       ((uint32_t)0x00000001)      /*!< LDO Disabled */
#define SYS_LDOCTL_LDO_LEVEL16  ((uint32_t)0x00000004)      /*!< LDO  Output 1.6  Voltage  */
#define SYS_LDOCTL_LDO_LEVEL18  ((uint32_t)0x00000008)      /*!< LDO  Output 1.8  Voltage  */


/********************* Bit definition of IRCTRIMCTL register **********************/
#define SYS_IRCTRIMCTL_SEL_MASK     ((uint32_t)0x00000003)      /*!<Trim frequency selection mask */
#define SYS_IRCTRIMCTL_TRIM_DISABLE ((uint32_t)0x00000000)      /*!<Disable auto-trim function */
#define SYS_IRCTRIMCTL_TRIM_11M     ((uint32_t)0x00000001)      /*!<Trim HIRC to 11.0592 MHz */
#define SYS_IRCTRIMCTL_TRIM_12M     ((uint32_t)0x00000002)      /*!<Trim HIRC to 12 MHz */
#define SYS_IRCTRIMCTL_TRIM_16M     ((uint32_t)0x00000003)      /*!<Trim HIRC to 16 MHz */

#define SYS_IRCTRIMCTL_LOOP_MASK  ((uint32_t)0x00000030)      /*!<Trim calculation loop mask */
#define SYS_IRCTRIMCTL_LOOP_4CLK  ((uint32_t)0x00000000)      /*!<Based on average difference in 4 x 32.768 kHz clock */
#define SYS_IRCTRIMCTL_LOOP_8CLK  ((uint32_t)0x00000010)      /*!<Based on average difference in 8 x 32.768 kHz clock */
#define SYS_IRCTRIMCTL_LOOP_16CLK ((uint32_t)0x00000020)      /*!<Based on average difference in 16 x 32.768 kHz clock */
#define SYS_IRCTRIMCTL_LOOP_32CLK ((uint32_t)0x00000030)      /*!<Based on average difference in 32 x 32.768 kHz clock */

#define SYS_IRCTRIMCTL_RETRY_COUNT  ((uint32_t)0x000000C0)      /*!<Trim value update limitation count mask */
#define SYS_IRCTRIMCTL_RETRY_64     ((uint32_t)0x00000000)      /*!<Trim retry count limitation is 64 */
#define SYS_IRCTRIMCTL_RETRY_128    ((uint32_t)0x00000040)      /*!<Trim retry count limitation is 128 */
#define SYS_IRCTRIMCTL_RETRY_256    ((uint32_t)0x00000080)      /*!<Trim retry count limitation is 256 */
#define SYS_IRCTRIMCTL_RETRY_512    ((uint32_t)0x000000C0)      /*!<Trim retry count limitation is 512 */

#define SYS_IRCTRIMCTL_CLKERR_STOP  ((uint32_t)0x00000100)      /*!<Clock error stop enable */

/********************* Bit definition of IRCTRIMIEN register **********************/
#define SYS_IRCTRIMIEN_FAIL_EN      ((uint32_t)0x00000002)      /*!<Trim failure interrupt enable */
#define SYS_IRCTRIMIEN_32KERR_EN    ((uint32_t)0x00000004)      /*!<32.768 kHz Clock Error Interrupt Enable */

/********************* Bit definition of IRCTRIMINT register **********************/
#define SYS_IRCTRIMINT_FREQLOCK     ((uint32_t)0x00000001)      /*!<HIRC frequency lock status */
#define SYS_IRCTRIMINT_FAIL_INT     ((uint32_t)0x00000002)      /*!<Trim failure interrupt status */
#define SYS_IRCTRIMINT_32KERR_ENNT  ((uint32_t)0x00000004)      /*!<32.768 kHz Clock Error Interrupt Status */

/********************* Bit definition of PA_L_MFP register **********************/
#define SYS_PA_L_MFP_PA7_MFP_GPA7         (0UL<<SYS_PA_L_MFP_PA7_MFP_Pos)     /*!<PA7 Pin Function - GPIOA[7] */
#define SYS_PA_L_MFP_PA7_MFP_ADC_CH7      (2UL<<SYS_PA_L_MFP_PA7_MFP_Pos)     /*!<PA7 Pin Function - ADC input channel 7 */
#define SYS_PA_L_MFP_PA7_MFP_SC1_CD       (4UL<<SYS_PA_L_MFP_PA7_MFP_Pos)     /*!<PA7 Pin Function - SmartCard 1 detect pin */


#define SYS_PA_L_MFP_PA6_MFP_GPA6         (0UL<<SYS_PA_L_MFP_PA6_MFP_Pos)     /*!<PA6 Pin Function - GPIOA[6] */
#define SYS_PA_L_MFP_PA6_MFP_ADC_CH6      (2UL<<SYS_PA_L_MFP_PA6_MFP_Pos)     /*!<PA6 Pin Function - ADC input channel 6 */
#define SYS_PA_L_MFP_PA6_MFP_ACMP0_O      (3UL<<SYS_PA_L_MFP_PA6_MFP_Pos)     /*!<PA6 Pin Function - Comparator0 output */
#define SYS_PA_L_MFP_PA6_MFP_SC0_RST      (4UL<<SYS_PA_L_MFP_PA6_MFP_Pos)     /*!<PA6 Pin Function - SmartCard0 RST pin */
#define SYS_PA_L_MFP_PA6_MFP_ACMP0_C      (9UL<<SYS_PA_L_MFP_PA6_MFP_Pos)     /*!<PA6 Pin Function - Comparator0 charge/discharge path */


#define SYS_PA_L_MFP_PA5_MFP_GPA5         (0UL<<SYS_PA_L_MFP_PA5_MFP_Pos)     /*!<PA5 Pin Function - GPIOA[5] */
#define SYS_PA_L_MFP_PA5_MFP_ADC_CH5      (2UL<<SYS_PA_L_MFP_PA5_MFP_Pos)     /*!<PA5 Pin Function - ADC input channel 5 */
#define SYS_PA_L_MFP_PA5_MFP_ACMP0_N      (3UL<<SYS_PA_L_MFP_PA5_MFP_Pos)     /*!<PA5 Pin Function - Comparator0 N-end input0 */
#define SYS_PA_L_MFP_PA5_MFP_SC0_PWR      (4UL<<SYS_PA_L_MFP_PA5_MFP_Pos)     /*!<PA5 Pin Function - SmartCard0 Power pin */
#define SYS_PA_L_MFP_PA5_MFP_I2C1_SDA     (5UL<<SYS_PA_L_MFP_PA5_MFP_Pos)     /*!<PA5 Pin Function - I2C1 data I/O pin */
#define SYS_PA_L_MFP_PA5_MFP_SPI1_SS0     (6UL<<SYS_PA_L_MFP_PA5_MFP_Pos)     /*!<PA5 Pin Function - SPI1 1st chip selection */
#define SYS_PA_L_MFP_PA5_MFP_ACMP0_C      (9UL<<SYS_PA_L_MFP_PA5_MFP_Pos)     /*!<PA5 Pin Function - Comparator0 charge/discharge path */

#define SYS_PA_L_MFP_PA4_MFP_GPA4         (0UL<<SYS_PA_L_MFP_PA4_MFP_Pos)     /*!<PA4 Pin Function - GPIOA[4] */
#define SYS_PA_L_MFP_PA4_MFP_ADC_CH4      (2UL<<SYS_PA_L_MFP_PA4_MFP_Pos)     /*!<PA4 Pin Function - ADC input channel 4 */
#define SYS_PA_L_MFP_PA4_MFP_ACMP0_P      (3UL<<SYS_PA_L_MFP_PA4_MFP_Pos)     /*!<PA4 Pin Function - Comparator0 P-end input0 */
#define SYS_PA_L_MFP_PA4_MFP_SC0_CD       (4UL<<SYS_PA_L_MFP_PA4_MFP_Pos)     /*!<PA4 Pin Function - SmartCard0 card detect pin */
#define SYS_PA_L_MFP_PA4_MFP_ACMP0_C      (9UL<<SYS_PA_L_MFP_PA4_MFP_Pos)     /*!<PA4 Pin Function - Comparator0 charge/discharge path */

#define SYS_PA_L_MFP_PA3_MFP_GPA3         (0UL<<SYS_PA_L_MFP_PA3_MFP_Pos)     /*!<PA3 Pin Function - GPIOA[3] */
#define SYS_PA_L_MFP_PA3_MFP_EINT1        (1UL<<SYS_PA_L_MFP_PA3_MFP_Pos)     /*!<PA3 Pin Function - External interrupt 1 */
#define SYS_PA_L_MFP_PA3_MFP_ADC_CH3      (2UL<<SYS_PA_L_MFP_PA3_MFP_Pos)     /*!<PA3 Pin Function - ADC input channel 3 */
#define SYS_PA_L_MFP_PA3_MFP_ACMP0_P      (3UL<<SYS_PA_L_MFP_PA3_MFP_Pos)     /*!<PA3 Pin Function - Comparator0 P-end input1 */
#define SYS_PA_L_MFP_PA3_MFP_SC0_DAT      (4UL<<SYS_PA_L_MFP_PA3_MFP_Pos)     /*!<PA3 Pin Function - SmartCard0 DATA */
#define SYS_PA_L_MFP_PA3_MFP_ACMP0_C      (9UL<<SYS_PA_L_MFP_PA3_MFP_Pos)     /*!<PA3 Pin Function - Comparator0 charge/discharge path */

#define SYS_PA_L_MFP_PA2_MFP_GPA2         (0UL<<SYS_PA_L_MFP_PA2_MFP_Pos)     /*!<PA2 Pin Function - GPIOA[2] */
#define SYS_PA_L_MFP_PA2_MFP_EINT0        (1UL<<SYS_PA_L_MFP_PA2_MFP_Pos)     /*!<PA2 Pin Function - External interrupt 0 */
#define SYS_PA_L_MFP_PA2_MFP_ADC_CH2      (2UL<<SYS_PA_L_MFP_PA2_MFP_Pos)     /*!<PA2 Pin Function - ADC input channel 2 */
#define SYS_PA_L_MFP_PA2_MFP_ACMP0_P      (3UL<<SYS_PA_L_MFP_PA2_MFP_Pos)     /*!<PA2 Pin Function - Comparator0 P-end input3 */
#define SYS_PA_L_MFP_PA2_MFP_SC0_CLK      (4UL<<SYS_PA_L_MFP_PA2_MFP_Pos)     /*!<PA2 Pin Function - SmartCard0 clock */
#define SYS_PA_L_MFP_PA2_MFP_ACMP0_C      (9UL<<SYS_PA_L_MFP_PA2_MFP_Pos)     /*!<PA2 Pin Function - Comparator0 charge/discharge path */

#define SYS_PA_L_MFP_PA1_MFP_GPA1         (0UL<<SYS_PA_L_MFP_PA1_MFP_Pos)     /*!<PA1 Pin Function - GPIOA[1] */
#define SYS_PA_L_MFP_PA1_MFP_ADC_CH1      (2UL<<SYS_PA_L_MFP_PA1_MFP_Pos)     /*!<PA1 Pin Function - ADC input channel 1 */
#define SYS_PA_L_MFP_PA1_MFP_ACMP0_P      (3UL<<SYS_PA_L_MFP_PA1_MFP_Pos)     /*!<PA1 Pin Function - Comparator0 P-end input3 */
#define SYS_PA_L_MFP_PA1_MFP_ACMP0_C      (9UL<<SYS_PA_L_MFP_PA1_MFP_Pos)     /*!<PA1 Pin Function - Comparator0 charge/discharge path */

#define SYS_PA_L_MFP_PA0_MFP_GPA0         (0UL<<SYS_PA_L_MFP_PA1_MFP_Pos)     /*!<PA0 Pin Function - GPIOA[0] */
#define SYS_PA_L_MFP_PA0_MFP_ADC_CH0      (2UL<<SYS_PA_L_MFP_PA1_MFP_Pos)     /*!<PA0 Pin Function - ADC input channel 0 */

/********************* Bit definition of PA_H_MFP register **********************/
#define SYS_PA_H_MFP_PA15_MFP_GPA15       (0UL<<SYS_PA_H_MFP_PA15_MFP_Pos)    /*!<PA15 Pin Function - GPIOA[15] */
#define SYS_PA_H_MFP_PA15_MFP_TMR3_CAP    (2UL<<SYS_PA_H_MFP_PA15_MFP_Pos)    /*!<PA15 Pin Function - Timer1 capture event */
#define SYS_PA_H_MFP_PA15_MFP_ACMP1_O     (3UL<<SYS_PA_H_MFP_PA15_MFP_Pos)    /*!<PA15 Pin Function - Comparator1 output */
#define SYS_PA_H_MFP_PA15_MFP_I2C1_SDA    (5UL<<SYS_PA_H_MFP_PA15_MFP_Pos)    /*!<PA15 Pin Function - I2C1 data I/O pin */
#define SYS_PA_H_MFP_PA15_MFP_SPI1_SS0    (6UL<<SYS_PA_H_MFP_PA15_MFP_Pos)    /*!<PA15 Pin Function - SPI1 1st chip selection */
#define SYS_PA_H_MFP_PA15_MFP_LCD_S16     (8UL<<SYS_PA_H_MFP_PA15_MFP_Pos)    /*!<PA15 Pin Function - LCD SEG 16 */
#define SYS_PA_H_MFP_PA15_MFP_LCD_S30     (8UL<<SYS_PA_H_MFP_PA15_MFP_Pos)    /*!<PA15 Pin Function - LCD SEG 30 */

#define SYS_PA_H_MFP_PA14_MFP_GPA14       (0UL<<SYS_PA_H_MFP_PA14_MFP_Pos)    /*!<PA14 Pin Function - GPIOA[14] */
#define SYS_PA_H_MFP_PA14_MFP_I2C1_SCL    (5UL<<SYS_PA_H_MFP_PA14_MFP_Pos)    /*!<PA14 Pin Function - I2C1 clock pin */
#define SYS_PA_H_MFP_PA14_MFP_SPI1_SCLK   (6UL<<SYS_PA_H_MFP_PA14_MFP_Pos)    /*!<PA14 Pin Function - SPI1 clock pin */
#define SYS_PA_H_MFP_PA14_MFP_LCD_S17     (8UL<<SYS_PA_H_MFP_PA14_MFP_Pos)    /*!<PA14 Pin Function - LCD SEG 17 */
#define SYS_PA_H_MFP_PA14_MFP_LCD_S31     (8UL<<SYS_PA_H_MFP_PA14_MFP_Pos)    /*!<PA14 Pin Function - LCD SEG 31 */
#define SYS_PA_H_MFP_PA14_MFP_ACMP0_C     (9UL<<SYS_PA_H_MFP_PA14_MFP_Pos)    /*!<PA14 Pin Function - Comparator0 charge/discharge path */

#define SYS_PA_H_MFP_PA13_MFP_GPA13       (0UL<<SYS_PA_H_MFP_PA13_MFP_Pos)    /*!<PA13 Pin Function - GPIOA[13] */
#define SYS_PA_H_MFP_PA13_MFP_ACMP1_N     (3UL<<SYS_PA_H_MFP_PA13_MFP_Pos)    /*!<PA13 Pin Function - Comparator1 N-end input */
#define SYS_PA_H_MFP_PA13_MFP_I2C0_SDA    (5UL<<SYS_PA_H_MFP_PA13_MFP_Pos)    /*!<PA13 Pin Function - I2C0 data I/O pin*/
#define SYS_PA_H_MFP_PA13_MFP_SPI1_MISO0  (6UL<<SYS_PA_H_MFP_PA13_MFP_Pos)    /*!<PA13 Pin Function - SPI1 MISO[0] */
#define SYS_PA_H_MFP_PA13_MFP_UART0_RX    (7UL<<SYS_PA_H_MFP_PA13_MFP_Pos)    /*!<PA13 Pin Function - UART1 RX */
#define SYS_PA_H_MFP_PA13_MFP_LCD_S18     (8UL<<SYS_PA_H_MFP_PA13_MFP_Pos)    /*!<PA13 Pin Function - LCD SEG 18 */

#define SYS_PA_H_MFP_PA12_MFP_GPA12       (0UL<<SYS_PA_H_MFP_PA12_MFP_Pos)    /*!<PA12 Pin Function - GPIOA[12] */
#define SYS_PA_H_MFP_PA12_MFP_ACMP1_P     (3UL<<SYS_PA_H_MFP_PA12_MFP_Pos)    /*!<PA12 Pin Function - Comparator1 N-end input */
#define SYS_PA_H_MFP_PA12_MFP_I2C0_SCL    (5UL<<SYS_PA_H_MFP_PA12_MFP_Pos)    /*!<PA12 Pin Function - I2C0 clock pin */
#define SYS_PA_H_MFP_PA12_MFP_SPI1_MOSI0  (6UL<<SYS_PA_H_MFP_PA12_MFP_Pos)    /*!<PA12 Pin Function - SPI1 MOSI[0] */
#define SYS_PA_H_MFP_PA12_MFP_UART0_TX    (7UL<<SYS_PA_H_MFP_PA12_MFP_Pos)    /*!<PA12 Pin Function - UART1 TX */
#define SYS_PA_H_MFP_PA12_MFP_LCD_S19     (8UL<<SYS_PA_H_MFP_PA12_MFP_Pos)    /*!<PA12 Pin Function - LCD SEG 19 */

#define SYS_PA_H_MFP_PA11_MFP_GPA11       (0UL<<SYS_PA_H_MFP_PA11_MFP_Pos)    /*!<PA11 Pin Function - GPIOA[11] */
#define SYS_PA_H_MFP_PA11_MFP_ADC_EXT     (2UL<<SYS_PA_H_MFP_PA11_MFP_Pos)    /*!<PA11 Pin Function - ADC external trigger */
#define SYS_PA_H_MFP_PA11_MFP_SC0_DAT     (4UL<<SYS_PA_H_MFP_PA11_MFP_Pos)    /*!<PA11 Pin Function - SmartCard0 DATA */

#define SYS_PA_H_MFP_PA10_MFP_GPA10       (0UL<<SYS_PA_H_MFP_PA10_MFP_Pos)    /*!<PA10 Pin Function - GPIOA[10] */
#define SYS_PA_H_MFP_PA10_MFP_SC0_CLK     (4UL<<SYS_PA_H_MFP_PA10_MFP_Pos)    /*!<PA10 Pin Function - SmartCard0 clock */

#define SYS_PA_H_MFP_PA9_MFP_GPA9         (0UL<<SYS_PA_H_MFP_PA9_MFP_Pos)     /*!<PA9 Pin Function - GPIOA[9] */
#define SYS_PA_H_MFP_PA9_MFP_SC0_RST      (4UL<<SYS_PA_H_MFP_PA9_MFP_Pos)     /*!<PA9 Pin Function - SmartCard0 RST */

#define SYS_PA_H_MFP_PA8_MFP_GPA8         (0UL<<SYS_PA_H_MFP_PA8_MFP_Pos)     /*!<PA8 Pin Function - GPIOA[8] */
#define SYS_PA_H_MFP_PA8_MFP_SC0_PWR      (4UL<<SYS_PA_H_MFP_PA8_MFP_Pos)     /*!<PA8 Pin Function - SmartCard0 power */

/********************* Bit definition of PB_L_MFP register **********************/
#define SYS_PB_L_MFP_PB7_MFP_GPB7         (0UL<<SYS_PB_L_MFP_PB7_MFP_Pos)     /*!<PB7 Pin Function - GPIOB[7] */
#define SYS_PB_L_MFP_PB7_MFP_SC0_CD       (4UL<<SYS_PB_L_MFP_PB7_MFP_Pos)     /*!<PB7 Pin Function - SmartCard0 card detect */
#define SYS_PB_L_MFP_PB7_MFP_UART1_CTS    (7UL<<SYS_PB_L_MFP_PB7_MFP_Pos)     /*!<PB7 Pin Function - UART1 CTSn */
#define SYS_PB_L_MFP_PB7_MFP_LCD_S33      (8UL<<SYS_PB_L_MFP_PB7_MFP_Pos)     /*!<PB7 Pin Function - LCD SEG 33 */

#define SYS_PB_L_MFP_PB6_MFP_GPB6         (0UL<<SYS_PB_L_MFP_PB6_MFP_Pos)     /*!<PB6 Pin Function - GPIOB[6] */
#define SYS_PB_L_MFP_PB6_MFP_CKO0         (1UL<<SYS_PB_L_MFP_PB6_MFP_Pos)     /*!<PB6 Pin Function - Frequency Divider0 output pin */
#define SYS_PB_L_MFP_PB6_MFP_SPI1_SS1     (6UL<<SYS_PB_L_MFP_PB6_MFP_Pos)     /*!<PB6 Pin Function - SPI2 2nd slave select pin */
#define SYS_PB_L_MFP_PB6_MFP_UART1_TX     (7UL<<SYS_PB_L_MFP_PB6_MFP_Pos)     /*!<PB6 Pin Function - UART1 TX */
#define SYS_PB_L_MFP_PB6_MFP_LCD_S25      (8UL<<SYS_PB_L_MFP_PB6_MFP_Pos)     /*!<PB6 Pin Function - LCD SEG 25 */
#define SYS_PB_L_MFP_PB6_MFP_LCD_S34      (8UL<<SYS_PB_L_MFP_PB6_MFP_Pos)     /*!<PB6 Pin Function - LCD SEG 34 */

#define SYS_PB_L_MFP_PB5_MFP_GPB5         (0UL<<SYS_PB_L_MFP_PB5_MFP_Pos)     /*!<PB5 Pin Function - GPIOB[5] */
#define SYS_PB_L_MFP_PB5_MFP_SPI1_MOSI1   (6UL<<SYS_PB_L_MFP_PB5_MFP_Pos)     /*!<PB5 Pin Function - SPI1 MOSI[1] */
#define SYS_PB_L_MFP_PB5_MFP_UART1_RX     (7UL<<SYS_PB_L_MFP_PB5_MFP_Pos)     /*!<PB5 Pin Function - UART1 RX */
#define SYS_PB_L_MFP_PB5_MFP_LCD_S35      (8UL<<SYS_PB_L_MFP_PB5_MFP_Pos)     /*!<PB5 Pin Function - LCD SEG 35 */

#define SYS_PB_L_MFP_PB4_MFP_GPB4         (0UL<<SYS_PB_L_MFP_PB4_MFP_Pos)     /*!<PB4 Pin Function - GPIOB[4] */
#define SYS_PB_L_MFP_PB4_MFP_SPI1_MISO1   (6UL<<SYS_PB_L_MFP_PB4_MFP_Pos)     /*!<PB4 Pin Function - SPI2 MISO[1] */
#define SYS_PB_L_MFP_PB4_MFP_UART1_RTS    (7UL<<SYS_PB_L_MFP_PB4_MFP_Pos)     /*!<PB4 Pin Function - UART1 RTS */

#define SYS_PB_L_MFP_PB3_MFP_GPB3         (0UL<<SYS_PB_L_MFP_PB3_MFP_Pos)     /*!<PB3 Pin Function - GPIOB[3] */
#define SYS_PB_L_MFP_PB3_MFP_TMR2_CNT     (2UL<<SYS_PB_L_MFP_PB3_MFP_Pos)     /*!<PB3 Pin Function - Timer2 external counter input  */
#define SYS_PB_L_MFP_PB3_MFP_I2C0_SDA     (5UL<<SYS_PB_L_MFP_PB3_MFP_Pos)     /*!<PB3 Pin Function - I2C0 data I/O pin */
#define SYS_PB_L_MFP_PB3_MFP_SPI1_MISO1   (6UL<<SYS_PB_L_MFP_PB3_MFP_Pos)     /*!<PB3 Pin Function - SPI1 MISO[1] */
#define SYS_PB_L_MFP_PB3_MFP_UART0_CTS    (7UL<<SYS_PB_L_MFP_PB3_MFP_Pos)     /*!<PB3 Pin Function - UART0 CTSn */
#define SYS_PB_L_MFP_PB3_MFP_LCD_S26      (8UL<<SYS_PB_L_MFP_PB3_MFP_Pos)     /*!<PB3 Pin Function - LCD SEG 26 */
#define SYS_PB_L_MFP_PB3_MFP_TMR2_OUT     (0xAUL<<SYS_PB_L_MFP_PB3_MFP_Pos)   /*!<PB3 Pin Function - Timer2 toggle out */

#define SYS_PB_L_MFP_PB2_MFP_GPB2         (0UL<<SYS_PB_L_MFP_PB2_MFP_Pos)     /*!<PB2 Pin Function - GPIOB[2] */
#define SYS_PB_L_MFP_PB2_MFP_TMR3_CNT     (2UL<<SYS_PB_L_MFP_PB2_MFP_Pos)     /*!<PB2 Pin Function - Timer3 external counter input  */
#define SYS_PB_L_MFP_PB2_MFP_I2C0_SCL     (5UL<<SYS_PB_L_MFP_PB2_MFP_Pos)     /*!<PB2 Pin Function - I2C0 clock pin */
#define SYS_PB_L_MFP_PB2_MFP_SPI1_MOSI1   (6UL<<SYS_PB_L_MFP_PB2_MFP_Pos)     /*!<PB2 Pin Function - SPI1 MOSI[1] */
#define SYS_PB_L_MFP_PB2_MFP_UART0_RTS    (7UL<<SYS_PB_L_MFP_PB2_MFP_Pos)     /*!<PB2 Pin Function - UART0 RTSn */
#define SYS_PB_L_MFP_PB2_MFP_LCD_S27      (8UL<<SYS_PB_L_MFP_PB2_MFP_Pos)     /*!<PB2 Pin Function - LCD SEG 27 */
#define SYS_PB_L_MFP_PB2_MFP_TMR3_OUT     (0xAUL<<SYS_PB_L_MFP_PB2_MFP_Pos)   /*!<PB2 Pin Function - Timer3 toggle out */

#define SYS_PB_L_MFP_PB1_MFP_GPB1         (0UL<<SYS_PB_L_MFP_PB1_MFP_Pos)     /*!<PB1 Pin Function - GPIOB[1] */
#define SYS_PB_L_MFP_PB1_MFP_EINT1        (1UL<<SYS_PB_L_MFP_PB1_MFP_Pos)     /*!<PB1 Pin Function - External interrupt1 input pin */
#define SYS_PB_L_MFP_PB1_MFP_TMR2_CAP     (2UL<<SYS_PB_L_MFP_PB1_MFP_Pos)     /*!<PB1 Pin Function - Timer2 capture event */
#define SYS_PB_L_MFP_PB1_MFP_UART0_RX     (7UL<<SYS_PB_L_MFP_PB1_MFP_Pos)     /*!<PB1 Pin Function - UART0 RX */
#define SYS_PB_L_MFP_PB1_MFP_LCD_S28      (8UL<<SYS_PB_L_MFP_PB1_MFP_Pos)     /*!<PB1 Pin Function - LCD SEG 28 */

#define SYS_PB_L_MFP_PB0_MFP_GPB0         (0UL<<SYS_PB_L_MFP_PB0_MFP_Pos)     /*!<PB0 Pin Function - GPIOB[0] */
#define SYS_PB_L_MFP_PB0_MFP_CKO1         (1UL<<SYS_PB_L_MFP_PB0_MFP_Pos)     /*!<PB0 Pin Function - Frequency Divider1 output pin */
#define SYS_PB_L_MFP_PB0_MFP_UART0_TX     (7UL<<SYS_PB_L_MFP_PB0_MFP_Pos)     /*!<PB0 Pin Function - UART0 TX */
#define SYS_PB_L_MFP_PB0_MFP_LCD_S29      (8UL<<SYS_PB_L_MFP_PB0_MFP_Pos)     /*!<PB0 Pin Function - LCD SEG 29 */

/********************* Bit definition of PB_H_MFP register **********************/
#define SYS_PB_H_MFP_PB15_MFP_GPB15       (0UL<<SYS_PB_H_MFP_PB15_MFP_Pos)    /*!<PB15 Pin Function - GPIOB[15] */
#define SYS_PB_H_MFP_PB15_MFP_SPI0_SS0    (6UL<<SYS_PB_H_MFP_PB15_MFP_Pos)    /*!<PB15 Pin Function - SPI0 1st slave select pin */
#define SYS_PB_H_MFP_PB15_MFP_UART0_CTS   (7UL<<SYS_PB_H_MFP_PB15_MFP_Pos)    /*!<PB15 Pin Function - UART0 CTSn */
#define SYS_PB_H_MFP_PB15_MFP_LCD_S12     (8UL<<SYS_PB_H_MFP_PB15_MFP_Pos)    /*!<PB15 Pin Function - LCD SEG 12 */
#define SYS_PB_H_MFP_PB15_MFP_LCD_S19     (8UL<<SYS_PB_H_MFP_PB15_MFP_Pos)    /*!<PB15 Pin Function - LCD SEG 19 */
#define SYS_PB_H_MFP_PB15_MFP_LCD_S23     (8UL<<SYS_PB_H_MFP_PB15_MFP_Pos)    /*!<PB15 Pin Function - LCD SEG 23 */

#define SYS_PB_H_MFP_PB14_MFP_GPB14       (0UL<<SYS_PB_H_MFP_PB14_MFP_Pos)    /*!<PB14 Pin Function - GPIOB[14] */
#define SYS_PB_H_MFP_PB14_MFP_SPI0_SCLK   (6UL<<SYS_PB_H_MFP_PB14_MFP_Pos)    /*!<PB15 Pin Function - SPI0 serial clock pin */
#define SYS_PB_H_MFP_PB14_MFP_UART0_TX    (7UL<<SYS_PB_H_MFP_PB14_MFP_Pos)    /*!<PB15 Pin Function - UART0 TX */
#define SYS_PB_H_MFP_PB14_MFP_LCD_S13     (8UL<<SYS_PB_H_MFP_PB14_MFP_Pos)    /*!<PB14 Pin Function - LCD SEG 13 */
#define SYS_PB_H_MFP_PB14_MFP_LCD_S20     (8UL<<SYS_PB_H_MFP_PB14_MFP_Pos)    /*!<PB14 Pin Function - LCD SEG 20 */
#define SYS_PB_H_MFP_PB14_MFP_LCD_S24     (8UL<<SYS_PB_H_MFP_PB14_MFP_Pos)    /*!<PB14 Pin Function - LCD SEG 24 */

#define SYS_PB_H_MFP_PB13_MFP_GPB13       (0UL<<SYS_PB_H_MFP_PB13_MFP_Pos)    /*!<PB13 Pin Function - GPIOB[13] */
#define SYS_PB_H_MFP_PB13_MFP_SPI0_MISO0  (6UL<<SYS_PB_H_MFP_PB13_MFP_Pos)    /*!<PB13 Pin Function - SPI0 MISO[0] */
#define SYS_PB_H_MFP_PB13_MFP_UART0_RX    (7UL<<SYS_PB_H_MFP_PB13_MFP_Pos)    /*!<PB13 Pin Function - UART0 RX */
#define SYS_PB_H_MFP_PB13_MFP_LCD_S14     (8UL<<SYS_PB_H_MFP_PB13_MFP_Pos)    /*!<PB13 Pin Function - LCD SEG 14 */
#define SYS_PB_H_MFP_PB13_MFP_LCD_S21     (8UL<<SYS_PB_H_MFP_PB13_MFP_Pos)    /*!<PB13 Pin Function - LCD SEG 21 */
#define SYS_PB_H_MFP_PB13_MFP_LCD_S25     (8UL<<SYS_PB_H_MFP_PB13_MFP_Pos)    /*!<PB13 Pin Function - LCD SEG 25 */

#define SYS_PB_H_MFP_PB12_MFP_GPB12       (0UL<<SYS_PB_H_MFP_PB12_MFP_Pos)    /*!<PB12 Pin Function - GPIOB[12] */
#define SYS_PB_H_MFP_PB12_MFP_CKO0        (1UL<<SYS_PB_H_MFP_PB12_MFP_Pos)    /*!<PB12 Pin Function - Frequency Divider0 output pin */
#define SYS_PB_H_MFP_PB12_MFP_TMR0_CNT    (2UL<<SYS_PB_H_MFP_PB12_MFP_Pos)    /*!<PB12 Pin Function - Timer0 external counter input */
#define SYS_PB_H_MFP_PB12_MFP_SPI0_MOSI0  (6UL<<SYS_PB_H_MFP_PB12_MFP_Pos)    /*!<PB12 Pin Function - SPI0 MOSI[0] */
#define SYS_PB_H_MFP_PB12_MFP_UART0_RTS   (7UL<<SYS_PB_H_MFP_PB12_MFP_Pos)    /*!<PB12 Pin Function - UART0 RTSn */
#define SYS_PB_H_MFP_PB12_MFP_LCD_S15     (8UL<<SYS_PB_H_MFP_PB12_MFP_Pos)    /*!<PB12 Pin Function - LCD SEG 15 */
#define SYS_PB_H_MFP_PB12_MFP_LCD_S22     (8UL<<SYS_PB_H_MFP_PB12_MFP_Pos)    /*!<PB12 Pin Function - LCD SEG 22 */
#define SYS_PB_H_MFP_PB12_MFP_LCD_S26     (8UL<<SYS_PB_H_MFP_PB12_MFP_Pos)    /*!<PB12 Pin Function - LCD SEG 26 */
#define SYS_PB_H_MFP_PB12_MFP_TMR0_OUT    (0xAUL<<SYS_PB_H_MFP_PB12_MFP_Pos)  /*!<PB12 Pin Function - Timer0 toggle out */

#define SYS_PB_H_MFP_PB11_MFP_GPB11       (0UL<<SYS_PB_H_MFP_PB11_MFP_Pos)   /*!<PB11 Pin Function - GPIOB[11] */
#define SYS_PB_H_MFP_PB11_MFP_TMR1_CNT    (2UL<<SYS_PB_H_MFP_PB11_MFP_Pos)   /*!<PB11 Pin Function - Timer1 external event input */
#define SYS_PB_H_MFP_PB11_MFP_SPI0_MISO1  (6UL<<SYS_PB_H_MFP_PB11_MFP_Pos)   /*!<PB11 Pin Function - SPI0 MISO[1] */
#define SYS_PB_H_MFP_PB11_MFP_UART1_RTS   (7UL<<SYS_PB_H_MFP_PB11_MFP_Pos)   /*!<PB11 Pin Function - UART1 RTSn */
#define SYS_PB_H_MFP_PB11_MFP_LCD_S23     (8UL<<SYS_PB_H_MFP_PB11_MFP_Pos)   /*!<PB11 Pin Function - LCD SEG 23 */
#define SYS_PB_H_MFP_PB11_MFP_LCD_S27     (8UL<<SYS_PB_H_MFP_PB11_MFP_Pos)   /*!<PB11 Pin Function - LCD SEG 27 */
#define SYS_PB_H_MFP_PB11_MFP_TMR1_OUT    (0xAUL<<SYS_PB_H_MFP_PB11_MFP_Pos) /*!<PB11 Pin Function - Timer1 toggle out */

#define SYS_PB_H_MFP_PB10_MFP_GPB10       (0UL<<SYS_PB_H_MFP_PB10_MFP_Pos)  /*!<PB10 Pin Function - GPIOB[10] */
#define SYS_PB_H_MFP_PB10_MFP_SPI0_MOSI1  (6UL<<SYS_PB_H_MFP_PB10_MFP_Pos)  /*!<PB10 Pin Function - SPI0 MOSI[1] */
#define SYS_PB_H_MFP_PB10_MFP_UART1_RX    (7UL<<SYS_PB_H_MFP_PB10_MFP_Pos)  /*!<PB10 Pin Function - UART1 RX */
#define SYS_PB_H_MFP_PB10_MFP_LCD_S24     (8UL<<SYS_PB_H_MFP_PB10_MFP_Pos)  /*!<PB10 Pin Function - LCD SEG 24 */
#define SYS_PB_H_MFP_PB10_MFP_LCD_S28     (8UL<<SYS_PB_H_MFP_PB10_MFP_Pos)  /*!<PB10 Pin Function - LCD SEG 28 */

#define SYS_PB_H_MFP_PB9_MFP_GPB9         (0UL<<SYS_PB_H_MFP_PB9_MFP_Pos) /*!<PB9 Pin Function - GPIOB[9] */
#define SYS_PB_H_MFP_PB9_MFP_PWM0_CH1     (3UL<<SYS_PB_H_MFP_PB9_MFP_Pos) /*!<PB9 Pin Function - PWM0 Channel1 output */
#define SYS_PB_H_MFP_PB9_MFP_LCD_S31      (8UL<<SYS_PB_H_MFP_PB9_MFP_Pos) /*!<PB9 Pin Function - LCD SEG 31 */

#define SYS_PB_H_MFP_PB8_MFP_GPB8         (0UL<<SYS_PB_H_MFP_PB8_MFP_Pos) /*!<PB8 Pin Function - GPIOB[8] */
#define SYS_PB_H_MFP_PB8_MFP_EINT1        (1UL<<SYS_PB_H_MFP_PB8_MFP_Pos) /*!<PB8 Pin Function - External interrupt1 input pin */
#define SYS_PB_H_MFP_PB8_MFP_TMR0_CNT     (2UL<<SYS_PB_H_MFP_PB8_MFP_Pos) /*!<PB8 Pin Function - Timer0 external counter input */
#define SYS_PB_H_MFP_PB8_MFP_PWM0_CH0     (3UL<<SYS_PB_H_MFP_PB8_MFP_Pos) /*!<PB8 Pin Function - PWM0 Channel0 output */
#define SYS_PB_H_MFP_PB8_MFP_SNOOPER      (4UL<<SYS_PB_H_MFP_PB8_MFP_Pos) /*!<PB8 Pin Function - Snooper pin */
#define SYS_PB_H_MFP_PB8_MFP_LCD_S32      (8UL<<SYS_PB_H_MFP_PB8_MFP_Pos) /*!<PB8 Pin Function - LCD SEG 32 */
#define SYS_PB_H_MFP_PB8_MFP_TMR0_OUT     (0xAUL<<SYS_PB_H_MFP_PB8_MFP_Pos) /*!<PB8 Pin Function - Timer0 toggle out */

/********************* Bit definition of PC_L_MFP register **********************/
#define SYS_PC_L_MFP_PC7_MFP_GPC7         (0UL<<SYS_PC_L_MFP_PC7_MFP_Pos)     /*!<PC7 Pin Function - GPIOC[7] */
#define SYS_PC_L_MFP_PC7_MFP_SC0_PWR      (4UL<<SYS_PC_L_MFP_PC7_MFP_Pos)     /*!<PC7 Pin Function - SmartCard0 Power pin */
#define SYS_PC_L_MFP_PC7_MFP_UART1_RX     (7UL<<SYS_PC_L_MFP_PC7_MFP_Pos)     /*!<PC7 Pin Function - UART1 RX */
#define SYS_PC_L_MFP_PC7_MFP_LCD_S4       (8UL<<SYS_PC_L_MFP_PC7_MFP_Pos)     /*!<PC7 Pin Function - LCD SEG 4 */
#define SYS_PC_L_MFP_PC7_MFP_LCD_S11      (8UL<<SYS_PC_L_MFP_PC7_MFP_Pos)     /*!<PC7 Pin Function - LCD SEG 11 */
#define SYS_PC_L_MFP_PC7_MFP_LCD_S15      (8UL<<SYS_PC_L_MFP_PC7_MFP_Pos)     /*!<PC7 Pin Function - LCD SEG 15 */

#define SYS_PC_L_MFP_PC6_MFP_GPC6         (0UL<<SYS_PC_L_MFP_PC6_MFP_Pos)     /*!<PC6 Pin Function - GPIOC[6] */
#define SYS_PC_L_MFP_PC6_MFP_SC0_DAT      (4UL<<SYS_PC_L_MFP_PC6_MFP_Pos)     /*!<PC6 Pin Function - SmartCard0 data */
#define SYS_PC_L_MFP_PC6_MFP_UART1_RTS    (7UL<<SYS_PC_L_MFP_PC6_MFP_Pos)     /*!<PC6 Pin Function - UART1 RTSn */
#define SYS_PC_L_MFP_PC6_MFP_LCD_S5       (8UL<<SYS_PC_L_MFP_PC6_MFP_Pos)     /*!<PC6 Pin Function - LCD SEG 5 */
#define SYS_PC_L_MFP_PC6_MFP_LCD_S12      (8UL<<SYS_PC_L_MFP_PC6_MFP_Pos)     /*!<PC6 Pin Function - LCD SEG 12 */
#define SYS_PC_L_MFP_PC6_MFP_LCD_S16      (8UL<<SYS_PC_L_MFP_PC6_MFP_Pos)     /*!<PC6 Pin Function - LCD SEG 16 */

#define SYS_PC_L_MFP_PC5_MFP_GPC5         (0UL<<SYS_PC_L_MFP_PC5_MFP_Pos)     /*!<PC5 Pin Function - GPIOC[5] */
#define SYS_PC_L_MFP_PC5_MFP_SC0_CD       (4UL<<SYS_PC_L_MFP_PC5_MFP_Pos)     /*!<PC5 Pin Function - SmartCard0 card detect pin */
#define SYS_PC_L_MFP_PC5_MFP_LCD_S6       (8UL<<SYS_PC_L_MFP_PC5_MFP_Pos)     /*!<PC5 Pin Function - LCD SEG 6 */
#define SYS_PC_L_MFP_PC5_MFP_LCD_S13      (8UL<<SYS_PC_L_MFP_PC5_MFP_Pos)     /*!<PC5 Pin Function - LCD SEG 13 */
#define SYS_PC_L_MFP_PC5_MFP_LCD_S17      (8UL<<SYS_PC_L_MFP_PC5_MFP_Pos)     /*!<PC5 Pin Function - LCD SEG 17 */

#define SYS_PC_L_MFP_PC4_MFP_GPC4         (0UL<<SYS_PC_L_MFP_PC4_MFP_Pos)     /*!<PC4 Pin Function - GPIOC[4] */
#define SYS_PC_L_MFP_PC4_MFP_EINT0        (1UL<<SYS_PC_L_MFP_PC4_MFP_Pos)     /*!<PC4 Pin Function - External interrupt0 input pin */
#define SYS_PC_L_MFP_PC4_MFP_SC0_CLK      (4UL<<SYS_PC_L_MFP_PC4_MFP_Pos)     /*!<PC4 Pin Function - SmartCard0 clock */
#define SYS_PC_L_MFP_PC4_MFP_UART1_CTS    (7UL<<SYS_PC_L_MFP_PC4_MFP_Pos)     /*!<PC4 Pin Function - UART1 CTSn */
#define SYS_PC_L_MFP_PC4_MFP_LCD_S7       (8UL<<SYS_PC_L_MFP_PC4_MFP_Pos)     /*!<PC4 Pin Function - LCD SEG 7 */
#define SYS_PC_L_MFP_PC4_MFP_LCD_S14      (8UL<<SYS_PC_L_MFP_PC4_MFP_Pos)     /*!<PC4 Pin Function - LCD SEG 14 */
#define SYS_PC_L_MFP_PC4_MFP_LCD_S18      (8UL<<SYS_PC_L_MFP_PC4_MFP_Pos)     /*!<PC4 Pin Function - LCD SEG 18 */

#define SYS_PC_L_MFP_PC3_MFP_GPC3         (0UL<<SYS_PC_L_MFP_PC3_MFP_Pos)     /*!<PC3 Pin Function - GPIOC[3] */
#define SYS_PC_L_MFP_PC3_MFP_PWM0_CH3     (3UL<<SYS_PC_L_MFP_PC3_MFP_Pos)     /*!<PC3 Pin Function - PWM0 Channel3 output */
#define SYS_PC_L_MFP_PC3_MFP_I2C1_SDA     (5UL<<SYS_PC_L_MFP_PC3_MFP_Pos)     /*!<PC3 Pin Function - I2C1 data I/O pin */
#define SYS_PC_L_MFP_PC3_MFP_LCD_S8       (8UL<<SYS_PC_L_MFP_PC3_MFP_Pos)     /*!<PC3 Pin Function - LCD SEG 8 */
#define SYS_PC_L_MFP_PC3_MFP_LCD_S15      (8UL<<SYS_PC_L_MFP_PC3_MFP_Pos)     /*!<PC3 Pin Function - LCD SEG 15 */
#define SYS_PC_L_MFP_PC3_MFP_LCD_S19      (8UL<<SYS_PC_L_MFP_PC3_MFP_Pos)     /*!<PC3 Pin Function - LCD SEG 19 */

#define SYS_PC_L_MFP_PC2_MFP_GPC2         (0UL<<SYS_PC_L_MFP_PC2_MFP_Pos)     /*!<PC2 Pin Function - GPIOC[2] */
#define SYS_PC_L_MFP_PC2_MFP_PWM0_CH2     (3UL<<SYS_PC_L_MFP_PC2_MFP_Pos)     /*!<PC2 Pin Function - PWM0 Channel2 output */
#define SYS_PC_L_MFP_PC2_MFP_I2C1_SCL     (5UL<<SYS_PC_L_MFP_PC2_MFP_Pos)     /*!<PC2 Pin Function - I2C1 clock */
#define SYS_PC_L_MFP_PC2_MFP_LCD_S9       (8UL<<SYS_PC_L_MFP_PC2_MFP_Pos)     /*!<PC2 Pin Function - LCD SEG 9 */
#define SYS_PC_L_MFP_PC2_MFP_LCD_S16      (8UL<<SYS_PC_L_MFP_PC2_MFP_Pos)     /*!<PC2 Pin Function - LCD SEG 16 */
#define SYS_PC_L_MFP_PC2_MFP_LCD_S20      (8UL<<SYS_PC_L_MFP_PC2_MFP_Pos)     /*!<PC2 Pin Function - LCD SEG 20 */

#define SYS_PC_L_MFP_PC1_MFP_GPC1         (0UL<<SYS_PC_L_MFP_PC1_MFP_Pos)     /*!<PC1 Pin Function - GPIOC[1] */
#define SYS_PC_L_MFP_PC1_MFP_PWM0_CH1     (3UL<<SYS_PC_L_MFP_PC1_MFP_Pos)     /*!<PC1 Pin Function - PWM0 Channel1 output */
#define SYS_PC_L_MFP_PC1_MFP_I2C0_SDA     (5UL<<SYS_PC_L_MFP_PC1_MFP_Pos)     /*!<PC1 Pin Function - I2C0 data I/O pin */
#define SYS_PC_L_MFP_PC1_MFP_LCD_S10      (8UL<<SYS_PC_L_MFP_PC1_MFP_Pos)     /*!<PC1 Pin Function - LCD SEG 10 */
#define SYS_PC_L_MFP_PC1_MFP_LCD_S17      (8UL<<SYS_PC_L_MFP_PC1_MFP_Pos)     /*!<PC1 Pin Function - LCD SEG 17 */
#define SYS_PC_L_MFP_PC1_MFP_LCD_S21      (8UL<<SYS_PC_L_MFP_PC1_MFP_Pos)     /*!<PC1 Pin Function - LCD SEG 21 */

#define SYS_PC_L_MFP_PC0_MFP_GPC0         (0UL<<SYS_PC_L_MFP_PC0_MFP_Pos)     /*!<PC0 Pin Function - GPIOC[0] */
#define SYS_PC_L_MFP_PC0_MFP_PWM0_CH0     (3UL<<SYS_PC_L_MFP_PC0_MFP_Pos)     /*!<PC0 Pin Function - PWM0 Channel0 output */
#define SYS_PC_L_MFP_PC0_MFP_I2C0_SCL     (5UL<<SYS_PC_L_MFP_PC0_MFP_Pos)     /*!<PC0 Pin Function - I2C0 clock pin */
#define SYS_PC_L_MFP_PC0_MFP_SPI0_SS1     (6UL<<SYS_PC_L_MFP_PC0_MFP_Pos)     /*!<PC0 Pin Function - SPI0 2nd slave select pin */
#define SYS_PC_L_MFP_PC0_MFP_LCD_S11      (8UL<<SYS_PC_L_MFP_PC0_MFP_Pos)     /*!<PC0 Pin Function - LCD SEG 11 */
#define SYS_PC_L_MFP_PC0_MFP_LCD_S18      (8UL<<SYS_PC_L_MFP_PC0_MFP_Pos)     /*!<PC0 Pin Function - LCD SEG 18 */
#define SYS_PC_L_MFP_PC0_MFP_LCD_S22      (8UL<<SYS_PC_L_MFP_PC0_MFP_Pos)     /*!<PC0 Pin Function - LCD SEG 22 */

/********************* Bit definition of PC_H_MFP register **********************/
#define SYS_PC_H_MFP_PC15_MFP_GPC15       (0UL<<SYS_PC_H_MFP_PC15_MFP_Pos)    /*!<PC15 Pin Function - GPIOC[15] */
#define SYS_PC_H_MFP_PC15_MFP_SC1_PWR     (4UL<<SYS_PC_H_MFP_PC15_MFP_Pos)    /*!<PC15 Pin Function - SmartCard1 PWR pin */
#define SYS_PC_H_MFP_PC15_MFP_LCD_S0      (8UL<<SYS_PC_H_MFP_PC15_MFP_Pos)    /*!<PC15 Pin Function - LCD SEG 0 */
#define SYS_PC_H_MFP_PC15_MFP_LCD_S7      (8UL<<SYS_PC_H_MFP_PC15_MFP_Pos)    /*!<PC15 Pin Function - LCD SEG 7 */

#define SYS_PC_H_MFP_PC14_MFP_GPC14       (0UL<<SYS_PC_H_MFP_PC14_MFP_Pos)    /*!<PC14 Pin Function - GPIOC[14] */
#define SYS_PC_H_MFP_PC14_MFP_SC1_CD      (4UL<<SYS_PC_H_MFP_PC14_MFP_Pos)    /*!<PC14 Pin Function - SmartCard1 card detect */
#define SYS_PC_H_MFP_PC14_MFP_LCD_S1      (7UL<<SYS_PC_H_MFP_PC14_MFP_Pos)    /*!<PC14 Pin Function - LCD SEG 1 */
#define SYS_PC_H_MFP_PC14_MFP_LCD_S8      (8UL<<SYS_PC_H_MFP_PC14_MFP_Pos)    /*!<PC14 Pin Function - LCD SEG 8 */

#define SYS_PC_H_MFP_PC13_MFP_GPC13       (0UL<<SYS_PC_H_MFP_PC13_MFP_Pos)    /*!<PC13 Pin Function - GPIOC[13] */
#define SYS_PC_H_MFP_PC13_MFP_SC1_DAT     (4UL<<SYS_PC_H_MFP_PC13_MFP_Pos)    /*!<PC13 Pin Function - SmartCard1 DATA */
#define SYS_PC_H_MFP_PC13_MFP_LCD_S9      (8UL<<SYS_PC_H_MFP_PC13_MFP_Pos)    /*!<PC13 Pin Function - LCD SEG 9 */

#define SYS_PC_H_MFP_PC12_MFP_GPC12       (0UL<<SYS_PC_H_MFP_PC12_MFP_Pos)    /*!<PC12 Pin Function - GPIOC[12] */
#define SYS_PC_H_MFP_PC12_MFP_SC1_CLK     (4UL<<SYS_PC_H_MFP_PC12_MFP_Pos)    /*!<PC12 Pin Function - SmartCard1 clock */
#define SYS_PC_H_MFP_PC12_MFP_LCD_S10     (8UL<<SYS_PC_H_MFP_PC12_MFP_Pos)    /*!<PC12 Pin Function - LCD SEG 10 */

#define SYS_PC_H_MFP_PC11_MFP_GPC11       (0UL<<SYS_PC_H_MFP_PC11_MFP_Pos)    /*!<PC11 Pin Function - GPIOC[11] */
#define SYS_PC_H_MFP_PC11_MFP_SC1_PWR     (4UL<<SYS_PC_H_MFP_PC11_MFP_Pos)    /*!<PC11 Pin Function - SmartCard1 PWR pin */
#define SYS_PC_H_MFP_PC11_MFP_I2C1_SDA    (5UL<<SYS_PC_H_MFP_PC11_MFP_Pos)    /*!<PC11 Pin Function - I2C1 data I/O pin */
#define SYS_PC_H_MFP_PC11_MFP_LCD_S11     (8UL<<SYS_PC_H_MFP_PC11_MFP_Pos)    /*!<PC11 Pin Function - LCD SEG 11 */

#define SYS_PC_H_MFP_PC10_MFP_GPC10       (0UL<<SYS_PC_H_MFP_PC10_MFP_Pos)    /*!<PC10 Pin Function - GPIOC[10] */
#define SYS_PC_H_MFP_PC10_MFP_SC1_CD      (4UL<<SYS_PC_H_MFP_PC10_MFP_Pos)    /*!<PC10 Pin Function - SmartCard1 card detect */
#define SYS_PC_H_MFP_PC10_MFP_I2C1_SCL    (5UL<<SYS_PC_H_MFP_PC10_MFP_Pos)    /*!<PC10 Pin Function - I2C1 clock pin */
#define SYS_PC_H_MFP_PC10_MFP_LCD_S12     (8UL<<SYS_PC_H_MFP_PC10_MFP_Pos)    /*!<PC10 Pin Function - LCD SEG 12 */

#define SYS_PC_H_MFP_PC9_MFP_GPC9         (0UL<<SYS_PC_H_MFP_PC9_MFP_Pos)     /*!<PC9 Pin Function - GPIOC[9] */
#define SYS_PC_H_MFP_PC9_MFP_LCD_S2       (8UL<<SYS_PC_H_MFP_PC9_MFP_Pos)     /*!<PC9 Pin Function - LCD SEG 2 */
#define SYS_PC_H_MFP_PC9_MFP_LCD_S9       (8UL<<SYS_PC_H_MFP_PC9_MFP_Pos)     /*!<PC9 Pin Function - LCD SEG 9 */
#define SYS_PC_H_MFP_PC9_MFP_LCD_S13      (8UL<<SYS_PC_H_MFP_PC9_MFP_Pos)     /*!<PC9 Pin Function - LCD SEG 13 */

#define SYS_PC_H_MFP_PC8_MFP_GPC8         (0UL<<SYS_PC_H_MFP_PC8_MFP_Pos)     /*!<PC8 Pin Function - GPIOC[8] */
#define SYS_PC_H_MFP_PC8_MFP_SC0_RST      (4UL<<SYS_PC_H_MFP_PC8_MFP_Pos)     /*!<PC8 Pin Function - SmartCard0 RST pin */
#define SYS_PC_H_MFP_PC8_MFP_UART1_TX     (7UL<<SYS_PC_H_MFP_PC8_MFP_Pos)     /*!<PC8 Pin Function - UART1 Data transmitter output pin */
#define SYS_PC_H_MFP_PC8_MFP_LCD_S3       (8UL<<SYS_PC_H_MFP_PC8_MFP_Pos)     /*!<PC8 Pin Function - LCD SEG 3 */
#define SYS_PC_H_MFP_PC8_MFP_LCD_S10      (8UL<<SYS_PC_H_MFP_PC8_MFP_Pos)     /*!<PC8 Pin Function - LCD SEG 10 */
#define SYS_PC_H_MFP_PC8_MFP_LCD_S14      (8UL<<SYS_PC_H_MFP_PC8_MFP_Pos)     /*!<PC8 Pin Function - LCD SEG 14 */

/********************* Bit definition of PD_L_MFP register **********************/
#define SYS_PD_L_MFP_PD7_MFP_GPD7         (0UL<<SYS_PD_L_MFP_PD7_MFP_Pos)     /*!<PD7 Pin Function - GPIOD[7] */
#define SYS_PD_L_MFP_PD7_MFP_SC1_CLK      (4UL<<SYS_PD_L_MFP_PD7_MFP_Pos)     /*!<PD7 Pin Function - SmartCard1 clock */
#define SYS_PD_L_MFP_PD7_MFP_LCD_COM3     (8UL<<SYS_PD_L_MFP_PD7_MFP_Pos)     /*!<PD7 Pin Function - LCD COM 3 */

#define SYS_PD_L_MFP_PD6_MFP_GPD6         (0UL<<SYS_PD_L_MFP_PD6_MFP_Pos)     /*!<PD6 Pin Function - GPIOD[6] */
#define SYS_PD_L_MFP_PD6_MFP_LCD_S0       (8UL<<SYS_PD_L_MFP_PD6_MFP_Pos)     /*!<PD6 Pin Function - LCD SEG 0 */
#define SYS_PD_L_MFP_PD6_MFP_LCD_COM4     (8UL<<SYS_PD_L_MFP_PD6_MFP_Pos)     /*!<PD6 Pin Function - LCD COM 4 */

#define SYS_PD_L_MFP_PD5_MFP_GPD5         (0UL<<SYS_PD_L_MFP_PD5_MFP_Pos)     /*!<PD5 Pin Function - GPIOD[5] */
#define SYS_PD_L_MFP_PD5_MFP_LCD_S1       (8UL<<SYS_PD_L_MFP_PD5_MFP_Pos)     /*!<PD5 Pin Function - LCD SEG 1 */
#define SYS_PD_L_MFP_PD5_MFP_LCD_COM5     (8UL<<SYS_PD_L_MFP_PD5_MFP_Pos)     /*!<PD5 Pin Function - LCD COM 5 */

#define SYS_PD_L_MFP_PD4_MFP_GPD4         (0UL<<SYS_PD_L_MFP_PD4_MFP_Pos)     /*!<PD4 Pin Function - GPIOD[4] */
#define SYS_PD_L_MFP_PD4_MFP_SC1_RST      (4UL<<SYS_PD_L_MFP_PD4_MFP_Pos)     /*!<PD4 Pin Function - SmartCard1 RST pin */
#define SYS_PD_L_MFP_PD4_MFP_LCD_S2       (8UL<<SYS_PD_L_MFP_PD4_MFP_Pos)     /*!<PD4 Pin Function - LCD SEG 4 */

#define SYS_PD_L_MFP_PD3_MFP_GPD3         (0UL<<SYS_PD_L_MFP_PD3_MFP_Pos)     /*!<PD3 Pin Function - GPIOD[3] */
#define SYS_PD_L_MFP_PD3_MFP_LCD_S3       (8UL<<SYS_PD_L_MFP_PD3_MFP_Pos)     /*!<PD3 Pin Function - LCD SEG 3 */

#define SYS_PD_L_MFP_PD2_MFP_GPD2         (0UL<<SYS_PD_L_MFP_PD2_MFP_Pos)     /*!<PD2 Pin Function - GPIOD[2] */
#define SYS_PD_L_MFP_PD2_MFP_LCD_S4       (8UL<<SYS_PD_L_MFP_PD2_MFP_Pos)     /*!<PD2 Pin Function - LCD SEG 4 */

#define SYS_PD_L_MFP_PD1_MFP_GPD1         (0UL<<SYS_PD_L_MFP_PD1_MFP_Pos)     /*!<PD1 Pin Function - GPIOD[1] */
#define SYS_PD_L_MFP_PD1_MFP_LCD_S5       (8UL<<SYS_PD_L_MFP_PD1_MFP_Pos)     /*!<PD1 Pin Function - LCD SEG 5 */

#define SYS_PD_L_MFP_PD0_MFP_GPD0         (0UL<<SYS_PD_L_MFP_PD0_MFP_Pos)     /*!<PD0 Pin Function - GPIOD[0] */
#define SYS_PD_L_MFP_PD0_MFP_LCD_S6       (8UL<<SYS_PD_L_MFP_PD0_MFP_Pos)     /*!<PD0 Pin Function - LCD SEG 6 */

/********************* Bit definition of PD_H_MFP register **********************/
#define SYS_PD_H_MFP_PD15_MFP_GPD15       (0UL<<SYS_PD_H_MFP_PD15_MFP_Pos)    /*!<PD15 Pin Function - GPIOD[15] */
#define SYS_PD_H_MFP_PD15_MFP_LCD_V3      (8UL<<SYS_PD_H_MFP_PD15_MFP_Pos)    /*!<PD15 Pin Function - LCD V3 */

#define SYS_PD_H_MFP_PD14_MFP_GPD14       (0UL<<SYS_PD_H_MFP_PD14_MFP_Pos)    /*!<PD14 Pin Function - GPIOD[14] */
#define SYS_PD_H_MFP_PD14_MFP_LCD_V2      (8UL<<SYS_PD_H_MFP_PD14_MFP_Pos)    /*!<PD14 Pin Function - LCD V2 */

#define SYS_PD_H_MFP_PD13_MFP_GPD13       (0UL<<SYS_PD_H_MFP_PD13_MFP_Pos)    /*!<PD13 Pin Function - GPIOD[13] */
#define SYS_PD_H_MFP_PD13_MFP_EINT1       (1UL<<SYS_PD_H_MFP_PD13_MFP_Pos)    /*!<PD13 Pin Function - External interrupt 1 input pin */
#define SYS_PD_H_MFP_PD13_MFP_LCD_V1      (8UL<<SYS_PD_H_MFP_PD13_MFP_Pos)    /*!<PD13 Pin Function - LCD V1 */

#define SYS_PD_H_MFP_PD12_MFP_GPD12       (0UL<<SYS_PD_H_MFP_PD12_MFP_Pos)    /*!<PD12 Pin Function - GPIOD[12] */
#define SYS_PD_H_MFP_PD12_MFP_CKO0        (1UL<<SYS_PD_H_MFP_PD12_MFP_Pos)    /*!<PD12 Pin Function - Frequency Divider0 output pin */
#define SYS_PD_H_MFP_PD12_MFP_TMR1_CNT    (2UL<<SYS_PD_H_MFP_PD12_MFP_Pos)    /*!<PD12 Pin Function - Timer1 external counter input */
#define SYS_PD_H_MFP_PD12_MFP_PWM0_CH0    (3UL<<SYS_PD_H_MFP_PD12_MFP_Pos)    /*!<PD12 Pin Function - PWM0 Channel0 output */
#define SYS_PD_H_MFP_PD12_MFP_LCD_DH1     (8UL<<SYS_PD_H_MFP_PD12_MFP_Pos)    /*!<PD12 Pin Function - LCD DH1 */
#define SYS_PD_H_MFP_PD12_MFP_CLK_Hz      (9UL<<SYS_PD_H_MFP_PD12_MFP_Pos)    /*!<PD12 Pin Function - RTC output pin */
#define SYS_PD_H_MFP_PD12_MFP_TMR1_OUT    (0xAUL<<SYS_PD_H_MFP_PD12_MFP_Pos)  /*!<PD12 Pin Function - Timer1 toggle out */

#define SYS_PD_H_MFP_PD11_MFP_GPD11       (0UL<<SYS_PD_H_MFP_PD11_MFP_Pos)    /*!<PD11 Pin Function - GPIOD[11] */
#define SYS_PD_H_MFP_PD11_MFP_TMR0_CAP    (2UL<<SYS_PD_H_MFP_PD11_MFP_Pos)    /*!<PD11 Pin Function - Timer0 capture input */
#define SYS_PD_H_MFP_PD11_MFP_PWM0_CH1    (3UL<<SYS_PD_H_MFP_PD11_MFP_Pos)    /*!<PD11 Pin Function - PWM0 Channel1 output */
#define SYS_PD_H_MFP_PD11_MFP_LCD_DH2     (8UL<<SYS_PD_H_MFP_PD11_MFP_Pos)    /*!<PD11 Pin Function - LCD DH2 */

#define SYS_PD_H_MFP_PD10_MFP_GPD10       (0UL<<SYS_PD_H_MFP_PD10_MFP_Pos)    /*!<PD10 Pin Function - GPIOD[10] */
#define SYS_PD_H_MFP_PD10_MFP_TMR1_CAP    (2UL<<SYS_PD_H_MFP_PD10_MFP_Pos)    /*!<PD10 Pin Function - Timer1 capture input  */
#define SYS_PD_H_MFP_PD10_MFP_PWM0_CH2    (3UL<<SYS_PD_H_MFP_PD10_MFP_Pos)    /*!<PD10 Pin Function - PWM0 Channel2 output */
#define SYS_PD_H_MFP_PD10_MFP_LCD_COM0    (8UL<<SYS_PD_H_MFP_PD10_MFP_Pos)    /*!<PD10 Pin Function - LCD COM0 */

#define SYS_PD_H_MFP_PD9_MFP_GPD9         (0UL<<SYS_PD_H_MFP_PD9_MFP_Pos)     /*!<PD9 Pin Function - GPIOD[9] */
#define SYS_PD_H_MFP_PD9_MFP_PWM0_CH3     (3UL<<SYS_PD_H_MFP_PD9_MFP_Pos)     /*!<PD9 Pin Function - PWM0 Channel3 output */
#define SYS_PD_H_MFP_PD9_MFP_SC1_RST      (4UL<<SYS_PD_H_MFP_PD9_MFP_Pos)     /*!<PD9 Pin Function - SmartCard1 RST pin */
#define SYS_PD_H_MFP_PD9_MFP_LCD_COM1     (8UL<<SYS_PD_H_MFP_PD9_MFP_Pos)     /*!<PD9 Pin Function - LCD COM1 */

#define SYS_PD_H_MFP_PD8_MFP_GPD8         (0UL<<SYS_PD_H_MFP_PD8_MFP_Pos)     /*!<PD8 Pin Function - GPIOD[8] */
#define SYS_PD_H_MFP_PD8_MFP_SC1_DAT      (4UL<<SYS_PD_H_MFP_PD8_MFP_Pos)     /*!<PD8 Pin Function - SmartCard1 data  */
#define SYS_PD_H_MFP_PD8_MFP_LCD_COM2     (8UL<<SYS_PD_H_MFP_PD8_MFP_Pos)     /*!<PD8 Pin Function - LCD COM2 */

/********************* Bit definition of PE_L_MFP register **********************/
#define SYS_PE_L_MFP_PE7_MFP_GPE7         (0UL<<SYS_PE_L_MFP_PE7_MFP_Pos)     /*!<PE7 Pin Function - GPIOE[7] */
#define SYS_PE_L_MFP_PE7_MFP_SC1_DAT      (4UL<<SYS_PE_L_MFP_PE7_MFP_Pos)     /*!<PE7 Pin Function - SmartCard1 data */

#define SYS_PE_L_MFP_PE6_MFP_GPE6         (0UL<<SYS_PE_L_MFP_PE6_MFP_Pos)     /*!<PE6 Pin Function - GPIOE[6] */
#define SYS_PE_L_MFP_PE6_MFP_SC1_CLK      (4UL<<SYS_PE_L_MFP_PE6_MFP_Pos)     /*!<PE6 Pin Function - SmartCard1 clock */

#define SYS_PE_L_MFP_PE5_MFP_GPE5         (0UL<<SYS_PE_L_MFP_PE5_MFP_Pos)     /*!<PE5 Pin Function - GPIOE[5] */
#define SYS_PE_L_MFP_PE5_MFP_SC1_PWR      (4UL<<SYS_PE_L_MFP_PE5_MFP_Pos)     /*!<PE5 Pin Function - SmartCard1 power */

#define SYS_PE_L_MFP_PE4_MFP_GPE4         (0UL<<SYS_PE_L_MFP_PE4_MFP_Pos)     /*!<PE4 Pin Function - GPIOE[4] */
#define SYS_PE_L_MFP_PE4_MFP_SC1_RST      (4UL<<SYS_PE_L_MFP_PE4_MFP_Pos)     /*!<PE4 Pin Function - SmartCard1 RST */

#define SYS_PE_L_MFP_PE3_MFP_GPE3         (0UL<<SYS_PE_L_MFP_PE3_MFP_Pos)     /*!<PE3 Pin Function - GPIOE[3] */
#define SYS_PE_L_MFP_PE3_MFP_SPI0_SS0     (4UL<<SYS_PE_L_MFP_PE3_MFP_Pos)     /*!<PE3 Pin Function - SPI0 1st slave select pin */

#define SYS_PE_L_MFP_PE2_MFP_GPE2         (0UL<<SYS_PE_L_MFP_PE2_MFP_Pos)     /*!<PE2 Pin Function - GPIOE[2] */
#define SYS_PE_L_MFP_PE2_MFP_SPI0_SCLK    (6UL<<SYS_PE_L_MFP_PE2_MFP_Pos)     /*!<PE2 Pin Function - SPI0 SCLK */

#define SYS_PE_L_MFP_PE1_MFP_GPE1         (0UL<<SYS_PE_L_MFP_PE1_MFP_Pos)     /*!<PE1 Pin Function - GPIOE[1] */
#define SYS_PE_L_MFP_PE1_MFP_SPI0_MISO0   (6UL<<SYS_PE_L_MFP_PE1_MFP_Pos)     /*!<PE1 Pin Function - SPI0 MISO[0] */

#define SYS_PE_L_MFP_PE0_MFP_GPE0         (0UL<<SYS_PE_L_MFP_PE0_MFP_Pos)     /*!<PE0 Pin Function - GPIOE[0] */
#define SYS_PE_L_MFP_PE0_MFP_SPI0_MOSI0   (6UL<<SYS_PE_L_MFP_PE0_MFP_Pos)     /*!<PE0 Pin Function - SPI0 MOSI[0] */

/********************* Bit definition of PE_H_MFP register **********************/
#define SYS_PE_H_MFP_PE9_MFP_GPE9         (0UL<<SYS_PE_H_MFP_PE9_MFP_Pos)     /*!<PE9 Pin Function - GPIOE[9] */
#define SYS_PE_H_MFP_PE9_MFP_PWM0_CH3     (3UL<<SYS_PE_H_MFP_PE9_MFP_Pos)     /*!<PE9 Pin Function - PWM0 Channel3 output */
#define SYS_PE_H_MFP_PE9_MFP_LCD_S29      (8UL<<SYS_PE_H_MFP_PE9_MFP_Pos)     /*!<PE9 Pin Function - LCD SEG 29 */

#define SYS_PE_H_MFP_PE8_MFP_GPE8         (0UL<<SYS_PE_H_MFP_PE8_MFP_Pos)     /*!<PE8 Pin Function - GPIOA[8] */
#define SYS_PE_H_MFP_PE8_MFP_PWM0_CH2     (3UL<<SYS_PE_H_MFP_PE8_MFP_Pos)     /*!<PE8 Pin Function - PWM0 Channel3 output */
#define SYS_PE_H_MFP_PE8_MFP_LCD_S30      (8UL<<SYS_PE_H_MFP_PE8_MFP_Pos)     /*!<PE8 Pin Function - LCD SEG 30 */

/********************* Bit definition of PF_L_MFP register **********************/
#define SYS_PF_L_MFP_PF5_MFP_GPF5         (0UL<<SYS_PF_L_MFP_PF5_MFP_Pos)     /*!<PF5 Pin Function - GPIOF[5] */
#define SYS_PF_L_MFP_PF5_MFP_TMR0_CAP     (2UL<<SYS_PF_L_MFP_PF5_MFP_Pos)     /*!<PF5 Pin Function - Timer0 capture input */
#define SYS_PF_L_MFP_PF5_MFP_PWM0_CH3     (3UL<<SYS_PF_L_MFP_PF5_MFP_Pos)     /*!<PF5 Pin Function - PWM0 Channel3 output */
#define SYS_PF_L_MFP_PF5_MFP_ACMP_C       (9UL<<SYS_PF_L_MFP_PF5_MFP_Pos)     /*!<PF5 Pin Function - Comparator0 charge/discharge path */
#define SYS_PF_L_MFP_PF5_MFP_ICE_DAT      (0xFUL<<SYS_PF_L_MFP_PF5_MFP_Pos)   /*!<PF5 Pin Function - ICE DAT */

#define SYS_PF_L_MFP_PF4_MFP_GPF4         (0UL<<SYS_PF_L_MFP_PF4_MFP_Pos)     /*!<PF4 Pin Function - GPIOF[4] */
#define SYS_PF_L_MFP_PF4_MFP_CKO1         (1UL<<SYS_PF_L_MFP_PF4_MFP_Pos)     /*!<PF4 Pin Function - Frequency Divider1 output pin */
#define SYS_PF_L_MFP_PF4_MFP_TMR1_CAP     (2UL<<SYS_PF_L_MFP_PF4_MFP_Pos)     /*!<PF4 Pin Function - Timer1 capture input  */
#define SYS_PF_L_MFP_PF4_MFP_PWM0_CH2     (3UL<<SYS_PF_L_MFP_PF4_MFP_Pos)     /*!<PF4 Pin Function - PWM0 Channel2 output */
#define SYS_PF_L_MFP_PF4_MFP_CLK_Hz       (9UL<<SYS_PF_L_MFP_PF4_MFP_Pos)     /*!<PF4 Pin Function - 1Hz clock output */
#define SYS_PF_L_MFP_PF4_MFP_ICE_CLK      (0xFUL<<SYS_PF_L_MFP_PF4_MFP_Pos)   /*!<PF4 Pin Function - ICE CLK */

#define SYS_PF_L_MFP_PF3_MFP_GPF3         (0UL<<SYS_PF_L_MFP_PF3_MFP_Pos)     /*!<PF3 Pin Function - GPIOF[3] */
#define SYS_PF_L_MFP_PF3_MFP_EINT0        (1UL<<SYS_PF_L_MFP_PF3_MFP_Pos)     /*!<PF3 Pin Function - External interrupt0 input pin */
#define SYS_PF_L_MFP_PF3_MFP_TMR2_CAP     (2UL<<SYS_PF_L_MFP_PF3_MFP_Pos)     /*!<PF3 Pin Function - Timer2 capture input  */
#define SYS_PF_L_MFP_PF3_MFP_UART1_TX     (7UL<<SYS_PF_L_MFP_PF3_MFP_Pos)     /*!<PF3 Pin Function - UART1 TX  */
#define SYS_PF_L_MFP_PF3_MFP_XT1_OUT      (0xFUL<<SYS_PF_L_MFP_PF3_MFP_Pos)   /*!<PF3 Pin Function - XT1 OUT */

#define SYS_PF_L_MFP_PF2_MFP_GPF2         (0UL<<SYS_PF_L_MFP_PF2_MFP_Pos)     /*!<PF2 Pin Function - GPIOF[2] */
#define SYS_PF_L_MFP_PF2_MFP_EINT1        (1UL<<SYS_PF_L_MFP_PF2_MFP_Pos)     /*!<PF2 Pin Function - External interrupt1 input pin */
#define SYS_PF_L_MFP_PF2_MFP_TMR3_CAP     (2UL<<SYS_PF_L_MFP_PF2_MFP_Pos)     /*!<PF2 Pin Function - Timer3 capture input  */
#define SYS_PF_L_MFP_PF2_MFP_UART1_RX     (7UL<<SYS_PF_L_MFP_PF2_MFP_Pos)     /*!<PF2 Pin Function - UART1 RX  */
#define SYS_PF_L_MFP_PF2_MFP_XT1_IN       (0xFUL<<SYS_PF_L_MFP_PF2_MFP_Pos)   /*!<PF2 Pin Function - XT1 IN */

#define SYS_PF_L_MFP_PF1_MFP_GPF1         (0UL<<SYS_PF_L_MFP_PF1_MFP_Pos)     /*!<PF1 Pin Function - GPIOF[1] */
#define SYS_PF_L_MFP_PF1_MFP_TMR2_CNT     (2UL<<SYS_PF_L_MFP_PF1_MFP_Pos)     /*!<PF1 Pin Function - Timer2 external counter input */
#define SYS_PF_L_MFP_PF1_MFP_X32_OUT      (0xFUL<<SYS_PF_L_MFP_PF1_MFP_Pos)   /*!<PF1 Pin Function - X32 OUT */
#define SYS_PF_L_MFP_PF1_MFP_TMR2_OUT     (0xAUL<<SYS_PF_L_MFP_PF1_MFP_Pos)   /*!<PF1 Pin Function - Timer2 toggle out */

#define SYS_PF_L_MFP_PF0_MFP_GPF0         (0UL<<SYS_PF_L_MFP_PF0_MFP_Pos)     /*!<PF0 Pin Function - GPIOF[0] */
#define SYS_PF_L_MFP_PF0_MFP_TMR3_CNT     (2UL<<SYS_PF_L_MFP_PF0_MFP_Pos)     /*!<PF0 Pin Function - Timer3 external counter input */
#define SYS_PF_L_MFP_PF0_MFP_X32_IN       (0xFUL<<SYS_PF_L_MFP_PF0_MFP_Pos)   /*!<PF0 Pin Function - X32 IN */
#define SYS_PF_L_MFP_PF0_MFP_TMR3_OUT     (0xAUL<<SYS_PF_L_MFP_PF0_MFP_Pos)   /*!<PF0 Pin Function - Timer3 toggle out */

/*!<Enable LCD for 100 Pin Package */
#define MFP_LCD_TYPEA() { \
                            SYS->PB_L_MFP = (SYS->PB_L_MFP &~(0xFFF00000)) | (0x88800000);  /* seg 35 ~ 33 */\
                            SYS->PB_H_MFP = 0x88888888;   /* seg 32 ~ 31, 28 ~ 23 */\
                            SYS->PC_L_MFP = 0x88888888;   /* seg 22 ~ 15 */\
                            SYS->PC_H_MFP = 0x88888888;   /* seg 14 ~ 7 */\
                            SYS->PD_L_MFP = 0x88888888;   /* seg 6 ~ 0, COM3 */\
                            SYS->PD_H_MFP = 0x88888888;   /* COM2 ~ 0, DH2 ~ 1, V3 ~1 */\
                            SYS->PE_H_MFP = (SYS->PE_H_MFP &~(0x000000FF)) | (0x00000088);   /* seg 30, 29 */\
                            PB->OFFD |= 0xFFE00000;\
                            PC->OFFD |= 0xFFFF0000;\
                            PD->OFFD |= 0xFFFF0000;\
                            PE->OFFD |= 0x03000000;\
                        }

/*@}*/ /* end of group NANO1X2_SYS_EXPORTED_CONSTANTS */

/** @addtogroup NANO1X2_SYS_EXPORTED_FUNCTIONS SYS Exported Functions
  @{
*/
/**
  * @brief      Clear Brown-out detector interrupt flag
  * @param      None  
  * @return     None
  * @details    This macro clear Brown-out detector interrupt flag.
  */
#define SYS_CLEAR_BOD_INT_FLAG()        (SYS->BODSTS |= SYS_BODSTS_BOD_INT_Msk)

/**
  * @brief      Disable Brown-out 2.5V detector function
  * @param      None  
  * @return     None
  * @details    This macro disable Brown-out 2.5V detector function.  
  */
#define SYS_DISABLE_BOD25()             (SYS->BODCTL |= SYS_BODCTL_BOD25_EN_Msk)

/**
  * @brief      Enable Brown-out 2.5V detector function
  * @param      None  
  * @return     None
  * @details    This macro enable Brown-out 2.5V detector function.  
  */
#define SYS_ENABLE_BOD25()              (SYS->BODCTL &= ~SYS_BODCTL_BOD25_EN_Msk)

/**
  * @brief      Disable Brown-out 2.0V detector function
  * @param      None  
  * @return     None
  * @details    This macro disable Brown-out 2.0V detector function.  
  */
#define SYS_DISABLE_BOD20()             (SYS->BODCTL |= SYS_BODCTL_BOD20_EN_Msk)

/**
  * @brief      Enable Brown-out 2.0V detector function
  * @param      None  
  * @return     None
  * @details    This macro enable Brown-out 2.0V detector function.  
  */
#define SYS_ENABLE_BOD20()              (SYS->BODCTL &= ~SYS_BODCTL_BOD20_EN_Msk)

/**
  * @brief      Disable Brown-out 1.7V detector function
  * @param      None  
  * @return     None
  * @details    This macro disable Brown-out 1.7V detector function.  
  */
#define SYS_DISABLE_BOD17()             (SYS->BODCTL |= SYS_BODCTL_BOD17_EN_Msk)

/**
  * @brief      Enable Brown-out 1.7V detector function
  * @param      None  
  * @return     None
  * @details    This macro enable Brown-out 1.7V detector function.  
  */
#define SYS_ENABLE_BOD17()              (SYS->BODCTL &= ~SYS_BODCTL_BOD17_EN_Msk)

/**
  * @brief      Get Brown-out detector interrupt flag
  * @param      None    
  * @retval     0   Brown-out detect interrupt flag is not set.
  * @retval     >=1 Brown-out detect interrupt flag is set.
  * @details    This macro get Brown-out detector interrupt flag.    
  */
#define SYS_GET_BOD_INT_FLAG()          (SYS->BODSTS & SYS_BODSTS_BOD_INT_Msk)

/**
  * @brief      Get Brown-out 2.5V detector status
  * @param      None 
  * @retval     0   System voltage is higher than 2.5V setting or BOD_EN is 0.
  * @retval     >=1 System voltage is lower than 2.5V setting.
  * @details    This macro get Brown-out detector output status.
  *             If the BOD_EN is 0, this function always return 0.
  */
#define SYS_GET_BOD25_OUTPUT()          (SYS->BODSTS & SYS_BODSTS_BOD25_Msk)

/**
  * @brief      Get Brown-out 2.0V detector status
  * @param      None 
  * @retval     0   System voltage is higher than 2.0V setting or BOD_EN is 0.
  * @retval     >=1 System voltage is lower than 2.0V setting.
  * @details    This macro get Brown-out detector output status.
  *             If the BOD_EN is 0, this function always return 0.
  */
#define SYS_GET_BOD20_OUTPUT()          (SYS->BODSTS & SYS_BODSTS_BOD20_Msk)

/**
  * @brief      Get Brown-out 1.7V detector status
  * @param      None 
  * @retval     0   System voltage is higher than 1.7V setting or BOD_EN is 0.
  * @retval     >=1 System voltage is lower than 1.7V setting.
  * @details    This macro get Brown-out detector output status.
  *             If the BOD_EN is 0, this function always return 0.
  */
#define SYS_GET_BOD17_OUTPUT()          (SYS->BODSTS & SYS_BODSTS_BOD17_Msk)

/**
  * @brief      Disable Brown-out 2.5V detector interrupt function
  * @param      None   
  * @return     None
  * @details    This macro enable Brown-out detector interrupt function.
  */
#define SYS_DISABLE_BOD25_RST()         (SYS->BODCTL &= ~SYS_BODCTL_BOD25_RST_EN_Msk)

/**
  * @brief      Enable Brown-out 2.5V detector reset function
  * @param      None     
  * @return     None
  * @details    This macro enable Brown-out detect reset function.  
  */
#define SYS_ENABLE_BOD25_RST()          (SYS->BODCTL |= SYS_BODCTL_BOD25_RST_EN_Msk)

/**
  * @brief      Disable Brown-out 2.0V detector interrupt function
  * @param      None   
  * @return     None
  * @details    This macro enable Brown-out detector interrupt function.
  */
#define SYS_DISABLE_BOD20_RST()         (SYS->BODCTL &= ~SYS_BODCTL_BOD20_RST_EN_Msk)

/**
  * @brief      Enable Brown-out 2.0V detector reset function
  * @param      None     
  * @return     None
  * @details    This macro enable Brown-out detect reset function.  
  */
#define SYS_ENABLE_BOD20_RST()          (SYS->BODCTL |= SYS_BODCTL_BOD20_RST_EN_Msk)

/**
  * @brief      Disable Brown-out 1.7V detector interrupt function
  * @param      None   
  * @return     None
  * @details    This macro enable Brown-out detector interrupt function.
  */
#define SYS_DISABLE_BOD17_RST()         (SYS->BODCTL &= ~SYS_BODCTL_BOD17_RST_EN_Msk)

/**
  * @brief      Enable Brown-out 1.7V detector reset function
  * @param      None     
  * @return     None
  * @details    This macro enable Brown-out detect reset function.  
  */
#define SYS_ENABLE_BOD17_RST()          (SYS->BODCTL |= SYS_BODCTL_BOD17_RST_EN_Msk)

/**
  * @brief      Get reset source is from Brown-out detector reset
  * @param      None    
  * @retval     0   Previous reset source is not from Brown-out detector reset
  * @retval     >=1 Previous reset source is from Brown-out detector reset
  * @details    This macro get previous reset source is from Brown-out detect reset or not.   
  */
#define SYS_IS_BOD_RST()                (SYS->RST_SRC & SYS_RST_SRC_RSTS_BOD_Msk)


/**
  * @brief      Get reset source is from CPU reset
  * @param      None     
  * @retval     0   Previous reset source is not from CPU reset
  * @retval     >=1 Previous reset source is from CPU reset
  * @details    This macro get previous reset source is from CPU reset. 
  */
#define SYS_IS_CPU_RST()                (SYS->RST_SRC & SYS_RST_SRC_RSTS_CPU_Msk)

/**
  * @brief      Get reset source is from Power-on Reset
  * @param      None     
  * @retval     0   Previous reset source is not from Power-on Reset
  * @retval     >=1 Previous reset source is from Power-on Reset
  * @details    This macro get previous reset source is from Power-on Reset.   
  */
#define SYS_IS_POR_RST()                (SYS->RST_SRC & SYS_RST_SRC_RSTS_POR_Msk)

/**
  * @brief      Get reset source is from system reset
  * @param      None     
  * @retval     0   Previous reset source is not from system reset
  * @retval     >=1 Previous reset source is from system reset
  * @details    This macro get previous reset source is from system reset.   
  */
#define SYS_IS_SYSTEM_RST()             (SYS->RST_SRC & SYS_RST_SRC_RSTS_SYS_Msk)


/**
  * @brief      Get reset source is from window watch dog reset
  * @param      None
  * @retval     0   Previous reset source is not from window watch dog reset
  * @retval     >=1 Previous reset source is from window watch dog reset
  * @details    This macro get previous reset source is from window watch dog reset.    
  */
#define SYS_IS_WDT_RST()                (SYS->RST_SRC & SYS_RST_SRC_RSTS_WDT_Msk)

/**
  * @brief      Disable Power-on Reset function
  * @param      None  
  * @return     None
  * @details    This macro disable Power-on Reset function.  
  */
#define SYS_DISABLE_POR()               (SYS->PORCTL = 0x5AA5)

/**
  * @brief      Enable Power-on Reset function
  * @param      None  
  * @return     None
  * @details    This macro enable Power-on Reset function.  
  */
#define SYS_ENABLE_POR()                (SYS->PORCTL = 0)


/**
  * @brief      Clear reset source flag
  * @param[in]  u32RstSrc is reset source. Including:
  *             - \ref SYS_RST_SRC_RSTS_POR_Msk
  *             - \ref SYS_RST_SRC_RSTS_PAD_Msk
  *             - \ref SYS_RST_SRC_RSTS_WDT_Msk
  *             - \ref SYS_RST_SRC_RSTS_BOD_Msk
  *             - \ref SYS_RST_SRC_RSTS_SYS_Msk 
  *             - \ref SYS_RST_SRC_RSTS_CPU_Msk   
  * @return     None
  * @details    This macro clear reset source flag.   
  */
#define SYS_CLEAR_RST_SOURCE(u32RstSrc) (SYS->RST_SRC | u32RstSrc )

void SYS_ClearResetSrc(uint32_t u32Src);
uint32_t SYS_GetBODStatus(void);
uint32_t SYS_GetResetSrc(void);
uint32_t SYS_IsRegLocked(void);
void SYS_LockReg(void);
void SYS_UnlockReg(void);
uint32_t  SYS_ReadPDID(void);
void SYS_ResetChip(void);
void SYS_ResetCPU(void);
void SYS_ResetModule(uint32_t u32ModuleIndex);
void SYS_EnableBOD(int32_t i32Mode, uint32_t u32BODLevel);
void SYS_DisableBOD(void);

/*@}*/ /* end of group NANO1X2_SYS_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NANO1X2_SYS_Driver */

/*@}*/ /* end of group NANO1X2_Device_Driver */

#ifdef __cplusplus
}
#endif

#endif //__SYS_H__

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/


