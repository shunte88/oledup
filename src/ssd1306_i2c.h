/*********************************************************************
SSD1306 I2C Library for Raspberry Pi.
Based on Adafruit SSD1306 Arduino library. 
Some functions from Adafruit GFX lib.
*/
/*********************************************************************
This is a library for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

These displays use SPI to communicate, 4 or 5 pins are required to
interface

Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/

#ifndef SSD1306_I2C_H_
#define SSD1306_I2C_H_

#include <stdint.h>

#define BLACK 0
#define WHITE 1
#define INVERSE 2

#define SSD1306_I2C_ADDRESS   0x3C	// 011110+SA0+RW - 0x3C or 0x3D
// Address for 128x32 is 0x3C
// Address for 128x64 is 0x3D (default) or 0x3C (if SA0 is grounded)

/*=========================================================================
    SSD1306 Displays
    -----------------------------------------------------------------------
    The driver is used in multiple displays (128x64, 128x32, etc.).
    Select the appropriate display below to create an appropriately
    sized framebuffer, etc.

    SSD1306_128_64  128x64 pixel display

    SSD1306_128_32  128x32 pixel display

    SSD1306_96_16

    -----------------------------------------------------------------------*/
// #define SSD1306_128_64
#define SSD1306_128_32
//   #define SSD1306_96_16
/*=========================================================================*/

#if defined SSD1306_128_32
        #define WIDTH 128
        #define HEIGHT 32
#endif

#if defined SSD1306_128_64
        #define WIDTH 128
        #define HEIGHT 64
#endif

#if defined SSD1306_96_16
        #define WIDTH 96
        #define HEIGHT 16
#endif

#if defined SSD1306_128_64 && defined SSD1306_128_32
        #error "Only one SSD1306 display can be specified at once in SSD1306.h"
#endif
#if !defined SSD1306_128_64 && !defined SSD1306_128_32 && !defined SSD1306_96_16
        #error "At least one SSD1306 display must be specified in SSD1306.h"
#endif

#if defined SSD1306_128_64
        #define SSD1306_LCDWIDTH                  128
        #define SSD1306_LCDHEIGHT                 64
#endif
#if defined SSD1306_128_32
        #define SSD1306_LCDWIDTH                  128
        #define SSD1306_LCDHEIGHT                 32
#endif
#if defined SSD1306_96_16
        #define SSD1306_LCDWIDTH                  96
        #define SSD1306_LCDHEIGHT                 16
#endif

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10

#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

void ssd1306_startWrite();
void ssd1306_endWrite();

void ssd1306_begin(uint16_t switchvcc, uint16_t i2caddr); //switchvcc should be SSD1306_SWITCHCAPVCC
void ssd1306_command(uint16_t c);

void ssd1306_clearDisplay(void);
void ssd1306_invertDisplay(uint16_t i);
void ssd1306_display();

void ssd1306_startscrollright(uint16_t start, uint16_t stop);
void ssd1306_startscrollleft(uint16_t start, uint16_t stop);

void ssd1306_displayOn(void);
void ssd1306_displayOff(void);

void ssd1306_startscrolldiagright(uint16_t start, uint16_t stop);
void ssd1306_startscrolldiagleft(uint16_t start, uint16_t stop);
void ssd1306_stopscroll(void);

void ssd1306_dim(uint16_t dim);

void ssd1306_drawPixel(int x, int y, uint16_t color);

void ssd1306_drawFastVLine(int x, int y, int h, uint16_t color);
void ssd1306_drawFastHLine(int x, int y, int w, uint16_t color);

void ssd1306_drawRect(int x, int y, int w, int h, uint16_t drawcolor);
void ssd1306_fillRect(int x, int y, int w, int h, uint16_t fillcolor);

void ssd1306_setTextSize(int s);
void ssd1306_drawString(char *str);
void ssd1306_drawText(int x, int y, char *str);
void ssd1306_drawChar(int x, int y, unsigned char c, int color, int size);

void ssd1306_drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[],
      uint16_t w, uint16_t h, uint16_t color);

#endif
