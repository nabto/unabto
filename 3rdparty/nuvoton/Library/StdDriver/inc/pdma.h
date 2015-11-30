/**************************************************************************//**
 * @file     pdma.h
 * @version  V1.00
 * $Revision: 10 $
 * $Date: 14/11/27 11:50a $
 * @brief    Nano102/112 series PDMA driver header file
 *
 * @note
 * Copyright (C) 2013~2014 Nuvoton Technology Corp. All rights reserved.
 *****************************************************************************/
#ifndef __PDMA_H__
#define __PDMA_H__

#ifdef __cplusplus
extern "C"
{
#endif


/** @addtogroup NANO1X2_Device_Driver NANO102/112 Device Driver
  @{
*/

/** @addtogroup NANO1X2_PDMA_Driver PDMA Driver
  @{
*/

/** @addtogroup NANO1X2_PDMA_EXPORTED_CONSTANTS PDMA Exported Constants
  @{
*/

/*---------------------------------------------------------------------------------------------------------*/
/*  Data Width Constant Definitions                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
#define PDMA_WIDTH_8        0x00080000UL            /*!<DMA Transfer Width 8-bit */
#define PDMA_WIDTH_16       0x00100000UL            /*!<DMA Transfer Width 16-bit */
#define PDMA_WIDTH_32       0x00000000UL            /*!<DMA Transfer Width 32-bit */

/*---------------------------------------------------------------------------------------------------------*/
/*  Address Attribute Constant Definitions                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
#define PDMA_SAR_INC        0x00000000UL            /*!<DMA SAR increment */
#define PDMA_SAR_FIX        0x00000020UL            /*!<DMA SAR fix address */
#define PDMA_SAR_WRA        0x00000030UL            /*!<DMA SAR wrap around */
#define PDMA_DAR_INC        0x00000000UL            /*!<DMA DAR increment */
#define PDMA_DAR_FIX        0x00000080UL            /*!<DMA DAR fix address */
#define PDMA_DAR_WRA        0x000000C0UL            /*!<DMA DAR wrap around */

/*---------------------------------------------------------------------------------------------------------*/
/*  Peripheral Transfer Mode Constant Definitions                                                          */
/*---------------------------------------------------------------------------------------------------------*/
#define PDMA_SPI0_TX        0x00000000UL            /*!<DMA Connect to SPI0 TX */
#define PDMA_SPI1_TX        0x00000001UL            /*!<DMA Connect to SPI1 TX */
#define PDMA_UART0_TX       0x00000002UL            /*!<DMA Connect to UART0 TX */
#define PDMA_UART1_TX       0x00000003UL            /*!<DMA Connect to UART1 TX */
#define PDMA_TMR0_TX        0x00000009UL            /*!<DMA Connect to TMR0 TX */
#define PDMA_TMR1_TX        0x0000000AUL            /*!<DMA Connect to TMR1 TX */
#define PDMA_TMR2_TX        0x0000000BUL            /*!<DMA Connect to TMR2 TX */
#define PDMA_TMR3_TX        0x0000000CUL            /*!<DMA Connect to TMR3 TX */

#define PDMA_SPI0_RX        0x00000010UL            /*!<DMA Connect to SPI0 RX */
#define PDMA_SPI1_RX        0x00000011UL            /*!<DMA Connect to SPI1 RX */
#define PDMA_UART0_RX       0x00000012UL            /*!<DMA Connect to UART0 RX */
#define PDMA_UART1_RX       0x00000013UL            /*!<DMA Connect to UART1 RX */
#define PDMA_ADC            0x00000016UL            /*!<DMA Connect to I2S1 RX */
#define PDMA_PWM0_CH0       0x00000019UL            /*!<DMA Connect to PWM0 CH0 */
#define PDMA_PWM0_CH2       0x0000001AUL            /*!<DMA Connect to PWM0 CH2 */
#define PDMA_MEM            0x0000001FUL            /*!<DMA Connect to Memory */

/*@}*/ /* end of group NANO1X2_PDMA_EXPORTED_CONSTANTS */

/** @addtogroup NANO1X2_PDMA_EXPORTED_FUNCTIONS PDMA Exported Functions
  @{
*/

/**
 * @brief       Get PDMA Interrupt Status
 *
 * @param[in]   None
 *
 * @return      None
 *
 * @details     This macro gets the interrupt status.
 * \hideinitializer
 */
#define PDMA_GET_INT_STATUS()   ((uint32_t)(PDMAGCR->GCRISR))

/**
 * @brief       Get PDMA Channel Interrupt Status
 *
 * @param[in]   u32Ch   Selected DMA channel
 *
 * @return      Interrupt Status
 *
 * @details     This macro gets the channel interrupt status.
 * \hideinitializer
 */
#define PDMA_GET_CH_INT_STS(u32Ch)   (*((__IO uint32_t *)((uint32_t)&PDMA1->ISR + (uint32_t)((u32Ch-1)*0x100))))

/**
 * @brief       Clear PDMA Channel Interrupt Flag
 *
 * @param[in]   u32Ch   Selected DMA channel
 * @param[in]   u32Mask Interrupt Mask
 *
 * @return      None
 *
 * @details     This macro clear the channel interrupt flag.
 * \hideinitializer
 */
#define PDMA_CLR_CH_INT_FLAG(u32Ch, u32Mask)   (*((__IO uint32_t *)((uint32_t)&PDMA1->ISR + (uint32_t)((u32Ch-1)*0x100))) = (u32Mask))

/**
 * @brief       Check Channel Status
 *
 * @param[in]   u32Ch     The selected channel
 *
 * @return      0 = idle
 * @return      1 = busy
 *
 * @details     Check the selected channel is busy or not.
 * \hideinitializer
 */
#define PDMA_IS_CH_BUSY(u32Ch)    (*((__IO uint32_t *)((uint32_t)&PDMA1->CSR +(uint32_t)((u32Ch-1)*0x100)))&PDMA_CSR_TRIG_EN_Msk)? 1 : 0)

/**
 * @brief       Set Source Address
 *
 * @param[in]   u32Ch     The selected channel
 * @param[in]   u32Addr   The selected address
 *
 * @return      None
 *
 * @details     This macro set the selected channel source address.
 * \hideinitializer
 */
#define PDMA_SET_SRC_ADDR(u32Ch, u32Addr) (*((__IO uint32_t *)((uint32_t)&PDMA1->SAR + (uint32_t)((u32Ch-1)*0x100))) = (u32Addr))

/**
 * @brief       Set Destination Address
 *
 * @param[in]   u32Ch     The selected channel
 * @param[in]   u32Addr   The selected address
 *
 * @return      None
 *
 * @details     This macro set the selected channel destination address.
 * \hideinitializer
 */
#define PDMA_SET_DST_ADDR(u32Ch, u32Addr) (*((__IO uint32_t *)((uint32_t)&PDMA1->DAR + (uint32_t)((u32Ch-1)*0x100))) = (u32Addr))

/**
 * @brief       Set Transfer Count
 *
 * @param[in]   u32Ch     The selected channel
 * @param[in]   u32Count  Transfer Count
 *
 * @return      None
 *
 * @details     This macro set the selected channel transfer count.
 * \hideinitializer
 */
#define PDMA_SET_TRANS_CNT(u32Ch, u32Count) {   \
    if (((uint32_t)*((__IO uint32_t *)((uint32_t)&PDMA1->CSR + (uint32_t)((u32Ch-1)*0x100))) & PDMA_CSR_APB_TWS_Msk) == PDMA_WIDTH_32)  \
        *((__IO uint32_t *)((uint32_t)&PDMA1->BCR + (uint32_t)((u32Ch-1)*0x100))) = ((u32Count) << 2);  \
    else if (((uint32_t)*((__IO uint32_t *)((uint32_t)&PDMA1->CSR + (uint32_t)((u32Ch-1)*0x100))) & PDMA_CSR_APB_TWS_Msk) == PDMA_WIDTH_8)  \
        *((__IO uint32_t *)((uint32_t)&PDMA1->BCR + (uint32_t)((u32Ch-1)*0x100))) = (u32Count); \
    else if (((uint32_t)*((__IO uint32_t *)((uint32_t)&PDMA1->CSR + (uint32_t)((u32Ch-1)*0x100))) & PDMA_CSR_APB_TWS_Msk) == PDMA_WIDTH_16) \
        *((__IO uint32_t *)((uint32_t)&PDMA1->BCR + (uint32_t)((u32Ch-1)*0x100))) = ((u32Count) << 1);  \
}

/**
 * @brief       Stop the channel
 *
 * @param[in]   u32Ch     The selected channel
 *
 * @return      None
 *
 * @details     This macro stop the selected channel.
 * \hideinitializer
 */
#define PDMA_STOP(u32Ch) (*((__IO uint32_t *)((uint32_t)&PDMA1->CSR + (uint32_t)((u32Ch-1)*0x100))) &= ~PDMA_CSR_PDMACEN_Msk)

void PDMA_Open(uint32_t u32Mask);
void PDMA_Close(void);
void PDMA_SetTransferCnt(uint32_t u32Ch, uint32_t u32Width, uint32_t u32TransCount);
void PDMA_SetTransferAddr(uint32_t u32Ch, uint32_t u32SrcAddr, uint32_t u32SrcCtrl, uint32_t u32DstAddr, uint32_t u32DstCtrl);
void PDMA_SetTransferMode(uint32_t u32Ch, uint32_t u32Periphral, uint32_t u32ScatterEn, uint32_t u32DescAddr);
void PDMA_SetTimeOut(uint32_t u32Ch, uint32_t u32OnOff, uint32_t u32TimeOutCnt);
void PDMA_Trigger(uint32_t u32Ch);
void PDMA_EnableInt(uint32_t u32Ch, uint32_t u32Mask);
void PDMA_DisableInt(uint32_t u32Ch, uint32_t u32Mask);

/*@}*/ /* end of group NANO1X2_PDMA_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NANO1X2_PDMA_Driver */

/*@}*/ /* end of group NANO1X2_Device_Driver */

#ifdef __cplusplus
}
#endif

#endif //__PDMA_H__

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
