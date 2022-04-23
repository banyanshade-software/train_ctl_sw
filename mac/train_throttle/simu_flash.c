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
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "simu_flash.h"

#define FLASH_SIZE (2*1024*2024)
static uint8_t *w25qmem = NULL;
static int w25fd = -1;

int W25qxx_Init(void)
{
    // mmap flash file
    w25fd = open("/tmp/w25q", O_RDWR|O_CREAT, 0755);
    if (!w25fd) {
        perror("open flash file");
        return -1;
    }
    lseek(w25fd, FLASH_SIZE-1, SEEK_SET);
    uint8_t c = 0;
    write(w25fd, &c, 1);
    w25qmem = mmap(NULL, FLASH_SIZE, PROT_READ|PROT_WRITE, MAP_FILE|MAP_SHARED, w25fd, 0);
    if (!w25qmem || ((long)w25qmem == -1)) {
        perror("mmap");
        return -1;
    }
    return 0;
}

int W25qxx_Deinit(void)
{
    msync(w25qmem, FLASH_SIZE, MS_SYNC);
    munmap(w25qmem, FLASH_SIZE);
    close(w25fd);
    return 0;
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



void W25qxx_EraseChip(void)
{
    memset(w25qmem, 0xFF, FLASH_SIZE);
}
//void W25qxx_EraseSector(uint32_t SectorAddr);
void W25qxx_EraseBlock(uint32_t BlockAddr)
{
    uint32_t addr = W25qxx_SectorToAddr(W25qxx_BlockToSector(BlockAddr));
    memset(w25qmem+addr, 0xFF, 64*1024);
    
}

//bool W25qxx_IsEmptyPage(uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize);
//bool W25qxx_IsEmptySector(uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize);
//bool W25qxx_IsEmptyBlock(uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize);

void W25qxx_WriteByte(uint8_t pBuffer, uint32_t Bytes_Address)
{
    w25qmem[Bytes_Address] &= pBuffer;
}
void W25qxx_WritePage(uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize)
{
    uint32_t p = Page_Address*256+OffsetInByte;
    memcpy(w25qmem+p, pBuffer, NumByteToWrite_up_to_PageSize);
}
void W25qxx_WriteSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_SectorSize)
{
    uint32_t addr = W25qxx_SectorToAddr(Sector_Address) + OffsetInByte;
    memcpy(w25qmem+addr, pBuffer, NumByteToWrite_up_to_SectorSize);
}
//void W25qxx_WriteBlock(uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize);

//void W25qxx_ReadByte(uint8_t *pBuffer, uint32_t Bytes_Address);
void W25qxx_ReadBytes(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{
    memcpy(pBuffer, w25qmem+ReadAddr, NumByteToRead);
}
void W25qxx_ReadPage(uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize)
{
    memcpy(pBuffer, w25qmem+Page_Address*256+OffsetInByte, NumByteToRead_up_to_PageSize);
}
//void W25qxx_ReadSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_SectorSize);
//void W25qxx_ReadBlock(uint8_t *pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize);



