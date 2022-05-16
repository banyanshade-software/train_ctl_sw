#include <string.h>
#include <stdio.h>
#include "ssd1306.h"
#include "ssd1306_tests.h"

#define DEVNUM 0

void ssd1306_TestBorder() {
    ssd1306_Fill(DEVNUM, Black);
   
    uint32_t start = HAL_GetTick();
    uint32_t end = start;
    uint8_t x = 0;
    uint8_t y = 0;
    do {
        ssd1306_DrawPixel(DEVNUM, x, y, Black);

        if((y == 0) && (x < 127))
            x++;
        else if((x == 127) && (y < 63))
            y++;
        else if((y == 63) && (x > 0)) 
            x--;
        else
            y--;

        ssd1306_DrawPixel(DEVNUM, x, y, White);
        ssd1306_UpdateScreen(DEVNUM);
    
        HAL_Delay(5);
        end = HAL_GetTick();
    } while((end - start) < 8000);
   
    HAL_Delay(1000);
}

void ssd1306_TestFonts() {
    ssd1306_Fill(DEVNUM, Black);
#ifdef SSD1306_INCLUDE_FONT_16x26
    ssd1306_SetCursor(DEVNUM, 2, 0);
    ssd1306_WriteString(DEVNUM, "Font 16x26", Font_16x26, White);
#endif
#ifdef SSD1306_INCLUDE_FONT_11x18
    ssd1306_SetCursor(DEVNUM, 2, 26);
    ssd1306_WriteString(DEVNUM, "Font 11x18", Font_11x18, White);
#endif
#ifdef SSD1306_INCLUDE_FONT_7x10
    ssd1306_SetCursor(DEVNUM, 2, 26+18);
    ssd1306_WriteString(DEVNUM, "Font 7x10", Font_7x10, White);
#endif
#ifdef SSD1306_INCLUDE_FONT_6x8
    ssd1306_SetCursor(DEVNUM, 2, 26+18+10);
    ssd1306_WriteString(DEVNUM, "Font 6x8", Font_6x8, White);
#endif
    ssd1306_UpdateScreen(DEVNUM);
}

void ssd1306_TestFPS() {
    ssd1306_Fill(DEVNUM, White);
#ifdef SSD1306_INCLUDE_FONT_11x18
    FontDef font = Font_11x18;
#elif defined(SSD1306_INCLUDE_FONT_7x10)
    FontDef font = Font_7x10;
#elif defined(SSD1306_INCLUDE_FONT_6x8)
    FontDef font = Font_6x8;
#elif defined(SSD1306_INCLUDE_FONT_16x26)
    FontDef font = Font_16x26;
#else
#error no font for fps
#endif
    uint32_t start = HAL_GetTick();
    uint32_t end = start;
    int fps = 0;
    char message[] = "ABCDEFGHIJK";
   
    ssd1306_SetCursor(DEVNUM, 2,0);

    ssd1306_WriteString(DEVNUM, "Testing...", font, Black);
   
    do {
        ssd1306_SetCursor(DEVNUM, 2, 18);
        ssd1306_WriteString(DEVNUM, message, font, Black);
        ssd1306_UpdateScreen(DEVNUM);
       
        char ch = message[0];
        memmove(message, message+1, sizeof(message)-2);
        message[sizeof(message)-2] = ch;

        fps++;
        end = HAL_GetTick();
    } while((end - start) < 5000);
   
    HAL_Delay(1000);

    char buff[64];
    fps = (float)fps / ((end - start) / 1000.0);
    snprintf(buff, sizeof(buff), "~%d FPS", fps);
   
    ssd1306_Fill(DEVNUM, White);
    ssd1306_SetCursor(DEVNUM, 2, 18);
    ssd1306_WriteString(DEVNUM, buff, font, Black);
    ssd1306_UpdateScreen(DEVNUM);
}

void ssd1306_TestLine() {

  ssd1306_Line(DEVNUM, 1,1,SSD1306_WIDTH - 1,SSD1306_HEIGHT - 1,White);
  ssd1306_Line(DEVNUM, SSD1306_WIDTH - 1,1,1,SSD1306_HEIGHT - 1,White);
  ssd1306_UpdateScreen(DEVNUM);
  return;
}

void ssd1306_TestRectangle() {
  uint32_t delta;

  for(delta = 0; delta < 5; delta ++) {
    ssd1306_DrawRectangle(DEVNUM, 1 + (5*delta),1 + (5*delta) ,SSD1306_WIDTH-1 - (5*delta),SSD1306_HEIGHT-1 - (5*delta),White);
  }
  ssd1306_UpdateScreen(DEVNUM);
  return;
}

void ssd1306_TestCircle() {
  uint32_t delta;

  for(delta = 0; delta < 5; delta ++) {
    ssd1306_DrawCircle(DEVNUM, 20* delta+30, 30, 10, White);
  }
  ssd1306_UpdateScreen(DEVNUM);
  return;
}

void ssd1306_TestArc() {

  ssd1306_DrawArc(DEVNUM, 30, 30, 30, 20, 270, White);
  ssd1306_UpdateScreen(DEVNUM);
  return;
}

void ssd1306_TestPolyline() {
  SSD1306_VERTEX loc_vertex[] =
  {
      {35,40},
      {40,20},
      {45,28},
      {50,10},
      {45,16},
      {50,10},
      {53,16}
  };

  ssd1306_Polyline(DEVNUM, loc_vertex,sizeof(loc_vertex)/sizeof(loc_vertex[0]),White);
  ssd1306_UpdateScreen(DEVNUM);
  return;
}

void ssd1306_TestAll() {
    ssd1306_Init(DEVNUM);
    ssd1306_TestFPS();
    HAL_Delay(3000);
    ssd1306_TestBorder();
    ssd1306_TestFonts();
    HAL_Delay(3000);
    ssd1306_Fill(DEVNUM, Black);
    ssd1306_TestRectangle();
    ssd1306_TestLine();
    HAL_Delay(3000);
    ssd1306_Fill(DEVNUM, Black);
    ssd1306_TestPolyline();
    HAL_Delay(3000);
    ssd1306_Fill(DEVNUM, Black);
    ssd1306_TestArc();
    HAL_Delay(3000);
    ssd1306_Fill(DEVNUM, Black);
    ssd1306_TestCircle();
    HAL_Delay(3000);
}

