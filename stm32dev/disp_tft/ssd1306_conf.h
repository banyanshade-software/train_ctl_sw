/**
 * Private configuration file for the SSD1306 library.
 * This example is configured for STM32F0, I2C and including all fonts.
 */

#ifndef __SSD1306_CONF_H__
#define __SSD1306_CONF_H__

#include "trainctl_config.h"


#ifdef TRN_BOARD_UI
#define MAX_DISPLAY 4
#else
#define MAX_DISPLAY 1
#endif
// Choose a microcontroller family
//#define STM32F0
//#define STM32F1
//#define STM32F4
//#define STM32L0
//#define STM32L4
//#define STM32F3
//#define STM32H7
//#define STM32F7

// Choose a bus
#define SSD1306_USE_I2C
//#define SSD1306_USE_SPI

// I2C Configuration
// TODO move port to boards_def.h
#ifndef SSD1306_I2C_PORTS
#define SSD1306_I2C_PORTS       { &hi2c1 }
#error define it in board_def.h
#endif
#ifndef SSD1306_I2C_ADDRS
#define SSD1306_I2C_ADDRS       { (0x3C << 1) }
#endif

// Mirror the screen if needed
// #define SSD1306_MIRROR_VERT
// #define SSD1306_MIRROR_HORIZ

// Set inverse color if needed
// # define SSD1306_INVERSE_COLOR

// Include only needed fonts
#define SSD1306_INCLUDE_FONT_6x8
#define SSD1306_INCLUDE_FONT_7x10
#define SSD1306_INCLUDE_FONT_11x18
//#define SSD1306_INCLUDE_FONT_16x26



// SSD1306 OLED height in pixels
#define SSD1306_HEIGHT          32
// SSD1306 width in pixels
#define SSD1306_WIDTH           128


#endif /* __SSD1306_CONF_H__ */
