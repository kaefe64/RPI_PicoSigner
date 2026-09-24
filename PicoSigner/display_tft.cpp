/*
 * 
 * 
 * Created: Aug 2025
 * Author: Klaus Fensterseifer


Library  TFT_eSPI by Bodmer


>>Mods in the Library files to fit to the project:
=================================================

--------------------------------------------------------------
--------------------------------------------------------------
>>TFT_eSPI LIBRARY:
----------------------------------------------------
>>Change Arduino/libraries/TFT_eSPI/User_Setup_Select.h
>>Comment:
//#include <User_Setup.h>           // Default setup is root library folder
>>UnComment:
#include <User_Setups/Setup60_RP2040_ILI9341.h>    // Setup file for RP2040 with SPI ILI9341

----------------------------------------------------
>>On User_Setups/Setup60_RP2040_ILI9341.h
>>Uncomment:
#define ILI9341_DRIVER
#define TFT_RGB_ORDER TFT_RGB  // Colour order Red-Green-Blue

>>Choose the SPI pins (SPI1):
// For the Pico use these #define lines
#define TFT_MISO  4    // GP4 - Display SDO/MISO (ou NC se nao ler o display)
#define TFT_MOSI  3    // GP3 - Display SDI/MOSI
#define TFT_SCLK  2    // GP2 - Display SCK
#define TFT_CS    5    // GP5 - Chip Select pin
#define TFT_DC    1    // GP1 - Data Command control pin
#define TFT_RST   0    // GP0 - Reset pin
//#define TFT_BL     // LED back-light

#define TOUCH_CS 6     // Chip select pin (T_CS) of touch screen

>>Choose SPI 1
#define TFT_SPI_PORT 1   // Set to 0 if SPI0 pins are used, or 1 if SPI1 pins used




Display TFT ILI9341 240x320 (anteriormente ST7789)


Display TFT SPI ILI9341 240x320 with touch 
2.4" SKU: MSP2402 
XC6206P332MR (662K) REG 3V3 200mA SOT23
XPT2046 touch
Info:
https://simple-circuit.com/interfacing-arduino-ili9341-tft-display/?utm_source=chatgpt.com
https://openhasp.haswitchplate.com/0.6.3/displays/MSPxxxx/?utm_source=chatgpt.com


VCC        5V (or 3V3 with jumper J1)
GND        GND
CS         GP5
RES        GP0
DC         GP1
SDI/MOSI   GP3
SCK        GP2
LED        3V3 (or logic level)
SDO/MISO   GP4 (or NC if no display reading)
T_CLK      GP2
T_CS       GP6
T_DIN      GP3
T_DO       GP4
T_IRQ      GP7 (or NC if polling to know  pressed - not used by library)





*/

#include "Arduino.h"
#include <string.h>
// A configuracao do TFT (ILI9341) e feita editando o User_Setup_Select.h da biblioteca TFT_eSPI
#include "TFT_eSPI.h"   //Library  TFT_eSPI by Bodmer
#include "display_tft.h"
#include "display_touch.h"
#include "key_input.h"
#include "Cam_OV7670.h"
#include "BitcoinWords.h"
#include "qr_code.h"
#include "bip84.h"


TFT_eSPI tft = TFT_eSPI();


/*
#define FONTS_QTD  3
struct st_font
{
  const GFXfont* font;
  uint16_t width;
  uint16_t height;
};
*/


// Used for displaying Leter board
const struct st_font Fonts[FONTS_QTD]={ {FONT1, X_CHAR1, Y_CHAR1},
                                        {FONT2, X_CHAR2, Y_CHAR2},
                                        {FONT3, X_CHAR3, Y_CHAR3},
                                        {FONT0, X_CHAR0, Y_CHAR0} };



uint16_t scr = SCR_MAIN;

uint16_t displayFont;
uint16_t displayRotation = 2;
uint32_t bk_color;
uint32_t c_color;
uint32_t hl_bk_color;
uint32_t hl_c_color;

uint16_t words_selec_num=0;
uint16_t words_selec_len;
char Words_selec[16];


//char Words[WORDS_NUM][16] = {"ab","","","","","","","","","","",""};
char Words[WORDS_NUM][16] = {"spy", "profit", "item", "promote", "equal", "wealth", "nice", "prize", "cute", "lawsuit", "stage", "capital"};
uint16_t Word_pos[WORDS_NUM] = {1690,	1374,	950,	1377,	608,	1985,	1195,	1370,	437,	1009,	1697,	272 };   //11 bits / word

//default de teste = vetor BIP-84 oficial (abandon x11 + about)
//char Words[WORDS_NUM][16] = {"abandon", "abandon", "abandon", "abandon", "abandon", "abandon", "abandon", "abandon", "abandon", "abandon", "abandon", "about"};
//uint16_t Word_pos[WORDS_NUM] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3 };   //11 bits / word

char BtcAddress[100] = "";   //BIP-84 derived address (bc1...) or empty

char SearchResults[SEARCH_RESULTS_MAX][16];
uint16_t search_results_num = 0;

uint16_t lin_selec;
uint16_t col_selec;
uint16_t lin_ini;
uint16_t col_ini;






/* used to allow calling from other modules, concentrate the use of tft variable locally */
//============================================================================
uint16_t tft_color565(uint16_t r, uint16_t g, uint16_t b)
{
  return tft.color565(r, g, b);
}


//============================================================================
void displayDrawImage(uint16_t *bitmapData, uint16_t w, uint16_t h) 
{
  tft.pushImage(0, 0, w, h, bitmapData);
}








//============================================================================
void drawCharToImage(uint16_t *img, int16_t imgW, int16_t imgH,
                     int16_t *x, int16_t y, char c, uint16_t color)
{
  const GFXfont *font = Fonts[displayFont].font;
  if(c < font->first || c > font->last) return;
  GFXglyph *glyph = &font->glyph[c - font->first];
  uint8_t  *bitmap = font->bitmap;

  uint16_t bo = glyph->bitmapOffset;
  uint8_t  w  = glyph->width;
  uint8_t  h  = glyph->height;
  int8_t   xo = glyph->xOffset;
  int8_t   yo = glyph->yOffset;

  uint8_t  bits = 0, bit = 0;
  for (uint8_t yy = 0; yy < h; yy++) 
  {
      for (uint8_t xx = 0; xx < w; xx++) 
      {
          if (!(bit++ & 7)) bits = bitmap[bo++];
          if (bits & 0x80) 
          {
              int16_t px = *x + xo + xx;
              int16_t py = y + yo + yy;
              if(px >= 0 && px < imgW && py >= 0 && py < imgH)
                  img[py * imgW + px] = (color>>8) | (color<<8);
          }
          bits <<= 1;
      }
  }
  //*x += glyph->width;
  *x += glyph->xAdvance;
}

//============================================================================
void drawStringToImage(uint16_t *img, int16_t w, int16_t h,
                       int16_t x, int16_t y, const char *str, uint16_t color)
{
  while(*str) {
    drawCharToImage(img, w, h, &x, y, *str++, color);  //x moves to the next position
  }
}





// Used for displaying Leter board
char Letters[KEYB_LINES][KEYB_COLS+1]={"abcdefg",
                                       "hijklmn",
                                       "opqrstu",
                                       "vwxyz_<"};




//============================================================================
void displayDrawKey(char c, uint16_t col, uint16_t lin, uint16_t highlight)
{
  uint16_t px = 12 + col * (Fonts[displayFont].width  + 11);
  uint16_t py =        lin * (Fonts[displayFont].height + 4);

  if(highlight == 0)
  {
    tft.fillRoundRect(px-5, py+5-Fonts[displayFont].height, Fonts[displayFont].width+9, Fonts[displayFont].height+2, 5, bk_color);
    tft.drawRoundRect(px-5, py+5-Fonts[displayFont].height, Fonts[displayFont].width+9, Fonts[displayFont].height+2, 5, c_color);
    tft.setTextColor(c_color, bk_color);
    tft.drawChar(c, px, py);
  }
  else
  {
    tft.fillRoundRect(px-5, py+5-Fonts[displayFont].height, Fonts[displayFont].width+9, Fonts[displayFont].height+2, 5, hl_bk_color);
    //tft.drawRoundRect(x-5, y+5-Fonts[displayFont].height, Fonts[displayFont].width+9, Fonts[displayFont].height+2, 5, hl_c_color);
    tft.setTextColor(hl_c_color, hl_bk_color);
    tft.drawChar(c, px, py);   
  }

//    tft.drawRoundRect(optTouch[i].x, optTouch[i].y, 
//                      optTouch[i].w, optTouch[i].h, 
//                      5, TFT_RED);

  if(isTouch == true)   //define touch option on this scr 
  {
    if(numTouch < NUM_MAX_TOUCH_OPTIONS) 
    {
      optTouch[numTouch].x = px;
      //ajuste por causa da posicao que a biblioteca desenha o caracter, nao e em x,y exatamente, 
      //tem um yoffset e x offset (e sao negativos)
      //optTouch[numTouch].y = py + Fonts[displayFont].font->glyph[c - Fonts[displayFont].font->first].yOffset;
      //obs.:    PROGMEM no RP2040 é no-op (memória unificada), acesso direto ok. Se um dia rodar em AVR, use pgm_read_byte(&g->yOffset).
      //optTouch[numTouch].y = py - (Fonts[displayFont].height * 3)/4; //Fonts[displayFont].font->glyph[c - displayFont].font->first].yOffset; 
      optTouch[numTouch].y = py + 9 - Fonts[displayFont].height;
      optTouch[numTouch].w = Fonts[displayFont].width;
      optTouch[numTouch].h = Fonts[displayFont].height;
      optTouch[numTouch].col = col;
      optTouch[numTouch].lin = lin;
      numTouch++;
    }
    isTouch = false;  //touch defined
  }
}


//============================================================================
uint16_t str_len(const char*s)
{
  uint16_t len;
  for(len=0; len<16; len++)  //get string length
    if(s[len]==0) break;
  return len;
}


//============================================================================
void displayDrawWordKey_xy(const char *s, uint16_t x, uint16_t y, uint16_t highlight)
{
  //uint32_t bk_color = TFT_BLUE;
  //uint32_t c_color = TFT_YELLOW;
  //uint32_t hl_bk_color = TFT_YELLOW;
  //uint32_t hl_c_color = TFT_BLUE;

  uint16_t len = str_len(s);

  if(highlight == 0)
  {
    tft.fillRoundRect(x-5, y+9-Fonts[displayFont].height, (Fonts[displayFont].width*len)+9, Fonts[displayFont].height+3, 5, bk_color);
    tft.drawRoundRect(x-5, y+9-Fonts[displayFont].height, (Fonts[displayFont].width*len)+9, Fonts[displayFont].height+3, 5, c_color);

    //drawChar(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size),
    //drawChar(x, y, 'A', TFT_YELLOW, TFT_BLACK, 2),
    tft.setTextColor(c_color, bk_color);
    tft.drawString(s, x, y-(Fonts[displayFont].height)+12);
  }
  else if(highlight == 1)
  {
    tft.fillRoundRect(x-5, y+9-Fonts[displayFont].height, (Fonts[displayFont].width*len)+9, Fonts[displayFont].height+3, 5, hl_bk_color);
    //tft.drawRoundRect(x-5, y+5-Fonts[displayFont].height, Fonts[displayFont].width+9, Fonts[displayFont].height+2, 5, hl_c_color);

    //drawChar(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size),
    //drawChar(x, y, 'A', TFT_YELLOW, TFT_BLACK, 2),
    tft.setTextColor(hl_c_color, hl_bk_color);
    tft.drawString(s, x, y-(Fonts[displayFont].height)+12);   
  }
/*  
  else if(highlight == 2)  //transparent background
  {
    tft.fillRoundRect(x-5, y+9-Fonts[displayFont].height, (Fonts[displayFont].width*len)+9, Fonts[displayFont].height+3, 5, hl_bk_color);
    //tft.drawRoundRect(x-5, y+5-Fonts[displayFont].height, Fonts[displayFont].width+9, Fonts[displayFont].height+2, 5, hl_c_color);

    //drawChar(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size),
    //drawChar(x, y, 'A', TFT_YELLOW, TFT_BLACK, 2),
    tft.setTextColor(c_color, TFT_TRANSPARENT);
    tft.drawString(s, x, y-(Fonts[displayFont].height)+12);   
  }
*/
  //tft.drawFastHLine (0, Y_MIN_DRAW, display_WIDTH, TFT_WHITE);
  //tft.drawPixel(x, y + Y_MIN_DRAW, TFT_RED); 
  //tft.fillRect((bargraph_X + ((bargraph_dX + bargraph_dX_space) * 0)), bargraph_Y, bargraph_dX, bargraph_dY, Smeter_table_color[0]);
}



//============================================================================
void displayDrawWordKey(const char *s, uint16_t col, uint16_t lin, uint16_t highlight)
{
  displayDrawWordKey_xy(s, (col*(Fonts[displayFont].width+11)), (lin*(Fonts[displayFont].height+4)), highlight);

  if(isTouch == true)   //define touch option (Back/OK/Search buttons)
  {
    if(numTouch < NUM_MAX_TOUCH_OPTIONS)
    {
      optTouch[numTouch].x   = (col*(Fonts[displayFont].width+11))-5;
      optTouch[numTouch].y   = (lin*(Fonts[displayFont].height+4))+9-Fonts[displayFont].height;
      optTouch[numTouch].w   = (Fonts[displayFont].width*str_len(s))+9;
      optTouch[numTouch].h   = Fonts[displayFont].height+3;
      optTouch[numTouch].col = col;
      optTouch[numTouch].lin = lin;
      numTouch++;
    }
    isTouch = false;  //touch defined
  }
}



void displayCursor(uint16_t col, uint16_t lin, uint16_t on_off)
{
  //drawFastHLine(int32_t x, int32_t y, int32_t w, uint32_t color),
  tft.drawFastHLine((col*Fonts[displayFont].width), ((lin+1)*Fonts[displayFont].height)-3, Fonts[displayFont].width, (on_off == 0 ? bk_color : c_color));
  tft.drawFastHLine((col*Fonts[displayFont].width), ((lin+1)*Fonts[displayFont].height)-2, Fonts[displayFont].width, (on_off == 0 ? bk_color : c_color));
}




//============================================================================
void displayDrawWord_xy(const char *s, uint16_t x, uint16_t y, uint16_t highlight)
{
  //uint32_t bk_color = TFT_BLUE;
  //uint32_t c_color = TFT_YELLOW;
  //uint32_t hl_bk_color = TFT_YELLOW;
  //uint32_t hl_c_color = TFT_BLUE;


  if(highlight == 0)
  {
    tft.setTextColor(c_color, bk_color);
    tft.drawString(s, x, y);
  }
  else if(highlight == 1)
  {
    tft.setTextColor(hl_c_color, hl_bk_color);
    tft.drawString(s, x, y);   
  }
/*  
  else if(highlight == 2)
  {
    tft.setTextColor(c_color, TFT_TRANSPARENT);
    tft.drawString(s, x, y);   
  }  
*/
}



//============================================================================
void displayDrawWord(const char *s, uint16_t col, uint16_t lin, uint16_t highlight)
{
  displayDrawWord_xy(s, (col*Fonts[displayFont].width), (lin*Fonts[displayFont].height), highlight);
}





//#define OPT_WIDTH  9
//============================================================================
void displayDrawOpt(const char *s, uint16_t col, uint16_t lin, uint16_t highlight)
{
  uint16_t x = col*(Fonts[displayFont].width); 
  uint16_t y = lin*(Fonts[displayFont].height); 

  if(highlight == 0)
  {
    //tft.fillRect(x-5, y+5-Fonts[displayFont].height, OPT_WIDTH*(Fonts[displayFont].width+9), Fonts[displayFont].height+2, bk_color);

    tft.setTextColor(c_color, bk_color);
    tft.drawString(s, x, y);
  }
  else
  {
    //tft.fillRect(x-5, y+5-Fonts[displayFont].height, OPT_WIDTH*(Fonts[displayFont].width+9), Fonts[displayFont].height+2, hl_bk_color);

    tft.setTextColor(hl_c_color, hl_bk_color);
    tft.drawString(s, x, y);
  }

  if(isTouch == true)   //define touch option on this scr 
  {
    if(numTouch < NUM_MAX_TOUCH_OPTIONS) 
    {
      optTouch[numTouch].x = x;
      optTouch[numTouch].y = y;
      optTouch[numTouch].w = strlen(s)*Fonts[displayFont].width;
      optTouch[numTouch].h = Fonts[displayFont].height;
      optTouch[numTouch].col = col;
      optTouch[numTouch].lin = lin;
      numTouch++;
    }
    isTouch = false;  //touch defined
  }

}


//============================================================================
//Desenha uma string na posicao do grid com cor de texto especifica
//(fundo bk_color), sem highlight.
void displayDrawOptColored(const char *s, uint16_t col, uint16_t lin, uint32_t txt_color)
{
  tft.setTextColor(txt_color, bk_color);
  tft.drawString(s, col*(Fonts[displayFont].width), lin*(Fonts[displayFont].height));
}


//============================================================================
//Desenha a tecla "Back" nas telas words/search com contorno arredondado,
//usando o mesmo grid (fonte/tamanho/posicao) das palavras da lista.
void displayDrawBackButton(uint16_t col, uint16_t lin, uint16_t highlight)
{
  const char *s = "Back";
  uint16_t len = str_len(s);
  uint16_t x = col*(Fonts[displayFont].width);
  uint16_t y = lin*(Fonts[displayFont].height)+3;
  uint16_t bx = x-5;
  uint16_t by = y;
  uint16_t bw = (Fonts[displayFont].width*len)+9;
  uint16_t bh = Fonts[displayFont].height+2;

  if(highlight == 0)
  {
    tft.fillRoundRect(bx, by, bw, bh, 5, bk_color);
    tft.setTextColor(c_color, bk_color);
    tft.drawString(s, x, y);
    tft.drawRoundRect(bx, by, bw, bh, 5, c_color);
  }
  else
  {
    tft.fillRoundRect(bx, by, bw, bh, 5, hl_bk_color);
    tft.setTextColor(hl_c_color, hl_bk_color);
    tft.drawString(s, x, y);
    tft.drawRoundRect(bx, by, bw, bh, 5, hl_c_color);
  }

  if(isTouch == true)   //define touch option (botao Back)
  {
    if(numTouch < NUM_MAX_TOUCH_OPTIONS)
    {
      optTouch[numTouch].x = bx;
      optTouch[numTouch].y = by;
      optTouch[numTouch].w = bw;
      optTouch[numTouch].h = bh;
      optTouch[numTouch].col = col;
      optTouch[numTouch].lin = lin;
      numTouch++;
    }
    isTouch = false;
  }
}






const char MainOpts[MAIN_OPTS][12] =  { "Camera",  "Words", "QR Code", "Address"};
//const int16_t MainOpts_touch_len[MAIN_OPTS] =  { 6,  5, 7, 7};
//const uint16_t MainOptsScr[MAIN_OPTS]={ SCR_CAM,  SCR_WORDS, SCR_QRCODE, SCR_ADDR };
void (*MainOptsFunc[MAIN_OPTS])(void)={ scr_cam_setup,  scr_words_setup, scr_qrcode_setup, scr_addr_setup };
                              
/*
#define MAIN_COLS  1
#define MAIN_LINES 4
#define MAIN_LINES_INI  2
#define MAIN_FONT  1

//#define FONTS_QTD  4
struct st_touch
{
  uint16_t x;
  uint16_t width;
  uint16_t y;
  uint16_t height;
};
const uint16_t MainTouchOpts[MAIN_LINES * MAIN_COLS] = 

[MAIN_OPTS]
text
lin
col
  uint16_t x = col*(Fonts[displayFont].width);  //+11);
  uint16_t y = lin*(Fonts[displayFont].height);  //+ 4);

keyboard
  x = 12+((col_ini+col)*(Fonts[displayFont].width+11)), 
  y = ((lin_ini+lin)*(Fonts[displayFont].height+4)), highlight);

*/

//============================================================================
void scr_main_setup()
{
/*
se tela = SCR_MAIN
   titulo e 4 linhas com opções de menu
   espera teclas
   se teclas up down, nuda opcao
   se enter, inicializa nova tela e vai para a tela
*/
  scr = SCR_MAIN;
  bk_color = TFT_BLUE;
  c_color = TFT_YELLOW;
  hl_bk_color = TFT_YELLOW;
  hl_c_color = TFT_BLUE;
  displayFont = 1;
  displayRotation = 2;
  tft.setRotation(displayRotation);           // Pode ser 0, 1, 2 ou 3, dependendo da orientação desejada
  tft.setFreeFont(Fonts[displayFont].font);      // Select the font
  tft.fillScreen(bk_color);            // Preenche a tela

  lin_selec = 0;
  col_selec = 0;
  lin_ini = 2;
  col_ini = 0;

  numTouch = 0;   //new scr starts with no touch areas
  displayDrawOpt("PicoSigner", 0, 0, 0);

  for(uint16_t lin=0; lin<MAIN_OPTS; lin++)
  {
    isTouch = true;   //define each touch option on this scr 
    displayDrawOpt(MainOpts[lin], col_ini, lin_ini+lin, 0);
  }

  displayDrawOpt(MainOpts[lin_selec], col_ini, lin_ini+lin_selec, 1);
  touch_area();   //desenha uma borda de cada area de toque desta tela
}
//============================================================================
void scr_main_loop()
{
  int16_t tx, ty;
  if(touch_edge(&tx, &ty))
  {
    touch_main(tx, ty);
    return;
  }

  uint16_t tec = trata_teclas();
  if(tec<NUM_SWITCHES)
  {
    switch(tec)  
    {
      case 0:   //"Left"
        break;
      case 1:   //"Up"
        if(lin_selec>0)
        {
          displayDrawOpt(MainOpts[lin_selec], col_ini, lin_ini+lin_selec, 0);
          lin_selec--;
          displayDrawOpt(MainOpts[lin_selec], col_ini, lin_ini+lin_selec, 1);
        }      
        break;
      case 2:   //"Down"
        if(lin_selec<MAIN_OPTS-1)
        {
          displayDrawOpt(MainOpts[lin_selec], col_ini, lin_ini+lin_selec, 0);
          lin_selec++;
          displayDrawOpt(MainOpts[lin_selec], col_ini, lin_ini+lin_selec, 1);
        }
        break;
      case 3:   //"Right"
        break;
      case 4:   //"Enter"
        MainOptsFunc[lin_selec]();
        break;
      default:
        break;
    }
  }

}



//============================================================================
void displayDrawEdit()
{
  tft.fillRoundRect(12, 13, (Fonts[displayFont].width*10), (Fonts[displayFont].height*2)-3, 8, bk_color);
  tft.drawRoundRect(12, 13, (Fonts[displayFont].width*10), (Fonts[displayFont].height*2)-3, 8, c_color);

  displayDrawWord(Words_selec, 1, 1, 1);

  uint16_t BtcW_pos = SearchBtcWords(Words_selec);
  if(BtcW_pos < BTCWORDS_NUM)  //found similar
  {
    uint16_t BtcW_len = str_len(BtcWords[BtcW_pos]);
    if(BtcW_len > words_selec_len)
      displayDrawWord(&BtcWords[BtcW_pos][words_selec_len], 1+words_selec_len, 1, 0);
  }

  displayCursor(1+words_selec_len, 1, 1); 
}

//============================================================================
void scr_keyboard_setup()
{
/*
se tela = SCR_KEYBOARD
   entrada de teclado e palavras
   se OK, vai p tela 2
*/
  scr = SCR_KEYBOARD;
  bk_color = TFT_BLUE;
  c_color = TFT_YELLOW;
  hl_bk_color = TFT_YELLOW;
  hl_c_color = TFT_BLUE;
  displayFont = 1;
  displayRotation = 2;
  tft.setRotation(displayRotation);           // Pode ser 0, 1, 2 ou 3, dependendo da orientação desejada
  tft.setFreeFont(Fonts[displayFont].font);      // Select the font
  tft.fillScreen(bk_color);            // Preenche a tela

  lin_selec = 2;
  col_selec = 4;
  lin_ini = 3;
  col_ini = 0;

  numTouch = 0;   //new scr starts with no touch areas

  for(uint16_t col=0; col<KEYB_COLS; col++)
    for(uint16_t lin=0; lin<KEYB_LINES; lin++)
    {
      isTouch = true;   //define each touch option on this scr 
      displayDrawKey(Letters[lin][col], col_ini+col, lin_ini+lin, 0);
    }
  displayDrawKey(Letters[lin_selec][col_selec], col_ini+col_selec, lin_ini+lin_selec, 1);

/*
falta uma linha para ir colocando a palavra digitada
esta parte deve receber uma palavra pre preenchida
  recebe o índice de Words[WORDS_NUM][16] para editar em word_selec
colocar um cursor no final do que ja esta preechido
  serve só para BtcWords ?
incluir um botao de back ou escape que deixa a palavra como era e volta
incluir um botao de OK que aceita o que ja foi editado
a cada letra, procura em BtcWords por uma palavra possivel
  as letras que ja estavam e a que for digitada fica em highlight 
  e o que achamos em BtcWords fica normal, montando o resto da palavra
  com o OK, aceita tudo
*/

  //tft.drawRoundRect(12, 13, (Fonts[displayFont].width*10), (Fonts[displayFont].height*2)-3, 8, c_color);

  //displayDrawWord("1234", 1, 1, 1);
  //displayDrawWord("5678", 5, 1, 0);
  //displayCursor(2, 1, 1);
  //displayCursor(5, 1, 1);
  //displayCursor(2, 1, 0);

  strcpy(Words_selec, Words[words_selec_num]);

  words_selec_len = str_len(Words_selec);
/*
  displayDrawWord(Words_selec, 1, 1, 1);

  uint16_t BtcW_pos = SearchBtcWords(Words_selec);
  if(BtcW_pos < BTCWORDS_NUM)  //found similar
  {
    uint16_t BtcW_len = str_len(BtcWords[BtcW_pos]);
    if(BtcW_len > words_selec_len)
      displayDrawWord(&BtcWords[BtcW_pos][words_selec_len], 1+words_selec_len, 1, 0);
  }

  displayCursor(1+words_selec_len, 1, 1);
*/
  displayDrawEdit();
  isTouch = true;   //define each touch option on this scr 
  displayDrawWordKey(KEY_BACK, 1, lin_ini+KEYB_LINES, 0);
  isTouch = true;   //define each touch option on this scr 
  displayDrawWordKey(KEY_OK, 5, lin_ini+KEYB_LINES, 0);
  isTouch = true;   //define each touch option on this scr 
  displayDrawWordKey(KEY_SEARCH, 2, lin_ini+KEYB_LINES+1, 0);

  touch_area();   //desenha uma borda de cada area de toque desta tela
}
//============================================================================
void scr_keyboard_loop()
{
  int16_t tx, ty;
  if(touch_edge(&tx, &ty))
  {
    touch_keyboard(tx, ty);
    return;
  }

  uint16_t tec = trata_teclas();
  if(tec<NUM_SWITCHES)
  {
    switch(tec)  
    {
      case 0:   //"Left"
        if((lin_selec<KEYB_LINES) && (col_selec>0))
        {
          displayDrawKey(Letters[lin_selec][col_selec], col_ini+col_selec, lin_ini+lin_selec, 0);
          col_selec--;
          displayDrawKey(Letters[lin_selec][col_selec], col_ini+col_selec, lin_ini+lin_selec, 1);
        }
        else if((lin_selec==KEYB_LINES) && (col_selec==1))
        {
          displayDrawWordKey(KEY_OK, 5, lin_ini+KEYB_LINES, 0);
          col_selec--;
          displayDrawWordKey(KEY_BACK, 1, lin_ini+KEYB_LINES, 1);
        }        
        break;
      case 1:   //"Up"
        if((lin_selec<KEYB_LINES) && (lin_selec>0))
        {
          displayDrawKey(Letters[lin_selec][col_selec], col_ini+col_selec, lin_ini+lin_selec, 0);
          lin_selec--;
          displayDrawKey(Letters[lin_selec][col_selec], col_ini+col_selec, lin_ini+lin_selec, 1);
        }   
        else if(lin_selec==KEYB_LINES+1)
        {
          displayDrawWordKey(KEY_SEARCH, 2, lin_ini+KEYB_LINES+1, 0);
          lin_selec--;
          if(col_selec==0)
            displayDrawWordKey(KEY_BACK, 1, lin_ini+KEYB_LINES, 1);
          else
            displayDrawWordKey(KEY_OK, 5, lin_ini+KEYB_LINES, 1);
        }   
        else if(lin_selec==KEYB_LINES)
        {
          lin_selec--;
          if(col_selec==0)
          {
            displayDrawWordKey(KEY_BACK, 1, lin_ini+KEYB_LINES, 0);
            col_selec = 1;
          }
          else
          {
            displayDrawWordKey(KEY_OK, 5, lin_ini+KEYB_LINES, 0);
            col_selec = 4;
          }
          displayDrawKey(Letters[lin_selec][col_selec], col_ini+col_selec, lin_ini+lin_selec, 1);
        }   
        break;
      case 2:   //"Down"
        if(lin_selec<KEYB_LINES-1)
        {
          displayDrawKey(Letters[lin_selec][col_selec], col_ini+col_selec, lin_ini+lin_selec, 0);
          lin_selec++;
          displayDrawKey(Letters[lin_selec][col_selec], col_ini+col_selec, lin_ini+lin_selec, 1);
        }
        else if(lin_selec==KEYB_LINES-1)
        {
          displayDrawKey(Letters[lin_selec][col_selec], col_ini+col_selec, lin_ini+lin_selec, 0);
          lin_selec++;
          if(col_selec < 4) col_selec = 0;  else col_selec = 1;
          if(col_selec==0)
            displayDrawWordKey(KEY_BACK, 1, lin_ini+KEYB_LINES, 1);
          else
            displayDrawWordKey(KEY_OK, 5, lin_ini+KEYB_LINES, 1);
        }
        else if(lin_selec==KEYB_LINES)
        {
          if(col_selec==0)
            displayDrawWordKey(KEY_BACK, 1, lin_ini+KEYB_LINES, 0);
          else
            displayDrawWordKey(KEY_OK, 5, lin_ini+KEYB_LINES, 0);
          lin_selec++;
          displayDrawWordKey(KEY_SEARCH, 2, lin_ini+KEYB_LINES+1, 1);
        }
        break;
      case 3:   //"Right"
        if((lin_selec<KEYB_LINES) && (col_selec<KEYB_COLS-1))
        {
          displayDrawKey(Letters[lin_selec][col_selec], col_ini+col_selec, lin_ini+lin_selec, 0);
          col_selec++;
          displayDrawKey(Letters[lin_selec][col_selec], col_ini+col_selec, lin_ini+lin_selec, 1);
        }      
        else if((lin_selec==KEYB_LINES) && (col_selec==0))
        {
          displayDrawWordKey(KEY_BACK, 1, lin_ini+KEYB_LINES, 0);
          col_selec++;
          displayDrawWordKey(KEY_OK, 5, lin_ini+KEYB_LINES, 1);
        }
        break;
      case 4:   //"Enter"
        if(lin_selec<KEYB_LINES)
        {
          if((lin_selec==KEYB_LINES-1) && (col_selec==KEYB_COLS-2))  //space
          {
            //do not use space
          }
          else if((lin_selec==KEYB_LINES-1) && (col_selec==KEYB_COLS-1))  // Backspace  <
          {
            if(words_selec_len > 0)
            {
              words_selec_len--;
              Words_selec[words_selec_len] = 0;
              displayDrawEdit();
            }
          }          
          else if(words_selec_len < BTCWORDS_MAX_LEN)
          {
            Words_selec[words_selec_len] = Letters[lin_selec][col_selec];
            words_selec_len++;
            displayDrawEdit();        
          }
        }
        else if(lin_selec==KEYB_LINES)
        {
          if(col_selec==0)    //Back
          {
            //scr_main_setup();
            scr_words_setup();  //come back without change
          }
          else  //OK
          {
            //uint16_t BtcW_len = str_len(BtcWords[BtcW_pos]);
            //if(BtcW_len > words_selec_len)              
            //scr_main_setup();
            bool copied = false;   //palavra aceita/copiada ou enviada p/ Search
            if(str_len(Words_selec)>0)  //some word
            {
              uint16_t BtcW_pos = SearchBtcWords(Words_selec);
              if(BtcW_pos < BTCWORDS_NUM)  //found similar
              {
                //12a palavra: so aceita se fechar o checksum. Se nao fechar,
                //vai para a tela Search, que so oferece palavras validas
                //(mesma regra do projeto BtcWords).
                if(words_selec_num == WORDS_NUM-1)
                {
                  const char *first11[WORDS_NUM-1];
                  for(uint16_t i=0; i<WORDS_NUM-1; i++) first11[i] = Words[i];
                  if(bip39_last_word_checksum_ok(first11, BtcWords[BtcW_pos]) == 1)
                  {
                    strcpy(Words[words_selec_num], BtcWords[BtcW_pos]); //copy the find
                    scr_words_setup();
                  }
                  else
                    scr_search_setup();   //escolher uma palavra com checksum valido
                  copied = true;
                }
                else
                {
                  strcpy(Words[words_selec_num], BtcWords[BtcW_pos]); //copy the find
                  scr_words_setup();
                  copied = true;
                }
              }
            }
            if(!copied)
              scr_words_setup();
          }
        } 
        else if(lin_selec==KEYB_LINES+1)
        {
          if(str_len(Words_selec)>0)  //some word
          {
            //Search - abre a tela de busca com as palavras relacionadas
            scr_search_setup();
          }
        }
        break;
      default:
        break;
    }
  }

/*
  uint16_t tec = trata_teclas();
  if(tec<NUM_SWITCHES)
    if(tec==4)
    {
      scr_main_setup();
    }
    else
    {
      keyboard_loop();
    }
*/

}



//============================================================================
// Acao/tecla escolhida por touch no teclado.
// col/lin sao a POSICAO de grade na tela (já incluem col_ini/lin_ini),
// como registrado em optTouch pelas funcoes de desenho. Aqui sao convertidos
// para o indice de conteudo (idx_col/idx_lin):
//   0..KEYB_LINES-1  -> letra do grid Letters[idx_lin][idx_col]
//   KEYB_LINES       -> Back (idx_col 1) / OK (idx_col 5)
//   KEYB_LINES+1     -> Search
void tft_keyboard_touch(uint16_t col, uint16_t lin)
{
  uint16_t idx_col = col - col_ini;   //posicao na tela -> indice de conteudo
  uint16_t idx_lin = lin - lin_ini;

  if(idx_lin < KEYB_LINES)
  {
    if((idx_lin == KEYB_LINES-1) && (idx_col == KEYB_COLS-2))     //space (_ / <)
    {
      //do not use space
    }
    else if((idx_lin == KEYB_LINES-1) && (idx_col == KEYB_COLS-1))  //Backspace <
    {
      if(words_selec_len > 0)
      {
        words_selec_len--;
        Words_selec[words_selec_len] = 0;
        displayDrawEdit();
      }
    }
    else if(words_selec_len < BTCWORDS_MAX_LEN)
    {
      Words_selec[words_selec_len] = Letters[idx_lin][idx_col];
      words_selec_len++;
      displayDrawEdit();
    }
  }
  else if(idx_lin == KEYB_LINES)
  {
    if(idx_col == 1)            //Back
    {
      scr_words_setup();    //come back without change
    }
    else                    //OK
    {
      bool copied = false;  //palavra aceita/copiada ou enviada p/ Search
      if(str_len(Words_selec) > 0)  //some word
      {
        uint16_t BtcW_pos = SearchBtcWords(Words_selec);
        if(BtcW_pos < BTCWORDS_NUM)  //found similar
        {
          //12a palavra: so aceita se fechar o checksum. Se nao fechar,
          //vai para a tela Search, que so oferece palavras validas
          if(words_selec_num == WORDS_NUM-1)
          {
            const char *first11[WORDS_NUM-1];
            for(uint16_t i=0; i<WORDS_NUM-1; i++) first11[i] = Words[i];
            if(bip39_last_word_checksum_ok(first11, BtcWords[BtcW_pos]) == 1)
            {
              strcpy(Words[words_selec_num], BtcWords[BtcW_pos]); //copy the find
              scr_words_setup();
            }
            else
              scr_search_setup();   //escolher uma palavra com checksum valido
            copied = true;
          }
          else
          {
            strcpy(Words[words_selec_num], BtcWords[BtcW_pos]); //copy the find
            scr_words_setup();
            copied = true;
          }
        }
      }
      if(!copied)
        scr_words_setup();
    }
  }
  else if(idx_lin == KEYB_LINES+1)   //Search
  {
    if(str_len(Words_selec) > 0)
      scr_search_setup();
  }
}


//============================================================================
//1 se as 12 palavras digitadas fecham o checksum BIP-39, 0 caso contrario.
int words_chksum_ok(void)
{
  const char *words12[WORDS_NUM];
  for(uint16_t i=0; i<WORDS_NUM; i++)
    words12[i] = Words[i];
  return bip39_words_checksum_ok(words12);
}

//============================================================================
void drawWordLin(uint16_t hl)
{
  bool reg = isTouch;   //quem chamou pede para registrar a linha como area de toque
  isTouch = false;      //as chamadas de displayDrawOpt abaixo nao registram areas parciais

  char s[5] = " 0-";
  if(lin_selec<9) { s[0]=' ';  s[1]='1'+lin_selec;  } 
  else            { s[0]='1';  s[1]='0'+lin_selec-9; }
  displayDrawOpt(s, 0, lin_ini+lin_selec, hl);

  //12a palavra em vermelho quando as 12 palavras nao fecham o checksum
  if((lin_selec == WORDS_NUM-1) && (hl == 0) && (words_chksum_ok() != 1))
    displayDrawOptColored(Words[lin_selec], col_ini, lin_ini+lin_selec, TFT_RED);
  else
    displayDrawOpt(Words[lin_selec], col_ini, lin_ini+lin_selec, hl);

  if(reg)   //area de toque = linha inteira (indice + palavra)
  {
    if(numTouch < NUM_MAX_TOUCH_OPTIONS)
    {
      optTouch[numTouch].x = 0;
      optTouch[numTouch].y = (lin_ini+lin_selec)*Fonts[displayFont].height;
      optTouch[numTouch].w = (col_ini + str_len(Words[lin_selec]))*Fonts[displayFont].width;
      optTouch[numTouch].h = Fonts[displayFont].height;
      optTouch[numTouch].col = 0;
      optTouch[numTouch].lin = lin_ini+lin_selec;
      numTouch++;
    }
  }
  isTouch = false;
}

//============================================================================
void scr_words_setup()
{
/*
se tela = SCR_WORDS
   mostra palavras
   se Left, volta para tela 0
*/
  scr = SCR_WORDS;
  displayFont = 0;
  tft.setFreeFont(Fonts[displayFont].font);      // Select the font
  tft.fillScreen(bk_color);            // Preenche a tela

  col_selec = 0;
  lin_ini = 1;
  col_ini = 3;

  numTouch = 0;   //new scr starts with no touch areas
  displayDrawOpt("PassWords", 3, 0, 0);

  for(lin_selec=0; lin_selec<WORDS_NUM; lin_selec++)
  {
    isTouch = true;   //define each touch option on this scr 
    drawWordLin(0);
  }
  lin_selec = 0;
  drawWordLin(1);
  isTouch = true;   //define each touch option on this scr 
  displayDrawBackButton(2, lin_ini+WORDS_NUM, 0);
  touch_area();   //desenha uma borda de cada area de toque desta tela
}
//============================================================================
void scr_words_loop()
{
  int16_t tx, ty;
  if(touch_edge(&tx, &ty))
  {
    touch_words(tx, ty);
    return;
  }

  uint16_t tec = trata_teclas();
  if(tec<NUM_SWITCHES)
  {
    switch(tec)  
    {
      case 0:   //"Left"
          scr_main_setup();
        break;
      case 1:   //"Up"
        if(lin_selec==WORDS_NUM)   //no botao Back
        {
          displayDrawBackButton(2, lin_ini+WORDS_NUM, 0);
          lin_selec--;
          drawWordLin(1);
        }
        else if(lin_selec==0)   //primeira linha -> botao Back
        {
          drawWordLin(0);
          lin_selec = WORDS_NUM;
          displayDrawBackButton(2, lin_ini+WORDS_NUM, 1);
        }
        else if(lin_selec>0)
        {
          drawWordLin(0);
          lin_selec--;
          drawWordLin(1);
        }      
        break;
      case 2:   //"Down"
        if(lin_selec<WORDS_NUM-1)
        {
          drawWordLin(0);
          lin_selec++;
          drawWordLin(1);
        }
        else if(lin_selec==WORDS_NUM-1)   //ultima palavra -> botao Back
        {
          drawWordLin(0);
          lin_selec++;
          displayDrawBackButton(2, lin_ini+WORDS_NUM, 1);
        }
        else if(lin_selec==WORDS_NUM)   //botao Back -> primeira palavra
        {
          displayDrawBackButton(2, lin_ini+WORDS_NUM, 0);
          lin_selec = 0;
          drawWordLin(1);
        }
        break;
      case 3:   //"Right"
        break;
      case 4:   //"Enter"
        if(lin_selec==WORDS_NUM)   //botao Back (touch/enter)
          scr_main_setup();
        else
        {
          words_selec_num = lin_selec;
          scr_keyboard_setup();
        }
        break;
      default:
        break;
    }
  }


}


//============================================================================
void drawSearchLin(uint16_t hl)
{
  bool reg = isTouch;   //quem chamou pede para registrar a linha como area de toque
  isTouch = false;      //as chamadas de displayDrawOpt abaixo nao registram areas parciais

  char s[5] = " 0-";
  if(lin_selec<9) { s[0]=' ';  s[1]='1'+lin_selec;  }
  else            { s[0]='1';  s[1]='0'+lin_selec-9; }
  displayDrawOpt(s, 0, lin_ini+lin_selec, hl);
  displayDrawOpt(SearchResults[lin_selec], col_ini, lin_ini+lin_selec, hl);

  if(reg)   //area de toque = linha inteira (indice + palavra)
  {
    if(numTouch < NUM_MAX_TOUCH_OPTIONS)
    {
      optTouch[numTouch].x = 0;
      optTouch[numTouch].y = (lin_ini+lin_selec)*Fonts[displayFont].height;
      optTouch[numTouch].w = (col_ini + str_len(SearchResults[lin_selec]))*Fonts[displayFont].width;
      optTouch[numTouch].h = Fonts[displayFont].height;
      optTouch[numTouch].col = 0;
      optTouch[numTouch].lin = lin_ini+lin_selec;
      numTouch++;
    }
  }
  isTouch = false;
}

//============================================================================
//Varre todas as 2048 palavras BIP-39 e guarda as 10 mais relacionadas ao
//texto digitado no campo do teclado (Words_selec). Mesmo heuristica usada
//no projeto de referencia BtcWords (prefixo + completacao curta + letras).
void findSearchResults(const char *input)
{
  uint16_t in_len = str_len(input);
  search_results_num = 0;
  if(in_len == 0) return;   //campo vazio nao gera resultados

  int32_t topScore[SEARCH_RESULTS_MAX];
  uint16_t topIdx[SEARCH_RESULTS_MAX];

  //Se esta editando a 12a palavra, so oferece palavras que fechem o checksum
  //BIP-39 (regra do projeto BtcWords, funcao bip39_valid_last_words).
  const bool lastWordFilter = (words_selec_num == (WORDS_NUM-1));
  const char *first11[WORDS_NUM-1];
  if(lastWordFilter)
    for(uint16_t i=0; i<WORDS_NUM-1; i++)
      first11[i] = Words[i];

  for(uint16_t i=0; i<BTCWORDS_NUM; i++)
  {
    const char *kw = BtcWords[i];

    if(lastWordFilter && (bip39_last_word_checksum_ok(first11, kw) != 1))
      continue;   //esta palavra nao fecharia o checksum -> nao oferece

    int32_t score = 0;

    //(A) prefixo: bonus forte
    if(strncmp(kw, input, in_len) == 0)
    {
      score += 10000;
      int32_t diff = (int32_t)str_len(kw) - (int32_t)in_len;
      if(diff <= 3)
        score += 1000 - diff*100;   //bonus por ser uma completacao curta
    }

    //(B) coincidencia por caractere: bonus fraco
    for(uint16_t c=0; c<in_len; c++)
    {
      if(strchr(kw, input[c]) != NULL)
        score += 2;
    }

    if(score <= 0)
      continue;

    //insere ordenado (score desc, depois alfabetico) mantendo ate 10
    if(search_results_num < SEARCH_RESULTS_MAX)
    {
      int32_t p = search_results_num;
      while(p > 0 &&
        (score > topScore[p-1] ||
         (score == topScore[p-1] && strcmp(kw, BtcWords[topIdx[p-1]]) < 0)))
      {
        topScore[p] = topScore[p-1];
        topIdx[p] = topIdx[p-1];
        p--;
      }
      topScore[p] = score;
      topIdx[p] = i;
      search_results_num++;
    }
    else
    {
      //lista cheia: so entra se superar o ultimo (pior) item
      int32_t last = topScore[SEARCH_RESULTS_MAX-1];
      if(score > last ||
         (score == last && strcmp(kw, BtcWords[topIdx[SEARCH_RESULTS_MAX-1]]) < 0))
      {
        int32_t p = SEARCH_RESULTS_MAX-1;
        while(p > 0 &&
          (score > topScore[p-1] ||
           (score == topScore[p-1] && strcmp(kw, BtcWords[topIdx[p-1]]) < 0)))
        {
          topScore[p] = topScore[p-1];
          topIdx[p] = topIdx[p-1];
          p--;
        }
        topScore[p] = score;
        topIdx[p] = i;
      }
    }
  }

  for(uint16_t i=0; i<search_results_num; i++)
    strcpy(SearchResults[i], BtcWords[topIdx[i]]);
}

//============================================================================
void scr_search_setup()
{
/*
se tela = SCR_SEARCH
   mostra palavras que combinam com o texto digitado
   se Left, volta para tela de words (sem alterar nada)
   se Enter, seleciona a palavra e volta para o teclado preenchendo o campo
*/
/*
  if(str_len(Words_selec) == 0)
  {
    scr_keyboard_setup();   //campo vazio nao gera resultados
    return;
  }
*/
  bk_color = TFT_BLUE;
  c_color = TFT_GREENYELLOW;
  hl_bk_color = TFT_GREENYELLOW;
  hl_c_color = TFT_BLUE;

  scr = SCR_SEARCH;
  displayFont = 0;
  tft.setFreeFont(Fonts[displayFont].font);      // Select the font
  tft.fillScreen(bk_color);            // Preenche a tela

  col_selec = 0;
  lin_ini = 2;
  col_ini = 3;

  numTouch = 0;   //new scr starts with no touch areas
  displayDrawOpt("PicoSigner", 3, 0, 0);
  displayDrawOpt("Search", 3, 1, 0);
  displayDrawOpt("Working...", col_ini, lin_ini, 0);   //aviso enquanto busca
  isTouch = true;   //define each touch option on this scr 
  displayDrawBackButton(2, lin_ini+SEARCH_RESULTS_MAX, 0);

  findSearchResults(Words_selec);   //bloqueia, mas o aviso ja esta na tela

  //apaga a area da lista (e o aviso) e desenha os resultados
  tft.fillRect(0, lin_ini*Fonts[displayFont].height, TFT_WIDTH, SEARCH_RESULTS_MAX*Fonts[displayFont].height, bk_color);

  for(lin_selec=0; lin_selec<search_results_num; lin_selec++)
  {
    isTouch = true;   //define cada linha como area de toque
    drawSearchLin(0);
  }
  if(search_results_num > 0)
  {
    lin_selec = 0;
    drawSearchLin(1);
  }
  else
  {
    lin_selec = 0;
  }

  touch_area();   //desenha uma borda de cada area de toque desta tela
}
//============================================================================
void scr_search_loop()
{
  int16_t tx, ty;
  if(touch_edge(&tx, &ty))
  {
    touch_search(tx, ty);
    return;
  }

  uint16_t tec = trata_teclas();
  if(tec<NUM_SWITCHES)
  {
    switch(tec)
    {
      case 0:   //"Left"
        scr_keyboard_setup();   //volta sem alterar nada
        break;
      case 1:   //"Up"
        if(lin_selec==SEARCH_RESULTS_MAX)   //no botao Back
        {
          displayDrawBackButton(2, lin_ini+SEARCH_RESULTS_MAX, 0);
          if(search_results_num > 0)
          {
            lin_selec = search_results_num-1;
            drawSearchLin(1);
          }
        }
        else if(search_results_num > 0 && lin_selec==0)   //primeira linha -> botao Back
        {
          drawSearchLin(0);
          lin_selec = SEARCH_RESULTS_MAX;
          displayDrawBackButton(2, lin_ini+SEARCH_RESULTS_MAX, 1);
        }
        else if(search_results_num > 0 && lin_selec>0)
        {
          drawSearchLin(0);
          lin_selec--;
          drawSearchLin(1);
        }
        break;
      case 2:   //"Down"
        if(search_results_num > 0 && lin_selec<search_results_num-1)
        {
          drawSearchLin(0);
          lin_selec++;
          drawSearchLin(1);
        }
        else if(search_results_num > 0 && lin_selec==search_results_num-1)
        {
          //ultima palavra -> botao Back
          drawSearchLin(0);
          lin_selec = SEARCH_RESULTS_MAX;
          displayDrawBackButton(2, lin_ini+SEARCH_RESULTS_MAX, 1);
        }
        else if(search_results_num > 0 && lin_selec==SEARCH_RESULTS_MAX)
        {
          //botao Back -> primeira palavra
          displayDrawBackButton(2, lin_ini+SEARCH_RESULTS_MAX, 0);
          lin_selec = 0;
          drawSearchLin(1);
        }
        break;
      case 3:   //"Right"
        break;
      case 4:   //"Enter"
        if(lin_selec==SEARCH_RESULTS_MAX)   //botao Back (touch/enter)
          scr_keyboard_setup();   //volta sem alterar nada
        else if(search_results_num > 0)
        {
          //grava a palavra selecionada em Words[words_selec_num]; o setup do
          //teclado entao copia para Words_selec, preenchendo o campo
          strcpy(Words[words_selec_num], SearchResults[lin_selec]);
          scr_keyboard_setup();   //preenche o campo e volta
        }
        break;
      default:
        break;
    }
  }
}


//============================================================================
void scr_cam_setup()
{
/*
se tela = SCR_CAM
   fica mostrando a camera
   se tecla, vai para tela 4
*/
  displayRotation = 1;
  tft.setRotation(displayRotation);           // Pode ser 0, 1, 2 ou 3, dependendo da orientação desejada

  scr = SCR_CAM;
  displayFont = 0;
  tft.setFreeFont(Fonts[displayFont].font);      // Select the font
  tft.setTextColor(TFT_YELLOW);        // one-arg => transparent background

  numTouch = 0;   //new scr starts with no touch areas

  //tft.fillScreen(TFT_GREEN);     // Preenche a tela
  //delay(3000);

  //Cam_DrawImage(); //display test
  //delay(3000);
}
//============================================================================
void scr_cam_loop()
{
  int16_t tx, ty;
  if(touch_edge(&tx, &ty))
  {
    //debug temporario
    //Serial.print("CAM raw touch: x=");
    //Serial.print(tx); Serial.print(" y=");
    //Serial.println(ty);
    touch_cam(tx, ty);
    return;
  }

  uint16_t tec = trata_teclas();
  if((tec<NUM_SWITCHES)&&(tec!=4))  //left right up or down
  {
    scr_main_setup();
  }
  else
  {
    Cam_OV7670_loop(tec);
  }
}

//============================================================================
void scr_qrcode_setup()
{
/*
se tela = SCR_QRCODE
   mostra resultado do QR Code
   e BACK ou OK
   se OK, volta para tela 0
*/
  scr = SCR_QRCODE;
  bk_color = TFT_BLACK;
  c_color = TFT_WHITE;
  hl_bk_color = TFT_WHITE;
  hl_c_color = TFT_BLACK;
  displayFont = 1;
  displayRotation = 2;
  tft.setRotation(displayRotation);           // Pode ser 0, 1, 2 ou 3, dependendo da orientação desejada
  //tft.setFreeFont(Fonts[displayFont].font);      // Select the font
  tft.fillScreen(c_color);            // Preenche a tela

  numTouch = 0;   //new scr starts with no touch areas

  QRCode_gen_test();
}
//============================================================================
void scr_qrcode_loop()
{
  int16_t tx, ty;
  if(touch_edge(&tx, &ty))
  {
    touch_qrcode(tx, ty);
    return;
  }

  uint16_t tec = trata_teclas();
  if(tec<NUM_SWITCHES)
  {
    scr_main_setup();
  }
}




//============================================================================
// Desenha o endereço (até ADDR_LINE_MAX chars por linha), com quebra manual,
// usando a fonte atual e espaçamento de linha em pixels (step). 
// As linhas são desenhadas em y = y0 + n*step. Retorna o numero de linhas usadas.
#define ADDR_LINE_MAX  18
#define ADDR_ROW_STEP  20   // pixels entre linhas (glyph 16pt + folga)
static uint16_t drawAddrLines(const char* s, uint16_t x, uint16_t y0, uint16_t step)
{
  uint16_t slen = 0;
  while(s[slen]) slen++;
  uint16_t count = 0;
  uint16_t start = 0;
  do
  {
    uint16_t n = slen - start;
    if(n > ADDR_LINE_MAX) n = ADDR_LINE_MAX;
    char line[ADDR_LINE_MAX+1];
    for(uint16_t i=0; i<n; i++) line[i] = s[start+i];
    line[n] = 0;
    tft.drawString(line, x, y0 + count*step);
    count++;
    start += n;
  } while(start < slen);
  return count;
}

// Desenha uma linha de texto com a fonte/cores atuais em x,y.
static void drawTextAt(const char* s, uint16_t x, uint16_t y, uint16_t highlight)
{
  if(highlight)
    tft.setTextColor(hl_c_color, hl_bk_color);
  else
    tft.setTextColor(c_color, bk_color);
  tft.drawString(s, x, y);
}

//============================================================================
void scr_addr_setup()
{
/*
se tela = SCR_ADDR
   deriva o endereco BIP-84 a partir das 12 palavras (Words)
   mostra o endereco (ou mensagem de erro)
   qualquer tecla volta ao menu
*/
  scr = SCR_ADDR;
  bk_color = TFT_BLUE;      //fundo do menu principal
  c_color = TFT_YELLOW;     //letra do menu principal
  hl_bk_color = TFT_YELLOW;
  hl_c_color = TFT_BLUE;
  displayFont = 3;   //FONT0 (FreeMonoBold9pt7b, 11x16 -> ~21 chars/linha)
  displayRotation = 2;
  tft.setRotation(displayRotation);           // Pode ser 0, 1, 2 ou 3, dependendo da orientação desejada
  tft.setFreeFont(Fonts[displayFont].font);
  tft.setTextSize(1);
  tft.fillScreen(bk_color);

  uint16_t x0 = 0;
  uint16_t y  = 0;
  const uint16_t ROW = ADDR_ROW_STEP;   //20 px por linha

  numTouch = 0;   //new scr starts with no touch areas

  drawTextAt("Address", x0, y, 0); y += ROW;
  drawTextAt("m/84'/0'/0'/0/0", x0, y, 0); y += ROW;
/*
O texto m/84'/0'/0'/0/0 é um caminho de derivação (derivation path) 
que mostra o endereço exato de uma carteira de Bitcoin gerado a partir 
da sua frase semente (seed phrase).
Cada parte do texto separada por uma barra (/) tem uma função específica:
m: Representa a chave mestre (master private key), que é a raiz de onde 
todas as outras chaves da carteira são criadas.
84': Indica o propósito do padrão (BIP84). O número 84 significa que este 
endereço usa o formato Native SegWit (endereços modernos de Bitcoin que 
omeçam com bc1). O apóstrofo (') significa que essa etapa usa uma 
"derivação endurecida" para maior segurança.
0': Indica o tipo de moeda. O número 0 é o código padrão para o Bitcoin (BTC).
0': Indica o número da conta. O primeiro valor de conta é sempre o zero (0).
0: Indica a cadeia (chain). O número 0 significa que é um endereço público 
para receber fundos. Se fosse 1, seria um endereço interno usado para trocos.
0: Indica o índice do endereço. O número 0 representa o primeiro 
endereço gerado nessa sequência específica. O próximo seria 1, 
depois 2, e assim por diante.
*/
  //mostra as 12 palavras usadas (2 por linha, com indice 1..12)
  char wline[40];
  for(uint16_t pair = 0; pair < 6; pair++)
  {
    uint16_t k = 0;
    for(uint16_t j = 0; j < 2; j++)
    {
      uint16_t i = pair*2 + j;
      uint16_t wlen = 0;
      const char* w = Words[i];
      while(w[wlen]) wlen++;
      if(i >= 9)      //indice 10..12 ocupa 2 digitos
      { wline[k++] = '1'; wline[k++] = (char)('0' + (i+1)%10); }
      else
        wline[k++] = (char)('1' + i);
      wline[k++] = ':';
      for(uint16_t c=0; c<wlen; c++) wline[k++] = w[c];
      wline[k++] = ' ';
    }
    wline[k] = 0;
    drawTextAt(wline, x0, y, 0); y += ROW;
  }
  y += ROW;   // linhas de respiro

  //monta as 12 palavras para o bip84
  const char* wordptr[WORDS_NUM];
  for(uint16_t i=0; i<WORDS_NUM; i++)
    wordptr[i] = Words[i];


  drawTextAt("Please wait...", x0, y, 0);

  //deriva o endereco (pode demorar alguns segundos)
  int rc = bip84_address_from_words(wordptr, BtcAddress, 100);

  if(rc == 0)
  {
    y = drawAddrLines(BtcAddress, x0, y, ROW) * ROW + y;

    drawTextAt("Public address sent", x0, 320 - (3*ROW), 0);
    drawTextAt("through serial.", x0, 320 - (2*ROW), 0);

    //envia endereco pelo serial para consulta no PC/celular
    Serial.println();
    Serial.println("======= PicoSigner BIP-84 =======");
    /*
    Serial.print("Mnemonic: ");
    for(uint16_t i=0; i<WORDS_NUM; i++)
    {
      if(i) Serial.print(" ");
      Serial.print(Words[i]);
    }
    Serial.println();
    */
    Serial.print("Address : ");
    Serial.println(BtcAddress);
    Serial.print("Explore : https://mempool.space/address/");
    Serial.println(BtcAddress);
    Serial.println("=================================");
  }
  else
  {
    drawTextAt("Checksum/words invalid", x0, y, 0); y += ROW;
    drawTextAt("adjust in Words screen", x0, y, 0); y += ROW;
  }

  //rodapé no fim da tela (320px de altura)
  drawTextAt("[Enter] menu", x0, 320 - ROW, 1);
}
//============================================================================
void scr_addr_loop()
{
  int16_t tx, ty;
  if(touch_edge(&tx, &ty))
  {
    touch_addr(tx, ty);
    return;
  }

  uint16_t tec = trata_teclas();
  if(tec<NUM_SWITCHES)
  {
    scr_main_setup();
  }
}






//============================================================================
void display_tft_setup(void) 
{

  tft.init();                    // Inicializa o display
  //tft.setRotation(ROTATION_SETUP);            // Pode ser 0, 1, 2 ou 3, dependendo da orientação desejada
  //tft.fillScreen(TFT_BLUE);     // Preenche a tela

  scr_main_setup();
  //scr_cam_setup();

  //tft.invertDisplay(false);
  //tft.fillScreen(TFT_GREEN);     // Preenche a tela
  //tft.fillScreen(TFT_BLACK);     // Preenche a tela 
  //tft.setBacklight(true); // Ligar o backlight

  Serial.println("Display tft setup");
/*
  // Preenche o bitmap com uma cor de teste, por exemplo, um gradiente
  for (int y = 0; y < SEGMENT_HEIGHT; y++) 
  {
    for (int x = 0; x < SEGMENT_WIDTH; x++) 
    {
      single_bitmap[y * SEGMENT_WIDTH + x] = tft.color565(y, x, 0); // Gradiente vermelho e amarelo
      //single_bitmap[x * SEGMENT_HEIGHT + y] = tft.color565(x, y, 0); // Gradiente vermelho e amarelo
    }
  }
*/
/*
  displayDrawKey('A', 50, 50, 0);
  displayDrawKey('B', 50+(1*(Fonts[displayFont].width+11)), 50, 0);
  displayDrawKey('C', 50+(2*(Fonts[displayFont].width+11)), 50, 1);
  displayDrawKey('D', 50+(3*(Fonts[displayFont].width+11)), 50, 1);

  displayDrawKey('K', 50, 50+(1*(Fonts[displayFont].height+4)), 0);
  displayDrawKey('L', 50+(1*(Fonts[displayFont].width+11)), 50+(1*(Fonts[displayFont].height+4)), 0);
  displayDrawKey('M', 50+(2*(Fonts[displayFont].width+11)), 50+(1*(Fonts[displayFont].height+4)), 1);
  displayDrawKey('N', 50+(3*(Fonts[displayFont].width+11)), 50+(1*(Fonts[displayFont].height+4)), 1);
  displayDrawKey('O', 50+(4*(Fonts[displayFont].width+11)), 50+(1*(Fonts[displayFont].height+4)), 1);
   
  displayDrawKey('B', 50, 150, 1);
*/


  //test_sha256();
  //QRCode_gen_test();
}



//#define BlinkDisplayTime    1000
//============================================================================
void display_tft_loop(void) 
{
/*
  static uint16_t cont = 0;
  static unsigned long lastDisplayTime = millis();
  if (millis() - lastDisplayTime > BlinkDisplayTime)
  {
    lastDisplayTime += BlinkDisplayTime;
    //LED blink
    //digitalWrite(LED_BUILTIN, (digitalRead(LED_BUILTIN) == HIGH ? LOW : HIGH));
    //Serial.print(">");



    tft.fillScreen(TFT_BLUE);     // Preenche a tela com preto
    drawSegment(single_bitmap, cont&3);


    Serial.print("Display  ");
    Serial.println(cont&3);
    cont++;
  
  }
*/

  switch(scr)
  {
    case SCR_MAIN:
      scr_main_loop();
      break;
    case SCR_KEYBOARD:
      scr_keyboard_loop();
      break;
    case SCR_WORDS:
      scr_words_loop();
      break;
    case SCR_CAM:
      scr_cam_loop();
      break;
    case SCR_QRCODE:
      scr_qrcode_loop();
      break;
    case SCR_ADDR:
      scr_addr_loop();
      break;
    case SCR_SEARCH:
      scr_search_loop();
      break;
    default:
      scr = SCR_MAIN;
      scr_main_loop();
      break;
  }

}






