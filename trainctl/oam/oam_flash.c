#include <stdint.h>
#include <memory.h>

#include "../misc.h"


#ifndef BOARD_HAS_FLASH
#error BOARD_HAS_FLASH is not defined but oam_flash.c included in build
#endif

#ifndef TRAIN_SIMU
#include "../stm32/w25qxx/w25qxx.h"
#else
#include "simu_flash.h"
#endif

#include "oam_flash.h"
#include "oam.h"
#include "../config/propag.h"

static void check_store_init(int force);

void oam_flash_deinit(void)
{
    W25qxx_Deinit();
}
void oam_flash_init(void)
{
	oam_flash_begin();
	itm_debug1(DBG_OAM|DBG_ERR, "flash init", 0);
	int rc = W25qxx_Init();
	if (rc) {
		oam_flash_end();
		FatalError("W25ini", "w25wxx_init", Error_FlashInit);

	}
	// PageSize = 256, PageCount = 8192, SectorSize = 4096, SectorCount = 512, BlockSize = 65536, BlockCount = 32, CapacityInKiloByte = 2048,
#ifndef TRAIN_SIMU
	itm_debug3(DBG_OAM, "FLASH s", w25qxx.PageSize, w25qxx.SectorSize, w25qxx.BlockSize);
	itm_debug3(DBG_OAM, "FLASH c", w25qxx.PageCount, w25qxx.SectorCount, w25qxx.BlockCount);
#endif


#ifndef TRAIN_SIMU
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
#endif

	check_store_init(1);


	if ((0)) {
		oam_flashstore_set_value(1, 1, 3, 1, 42);
		oam_flashstore_set_value(1, 1, 3, 2, 56);
		oam_flashstore_set_value(1, 1, 3, 2, 57);
		oam_flashstore_set_value(1, 1, 3, 2, 56);
	}
	if ((0)) {
		oam_flashstore_rd_rewind();
		unsigned int confnum, fieldnum, confbrd, instnum;
		int32_t v;
		while (0==oam_flashstore_rd_next(&confnum, &fieldnum, &confbrd, &instnum, &v)) {
			itm_debug3(DBG_OAM, "rd", fieldnum, instnum, v);
		}
	}
	oam_flash_end();
}

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
	//uint8_t startblk;
	uint8_t startsect;
	uint8_t stsect_idx;
	uint16_t gen;
	//
	uint16_t idx;
} blk_desc_t;


typedef union {
    struct {
        uint8_t confnum:4;
        uint16_t offset32:12; // x 32 bytes
        uint16_t len;         // x 4 bytes
    };
    uint32_t v32;
    uint8_t v8[4];
}  lfiledesc_t;

typedef struct {
    blk_desc_t b;
    uint8_t readdone:1;
    uint8_t needwrite:1;
    uint16_t nbfiledesc;
    lfiledesc_t fdesc[128];
} local_blk_desc_t;





#ifndef TRAIN_SIMU
static_assert(sizeof(lfiledesc_t) == 4);
#else
typedef char compile_assert[(sizeof(lfiledesc_t) == 4) ? 1 : -1];
#endif



static blk_desc_t       blk_bup0_str = {0};
static local_blk_desc_t blk_bup0_loc = {0};
static blk_desc_t       blk_n1_str = {0};
static local_blk_desc_t blk_n1_loc = {0};



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
static uint8_t use_backup = 0;

static void check_block(int blocknum, uint32_t magic, blk_desc_t *blkdesc);
static void format_store_block(int blocknum, uint32_t magic, blk_desc_t *blkdesc, int erase);




void oam_flash_erase(void)
{
	oam_flash_begin();
    W25qxx_EraseChip();
    memset(&blk_bup0_loc, 0, sizeof(blk_bup0_loc));
    memset(&blk_bup0_str, 0, sizeof(blk_bup0_str));
    memset(&blk_n1_str, 0, sizeof(blk_n1_str));
    memset(&blk_n1_loc, 0, sizeof(blk_n1_loc));
    check_store_init(1);
	oam_flash_end();
}


static void check_store_init(int force)
{
	check_block(1, CONF_STORE_MAGIC, &blk_bup0_str);
	check_block(2, CONF_LOCAL_MAGIC, (blk_desc_t *)&blk_bup0_loc);
	check_block(8, CONF_STORE_MAGIC, &blk_n1_str);
	check_block(9, CONF_LOCAL_MAGIC, (blk_desc_t *)&blk_n1_loc);

	currentGeneration = 0;
	if ((blk_bup0_str.valid) && (blk_bup0_str.gen > currentGeneration)) currentGeneration = blk_bup0_str.gen;
	if ((blk_bup0_loc.b.valid) && (blk_bup0_loc.b.gen > currentGeneration)) currentGeneration = blk_bup0_loc.b.gen;
	if ((blk_n1_str.valid) && (blk_n1_str.gen > currentGeneration)) currentGeneration = blk_n1_str.gen;
	if ((blk_n1_loc.b.valid) && (blk_n1_loc.b.gen > currentGeneration)) currentGeneration = blk_n1_loc.b.gen;
	currentGeneration++;

	if (!force) return;
    itm_debug1(DBG_OAM, "fl gen", currentGeneration);
	if (!blk_bup0_str.valid)    format_store_block(1,  CONF_STORE_MAGIC, &blk_bup0_str, 1);
	if (!blk_bup0_loc.b.valid)  format_store_block(2,  CONF_LOCAL_MAGIC, (blk_desc_t *) &blk_bup0_loc, 1);
	if (!blk_n1_str.valid)      format_store_block(8,  CONF_STORE_MAGIC, &blk_n1_str, 1);
	if (!blk_n1_loc.b.valid)    format_store_block(9,  CONF_LOCAL_MAGIC, (blk_desc_t *)&blk_n1_loc, 1);
    
    
}

static void check_block(int blocknum, uint32_t magic, blk_desc_t *blkdesc)
{
	// <magic4> <gen2> <startsect> <startsect> <FF>
	W25qxx_ReadPage(pagebuf.b, W25qxx_BlockToPage(blocknum), 0, sizeof(pagebuf));
	if (pagebuf.w[0] != magic) {
		itm_debug3(DBG_OAM, "mgic not found", blocknum, pagebuf.w[0], magic);
		return;
	}
    itm_debug1(DBG_OAM, "magic ok", blocknum);
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
    itm_debug2(DBG_OAM, "format", blocknum, magic);
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
    blkdesc->valid = 1;

}

/*
 * encoding is different than in OAM msg
 * 0 valid 1 bits, initially 1  > 0 = invalidate
 *   zero  1 bit  => so never 0xFF on first byte
 *   file  6 bits
 * 1 board 8 bits
 * 2 inst  8 bits
 * 3 field 8 bits
 * 4,5,6,7 value 32 bits, machine order
 */

static void store_encode(uint8_t *buf8, unsigned int  confnum, unsigned int  brd, unsigned int  inst, unsigned int  field, int32_t  v)
{
	buf8[0] = 0x80 | (confnum & 0x3F);
	buf8[1] = brd;
	buf8[2] = inst;
	buf8[3] = field;
	memcpy(buf8+4, &v, 4);
}


static void store_decode(uint8_t *buf8, unsigned int *confnum, unsigned int *brd, unsigned int *inst, unsigned int *field, int32_t *pv)
{
	*confnum = buf8[0] & 0x3F;
	*brd = buf8[1];
	*inst = buf8[2];
	*field = buf8[3];
	memcpy(pv, buf8+4, 4);
}

static int store_isvalid(uint8_t *buf8)
{
	return (buf8[0] & 0x80) ? 1 : 0;
}

int store_is_field(uint8_t *buf8, unsigned int  confnum, unsigned int  brd, unsigned int  inst, unsigned int  field, int32_t  *pv)
{
	if (0 == (buf8[0] & 0x80)) return 0; // invalid
	if ((buf8[0] & 0x3F) != confnum) return 0;
	if (buf8[1] != brd) return 0;
	if (buf8[2] != inst) return 0;
	if (buf8[3] != field) return 0;
	if (pv) {
		memcpy(pv, buf8+4, 4);
	}
	return 1;
}

static void store_rewind(blk_desc_t *d);
static int  store_read(blk_desc_t *d, uint8_t *buf);
static void store_disable_lastfield(blk_desc_t *d);
static void store_append_field(blk_desc_t *d, uint8_t *buf);

void oam_flashstore_set_value(int confnum, int fieldnum, int confbrd, int instnum, int32_t v)
{
	oam_flash_begin();

	blk_desc_t *desc = use_backup ? &blk_bup0_str : &blk_n1_str;
	if (!desc->valid) {
		oam_flash_end();
		itm_debug1(DBG_OAM|DBG_ERR, "blk not valid", desc->block_num);
		return;
	}

	// parse current store, check for identical field, invalidate if different value
	store_rewind(desc);
	uint8_t buf[8];
	int found = 0;
	while (0==store_read(desc, buf)) {
		int32_t ov;
        //printf("item: %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X\n",
        //       buf[0], buf[1], buf[2], buf[3],  buf[4], buf[5], buf[6], buf[7]);
		if (!store_isvalid(buf)) continue;
		if (!found && store_is_field(buf, confnum, confbrd, instnum, fieldnum, &ov)) {
			if (ov == v) {
				found = 1;
				continue;
			}
            store_disable_lastfield(desc);
		}
	}
	if (!found) {
		store_encode(buf, confnum, confbrd, instnum, fieldnum, v);
		store_append_field(desc, buf);
	}

	oam_flash_end();
}

uint32_t oam_flashstore_get_value(int confnum, int fieldnum, int confbrd, int instnum)
{
	oam_flash_begin();

    blk_desc_t *desc = use_backup ? &blk_bup0_str : &blk_n1_str;
    if (!desc->valid) {
		oam_flash_end();
        itm_debug1(DBG_OAM|DBG_ERR, "blk not valid", desc->block_num);
        return 0;
    }
    store_rewind(desc);
    uint8_t buf[8];

    int32_t v = conf_default_value(confnum, fieldnum, confbrd, instnum);
    while (0==store_read(desc, buf)) {
        int32_t ov;
        //printf("item: %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X\n",
        //       buf[0], buf[1], buf[2], buf[3],  buf[4], buf[5], buf[6], buf[7]);
        if (!store_isvalid(buf)) continue;
        if (store_is_field(buf, confnum, confbrd, instnum, fieldnum, &ov)) {
            v = ov;
    		oam_flash_end();
            return v;
        }
    }
	oam_flash_end();
	// value not found, get and return global default value (v is initialized with it)
    return v;
}


static void store_rewind(blk_desc_t *d)
{
	if (0== d->startsect) d->idx = 256;
	else d->idx = 0;
}

static int store_read(blk_desc_t *d, uint8_t *buf)
{
	uint32_t s = W25qxx_BlockToSector(d->block_num)+d->startsect;
	s = W25qxx_SectorToAddr(s)+d->idx;
	W25qxx_ReadBytes(buf, s, 8);
	if (0xFF == buf[0]) {
		// end of file
		return -1;
	}
	d->idx += 8;
	return 0;
}

static void store_disable_lastfield(blk_desc_t *d)
{
	uint32_t s = W25qxx_BlockToSector(d->block_num)+d->startsect;
	s = W25qxx_SectorToAddr(s)+d->idx - 8; // -8 because idx is pointing on next data to read
	W25qxx_WriteByte(0x7F, s);
}
static void store_append_field(blk_desc_t *d, uint8_t *buf)
{
	uint32_t s = W25qxx_BlockToSector(d->block_num)+d->startsect;
	//s = W25qxx_SectorToAddr(s)+d->idx;
	W25qxx_WriteSector(buf, s, d->idx, 8);
	d->idx += 8;
}




void oam_flashstore_rd_rewind(void)
{
	oam_flash_begin();
	blk_desc_t *desc = use_backup ? &blk_bup0_str : &blk_n1_str;
	if (!desc->valid) {
		itm_debug1(DBG_OAM|DBG_ERR, "blk not valid", desc->block_num);
		oam_flash_end();
		return;
	}
	store_rewind(desc);
	oam_flash_end();
}

int  oam_flashstore_rd_next(unsigned int *confnum, unsigned int *fieldnum, unsigned int *confbrd, unsigned int *instnum, int32_t *v)
{
	oam_flash_begin();
	blk_desc_t *desc = use_backup ? &blk_bup0_str : &blk_n1_str;
	if (!desc->valid) {
		itm_debug1(DBG_OAM|DBG_ERR, "blk not valid", desc->block_num);
		oam_flash_end();
		return -1;
	}
	for (;;)  {
		uint8_t buf[8];
		int rc = store_read(desc, buf);
		if (rc) {
			oam_flash_end();
			return rc;
		}
		if (!store_isvalid(buf)) continue;


		store_decode(buf, confnum, confbrd, instnum, fieldnum, v);
		oam_flash_end();
		return 0;
	}
	oam_flash_end();
}




static int store_local_read(local_blk_desc_t *desc, int fnum, void *ptr, unsigned int s);
static int store_local_write(local_blk_desc_t *desc, int fnum, void *ptr, unsigned int s);



static void _oam_flashlocal_read(unsigned int confnum)
{
    void *ptr = conf_local_ptr(confnum);
    if (!ptr) return;
    unsigned int s = conf_local_size(confnum);
    if (!s) return;
    local_blk_desc_t *desc = use_backup ? &blk_bup0_loc : &blk_n1_loc;
    store_local_read(desc, confnum, ptr, s);
}
void oam_flashlocal_read(int confnum)
{
	oam_flash_begin();
    if (-1 == confnum) {
        for (int i=0; i<16; i++) {
            _oam_flashlocal_read((unsigned int) i);
        }
    } else {
        _oam_flashlocal_read((unsigned int) confnum);
    }
	oam_flash_end();
}


static void _oam_flashlocal_commit(unsigned int confnum)
{
    void *ptr = conf_local_ptr(confnum);
    if (!ptr) return    ;
    unsigned int s = conf_local_size(confnum);
    if (!s) return;
    local_blk_desc_t *desc = use_backup ? &blk_bup0_loc : &blk_n1_loc;
    store_local_write(desc, confnum, ptr, s);
}
void oam_flashlocal_commit(int confnum)
{
	oam_flash_begin();
    if (-1 == confnum) {
        for (int i=0; i<16; i++) {
            _oam_flashlocal_commit((unsigned int)i);
        }
    } else {
        _oam_flashlocal_commit((unsigned int)confnum);
    }
    oam_flash_end();
}


void oam_flashlocal_set_value(int confnum, int fieldnum, int instnum, int32_t v)
{
    conf_local_set(confnum, fieldnum, instnum, v);
}
uint32_t oam_flashlocal_get_value(int confnum, int fieldnum,  int instnum)
{
    return conf_local_get(confnum, fieldnum, instnum);
}



static void store_local_read_fdesc(local_blk_desc_t *desc)
{
    if (sizeof(lfiledesc_t) != 4) FatalError("sizeof", "bad size", Error_Sizeof);
    store_rewind(&desc->b);
    desc->nbfiledesc = 0;
    for (;;) {
        uint32_t addr = W25qxx_BlockToSector(desc->b.block_num)+desc->b.startsect;
        addr = W25qxx_SectorToAddr(addr) + desc->b.idx;
        itm_debug2(DBG_OAM, "read desc", addr, sizeof(lfiledesc_t));
        W25qxx_ReadBytes((uint8_t *)&(desc->fdesc[desc->nbfiledesc]), addr, sizeof(lfiledesc_t));
        desc->b.idx += sizeof(lfiledesc_t);
        if (desc->fdesc[desc->nbfiledesc].v32 == 0xFFFFFFFF) break;
        itm_debug3(DBG_OAM, "r /desc", desc->fdesc[desc->nbfiledesc].confnum, desc->fdesc[desc->nbfiledesc].offset32, desc->fdesc[desc->nbfiledesc].len);
        desc->nbfiledesc++;
    }
    desc->readdone = 1;
}

static void _local_read(local_blk_desc_t *desc, lfiledesc_t *fd, void *ptr, unsigned int s)
{
    uint32_t addr = W25qxx_BlockToSector(desc->b.block_num)+desc->b.startsect;
    addr = W25qxx_SectorToAddr(addr) + fd->offset32*32;
    itm_debug2(DBG_OAM, "LREAD", s, addr);
    W25qxx_ReadBytes(ptr, addr, s);
}


static void _local_write(local_blk_desc_t *desc, lfiledesc_t *fd, void *ptr, unsigned int s)
{
    uint8_t *bptr = (uint8_t *)ptr;
    uint32_t addr = W25qxx_BlockToSector(desc->b.block_num)+desc->b.startsect;
    addr = W25qxx_SectorToAddr(addr) + fd->offset32*32;
    itm_debug2(DBG_OAM, "LWRITE", s, addr);
    for (unsigned int i=0; i<s; i++) { // TODO optimize and write page or sect
        W25qxx_WriteByte(bptr[i], addr+i);
    }
}

static int store_local_read(local_blk_desc_t *desc, int fnum, void *ptr, unsigned int s)
{
    if (!desc->readdone) {
        store_local_read_fdesc(desc);
    }
    for (int i=0; i < desc->nbfiledesc; i++) {
        if (desc->fdesc[i].len == 0) continue;
        if (desc->fdesc[i].confnum != fnum) continue;
        lfiledesc_t *fd = &desc->fdesc[i];
        _local_read(desc, fd, ptr, s);
        return 0;
    }
    return -1;
}

static int store_local_write(local_blk_desc_t *desc, int fnum, void *ptr, unsigned int s)
{
    if (!desc->readdone) {
        store_local_read_fdesc(desc);
    }
    store_rewind(&desc->b);
    uint32_t lastaddr32 = 512/32;
    for (int i=0; i < desc->nbfiledesc; i++) {
        if (desc->fdesc[i].len == 0) continue;
        lastaddr32 = desc->fdesc[i].offset32 + (desc->fdesc[i].len+31)/32;
        if (desc->fdesc[i].confnum != fnum) continue;
        // invalidate this fdesc locally
        desc->fdesc[i].len = 0;
        // ..and on flash
        uint32_t addr = W25qxx_BlockToSector(desc->b.block_num)+desc->b.startsect;
        addr = W25qxx_SectorToAddr(addr) + desc->b.idx+i*sizeof(lfiledesc_t);
        addr += offsetof(lfiledesc_t, len);
        W25qxx_WriteByte(0, addr);
        W25qxx_WriteByte(0, addr+1);
    }
    // append new filedesc localy
    if (desc->nbfiledesc >= 128) FatalError("FLASH", "flash nbdesc", Error_FlashNbDesc);
    lfiledesc_t *fd = &desc->fdesc[desc->nbfiledesc];

    fd->confnum = fnum;
    fd->len = s;
    fd->offset32 = lastaddr32;
    // write it on flash
    uint32_t addr = W25qxx_BlockToSector(desc->b.block_num)+desc->b.startsect;
    addr = W25qxx_SectorToAddr(addr) + desc->b.idx+desc->nbfiledesc*sizeof(lfiledesc_t);
    itm_debug3(DBG_OAM, "w /desc", fd->confnum, fd->offset32, fd->len);
    itm_debug2(DBG_OAM, "write desc", addr, sizeof(lfiledesc_t));
    W25qxx_WriteByte(fd->v8[0], addr++);
    W25qxx_WriteByte(fd->v8[1], addr++);
    W25qxx_WriteByte(fd->v8[2], addr++);
    W25qxx_WriteByte(fd->v8[3], addr++);
    
    desc->nbfiledesc ++;

    // do actual write
    _local_write(desc, fd, ptr, s);
    return 0;
}



__weak void oam_flash_begin(void)
{
}
__weak void oam_flash_end(void)
{
}

__weak void oam_flash_unneeded(void)
{
}



    
