/**************************************************************************//**
 * @file     fmc.c
 * @version  V1.00
 * $Revision: 10 $
 * $Date: 14/12/18 7:10p $
 * @brief    Nano 102/112 series FMC driver source file
 *
 * @note
 * Copyright (C) 2013~2014 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

//* Includes ------------------------------------------------------------------*/
#include <stdio.h>

#include "Nano1x2Series.h"


/** @addtogroup NANO1X2_Device_Driver NANO102/112 Device Driver
  @{
*/

/** @addtogroup NANO1X2_FMC_Driver FMC Driver
  @{
*/


/** @addtogroup NANO1X2_FMC_EXPORTED_FUNCTIONS FMC Exported Functions
  @{
*/


/**
  * @brief Set boot from APROM or LDROM of next software reset.
  * @param[in] i32BootSrc Next software boot selection.
  *            - \ref IS_BOOT_FROM_LDROM
  *            - \ref IS_BOOT_FROM_APROM
  * @return None
  */
void FMC_SetBootSource (int32_t i32BootSrc)
{
    if (i32BootSrc == 1)
        FMC->ISPCON |= FMC_ISPCON_BS_Msk;
    else
        FMC->ISPCON &= ~FMC_ISPCON_BS_Msk;
}


/**
  * @brief Disable FMC ISP function.
  * @return None
  */
void FMC_Close(void)
{
    FMC->ISPCON &= ~FMC_ISPCON_ISPEN_Msk;
}


/**
  * @brief Disable ISP erase/program APROM function.
  * @return None
  */
void FMC_DisableAPUpdate(void)
{
    FMC->ISPCON &= ~FMC_ISPCON_APUEN_Msk;
}


/**
  * @brief Disable ISP erase/program User Configuration function.
  * @return None
  */
void FMC_DisableConfigUpdate(void)
{
    FMC->ISPCON &= ~FMC_ISPCON_CFGUEN_Msk;
}


/**
  * @brief Disable ISP erase/program LDROM function.
  * @return None
  */
void FMC_DisableLDUpdate(void)
{
    FMC->ISPCON &= ~FMC_ISPCON_LDUEN_Msk;
}


/**
  * @brief    Enable APROM update function
  * @return None
  */
void FMC_EnableAPUpdate(void)
{
    FMC->ISPCON |= FMC_ISPCON_APUEN_Msk;
}


/**
  * @brief Enable ISP erase/program User Configuration function.
  * @return None
  */
void FMC_EnableConfigUpdate(void)
{
    FMC->ISPCON |= FMC_ISPCON_CFGUEN_Msk;
}


/**
  * @brief Enable ISP erase/program LDROM function.
  * @return None
  */
void FMC_EnableLDUpdate(void)
{
    FMC->ISPCON |= FMC_ISPCON_LDUEN_Msk;
}


/**
  * @brief Execute ISP command to erase a flash page. The page size is 512 bytes.
  * @param[in]  u32PageAddr Address of the flash page to be erased.
  *             It must be a 512-byte aligned address.
  * @return ISP page erase success or not.
  * @retval   0  Success
  * @retval   -1  Erase failed
  */
int32_t FMC_Erase(uint32_t u32PageAddr)
{
    FMC->ISPCMD = FMC_ISPCMD_PAGE_ERASE;
    FMC->ISPADR = u32PageAddr;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    while (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk) ;

    if (FMC->ISPCON & FMC_ISPCON_ISPFF_Msk) {
        FMC->ISPCON |= FMC_ISPCON_ISPFF_Msk;
        return -1;
    }
    return 0;
}


/**
  * @brief Get the current boot source.
  * @return The current boot source.
  * @retval   0  Is boot from APROM.
  * @retval   1  Is boot from LDROM.
  */
int32_t FMC_GetBootSource (void)
{
    if (FMC->ISPCON & FMC_ISPCON_BS_Msk)
        return 1;
    else
        return 0;
}


/**
  * @brief Enable FMC ISP function
  * @return None
  */
void FMC_Open(void)
{
    FMC->ISPCON |=  FMC_ISPCON_ISPEN_Msk;
}


/**
  * @brief Execute ISP command to read a word from flash.
  * @param[in]  u32Addr Address of the flash location to be read.
  *             It must be a word aligned address.
  * @return The word data read from specified flash address.
  */
uint32_t FMC_Read(uint32_t u32Addr)
{
    FMC->ISPCMD = FMC_ISPCMD_READ;
    FMC->ISPADR = u32Addr;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    while (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk) ;

    return FMC->ISPDAT;
}


/**
  * @brief    Read company ID.
  * @return   The company ID.
  */
uint32_t FMC_ReadCID(void)
{
    FMC->ISPCMD = FMC_ISPCMD_READ_CID;
    FMC->ISPADR = 0x0;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
    while (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk) ;
    return FMC->ISPDAT;
}


/**
  * @brief    Read device ID.
  * @return   The device ID.
  */
uint32_t FMC_ReadDID(void)
{
    FMC->ISPCMD = FMC_ISPCMD_READ_DID;
    FMC->ISPADR = 0;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
    while (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk) ;
    return FMC->ISPDAT;
}


/**
  * @brief    Read product ID.
  * @return   The product ID.
  */
uint32_t FMC_ReadPID(void)
{
    FMC->ISPCMD = FMC_ISPCMD_READ_DID;
    FMC->ISPADR = 0x04;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
    while (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk) ;
    return FMC->ISPDAT;
}


/**
  * @brief    This function reads one of the four UCID.
  * @param[in]   u32Index  Index of the UCID to read. u32Index must be 0, 1, 2, or 3.
  * @return   The UCID.
  */
uint32_t FMC_ReadUCID(uint32_t u32Index)
{
    FMC->ISPCMD = FMC_ISPCMD_READ_UID;
    FMC->ISPADR = (0x04 * u32Index) + 0x10;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    while (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk) ;

    return FMC->ISPDAT;
}


/**
  * @brief    This function reads one of the three UID.
  * @param[in]  u32Index Index of the UID to read. u32Index must be 0, 1, or 2.
  * @return   The UID.
  */
uint32_t FMC_ReadUID(uint32_t u32Index)
{
    FMC->ISPCMD = FMC_ISPCMD_READ_UID;
    FMC->ISPADR = 0x04 * u32Index;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;

    while (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk) ;

    return FMC->ISPDAT;
}


/**
  * @brief    Get the base address of Data Flash if enabled.
  * @return   The base address of Data Flash
  */
uint32_t FMC_ReadDataFlashBaseAddr(void)
{
    return FMC->DFBADR;
}


/**
  * @brief    This function will force re-map assigned flash page to CPU address 0x0.
  * @param[in]  u32PageAddr Address of the page to be mapped to CPU address 0x0.
  * @return  None
  */
void FMC_SetVectorPageAddr(uint32_t u32PageAddr)
{
    FMC->ISPCMD = FMC_ISPCMD_VECMAP;
    FMC->ISPADR = u32PageAddr;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
    while (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk) ;
}


/**
  * @brief Execute ISP command to program a word to flash.
  * @param[in]  u32Addr Address of the flash location to be programmed.
  *             It must be a word aligned address.
  * @param[in]  u32Data The word data to be programmed.
  * @return None
  */
void FMC_Write(uint32_t u32Addr, uint32_t u32Data)
{
    FMC->ISPCMD = FMC_ISPCMD_WRITE;
    FMC->ISPADR = u32Addr;
    FMC->ISPDAT = u32Data;
    FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
    while (FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk) ;
}


/**
  * @brief Execute ISP command to read User Configuration.
  * @param[out]  u32Config A two-word array.
  *              u32Config[0] holds CONFIG0, while u32Config[1] holds CONFIG1.
  * @param[in] u32Count Available word count in u32Config.
  * @return Success or not.
  * @retval   0  Success.
  * @retval   -1  Invalid parameter.
  */
int32_t FMC_ReadConfig(uint32_t *u32Config, uint32_t u32Count)
{
    u32Config[0] = FMC_Read(FMC_CONFIG_BASE);
    if (u32Count < 2)
        return 0;
    u32Config[1] = FMC_Read(FMC_CONFIG_BASE+4);
    return 0;
}


/**
  * @brief Execute ISP command to write User Configuration.
  * @param[in] u32Config A two-word array.
  *            u32Config[0] holds CONFIG0, while u32Config[1] holds CONFIG1.
  * @param[in] u32Count Available word count in u32Config.
  * @return Success or not.
  * @retval   0  Success.
  * @retval   -1  Invalid parameter.
  */
int32_t FMC_WriteConfig(uint32_t *u32Config, uint32_t u32Count)
{
    FMC_EnableConfigUpdate();
    FMC_Erase(FMC_CONFIG_BASE);
    FMC_Write(FMC_CONFIG_BASE, u32Config[0]);
    FMC_Write(FMC_CONFIG_BASE+4, u32Config[1]);
    FMC_DisableConfigUpdate();
    return 0;
}


/*@}*/ /* end of group NANO1X2_FMC_EXPORTED_FUNCTIONS */

/*@}*/ /* end of group NANO1X2_FMC_Driver */

/*@}*/ /* end of group NANO1X2_Device_Driver */

/*** (C) COPYRIGHT 2014 Nuvoton Technology Corp. ***/


