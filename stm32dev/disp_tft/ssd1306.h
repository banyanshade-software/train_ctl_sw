/**
 * This Library was originally written by Olivier Van den Eede (4ilo) in 2016.
 * Some refactoring was done and SPI support was added by Aleksander Alekseev (afiskon) in 2018.
 *
 * https://github.com/afiskon/stm32-ssd1306
 */

#ifndef __SSD1306_H__
#define __SSD1306_H__

#include <stddef.h>
#include <_ansi.h>

_BEGIN_STD_C

#include "ssd1306_conf.h"

#if defined(STM32F0)
#include "stm32f0xx_hal.h"
#elif defined(STM32F1)
#include "stm32f1xx_hal.h"
#elif defined(STM32F4)
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#elif defined(STM32L0)
#include "stm32l0xx_hal.h"
#elif defined(STM32L4)
#include "stm32l4xx_hal.h"
#elif defined(STM32F3)
#include "stm32f3xx_hal.h"
#elif defined(STM32H7)
#include "stm32h7xx_hal.h"
#elif defined(STM32F7)
#include "stm32f7xx_hal.h"
#else
#error "SSD1306 library was tested only on  STM32F0, STM32F1, STM32F3, STM32F4, STM32F7, STM32L0, STM32L4, STM32H7 MCU families. Please modify ssd1306.h if you know what you are doing. Also please send a pull request if it turns out the library works on other MCU's as well!"
#endif

#include "ssd1306_fonts.h"

/* vvv I2C config vvv */

#ifndef SSD1306_I2C_PORT
#define SSD1306_I2C_PORT        hi2c1
#error define it in board_def.h
#endif

#ifndef SSD1306_I2C_ADDR
#define SSD1306_I2C_ADDR        (0x3C << 1)
#endif

/* ^^^ I2C config ^^^ */

/* vvv SPI config vvv */

#ifndef SSD1306_SPI_PORT
#define SSD1306_SPI_PORT        hspi2
#endif

#ifndef SSD1306_CS_Port
#define SSD1306_CS_Port         GPIOB
#endif
#ifndef SSD1306_CS_Pin
#define SSD1306_CS_Pin          GPIO_PIN_12
#endif

#ifndef SSD1306_DC_Port
#define SSD1306_DC_Port         GPIOB
#endif
#ifndef SSD1306_DC_Pin
#define SSD1306_DC_Pin          GPIO_PIN_14
#endif

#ifndef SSD1306_Reset_Port
#define SSD1306_Reset_Port      GPIOA
#endif
#ifndef SSD1306_Reset_Pin
#define SSD1306_Reset_Pin       GPIO_PIN_8
#endif

/* ^^^ SPI config ^^^ */

#if defined(SSD1306_USE_I2C)
extern I2C_HandleTypeDef SSD1306_I2C_PORT;
#elif defined(SSD1306_USE_SPI)
extern SPI_HandleTypeDef SSD1306_SPI_PORT;
#else
#error "You should define SSD1306_USE_SPI or SSD1306_USE_I2C macro!"
#endif

// SSD1306 OLED height in pixels
#ifndef SSD1306_HEIGHT
#define SSD1306_HEIGHT          64
#endif

// SSD1306 width in pixels
#ifndef SSD1306_WIDTH
#define SSD1306_WIDTH           128
#endif

// some LEDs don't display anything in first two columns
// #define SSD1306_WIDTH           130

#ifndef SSD1306_BUFFER_SIZE
#define SSD1306_BUFFER_SIZE   SSD1306_WIDTH * SSD1306_HEIGHT / 8
#endif

// Enumeration for screen colors
typedef enum {
    Black = 0x00, // Black color, no pixel
    White = 0x01  // Pixel is set. Color depends on OLED
} SSD1306_COLOR;

typedef enum {
    SSD1306_OK = 0x00,
    SSD1306_ERR = 0x01  // Generic error.
} SSD1306_Error_t;

// Struct to store transformations

typedef struct {
    uint8_t x;
    uint8_t y;
} SSD1306_VERTEX;

// Procedure definitions
void ssd1306_Init(uint8_t devnum);
void ssd1306_Fill(uint8_t devnum, SSD1306_COLOR color);
void ssd1306_UpdateScreen(uint8_t devnum);
void ssd1306_DrawPixel(uint8_t devnum, uint8_t x, uint8_t y, SSD1306_COLOR color);
char ssd1306_WriteChar(uint8_t devnum, char ch, FontDef Font, SSD1306_COLOR color);
char ssd1306_WriteString(uint8_t devnum, const char* str, FontDef Font, SSD1306_COLOR color);
char ssd1306_WriteNString(uint8_t devnum, const char* str, int max, FontDef Font, SSD1306_COLOR color);
void ssd1306_SetCursor(uint8_t devnum, uint8_t x, uint8_t y);
void ssd1306_Line(uint8_t devnum, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color);
void ssd1306_DrawArc(uint8_t devnum, uint8_t x, uint8_t y, uint8_t radius, uint16_t start_angle, uint16_t sweep, SSD1306_COLOR color);
void ssd1306_DrawCircle(uint8_t devnum, uint8_t par_x, uint8_t par_y, uint8_t par_r, SSD1306_COLOR color);
void ssd1306_Polyline(uint8_t devnum, const SSD1306_VERTEX *par_vertex, uint16_t par_size, SSD1306_COLOR color);
void ssd1306_DrawRectangle(uint8_t devnum, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, SSD1306_COLOR color);
/**
 * @brief Sets the contrast of the display.
 * @param[in] value contrast to set.
 * @note Contrast increases as the value increases.
 * @note RESET = 7Fh.
 */
void ssd1306_SetContrast(uint8_t devnum, const uint8_t value);
/**
 * @brief Set Display ON/OFF.
 * @param[in] on 0 for OFF, any for ON.
 */
void ssd1306_SetDisplayOn(uint8_t devnum, const uint8_t on);
/**
 * @brief Reads DisplayOn state.
 * @return  0: OFF.
 *          1: ON.
 */
uint8_t ssd1306_GetDisplayOn(uint8_t devnum);

// Low-level procedures
//void ssd1306_Reset(void);
//void ssd1306_WriteCommand(uint8_t byte);
//void ssd1306_WriteData(uint8_t* buffer, size_t buff_size);
SSD1306_Error_t ssd1306_FillBuffer(uint8_t devnum, uint8_t* buf, uint32_t len);

/* ---------- DBN addon -------- */

// fill a zone. y and wy are supposed to be multiple of 8
void ssd1306_FillZone(uint8_t devnum, uint8_t x, uint8_t y, uint8_t wx, uint8_t wy, SSD1306_COLOR color);
uint8_t ssd1306_GetCursorX(uint8_t devnum);
uint8_t ssd1306_GetCursorY(uint8_t devnum);


_END_STD_C

#endif // __SSD1306_H__
