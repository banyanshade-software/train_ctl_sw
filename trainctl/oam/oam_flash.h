/*
 * oam_flash.h
 *
 *  Created on: Apr 13, 2022
 *      Author: danielbraun
 */

#ifndef OAM_OAM_FLASH_H_
#define OAM_OAM_FLASH_H_

#include <stdint.h>

#include "trainctl_config.h"
#ifndef BOARD_HAS_FLASH
#error BOARD_HAS_FLASH is not defined but oam_flash.c included in build
#endif

void oam_flash_init(void);
void oam_flash_deinit(void);

void oam_flash_erase(void); // full erase

// for 'normal' multiboard configuration store

void oam_flashstore_set_value(int confnum, int fieldnum, int confbrd, int instnum, int32_t v);
uint32_t oam_flashstore_get_value(int confnum, int fieldnum, int confbrd, int instnum);

// stepped read, with single hidden read pointer
void oam_flashstore_rd_rewind(void);
int  oam_flashstore_rd_next(unsigned int *confnum, unsigned int *fieldnum, unsigned int *confbrd, unsigned int *instnum, int32_t *v);


// for 'local' master-only configuration file


void oam_flashlocal_read(int confnum);
void oam_flashlocal_commit(int confnum);


void oam_flashlocal_set_value(int confnum, int fieldnum, int instnum, int32_t v);
uint32_t oam_flashlocal_get_value(int confnum, int fieldnum,  int instnum);

// Hooks and function
// allowing swapping between SPI1 and SWO (PB3)
// functions are weakly defined in OAM, and can be redefined (in spi1_swo_trace.c)
// oam_flash_begin() and oam_flash_end() hooks are called before and after flash access
// oam_flash_unneeded() may be called manually at init, to enable SWO

void oam_flash_begin(void);
void oam_flash_end(void);
void oam_flash_unneeded(void);


#endif /* OAM_OAM_FLASH_H_ */
