

#include "trainctl_config.h"
#ifndef BOARD_HAS_FLASH
#error BOARD_HAS_FLASH is not defined but oam_flash.c included in build
#endif


#ifndef _W25QXXCONFIG_H
#define _W25QXXCONFIG_H

#define _W25QXX_SPI                   hspi1
#define _W25QXX_CS_GPIO               FLASH_CS_GPIO_Port
#define _W25QXX_CS_PIN                FLASH_CS_Pin
#define _W25QXX_USE_FREERTOS          1
#define _W25QXX_DEBUG                 0

#endif

