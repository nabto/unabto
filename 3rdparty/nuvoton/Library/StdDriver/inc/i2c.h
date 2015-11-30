/****************************************************************************//**
 * @file     i2c.h
 * @version  V1.00
 * $Revision: 10 $
 * $Date: 14/11/27 11:50a $
 * @brief    Nano102/112 series I2C driver header file
 *
 * @note
 * Copyright (C) 2013~2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C"
{
#endif


/** @addtogroup NANO1X2_Device_Driver NANO102/112 Device Driver
  @{
*/

/** @addtogroup NANO1X2_I2C_Driver I2C Driver
  @{
*/

/** @addtogroup NANO1X2_I2C_EXPORTED_CONSTANTS I2C Exported Constants
  @{
*/

#define I2C_STA 0x08    /*!< I2C START bit value */
#define I2C_STO 0x04    /*!< I2C STOP bit value*/
#define I2C_SI  0x10    /*!< I2C SI bit value */
#define I2C_AA  0x02    /*!< I2C ACK bit value */

#define I2C_GCMODE_ENABLE   1    /*!< Enable I2C GC Mode */
#define I2C_GCMODE_DISABLE  0    /*!< Disable I2C GC Mode */

/*@}*/ /* end of group NANO1X2_I2C_EXPORTED_CONSTANTS */


/** @addtogroup NANO1X2_I2C_EXPORTED_FUNCTIONS I2C Exported Functions
  @{
*/

/**
  * @brief This macro sets the I2C control register at one time.
  * @param[in] i2c is the base address of I2C module.
  * @param[in] u8Ctrl is the register value of I2C control register.
  * @return none
  * \hideinitializer
  */
#define I2C_SET_CONTROL_REG(i2c, u8Ctrl) ( (i2c)->CON = ((i2c)->CON & ~0x1e) | u8Ctrl )

/**
  * @brief This macro only set START bit to the control register of I2C module.
  * @param[in] i2c is the base address of I2C module.
  * @return none
  * \hideinitializer
  */
#define I2C_START(i2c) ( (i2c)->CON = ((i2c)->CON & ~I2C_CON_I2C_STS_Msk) | I2C_CON_START_Msk )

/**
  * @brief This macro only set STOP bit to the control register of I2C module.
  * @param[in] i2c is the base address of I2C module.
  * @return none
  * \hideinitializer
  */
#define I2C_STOP(i2c) \
do { \
    (i2c)->CON |= (I2C_CON_I2C_STS_Msk | I2C_CON_STOP_Msk); \
    while((i2c)->CON & I2C_CON_STOP_Msk); \
} while(0)

/**
  * @brief This macro will return when I2C module is ready and flag is cleared.
  * @param[in] i2c is the base address of I2C module.
  * @return none
  * \hideinitializer
  */
#define I2C_WAIT_READY(i2c) \
do { \
	while(!((i2c)->INTSTS & I2C_INTSTS_INTSTS_Msk)); \
	(i2c)->INTSTS |= I2C_INTSTS_INTSTS_Msk; \
} while(0)

/**
  * @brief This macro returns the data stored in data register of I2C module.
  * @param[in] i2c is the base address of I2C module.
  * @return Data.
  * \hideinitializer
  */
#define I2C_GET_DATA(i2c) ((i2c)->DATA ) 

/**
  * @brief This macro writes the data to data register of I2C module.
  * @param[in] i2c is the base address of I2C module.
  * @param[in] u8Data is the data which will be write to data register of I2C module.
  * @return none
  * \hideinitializer
  */
#define I2C_SET_DATA(i2c, u8Data) ( (i2c)->DATA = u8Data )

/**
  * @brief This macro returns the status of I2C module.
  * @param[in] i2c is the base address of I2C module.
  * @return Status.
  * \hideinitializer
  */
#define I2C_GET_STATUS(i2c) ( (i2c)->STATUS )

/**
  * @brief This macro returns timeout flag.
  * @param[in] i2c is the base address of I2C module.
  * @return Status.
  * @retval 0 Flag is not set.
  * @retval 1 Flag is set.
  * \hideinitializer
  */
#define I2C_GET_TIMEOUT_FLAG(i2c) ( ((i2c)->INTSTS & I2C_INTSTS_TIF_Msk) == I2C_INTSTS_TIF_Msk ? 1:0  )

/**
  * @brief This macro clears timeout flag.
  * @param[in] i2c is the base address of I2C module.
  * @return none
  * \hideinitializer
  */
#define I2C_CLEAR_TIMEOUT_FLAG(i2c) ( (i2c)->INTSTS |= I2C_INTSTS_TIF_Msk )

/**
  * @brief This macro returns wakeup flag.
  * @param[in] i2c is the base address of I2C module.
  * @return Status.
  * @retval 0 Flag is not set.
  * @retval 1 Flag is set.
  * \hideinitializer
  */
#define I2C_GET_WAKEUP_FLAG(i2c) ( ((i2c)->STATUS2 & I2C_STATUS2_WKUPIF_Msk) == I2C_STATUS2_WKUPIF_Msk ? 1:0  )

/**
  * @brief This macro returns acknowledge status after waking up.
  * @param[in] i2c is the base address of I2C module.
  * @return Status.
  * @retval 0 Acknowledge signal isn't detected.
  * @retval 1 Acknowledge signal is detected.
  * \hideinitializer
  */
#define I2C_GET_WAKEUP_ACK_DONE_FLAG(i2c) ( ((i2c)->INTSTS & I2C_INTSTS_WAKEUP_ACK_DONE_Msk) == I2C_INTSTS_WAKEUP_ACK_DONE_Msk ? 1:0  )

/**
  * @brief This macro clears wakeup flag.
  * @param[in] i2c is the base address of I2C module.
  * @return none
  * \hideinitializer
  */
#define I2C_CLEAR_WAKEUP_FLAG(i2c) ( (i2c)->STATUS2 |= I2C_STATUS2_WKUPIF_Msk )

/**
  * @brief This macro clears acknowledge status after waking up.
  *        And also release SCK pin after clearing this flag.
  * @param[in] i2c is the base address of I2C module.
  * @return none
  * \hideinitializer
  */
#define I2C_CLEAR_WAKEUP_ACK_DONE_FLAG(i2c) ( (i2c)->INTSTS |= I2C_INTSTS_WAKEUP_ACK_DONE_Msk )

/**
  * @brief This macro disables the FIFO function.
  * @param[in] i2c is the base address of I2C module.
  * @return none
  * \hideinitializer
  */
#define I2C_DISABLE_FIFO(i2c) ( (i2c)->CON2 &= ~I2C_CON2_TWOFF_EN_Msk )

/**
  * @brief This macro enables the FIFO function.
  * @param[in] i2c is the base address of I2C module.
  * @return none
  * \hideinitializer
  */
#define I2C_ENABLE_FIFO(i2c) ( (i2c)->CON2 |= I2C_CON2_TWOFF_EN_Msk )

/**
  * @brief This macro disables clock stretch function.
  * @param[in] i2c is the base address of I2C module.
  * @return none
  * \hideinitializer
  */
#define I2C_DISABLE_CLOCK_STRETCH(i2c) ( (i2c)->CON2 |= I2C_CON2_NOSTRETCH_Msk )

/**
  * @brief This macro enables clock stretch function.
  * @param[in] i2c is the base address of I2C module.
  * @return none
  * \hideinitializer
  */
#define I2C_ENABLE_CLOCK_STRETCH(i2c) ( (i2c)->CON2 &= ~I2C_CON2_NOSTRETCH_Msk )

/**
  * @brief This macro disables over-run interrupt.
  * @param[in] i2c is the base address of I2C module.
  * @return none
  * \hideinitializer
  */
#define I2C_DISABLE_OVERRUN_INT(i2c) ( (i2c)->CON2 &= ~I2C_CON2_OVER_INTEN_Msk )

/**
  * @brief This macro enables over-run interrupt.
  * @param[in] i2c is the base address of I2C module.
  * @return none
  * \hideinitializer
  */
#define I2C_ENABLE_OVERRUN_INT(i2c) ( (i2c)->CON2 |= I2C_CON2_OVER_INTEN_Msk )

/**
  * @brief This macro enables under-run interrupt.
  * @param[in] i2c is the base address of I2C module.
  * @return none
  * \hideinitializer
  */
#define I2C_ENABLE_UNDERRUN_INT(i2c) ( (i2c)->CON2 |= I2C_CON2_UNDER_INTEN_Msk )

/**
  * @brief This macro disables under-run interrupt.
  * @param[in] i2c is the base address of I2C module.
  * @return none
  * \hideinitializer
  */
#define I2C_DISABLE_UNDERRUN_INT(i2c) ((i2c)->CON2 &= ~I2C_CON2_UNDER_INTEN_Msk )

/**
  * @brief This macro returns I2C bus status flag.
  * @param[in] i2c is the base address of I2C module.
  * @return Status.
  * @retval 0 Bus is busy.
  * @retval 1 Bus is free.
  * \hideinitializer
  */
#define I2C_GET_BUS_FREE_FLAG(i2c) ( ((i2c)->STATUS2 & I2C_STATUS2_BUS_FREE_Msk) == I2C_STATUS2_BUS_FREE_Msk ? 1:0  )

/**
  * @brief This macro returns wakeup read/write flag.
  *        After system wake up, this bit indicates that the master wants to read or write in R/W bit of I2C data.
  * @param[in] i2c is the base address of I2C module.
  * @return Status.
  * @retval 0 Write command.
  * @retval 1 Read command.
  * \hideinitializer
  */
#define I2C_GET_WAKEUP_RW_FLAG(i2c) ( ((i2c)->STATUS2 & I2C_STATUS2_WR_STATUS_Msk) == I2C_STATUS2_WR_STATUS_Msk ? 1:0  )

uint32_t I2C_Open(I2C_T *i2c, uint32_t u32BusClock);
void I2C_Close(I2C_T *i2c);
void I2C_ClearTimeoutFlag(I2C_T *i2c);
void I2C_Trigger(I2C_T *i2c, uint8_t u8Start, uint8_t u8Stop, uint8_t u8Si, uint8_t u8Ack);
void I2C_DisableInt(I2C_T *i2c);
void I2C_EnableInt(I2C_T *i2c);
uint32_t I2C_GetBusClockFreq(I2C_T *i2c);
uint32_t I2C_SetBusClockFreq(I2C_T *i2c, uint32_t u32BusClock);
uint32_t I2C_GetIntFlag(I2C_T *i2c);
uint32_t I2C_GetStatus(I2C_T *i2c);
uint32_t I2C_GetData(I2C_T *i2c);
void I2C_SetData(I2C_T *i2c, uint8_t u8Data);
void I2C_SetSlaveAddr(I2C_T *i2c, uint8_t u8SlaveNo, uint8_t u8SlaveAddr, uint8_t u8GCMode);
void I2C_SetSlaveAddrMask(I2C_T *i2c, uint8_t u8SlaveNo, uint8_t u8SlaveAddrMask);
void I2C_EnableTimeout(I2C_T *i2c, uint8_t u8LongTimeout);
void I2C_DisableTimeout(I2C_T *i2c);
void I2C_EnableWakeup(I2C_T *i2c);
void I2C_DisableWakeup(I2C_T *i2c);

/*@}*/ /* end of group NANO1X2_I2C_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NANO1X2_I2C_Driver */

/*@}*/ /* end of group NANO1X2_Device_Driver */

#ifdef __cplusplus
}
#endif

#endif //__I2C_H__

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
