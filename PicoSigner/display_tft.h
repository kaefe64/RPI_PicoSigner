#ifndef __DISPLAY_TFT_H__
#define __DISPLAY_TFT_H__

#ifdef __cplusplus
extern "C" {
#endif



// Definição dos buffers de memória para os bitmaps
// Cada bitmap tem 80x240 pixels, com 2 bytes por pixel (RGB565)
//#define SEGMENT_WIDTH 240
//#define SEGMENT_HEIGHT 80
//#define PIXEL_SIZE 2

/*
#define ROTATION_SETUP  1   // 0, 1, 2 or 3


#if ROTATION_SETUP == 0 || ROTATION_SETUP == 2
//ILI9341
#define display_WIDTH  TFT_WIDTH   //TFT_HEIGHT=240
#define display_HEIGHT TFT_HEIGHT    //TFT_WIDTH=320
#endif

#if ROTATION_SETUP == 1 || ROTATION_SETUP == 3
//ILI9341
#define display_WIDTH  TFT_HEIGHT    //TFT_WIDTH=320
#define display_HEIGHT TFT_WIDTH   //TFT_HEIGHT=240
#endif
*/


/*
#define FONT &FreeMonoBold9pt7b
#define X_CHAR  11
#define Y_CHAR  16
*/
#define FONT0 &FreeMonoBold9pt7b
#define X_CHAR0  11
#define Y_CHAR0  16
#define SIZE0    1

#define FONT3 &FreeMonoBold24pt7b
//#define FONT &FreeMono24pt7b
//#define FONT &FreeSans24pt7b
//#define FONT &FreeSerif24pt7b
#define X_CHAR3  28
#define Y_CHAR3  42
#define SIZE3    1

#define FONT2 &FreeMonoBold18pt7b
//#define FONT2 &FreeMono18pt7b
#define X_CHAR2  21
#define Y_CHAR2  32
#define SIZE2    1

#define FONT1 &FreeMonoBold12pt7b
//#define FONT2 &FreeSans9pt7b
#define X_CHAR1  14
#define Y_CHAR1  22
#define SIZE1    1









// Default color definitions
#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xD69A      /* 211, 211, 211 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
#define TFT_PINK        0xFE19      /* 255, 192, 203 */    
#define TFT_BROWN       0x9A60      /* 150,  75,   0 */
#define TFT_GOLD        0xFEA0      /* 255, 215,   0 */
#define TFT_SILVER      0xC618      /* 192, 192, 192 */
#define TFT_SKYBLUE     0x867D      /* 135, 206, 235 */
#define TFT_VIOLET      0x915C      /* 180,  46, 226 */

/*
uint16_t red =    tft.color565(255, 0, 0);
uint16_t green =  tft.color565(0, 255, 0);
uint16_t blue =   tft.color565(0, 0, 255);
uint16_t yellow = tft.color565(255, 255, 0);
*/


#define FONTS_QTD  4
struct st_font
{
  const GFXfont* font;
  uint16_t width;
  uint16_t height;
};

// Used for displaying Leter board
extern const struct st_font Fonts[FONTS_QTD];



extern TFT_eSPI tft;

#define WORDS_NUM    12
extern char Words[WORDS_NUM][16];
extern uint16_t Word_pos[WORDS_NUM];  //11 bits / word

extern char BtcAddress[100];   //BIP-84 derived address (bc1...) or empty

extern uint16_t displayFont;
extern uint32_t bk_color;
extern uint32_t c_color;
extern uint32_t hl_bk_color;
extern uint32_t hl_c_color;

//extern uint16_t single_bitmap[SEGMENT_WIDTH * SEGMENT_HEIGHT];


//void displayDrawKey(char c, uint16_t col, uint16_t line, uint16_t highlight);
//void displayDrawCharKey(char c, uint16_t x, uint16_t y, uint16_t highlight);
//void drawSegment(uint16_t *bitmapData, int segment_index);
uint16_t tft_color565(uint16_t r, uint16_t g, uint16_t b);
void drawStringToImage(uint16_t *img, int16_t w, int16_t h, int16_t x, int16_t y, const char *str, uint16_t color);
void displayDrawImage(uint16_t *bitmapData, uint16_t w, uint16_t h);
void display_tft_setup(void);
void display_tft_loop(void);
void scr_addr_setup(void);
void scr_addr_loop(void);






#ifdef __cplusplus
}
#endif
#endif
