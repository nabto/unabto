/**************************************************************************//**
 * @file     fmc.h
 * @version  V1.00
 * $Revision: 5 $
 * $Date: 14/11/27 11:50a $
 * @brief    Nano102/112 Series Flash Memory Controller Driver Header File
 *
 * @note
 * Copyright (C) 2013~2014 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#ifndef __FMC_H__
#define __FMC_H__

#ifdef __cplusplus
extern "C"
{
#endif

/** @addtogroup NANO1X2_Device_Driver NANO102/112 Device Driver
  @{
*/

/** @addtogroup NANO1X2_FMC_Driver FMC Driver
  @{
*/


/** @addtogroup NANO1X2_FMC_EXPORTED_CONSTANTS FMC Exported Constants
  @{
*/


/*---------------------------------------------------------------------------------------------------------*/
/* Define Base Address                                                                                     */
/*---------------------------------------------------------------------------------------------------------*/
#define FMC_APROM_BASE          0x00000000UL    /*!< APROM Base Address          */
#define FMC_APROM_END           0x00008000UL    /*!< APROM End Address           */
#define FMC_LDROM_BASE          0x00100000UL    /*!< LDROM Base Address          */
#define FMC_LDROM_END           0x00101000UL    /*!< LDROM End Address           */
#define FMC_SPROM_BASE          0x00200000UL    /*!< SPROM Base Address          */
#define FMC_SPROM_END           0x00200200UL    /*!< SPROM End Address           */
#define FMC_CONFIG_BASE         0x00300000UL    /*!< User Configuration Address  */

#define FMC_FLASH_PAGE_SIZE     0x200           /*!< Flash Page Size (512 bytes) */
#define FMC_LDROM_SIZE          0x1000          /*!< LDROM Size (4 Kbytes)       */

/*---------------------------------------------------------------------------------------------------------*/
/*  ISPCMD constant definitions                                                                            */
/*---------------------------------------------------------------------------------------------------------*/
#define FMC_ISPCMD_READ         0x00            /*!< ISP Command: Read flash word         */
#define FMC_ISPCMD_WRITE        0x21            /*!< ISP Command: Write flash word        */
#define FMC_ISPCMD_PAGE_ERASE   0x22            /*!< ISP Command: Page Erase Flash        */
#define FMC_ISPCMD_READ_CID     0x0B            /*!< ISP Command: Read Company ID         */
#define FMC_ISPCMD_READ_DID     0x0C            /*!< ISP Command: Read Device ID          */
#define FMC_ISPCMD_READ_UID     0x04            /*!< ISP Command: Read Unique ID          */
#define FMC_ISPCMD_VECMAP       0x2E            /*!< ISP Command: Vector Page Remap       */

#define IS_BOOT_FROM_APROM      0               /*!< Is booting from APROM                */
#define IS_BOOT_FROM_LDROM      1               /*!< Is booting from LDROM                */


/*@}*/ /* end of group NANO1X2_FMC_EXPORTED_CONSTANTS */


/** @addtogroup NANO1X2_FMC_EXPORTED_FUNCTIONS FMC Exported Functions
  @{
*/

/*---------------------------------------------------------------------------------------------------------*/
/*  Macros                                                                                                 */
/*---------------------------------------------------------------------------------------------------------*/

#define FMC_SET_APROM_BOOT()        (FMC->ISPCON &= ~FMC_ISPCON_BS_Msk)         /*!< Select booting from APROM  */
#define FMC_SET_LDROM_BOOT()        (FMC->ISPCON |= FMC_ISPCON_BS_Msk)          /*!< Select booting from LDROM  */
#define FMC_ENABLE_AP_UPDATE()      (FMC->ISPCON |=  FMC_ISPCON_APUEN_Msk)      /*!< Enable APROM update        */
#define FMC_DISABLE_AP_UPDATE()     (FMC->ISPCON &= ~FMC_ISPCON_APUEN_Msk)      /*!< Disable APROM update       */
#define FMC_ENABLE_CFG_UPDATE()     (FMC->ISPCON |=  FMC_ISPCON_CFGUEN_Msk)     /*!< Enable User Config update  */
#define FMC_DISABLE_CFG_UPDATE()    (FMC->ISPCON &= ~FMC_ISPCON_CFGUEN_Msk)     /*!< Disable User Config update */
#define FMC_ENABLE_LD_UPDATE()      (FMC->ISPCON |=  FMC_ISPCON_LDUEN_Msk)      /*!< Enable LDROM update        */
#define FMC_DISABLE_LD_UPDATE()     (FMC->ISPCON &= ~FMC_ISPCON_LDUEN_Msk)      /*!< Disable LDROM update       */
#define FMC_DISABLE_ISP()           (FMC->ISPCON &= ~FMC_ISPCON_ISPEN_Msk)      /*!< Disable ISP function       */
#define FMC_ENABLE_ISP()            (FMC->ISPCON |=  FMC_ISPCON_ISPEN_Msk)      /*!< Enable ISP function        */
#define FMC_GET_FAIL_FLAG()         ((FMC->ISPCON & FMC_ISPCON_ISPFF_Msk) ? 1 : 0)  /*!< Get ISP fail flag  */
#define FMC_CLR_FAIL_FLAG()         (FMC->ISPCON |= FMC_ISPCON_ISPFF_Msk)       /*!< Clear ISP fail flag        */

/*---------------------------------------------------------------------------------------------------------*/
/*  Functions                                                                                              */
/*---------------------------------------------------------------------------------------------------------*/

extern void FMC_SetBootSource(int32_t i32BootSrc);
extern void FMC_Close(void);
extern void FMC_DisableAPUpdate(void);
extern void FMC_DisableConfigUpdate(void);
extern void FMC_DisableLDUpdate(void);
extern void FMC_EnableAPUpdate(void);
extern void FMC_EnableConfigUpdate(void);
extern void FMC_EnableLDUpdate(void);
extern int32_t FMC_Erase(uint32_t u32PageAddr);
extern int32_t FMC_GetBootSource(void);
extern void FMC_Open(void);
extern uint32_t FMC_Read(uint32_t u32Addr);
extern uint32_t FMC_ReadCID(void);
extern uint32_t FMC_ReadDID(void);
extern uint32_t FMC_ReadPID(void);
extern uint32_t FMC_ReadUCID(uint32_t u32Index);
extern uint32_t FMC_ReadUID(uint32_t u32Index);
extern uint32_t FMC_ReadDataFlashBaseAddr(void);
extern void FMC_SetVectorPageAddr(uint32_t u32PageAddr);
extern void FMC_Write(uint32_t u32Addr, uint32_t u32Data);
extern int32_t FMC_ReadConfig(uint32_t *u32Config, uint32_t u32Count);
extern int32_t FMC_WriteConfig(uint32_t *u32Config, uint32_t u32Count);


/*@}*/ /* end of group NANO1X2_FMC_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NANO1X2_FMC_Driver */

/*@}*/ /* end of group NANO1X2_Device_Driver */

#ifdef __cplusplus
}
#endif

#endif   // __FMC_H__

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/
