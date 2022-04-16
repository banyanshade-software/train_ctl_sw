#include <stdint.h>
#include <memory.h>

#include "../misc.h"



#include "../stm32/w25qxx/w25qxx.h"
#include "oam_flash.h"
#include "oam.h"



static void check_store_init(int force);

void oam_flash_init(void)
{
	itm_debug1(DBG_OAM|DBG_ERR, "flash init", 0);
	int rc = W25qxx_Init();
	if (rc) {
		Error_Handler();
	}
	// PageSize = 256, PageCount = 8192, SectorSize = 4096, SectorCount = 512, BlockSize = 65536, BlockCount = 32, CapacityInKiloByte = 2048,
	itm_debug3(DBG_OAM, "FLASH s", w25qxx.PageSize, w25qxx.SectorSize, w25qxx.BlockSize);
	itm_debug3(DBG_OAM, "FLASH c", w25qxx.PageCount, w25qxx.SectorCount, w25qxx.BlockCount);




	if ((0)) {
		if ((1)) {
			W25qxx_EraseSector(1);
			char *str = "Hello world";
			W25qxx_WritePage((uint8_t *) str, W25qxx_SectorToPage(1), 0, strlen(str));

		}



		if ((1)) {
			static uint8_t buf[256] = {0};
			W25qxx_ReadBytes(buf, 4096, 32);
			itm_debug3(DBG_OAM, "buf r", buf[0], buf[1], buf[2]);
		}


		if ((1)) {
			uint8_t t = 0xF;
			W25qxx_WritePage(&t, W25qxx_SectorToPage(1), 2, 1);
		}


		if ((1)) {
			static uint8_t buf[256] = {0};
			W25qxx_ReadBytes(buf, 4096, 32);
			itm_debug3(DBG_OAM, "buf r", buf[0], buf[1], buf[2]);
		}
	}

	check_store_init(1);

}

/*
 * generic page buffer, used by any function
 */
typedef union {
	uint8_t b[256];
	uint16_t h[128];
	uint32_t w[64];
	uint64_t ll[32];
} pbuf_t;

static pbuf_t pagebuf;

/*
 * BLOCK 0 (64K) - write protected area, reserved for future use, storing e.g. config version, factory setting, etc
 *
 * BLOCK 1 (64k) - main store (backup=0), possibly write protected
 * BLOCK 2 (64k) - file local store (backup=0), possibly write protected
 * BLOCK 3-7	 - reserved, possibly write protected
 *
 * BLOCK 8		 - main store store (normal=1)
 * BLOCK 9       - file local store (normal=1)
 *
 * store block starts with 1 page of metadata
 */

// blocks store (1,2 and 8,9) starts with magic + configuration generation



static uint16_t currentGeneration = 0;

#define LE_CHR(a,b,c,d) ( ((a)<<24) | ((b)<<16) | ((c)<<8) | (d) )

#define CONF_STORE_MAGIC LE_CHR('s', 't', 'o', 'r')
#define CONF_LOCAL_MAGIC LE_CHR('s', 'l', 'o', 'c')


typedef struct {
	uint8_t valid:1;
	//uint8_t backup:1;
	uint8_t block_num;
	uint8_t startblk;
	uint8_t startsect;
	uint8_t stsect_idx;
	uint16_t gen;
} blk_desc_t;

static blk_desc_t blk_bup0_str = {0};
static blk_desc_t blk_bup0_loc = {0};
static blk_desc_t blk_n1_str = {0};
static blk_desc_t blk_n1_loc = {0};


static void check_block(int blocknum, uint32_t magic, blk_desc_t *blkdesc);
static void format_store_block(int blocknum, uint32_t magic, blk_desc_t *blkdesc, int erase);

static void check_store_init(int force)
{
	check_block(1, CONF_STORE_MAGIC, &blk_bup0_str);
	check_block(2, CONF_LOCAL_MAGIC, &blk_bup0_loc);
	check_block(8, CONF_STORE_MAGIC, &blk_n1_str);
	check_block(9, CONF_LOCAL_MAGIC, &blk_n1_loc);

	currentGeneration = 0;
	if ((blk_bup0_str.valid) && (blk_bup0_str.gen > currentGeneration)) currentGeneration = blk_bup0_str.gen;
	if ((blk_bup0_loc.valid) && (blk_bup0_loc.gen > currentGeneration)) currentGeneration = blk_bup0_loc.gen;
	if ((blk_n1_str.valid) && (blk_n1_str.gen > currentGeneration)) currentGeneration = blk_n1_str.gen;
	if ((blk_n1_loc.valid) && (blk_n1_loc.gen > currentGeneration)) currentGeneration = blk_n1_loc.gen;
	currentGeneration++;

	if (!force) return;
	if (!blk_bup0_str.valid) format_store_block(1,  CONF_STORE_MAGIC, &blk_bup0_str, 1);
	if (!blk_bup0_loc.valid) format_store_block(2,  CONF_LOCAL_MAGIC, &blk_bup0_loc, 1);
	if (!blk_n1_str.valid)   format_store_block(8,  CONF_STORE_MAGIC, &blk_n1_str, 1);
	if (!blk_n1_loc.valid)   format_store_block(9,  CONF_LOCAL_MAGIC, &blk_n1_loc, 1);
}

static void check_block(int blocknum, uint32_t magic, blk_desc_t *blkdesc)
{
	// <magic4> <gen2> <startsect> <startsect> <FF>
	W25qxx_ReadPage(pagebuf.b, W25qxx_BlockToPage(blocknum), 0, sizeof(pagebuf));
	if (pagebuf.w[0] != magic) {
		itm_debug2(DBG_OAM, "mgic not found", pagebuf.w[0], magic);
		return;
	}
	blkdesc->valid = 1;
	blkdesc->block_num = blocknum;
	blkdesc->gen = pagebuf.h[2];
	blkdesc->startsect = 0;
	blkdesc->stsect_idx = 0;
	for (int i=6; i<64; i++) {
		if (pagebuf.b[i]==0xFF) break;
		blkdesc->startsect = pagebuf.b[i];
		blkdesc->stsect_idx = i;
	}
	blkdesc->valid = 1;
	blkdesc->block_num = blocknum;
	blkdesc->gen = pagebuf.h[2];
	blkdesc->startsect = 0;
}

static void format_store_block(int blocknum, uint32_t magic, blk_desc_t *blkdesc, int erase)
{
	memset(&pagebuf, 0xFF, sizeof(pagebuf));
	if (erase) {
		W25qxx_EraseBlock(blocknum);
	}
	pagebuf.w[0] = magic;
	pagebuf.h[2] = currentGeneration;
	W25qxx_WritePage(pagebuf.b, W25qxx_BlockToPage(blocknum), 0, 6);

	blkdesc->stsect_idx = 0;
	blkdesc->block_num = blocknum;
	blkdesc->gen = pagebuf.h[2];
	blkdesc->startsect = 0;

}

void oam_flashstore_set_value(int confnum, int fieldnum, int confbrd, int instnum, int32_t v)
{
	uint64_t enc;
	oam_encode_val40(&enc, confnum, confbrd, instnum, fieldnum, v);
	// TODO
}

static uint32_t rdaddr = 0;
void oam_flashstore_rd_rewind(void)
{
	rdaddr = 0;
}
int  oam_flashstore_rd_next(uint64_t *pval)
{
	*pval = 0LL;
	return 1;
}
