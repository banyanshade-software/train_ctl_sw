/*
 * oam_flash.h
 *
 *  Created on: Apr 13, 2022
 *      Author: danielbraun
 */

#ifndef OAM_OAM_FLASH_H_
#define OAM_OAM_FLASH_H_

#include <stdint.h>


void oam_flash_init(void);
void oam_flashstore_set_value(int confnum, int fieldnum, int confbrd, int instnum, int32_t v);


#endif /* OAM_OAM_FLASH_H_ */
