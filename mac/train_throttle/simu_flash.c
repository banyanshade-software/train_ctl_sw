//
//  simu_flash.c
//  train_throttle
//
//  Created by danielbraun on 17/04/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#include "simu_flash.h"



int W25qxx_Init(void);

//void W25qxx_EraseChip(void);
//void W25qxx_EraseSector(uint32_t SectorAddr);
void W25qxx_EraseBlock(uint32_t BlockAddr)
{
    
}



//###################################################################################################################
uint32_t W25qxx_PageToSector(uint32_t PageAddress)
{
    return ((PageAddress * 256) / 4096);
}
//###################################################################################################################
uint32_t W25qxx_PageToBlock(uint32_t PageAddress)
{
    return ((PageAddress * 256) / 65536);
}
//###################################################################################################################
uint32_t W25qxx_SectorToBlock(uint32_t SectorAddress)
{
    return ((SectorAddress * 4096) / 65536);
}
//###################################################################################################################
uint32_t W25qxx_SectorToPage(uint32_t SectorAddress)
{
    return (SectorAddress * 4096) / 256;
}

//###################################################################################################################
uint32_t W25qxx_SectorToAddr(uint32_t SectorAddress)
{
    return (SectorAddress * 4096);
}


//###################################################################################################################
uint32_t W25qxx_BlockToPage(uint32_t BlockAddress)
{
    return (BlockAddress * 65536) / 256;
}
//###################################################################################################################
uint32_t W25qxx_BlockToSector(uint32_t BlockAddress)
{
    return (BlockAddress * 65536) / 4096;
}



//bool W25qxx_IsEmptyPage(uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize);
//bool W25qxx_IsEmptySector(uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize);
//bool W25qxx_IsEmptyBlock(uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize);

void W25qxx_WriteByte(uint8_t pBuffer, uint32_t Bytes_Address)
{}
void W25qxx_WritePage(uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize)
{}
void W25qxx_WriteSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_SectorSize)
{}
//void W25qxx_WriteBlock(uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize);

//void W25qxx_ReadByte(uint8_t *pBuffer, uint32_t Bytes_Address);
void W25qxx_ReadBytes(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{}
void W25qxx_ReadPage(uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize)
{}
//void W25qxx_ReadSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_SectorSize);
//void W25qxx_ReadBlock(uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize);

int W25qxx_Init(void)
{
    return 0;
}

