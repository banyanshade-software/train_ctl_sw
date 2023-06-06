//
//  simu_flash.c
//  train_throttle
//
//  Created by danielbraun on 17/04/2022.
//  Copyright © 2022 Daniel BRAUN. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "oam_flash.h"
#include "oam.h"
#include "../config/propag.h"

/*
typedef struct
{
	W25QXX_ID_t ID;
	uint8_t UniqID[8];
	uint16_t PageSize;
	uint32_t PageCount;
	uint32_t SectorSize;
	uint32_t SectorCount;
	uint32_t BlockSize;
	uint32_t BlockCount;
	uint32_t CapacityInKiloByte;
	uint8_t StatusRegister1;
	uint8_t StatusRegister2;
	uint8_t StatusRegister3;
	uint8_t Lock;

} w25qxx_t;
*/

// real w25 : PageSize = 256, PageCount = 8192, SectorSize = 4096, SectorCount = 512, BlockSize = 65536, BlockCount = 32, CapacityInKiloByte = 2048,


#define PAGE_SIZE 256
#define PAGE_PER_SECTOR 1
#define SECTOR_SIZE (PAGE_SIZE*PAGE_PER_SECTOR)
#define SECTOR_PER_BLOCK 2
#define NUM_BLOCK 5

#define NUM_PAGES (NUM_BLOCK*PAGE_PER_SECTOR)
#define BLOCK_SIZE (SECTOR_PER_BLOCK*PAGE_PER_SECTOR*PAGE_SIZE)

#define FLASH_SIZE (BLOCK_SIZE*NUM_BLOCK)
static uint8_t w25qmem[FLASH_SIZE];

static_assert(FLASH_SIZE == 2560);



int W25qxx_Init(void)
{
    memset(w25qmem, 0xFF, sizeof(w25qmem));
    return 0;
}

int W25qxx_Deinit(void)
{

    return 0;
}



//###################################################################################################################
uint32_t W25qxx_PageToSector(uint32_t PageAddress)
{
    return ((PageAddress * PAGE_SIZE) / SECTOR_SIZE);
}
//###################################################################################################################
uint32_t W25qxx_PageToBlock(uint32_t PageAddress)
{
    return ((PageAddress * PAGE_SIZE) / BLOCK_SIZE);
}
//###################################################################################################################
uint32_t W25qxx_SectorToBlock(uint32_t SectorAddress)
{
    return ((SectorAddress * SECTOR_SIZE) / BLOCK_SIZE);
}
//###################################################################################################################
uint32_t W25qxx_SectorToPage(uint32_t SectorAddress)
{
    return (SectorAddress * SECTOR_SIZE) / PAGE_SIZE;
}

//###################################################################################################################
uint32_t W25qxx_SectorToAddr(uint32_t SectorAddress)
{
    return (SectorAddress * SECTOR_SIZE);
}


//###################################################################################################################
uint32_t W25qxx_BlockToPage(uint32_t BlockAddress)
{
    return (BlockAddress * BLOCK_SIZE) / PAGE_SIZE;
}
//###################################################################################################################
uint32_t W25qxx_BlockToSector(uint32_t BlockAddress)
{
    return (BlockAddress * BLOCK_SIZE) / SECTOR_SIZE;
}



void W25qxx_EraseChip(void)
{
    memset(w25qmem, 0xFF, FLASH_SIZE);
}
//void W25qxx_EraseSector(uint32_t SectorAddr);
void W25qxx_EraseBlock(uint32_t BlockAddr)
{
    uint32_t addr = W25qxx_SectorToAddr(W25qxx_BlockToSector(BlockAddr));
    memset(w25qmem+addr, 0xFF, BLOCK_SIZE);

}

//bool W25qxx_IsEmptyPage(uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize);
//bool W25qxx_IsEmptySector(uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize);
//bool W25qxx_IsEmptyBlock(uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize);


void W25qxx_WriteByte(uint8_t pBuffer, uint32_t Bytes_Address)
{
	if (Bytes_Address>=FLASH_SIZE) return;
    w25qmem[Bytes_Address] &= pBuffer;
}

void W25qxx_WritePage(uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize)
{
	if (Page_Address>=NUM_PAGES) return;
    uint32_t p = Page_Address*256+OffsetInByte;
    memcpy(w25qmem+p, pBuffer, NumByteToWrite_up_to_PageSize);
}
void W25qxx_WriteSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_SectorSize)
{
	uint32_t Page_Address = W25qxx_SectorToPage(Sector_Address);
	if (Page_Address>=NUM_PAGES) return;
    uint32_t addr = W25qxx_SectorToAddr(Sector_Address) + OffsetInByte;
    memcpy(w25qmem+addr, pBuffer, NumByteToWrite_up_to_SectorSize);
}
//void W25qxx_WriteBlock(uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize);

//void W25qxx_ReadByte(uint8_t *pBuffer, uint32_t Bytes_Address);
void W25qxx_ReadBytes(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
	if (ReadAddr>=FLASH_SIZE) return;
    memcpy(pBuffer, w25qmem+ReadAddr, NumByteToRead);
}

void W25qxx_ReadPage(uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize)
{
	if (Page_Address>=NUM_PAGES) return;
    memcpy(pBuffer, w25qmem+Page_Address*256+OffsetInByte, NumByteToRead_up_to_PageSize);
}
//void W25qxx_ReadSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_SectorSize);
//void W25qxx_ReadBlock(uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize);



