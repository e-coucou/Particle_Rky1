#include "font.h"
#include "tft.h"
#include "math.h"


#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

GFX::GFX(int16_t w, int16_t h): WIDTH(w), HEIGHT(h)
{
    _width    = WIDTH;
    _height   = HEIGHT;
    rotation  = 0;
    cursor_y  = cursor_x    = 0;
    textsize  = 1;
    textcolor = textbgcolor = 0xFFFF;
    wrap      = true;
}
// Draw Tor !
void GFX::drawTor(int16_t x0, int16_t y0, int16_t r, int16_t R, int16_t v, uint16_t color) {
    int16_t L,l,m,c;
    float a= ((100-v) / 100.0 * 270.0) - 45;
    float t=tan(a*M_PI/180.0);
    
    for (int16_t y=0;y<=R;y++) {
        if (y<= r) l = sqrt(r*r-y*y);
        else l=0;
        L = sqrt((R*R)-(y*y));
        c=MIN(MAX(abs(y/t),l),L);
        m= MIN(MAX(l,y),L);

        if ( a >= 180) {
            drawFastHLine(x0-L,y+y0,L-c,color); // bas gauche
        } else {
            if ( a>= 90) {
                drawFastHLine(x0-L,y+y0,L-m,color); // bas gauche
                drawFastHLine(x0-L,y0-y,L-c,color); // haut gauche
            } else {
                if ( a >=0) {
                    drawFastHLine(x0-L,y+y0,L-m,color); // bas gauche
                    drawFastHLine(x0-L,y0-y,L-l,color); // haut gauche
                    drawFastHLine(x0+l,y0-y,c-l,color); // haut droit
                } else {
                    drawFastHLine(x0-L,y+y0,L-m,color); // bas gauche
                    drawFastHLine(x0-L,y0-y,L-l,color); // haut gauche
                    drawFastHLine(x0+l,y0-y,L-l,color); // haut droit 
                    drawFastHLine(x0+c,y+y0,L-c,color); // bas droit
                }
            }
        }
    }
}
// Draw a circle outline
void GFX::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
int16_t f = 1 - r;
int16_t ddF_x = 1;
int16_t ddF_y = -2 * r;
int16_t x = 0;
int16_t y = r;

drawPixel(x0  , y0+r, color);
drawPixel(x0  , y0-r, color);
drawPixel(x0+r, y0  , color);
drawPixel(x0-r, y0  , color);

while (x<y) {
if (f >= 0) {
y--;
ddF_y += 2;
f += ddF_y;
}
x++;
ddF_x += 2;
f += ddF_x;

drawPixel(x0 + x, y0 + y, color);
drawPixel(x0 - x, y0 + y, color);
drawPixel(x0 + x, y0 - y, color);
drawPixel(x0 - x, y0 - y, color);
drawPixel(x0 + y, y0 + x, color);
drawPixel(x0 - y, y0 + x, color);
drawPixel(x0 + y, y0 - x, color);
drawPixel(x0 - y, y0 - x, color);
}
}

void GFX::drawCircleHelper( int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color) {
int16_t f     = 1 - r;
int16_t ddF_x = 1;
int16_t ddF_y = -2 * r;
int16_t x     = 0;
int16_t y     = r;

while (x<y) {
if (f >= 0) {
y--;
ddF_y += 2;
f     += ddF_y;
}
x++;
ddF_x += 2;
f     += ddF_x;
if (cornername & 0x4) {
drawPixel(x0 + x, y0 + y, color);
drawPixel(x0 + y, y0 + x, color);
}
if (cornername & 0x2) {
drawPixel(x0 + x, y0 - y, color);
drawPixel(x0 + y, y0 - x, color);
}
if (cornername & 0x8) {
drawPixel(x0 - y, y0 + x, color);
drawPixel(x0 - x, y0 + y, color);
}
if (cornername & 0x1) {
drawPixel(x0 - y, y0 - x, color);
drawPixel(x0 - x, y0 - y, color);
}
}
}

void GFX::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    drawFastVLine(x0, y0-r, 2*r+1, color);
    fillCircleHelper(x0, y0, r, 3, 0, color);
}

// Used to do circles and roundrects
void GFX::fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color) {

    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;

        if (cornername & 0x1) {
            drawFastVLine(x0+x, y0-y, 2*y+1+delta, color);
            drawFastVLine(x0+y, y0-x, 2*x+1+delta, color);
        }
        if (cornername & 0x2) {
            drawFastVLine(x0-x, y0-y, 2*y+1+delta, color);
            drawFastVLine(x0-y, y0-x, 2*x+1+delta, color);
        }
    }
}

// Bresenham's algorithm - thx wikpedia
void GFX::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0<=x1; x0++) {
        if (steep) {
            drawPixel(y0, x0, color);
        } else {
            drawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

// Draw a rectangle
void GFX::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    drawFastHLine(x, y, w, color);
    drawFastHLine(x, y+h-1, w, color);
    drawFastVLine(x, y, h, color);
    drawFastVLine(x+w-1, y, h, color);
}

void GFX::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
// Update in subclasses if desired!
    drawLine(x, y, x, y+h-1, color);
}

void GFX::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
// Update in subclasses if desired!
    drawLine(x, y, x+w-1, y, color);
}

void GFX::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
// Update in subclasses if desired!
    for (int16_t i=x; i<x+w; i++) {
        drawFastVLine(i, y, h, color);
    }
}

void GFX::fillScreen(uint16_t color) {
fillRect(0, 0, _width, _height, color);
}

// Draw a rounded rectangle
void GFX::drawRoundRect(int16_t x, int16_t y, int16_t w,
int16_t h, int16_t r, uint16_t color) {
// smarter version
drawFastHLine(x+r  , y    , w-2*r, color); // Top
drawFastHLine(x+r  , y+h-1, w-2*r, color); // Bottom
drawFastVLine(x    , y+r  , h-2*r, color); // Left
drawFastVLine(x+w-1, y+r  , h-2*r, color); // Right
// draw four corners
drawCircleHelper(x+r    , y+r    , r, 1, color);
drawCircleHelper(x+w-r-1, y+r    , r, 2, color);
drawCircleHelper(x+w-r-1, y+h-r-1, r, 4, color);
drawCircleHelper(x+r    , y+h-r-1, r, 8, color);
}

// Fill a rounded rectangle
void GFX::fillRoundRect(int16_t x, int16_t y, int16_t w,
int16_t h, int16_t r, uint16_t color) {
// smarter version
fillRect(x+r, y, w-2*r, h, color);

// draw four corners
fillCircleHelper(x+w-r-1, y+r, r, 1, h-2*r-1, color);
fillCircleHelper(x+r    , y+r, r, 2, h-2*r-1, color);
}

// Draw a triangle
void GFX::drawTriangle(int16_t x0, int16_t y0,
int16_t x1, int16_t y1,
int16_t x2, int16_t y2, uint16_t color) {
drawLine(x0, y0, x1, y1, color);
drawLine(x1, y1, x2, y2, color);
drawLine(x2, y2, x0, y0, color);
}

// Fill a triangle
void GFX::fillTriangle ( int16_t x0, int16_t y0,
int16_t x1, int16_t y1,
int16_t x2, int16_t y2, uint16_t color) {

int16_t a, b, y, last;

// Sort coordinates by Y order (y2 >= y1 >= y0)
if (y0 > y1) {
swap(y0, y1); swap(x0, x1);
}
if (y1 > y2) {
swap(y2, y1); swap(x2, x1);
}
if (y0 > y1) {
swap(y0, y1); swap(x0, x1);
}

if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
a = b = x0;
if(x1 < a)      a = x1;
else if(x1 > b) b = x1;
if(x2 < a)      a = x2;
else if(x2 > b) b = x2;
drawFastHLine(a, y0, b-a+1, color);
return;
}

int16_t
dx01 = x1 - x0,
dy01 = y1 - y0,
dx02 = x2 - x0,
dy02 = y2 - y0,
dx12 = x2 - x1,
dy12 = y2 - y1,
sa   = 0,
sb   = 0;

// For upper part of triangle, find scanline crossings for segments
// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
// is included here (and second loop will be skipped, avoiding a /0
// error there), otherwise scanline y1 is skipped here and handled
// in the second loop...which also avoids a /0 error here if y0=y1
// (flat-topped triangle).
if(y1 == y2) last = y1;   // Include y1 scanline
else         last = y1-1; // Skip it

for(y=y0; y<=last; y++) {
a   = x0 + sa / dy01;
b   = x0 + sb / dy02;
sa += dx01;
sb += dx02;
/* longhand:
a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
*/
if(a > b) swap(a,b);
drawFastHLine(a, y, b-a+1, color);
}

// For lower part of triangle, find scanline crossings for segments
// 0-2 and 1-2.  This loop is skipped if y1=y2.
sa = dx12 * (y - y1);
sb = dx02 * (y - y0);
for(; y<=y2; y++) {
a   = x1 + sa / dy12;
b   = x0 + sb / dy02;
sa += dx12;
sb += dx02;
/* longhand:
a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
*/
if(a > b) swap(a,b);
drawFastHLine(a, y, b-a+1, color);
}
}

void GFX::drawBitmap(int16_t x, int16_t y,
const uint8_t *bitmap, int16_t w, int16_t h,
uint16_t color) {

int16_t i, j, byteWidth = (w + 7) / 8;

for(j=0; j<h; j++) {
for(i=0; i<w; i++ ) {
if(pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
drawPixel(x+i, y+j, color);
}
}
}
}

size_t GFX::write(uint8_t c) {
    if (c == '\n') {
        cursor_y += textsize*8;
        cursor_x  = 0;
    } else if (c == '\r') {
    // skip em
    } else {
        drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
        cursor_x += textsize*6;
    if (wrap && (cursor_x > (_width - textsize*6))) {
        cursor_y += textsize*8;
        cursor_x = 0;
        }
    }
    return 1;
}

// Draw a character
void GFX::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {

    if((x >= _width)            || // Clip right
        (y >= _height)           || // Clip bottom
        ((x + 6 * size - 1) < 0) || // Clip left
        ((y + 8 * size - 1) < 0))   // Clip top
        return;

    for (int8_t i=0; i<6; i++ ) {
        uint8_t line;
        if (i == 5)
            line = 0x0;
        else
            line = pgm_read_byte(font+(c*5)+i);
        for (int8_t j = 0; j<8; j++) {
            if (line & 0x1) {
                if (size == 1) // default size
                    drawPixel(x+i, y+j, color);
                else {  // big size
                    fillRect(x+(i*size), y+(j*size), size, size, color);
                }
            } else if (bg != color) {
                        if (size == 1) // default size
                            drawPixel(x+i, y+j, bg);
                        else {  // big size
                            fillRect(x+i*size, y+j*size, size, size, bg);
                        }
            }
            line >>= 1;
        }
    }
}

void GFX::setCursor(int16_t x, int16_t y) {
    cursor_x = x;
    cursor_y = y;
}

void GFX::setTextSize(uint8_t s) {
    textsize = (s > 0) ? s : 1;
}

void GFX::setTextColor(uint16_t c) {
// For 'transparent' background, we'll set the bg
// to the same as fg instead of using a flag
    textcolor = textbgcolor = c;
}

void GFX::setTextColor(uint16_t c, uint16_t b) {
    textcolor   = c;
    textbgcolor = b;
}

void GFX::setTextWrap(boolean w) {
    wrap = w;
}

uint8_t GFX::getRotation(void) {
    return rotation;
}

void GFX::setRotation(uint8_t x) {
    rotation = (x & 3);
    switch(rotation) {
        case 0:
        case 2:
            _width  = WIDTH;
            _height = HEIGHT;
            break;
        case 1:
        case 3:
            _width  = HEIGHT;
            _height = WIDTH;
            break;
    }
}

// Return the size of the display (per current rotation)
int16_t GFX::width(void) {
    return _width;
}

int16_t GFX::height(void) {
    return _height;
}

void GFX::invertDisplay(boolean i) {
// Do nothing, must be subclassed if supported
}


inline uint16_t swapcolor(uint16_t x) {
    return (x << 11) | (x & 0x07E0) | (x >> 11);
}


// Constructor when using software SPI.  All output pins are configurable.
ST7735::ST7735(uint8_t cs, uint8_t rs, uint8_t sid, uint8_t sclk, uint8_t rst) : GFX(ST7735_TFTWIDTH, ST7735_TFTHEIGHT) {
    _cs   = cs;
    _rs   = rs;
    _sid  = sid;
    _sclk = sclk;
    _rst  = rst;
    hwSPI = false;
}

// Constructor when using hardware SPI.  Faster, but must use SPI pins
// specific to each board type (e.g. 11,13 for Uno, 51,52 for Mega, etc.)
ST7735::ST7735(uint8_t cs, uint8_t rs, uint8_t rst) : GFX(ST7735_TFTWIDTH, ST7735_TFTHEIGHT) {
    _cs   = cs;
    _rs   = rs;
    _rst  = rst;
    hwSPI = true;
    _sid  = _sclk = 0;
}

inline void ST7735::spiwrite(uint8_t c) {

    if (hwSPI) {
        SPI.transfer(c);
    } else {
    // Fast SPI bitbang swiped from LPD8806 library
        for(uint8_t bit = 0x80; bit; bit >>= 1) {
            if(c & bit) digitalWrite(_sid, HIGH);
            else        digitalWrite(_sid, LOW);
            digitalWrite(_sclk, HIGH);
            digitalWrite(_sclk, LOW);
        }
    }
}

void ST7735::writecommand(uint8_t c) {
    digitalWrite(_rs, LOW);
    digitalWrite(_cs, LOW);
    spiwrite(c);
    digitalWrite(_cs, HIGH);
}

void ST7735::writedata(uint8_t c) {
    digitalWrite(_rs, HIGH);
    digitalWrite(_cs, LOW);
    spiwrite(c);
    digitalWrite(_cs, HIGH);
}


// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80
static const uint8_t PROGMEM
Bcmd[] = {                  // Initialization commands for 7735B screens
18,                       // 18 commands in list:
ST7735_SWRESET,   DELAY,  //  1: Software reset, no args, w/delay
50,                     //     50 ms delay
ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, no args, w/delay
255,                    //     255 = 500 ms delay
ST7735_COLMOD , 1+DELAY,  //  3: Set color mode, 1 arg + delay:
0x05,                   //     16-bit color
10,                     //     10 ms delay
ST7735_FRMCTR1, 3+DELAY,  //  4: Frame rate control, 3 args + delay:
0x00,                   //     fastest refresh
0x06,                   //     6 lines front porch
0x03,                   //     3 lines back porch
10,                     //     10 ms delay
ST7735_MADCTL , 1      ,  //  5: Memory access ctrl (directions), 1 arg:
0x08,                   //     Row addr/col addr, bottom to top refresh
ST7735_DISSET5, 2      ,  //  6: Display settings #5, 2 args, no delay:
0x15,                   //     1 clk cycle nonoverlap, 2 cycle gate
//     rise, 3 cycle osc equalize
0x02,                   //     Fix on VTL
ST7735_INVCTR , 1      ,  //  7: Display inversion control, 1 arg:
0x0,                    //     Line inversion
ST7735_PWCTR1 , 2+DELAY,  //  8: Power control, 2 args + delay:
0x02,                   //     GVDD = 4.7V
0x70,                   //     1.0uA
10,                     //     10 ms delay
ST7735_PWCTR2 , 1      ,  //  9: Power control, 1 arg, no delay:
0x05,                   //     VGH = 14.7V, VGL = -7.35V
ST7735_PWCTR3 , 2      ,  // 10: Power control, 2 args, no delay:
0x01,                   //     Opamp current small
0x02,                   //     Boost frequency
ST7735_VMCTR1 , 2+DELAY,  // 11: Power control, 2 args + delay:
0x3C,                   //     VCOMH = 4V
0x38,                   //     VCOML = -1.1V
10,                     //     10 ms delay
ST7735_PWCTR6 , 2      ,  // 12: Power control, 2 args, no delay:
0x11, 0x15,
ST7735_GMCTRP1,16      ,  // 13: Magical unicorn dust, 16 args, no delay:
0x09, 0x16, 0x09, 0x20, //     (seriously though, not sure what
0x21, 0x1B, 0x13, 0x19, //      these config values represent)
0x17, 0x15, 0x1E, 0x2B,
0x04, 0x05, 0x02, 0x0E,
ST7735_GMCTRN1,16+DELAY,  // 14: Sparkles and rainbows, 16 args + delay:
0x0B, 0x14, 0x08, 0x1E, //     (ditto)
0x22, 0x1D, 0x18, 0x1E,
0x1B, 0x1A, 0x24, 0x2B,
0x06, 0x06, 0x02, 0x0F,
10,                     //     10 ms delay
ST7735_CASET  , 4      ,  // 15: Column addr set, 4 args, no delay:
0x00, 0x02,             //     XSTART = 2
0x00, 0x81,             //     XEND = 129
ST7735_RASET  , 4      ,  // 16: Row addr set, 4 args, no delay:
0x00, 0x02,             //     XSTART = 1
0x00, 0x81,             //     XEND = 160
ST7735_NORON  ,   DELAY,  // 17: Normal display on, no args, w/delay
10,                     //     10 ms delay
ST7735_DISPON ,   DELAY,  // 18: Main screen turn on, no args, w/delay
255 },                  //     255 = 500 ms delay

Rcmd1[] = {                 // Init for 7735R, part 1 (red or green tab)
15,                       // 15 commands in list:
ST7735_SWRESET,   DELAY,  //  1: Software reset, 0 args, w/delay
150,                    //     150 ms delay
ST7735_SLPOUT ,   DELAY,  //  2: Out of sleep mode, 0 args, w/delay
255,                    //     500 ms delay
ST7735_FRMCTR1, 3      ,  //  3: Frame rate ctrl - normal mode, 3 args:
0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
ST7735_FRMCTR2, 3      ,  //  4: Frame rate control - idle mode, 3 args:
0x01, 0x2C, 0x2D,       //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
ST7735_FRMCTR3, 6      ,  //  5: Frame rate ctrl - partial mode, 6 args:
0x01, 0x2C, 0x2D,       //     Dot inversion mode
0x01, 0x2C, 0x2D,       //     Line inversion mode
ST7735_INVCTR , 1      ,  //  6: Display inversion ctrl, 1 arg, no delay:
0x07,                   //     No inversion
ST7735_PWCTR1 , 3      ,  //  7: Power control, 3 args, no delay:
0xA2,
0x02,                   //     -4.6V
0x84,                   //     AUTO mode
ST7735_PWCTR2 , 1      ,  //  8: Power control, 1 arg, no delay:
0xC5,                   //     VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
ST7735_PWCTR3 , 2      ,  //  9: Power control, 2 args, no delay:
0x0A,                   //     Opamp current small
0x00,                   //     Boost frequency
ST7735_PWCTR4 , 2      ,  // 10: Power control, 2 args, no delay:
0x8A,                   //     BCLK/2, Opamp current small & Medium low
0x2A,
ST7735_PWCTR5 , 2      ,  // 11: Power control, 2 args, no delay:
0x8A, 0xEE,
ST7735_VMCTR1 , 1      ,  // 12: Power control, 1 arg, no delay:
0x0E,
ST7735_INVOFF , 0      ,  // 13: Don't invert display, no args, no delay
ST7735_MADCTL , 1      ,  // 14: Memory access control (directions), 1 arg:
0xC8,                   //     row addr/col addr, bottom to top refresh
ST7735_COLMOD , 1      ,  // 15: set color mode, 1 arg, no delay:
0x05 },                 //     16-bit color

Rcmd2green[] = {            // Init for 7735R, part 2 (green tab only)
2,                        //  2 commands in list:
ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
0x00, 0x02,             //     XSTART = 0
0x00, 0x7F+0x02,        //     XEND = 127
ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
0x00, 0x01,             //     XSTART = 0
0x00, 0x9F+0x01 },      //     XEND = 159
Rcmd2red[] = {              // Init for 7735R, part 2 (red tab only)
2,                        //  2 commands in list:
ST7735_CASET  , 4      ,  //  1: Column addr set, 4 args, no delay:
0x00, 0x00,             //     XSTART = 0
0x00, 0x7F,             //     XEND = 127
ST7735_RASET  , 4      ,  //  2: Row addr set, 4 args, no delay:
0x00, 0x00,             //     XSTART = 0
0x00, 0x9F },           //     XEND = 159

Rcmd3[] = {                 // Init for 7735R, part 3 (red or green tab)
4,                        //  4 commands in list:
ST7735_GMCTRP1, 16      , //  1: Magical unicorn dust, 16 args, no delay:
0x02, 0x1c, 0x07, 0x12,
0x37, 0x32, 0x29, 0x2d,
0x29, 0x25, 0x2B, 0x39,
0x00, 0x01, 0x03, 0x10,
ST7735_GMCTRN1, 16      , //  2: Sparkles and rainbows, 16 args, no delay:
0x03, 0x1d, 0x07, 0x06,
0x2E, 0x2C, 0x29, 0x2D,
0x2E, 0x2E, 0x37, 0x3F,
0x00, 0x00, 0x02, 0x10,
ST7735_NORON  ,    DELAY, //  3: Normal display on, no args, w/delay
10,                     //     10 ms delay
ST7735_DISPON ,    DELAY, //  4: Main screen turn on, no args w/delay
100 };                  //     100 ms delay


// Companion code to the above tables.  Reads and issues
// a series of LCD commands stored in PROGMEM byte array.
void ST7735::commandList(const uint8_t *addr) {

    uint8_t  numCommands, numArgs;
    uint16_t ms;

    numCommands = pgm_read_byte(addr++);   // Number of commands to follow
    while(numCommands--) {                 // For each command...
        writecommand(pgm_read_byte(addr++)); //   Read, issue command
        numArgs  = pgm_read_byte(addr++);    //   Number of args to follow
        ms       = numArgs & DELAY;          //   If hibit set, delay follows args
        numArgs &= ~DELAY;                   //   Mask out delay bit
        while(numArgs--) {                   //   For each argument...
            writedata(pgm_read_byte(addr++));  //     Read, issue argument
        }
        if(ms) {
            ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
            if(ms == 255) ms = 500;     // If 255, delay for 500 ms
            delay(ms);
        }
    }
}


// Initialization code common to both 'B' and 'R' type displays
void ST7735::commonInit(const uint8_t *cmdList) {
    colstart  = rowstart = 0; // May be overridden in init func

    pinMode(_rs, OUTPUT);
    pinMode(_cs, OUTPUT);
    if(hwSPI) { // Using hardware SPI
        SPI.begin();
        SPI.setBitOrder(MSBFIRST);
        SPI.setDataMode(SPI_MODE0);
    } else {
        pinMode(_sclk, OUTPUT);
        pinMode(_sid , OUTPUT);

        digitalWrite(_sclk, LOW);
        digitalWrite(_sid, LOW);
    }
// toggle RST low to reset; CS low so it'll listen to us
    digitalWrite(_cs, LOW);
    if (_rst) {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, HIGH);
        delay(500);
        digitalWrite(_rst, LOW);
        delay(500);
        digitalWrite(_rst, HIGH);
        delay(500);
    }

    if(cmdList) commandList(cmdList);
}


// Initialization for ST7735B screens
void ST7735::initB(void) {
    commonInit(Bcmd);
}


// Initialization for ST7735R screens (green or red tabs)
void ST7735::initR(uint8_t options) {
    commonInit(Rcmd1);
    if(options == INITR_GREENTAB) {
        commandList(Rcmd2green);
        colstart = 2;
        rowstart = 1;
    } else {
    // colstart, rowstart left at default '0' values
        commandList(Rcmd2red);
    }
    commandList(Rcmd3);
    // if black, change MADCTL color filter
    if (options == INITR_BLACKTAB) {
        writecommand(ST7735_MADCTL);
        writedata(0xC0);
    }
    tabcolor = options;
}


void ST7735::setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {

    writecommand(ST7735_CASET); // Column addr set
    writedata(0x00);
    writedata(x0+colstart);     // XSTART
    writedata(0x00);
    writedata(x1+colstart);     // XEND

    writecommand(ST7735_RASET); // Row addr set
    writedata(0x00);
    writedata(y0+rowstart);     // YSTART
    writedata(0x00);
    writedata(y1+rowstart);     // YEND

    writecommand(ST7735_RAMWR); // write to RAM
}


void ST7735::pushColor(uint16_t color) {
    digitalWrite(_rs, HIGH);
    digitalWrite(_cs, LOW);
    
    spiwrite(color >> 8);
    spiwrite(color);

    digitalWrite(_cs, HIGH);
}

void ST7735::drawPixel(int16_t x, int16_t y, uint16_t color) {

    if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

    setAddrWindow(x,y,x+1,y+1);

    digitalWrite(_rs, HIGH);
    digitalWrite(_cs, LOW);

    spiwrite(color >> 8);
    spiwrite(color);

    digitalWrite(_cs, HIGH);
}


void ST7735::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {

// Rudimentary clipping
    if((x >= _width) || (y >= _height)) return;
    if((y+h-1) >= _height) h = _height-y;
    setAddrWindow(x, y, x, y+h-1);

    uint8_t hi = color >> 8, lo = color;

    digitalWrite(_rs, HIGH);
    digitalWrite(_cs, LOW);

    while (h--) {
        spiwrite(hi);
        spiwrite(lo);
    }
    digitalWrite(_cs, HIGH);
}


void ST7735::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {

// Rudimentary clipping
    if((x >= _width) || (y >= _height)) return;
    if((x+w-1) >= _width)  w = _width-x;
    setAddrWindow(x, y, x+w-1, y);

    uint8_t hi = color >> 8, lo = color;

    digitalWrite(_rs, HIGH);
    digitalWrite(_cs, LOW);

    while (w--) {
        spiwrite(hi);
        spiwrite(lo);
    }
    digitalWrite(_cs, HIGH);
}

void ST7735::fillScreen(uint16_t color) {
    fillRect(0, 0,  _width, _height, color);
}

// fill a rectangle
void ST7735::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {

// rudimentary clipping (drawChar w/big text requires this)
    if((x >= _width) || (y >= _height)) return;
    if((x + w - 1) >= _width)  w = _width  - x;
    if((y + h - 1) >= _height) h = _height - y;

    setAddrWindow(x, y, x+w-1, y+h-1);

    uint8_t hi = color >> 8, lo = color;
    digitalWrite(_rs, HIGH);
    digitalWrite(_cs, LOW);

    for(y=h; y>0; y--) {
        for(x=w; x>0; x--) {
            spiwrite(hi);
            spiwrite(lo);
        }
    }
    digitalWrite(_cs, HIGH);
}


// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t ST7735::Color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x00
#define MADCTL_BGR 0x08
#define MADCTL_MH  0x04

void ST7735::setRotation(uint8_t m) {

    writecommand(ST7735_MADCTL);
    rotation = m % 4; // can't be higher than 3
    switch (rotation) {
    case 0:
        if (tabcolor == INITR_BLACKTAB) {
            writedata(MADCTL_MX | MADCTL_MY | MADCTL_RGB);
        } else {
            writedata(MADCTL_MX | MADCTL_MY | MADCTL_BGR);
        }
        _width  = ST7735_TFTWIDTH;
        _height = ST7735_TFTHEIGHT;
        break;
    case 1:
        if (tabcolor == INITR_BLACKTAB) {
            writedata(MADCTL_MY | MADCTL_MV | MADCTL_RGB);
        } else {
            writedata(MADCTL_MY | MADCTL_MV | MADCTL_BGR);
        }
        _width  = ST7735_TFTHEIGHT;
        _height = ST7735_TFTWIDTH;
        break;
    case 2:
        if (tabcolor == INITR_BLACKTAB) {
            writedata(MADCTL_RGB);
        } else {
            writedata(MADCTL_BGR);
        }
        _width  = ST7735_TFTWIDTH;
        _height = ST7735_TFTHEIGHT;
        break;
    case 3:
        if (tabcolor == INITR_BLACKTAB) {
            writedata(MADCTL_MX | MADCTL_MV | MADCTL_RGB);
        } else {
            writedata(MADCTL_MX | MADCTL_MV | MADCTL_BGR);
        }
        _width  = ST7735_TFTHEIGHT;
        _height = ST7735_TFTWIDTH;
        break;
    }
}


void ST7735::invertDisplay(boolean i) {
    writecommand(i ? ST7735_INVON : ST7735_INVOFF);
}
