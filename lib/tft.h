/***************************************************
This is a library for the Adafruit 1.8" SPI display.
This library works with the Adafruit 1.8" TFT Breakout w/SD card
----> http://www.adafruit.com/products/358
as well as Adafruit raw 1.8" TFT display
----> http://www.adafruit.com/products/618

Check out the links above for our tutorials and wiring diagrams
These displays use SPI to communicate, 4 or 5 pins are required to
interface (RST is optional)
Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.
MIT license, all text above must be included in any redistribution
****************************************************/

#include "application.h"
#define swap(a, b) { int16_t t = a; a = b; b = t; }

class GFX : public Print {

public:
    GFX(int16_t w, int16_t h); // Constructor

// This MUST be defined by the subclass:
virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;

// These MAY be overridden by the subclass to provide device-specific
// optimized code.  Otherwise 'generic' versions are used.

virtual void
    drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color),
    drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color),
    drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color),
    drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
    fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color),
    fillScreen(uint16_t color),
    invertDisplay(boolean i);

// These exist only with Adafruit_GFX (no subclass overrides)
void
    drawTor(int16_t x0, int16_t y0, int16_t r, int16_t R, int16_t v, uint16_t color),  // by EP
    drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color),
    drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,uint16_t color),
    fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color),
    fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername,int16_t delta, uint16_t color),
    drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,int16_t x2, int16_t y2, uint16_t color),
    fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1,int16_t x2, int16_t y2, uint16_t color),
    drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,int16_t radius, uint16_t color),
    fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h,int16_t radius, uint16_t color),
    drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap,int16_t w, int16_t h, uint16_t color),
    drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color,uint16_t bg, uint8_t size),
    setCursor(int16_t x, int16_t y),
    setTextColor(uint16_t c),
    setTextColor(uint16_t c, uint16_t bg),
    setTextSize(uint8_t s),
    setTextWrap(boolean w),
    setRotation(uint8_t r);

virtual size_t write(uint8_t);

int16_t height(void), width(void);

uint8_t getRotation(void);

protected:
const       int16_t WIDTH, HEIGHT;   // This is the 'raw' display w/h - never changes
int16_t     _width, _height, // Display w/h as modified by current rotation
            cursor_x, cursor_y;
uint16_t    textcolor, textbgcolor;
uint8_t     textsize, rotation;
boolean     wrap; // If set, 'wrap' text at right edge of display
};

typedef unsigned char prog_uchar;

// some flags for initR() :(
#define INITR_GREENTAB 0x0
#define INITR_REDTAB   0x1
#define INITR_BLACKTAB   0x2

#define ST7735_TFTWIDTH  128
#define ST7735_TFTHEIGHT 160

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// Color definitions
#define	ST7735_BLACK   0x0000
#define	ST7735_BLUE    0x001F
#define	ST7735_RED     0xF800
#define	ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0
#define ST7735_WHITE   0xFFFF

//-- Class du ST7735 --
class ST7735 : public GFX {

public:

ST7735(uint8_t CS, uint8_t RS, uint8_t SID, uint8_t SCLK,uint8_t RST);
ST7735(uint8_t CS, uint8_t RS, uint8_t RST);

void     
    initB(void),                             // for ST7735B displays
    initR(uint8_t options = INITR_GREENTAB), // for ST7735R
    setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1),
    pushColor(uint16_t color),
    fillScreen(uint16_t color),
    drawPixel(int16_t x, int16_t y, uint16_t color),
    drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color),
    drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color),
    fillRect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color),
    setRotation(uint8_t r),
    invertDisplay(boolean i);
    uint16_t Color565(uint8_t r, uint8_t g, uint8_t b);

private:
    uint8_t  tabcolor;

void
    spiwrite(uint8_t),
    writecommand(uint8_t c),
    writedata(uint8_t d),
    commandList(const uint8_t *addr),
    commonInit(const uint8_t *cmdList);
    //uint8_t  spiread(void);

    boolean  hwSPI;

    uint32_t  _cs, _rs, _rst, _sid, _sclk, datapinmask, clkpinmask, cspinmask, rspinmask, colstart, rowstart; // some displays need this changed

};

//#endif
