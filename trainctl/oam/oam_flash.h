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

/// OaM flash handling
///
/// OAM flash is used to store (and possibly modify) config files
///
///  # type of config file store
///
///  we handle two kind onf config file store
///
///  - "normal" flashstore : config values are to be dispatched to remote boards ; values are thus indexed by (key, board, instance) ; files are store as change sequence
///
/// - "local" flashstore : used only local (master) baord ; files are stored as raw binary sequence
///
/// # simulation
///
/// on Mac simulation, the W25qxx driver is replaced by a mmap() file
///
/// on normal STM32F407VET6 bords, flash is external w25qxx SPI flash
///



/// initialise OAM flash modules
void oam_flash_init(void);

/// end usage of OAM flash modules
void oam_flash_deinit(void);

/// full flash erase
void oam_flash_erase(void); // full erase

/// store value  for 'normal' multiboard configuration store
void oam_flashstore_set_value(int confnum, int fieldnum, int confbrd, int instnum, int32_t v);

/// get value from 'normal' multiboard configuration store
uint32_t oam_flashstore_get_value(int confnum, int fieldnum, int confbrd, int instnum);

// stepped read, with single hidden read pointer

/// rewind flash store (read index is stored locally)
///
/// note that there can be a single use (and thus single task) of oam_flash_rd_* functions at a given time, and a single 'file' open at a time
///
void oam_flashstore_rd_rewind(void);

/// read one item in flash store, returns -1 at EOF
int  oam_flashstore_rd_next(unsigned int *confnum, unsigned int *fieldnum, unsigned int *confbrd, unsigned int *instnum, int32_t *v);


// for 'local' master-only configuration file


/// read local configuration ; file content is stored in array define by conf_*
/// @param confnum file number
void oam_flashlocal_read(int confnum);

/// commit changes performed in array (defined by conf_*)
/// @param confnum file number
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
