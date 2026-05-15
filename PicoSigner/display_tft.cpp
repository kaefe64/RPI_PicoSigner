/*
 * display_tft.cpp
 * 
 * Created: Aug 2025
 * Author: Klaus Fensterseifer

Display TFT ST7789 240x320
  
*/

#include "Arduino.h"
//#include "tft_setup_RP2040_ST7789_240x320.h"
#include "TFT_eSPI.h"   //Library  TFT_eSPI by Bodmer
#include "display_tft.h"
#include "key_input.h"
#include "Cam_OV7670.h"
#include "BitcoinWords.h"
#include "qr_code.h"


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
                                        {FONT3, X_CHAR3, Y_CHAR3} };



#define SCR_MAIN        0
#define SCR_KEYBOARD    10
#define SCR_WORDS       20
#define SCR_CAM         30
#define SCR_QRCODE      40

uint16_t scr = SCR_MAIN;

uint16_t displayFont;
uint32_t bk_color;
uint32_t c_color;
uint32_t hl_bk_color;
uint32_t hl_c_color;

uint16_t words_selec=0;
uint16_t words_len;


//char Words[WORDS_NUM][16] = {"ab","","","","","","","","","","",""};
char Words[WORDS_NUM][16] = {"spy", "profit", "item", "promote", "equal", "wealth", "nice", "prize", "cute", "lawsuit", "stage", "capital"};
uint16_t Word_pos[WORDS_NUM] = {1690,	1374,	950,	1377,	608,	1985,	1195,	1370,	437,	1009,	1697,	272 };   //11 bits / word

uint16_t lin_selec;
uint16_t col_selec;
uint16_t lin_ini;
uint16_t col_ini;



void displayDrawCharKey(char c, uint16_t x, uint16_t y, uint16_t highlight);

void scr_main_setup();
void scr_main_loop();
void scr_keyboard_setup();
void scr_keyboard_loop();
void scr_words_setup();
void scr_words_loop();
void scr_cam_setup();
void scr_cam_loop();
void scr_qrcode_setup();
void scr_qrcode_loop();







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

#if 0

// Rotation: 0, 1, 2, 3 (same as TFT_eSPI)
void setPixelRot(uint16_t *img, int16_t w, int16_t h,
                 int16_t x, int16_t y, uint16_t color, uint8_t rot)
{
    int16_t tx, ty;
    switch(rot & 3) {
      case 0: // 0 degrees
        tx = x; ty = y;
        break;
      case 1: // 90 degrees
        tx = h - y - 1; ty = x;
        break;
      case 2: // 180 degrees
        tx = w - x - 1; ty = h - y - 1;
        break;
      case 3: // 270 degrees
        tx = y; ty = w - x - 1;
        break;
    }
    if (tx >= 0 && tx < w && ty >= 0 && ty < h)
        img[ty * w + tx] = color;
}

void drawStringToImageRot(uint16_t *img, int16_t imgW, int16_t imgH,
                          int16_t x, int16_t y, const char *str,
                          uint16_t color, const GFXfont *font,
                          uint8_t rot)
{
    int16_t cursor_x = x;
    int16_t cursor_y = y;

    while (*str) {
        char c = *str++;
        if (c < font->first || c > font->last) continue;

        GFXglyph *glyph = &font->glyph[c - font->first];
        uint8_t  *bitmap = font->bitmap;

        uint16_t bo = glyph->bitmapOffset;
        uint8_t  w  = glyph->width;
        uint8_t  h  = glyph->height;
        int8_t   xo = glyph->xOffset;
        int8_t   yo = glyph->yOffset;

        uint8_t bits = 0, bit = 0;
        for (uint8_t yy = 0; yy < h; yy++) {
            for (uint8_t xx = 0; xx < w; xx++) {
                if (!(bit++ & 7)) bits = bitmap[bo++];
                if (bits & 0x80) {
                    int16_t px = cursor_x + xo + xx;
                    int16_t py = cursor_y + yo + yy;
                    setPixelRot(img, imgW, imgH, px, py, color, rot);
                }
                bits <<= 1;
            }
        }
        // Advance cursor according to rotation
        switch(rot & 3) {
          case 0: cursor_x += glyph->xAdvance; break;
          case 1: cursor_y += glyph->xAdvance; break;
          case 2: cursor_x -= glyph->xAdvance; break;
          case 3: cursor_y -= glyph->xAdvance; break;
        }
    }
}



//drawStringToImageRot(myImage, 240, 240, 20, 50, "HELLO", TFT_YELLOW, &FreeMonoBold12pt7b, 1);




#endif





#define KEYB_LINES  4
#define KEYB_COLS   7


// Used for displaying Leter board
char Letters[KEYB_LINES][KEYB_COLS+1]={"abcdefg",
                                       "hijklmn",
                                       "opqrstu",
                                       "vwxyz_<"};








//============================================================================
void displayDrawCharKey(char c, uint16_t x, uint16_t y, uint16_t highlight)
{
/*
  char s[20];

  tft.setFreeFont(FONT1);                 // Select the font
  txt_size = 1;
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  tft.setTextSize(txt_size);  //size 1 = 10 pixels, size 2 =20 pixels, and so on

  tft.drawString(vet_char, x * X_CHAR1 * txt_size, y * Y_CHAR1 * txt_size, 1);// Print the string name of the font
  sprintf(s, "Arjan-5");  //name changed from uSDR Pico FFT
  tft_writexy_plus(3, TFT_YELLOW, TFT_BLACK, 2,10,1,0,(uint8_t *)s);
*/
  //uint16_t font = 10;
  uint32_t bk_color = TFT_BLUE;
  uint32_t c_color = TFT_YELLOW;
  uint32_t hl_bk_color = TFT_YELLOW;
  uint32_t hl_c_color = TFT_BLUE;

  //tft.setRotation(0);            // Pode ser 0, 1, 2 ou 3, dependendo da orientação desejada

  //tft.setFreeFont(FONT2);                 // Select the font
  //tft.setTextSize(SIZE2);     



  if(highlight == 0)
  {
    tft.fillRoundRect(x-5, y+5-Fonts[displayFont].height, Fonts[displayFont].width+9, Fonts[displayFont].height+2, 5, bk_color);
    tft.drawRoundRect(x-5, y+5-Fonts[displayFont].height, Fonts[displayFont].width+9, Fonts[displayFont].height+2, 5, c_color);

    //drawChar(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size),
    //drawChar(x, y, 'A', TFT_YELLOW, TFT_BLACK, 2),
    tft.setTextColor(c_color, bk_color);
    tft.drawChar(c, x, y);
  }
  else
  {
    tft.fillRoundRect(x-5, y+5-Fonts[displayFont].height, Fonts[displayFont].width+9, Fonts[displayFont].height+2, 5, hl_bk_color);
    //tft.drawRoundRect(x-5, y+5-Fonts[displayFont].height, Fonts[displayFont].width+9, Fonts[displayFont].height+2, 5, hl_c_color);

    //drawChar(int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size),
    //drawChar(x, y, 'A', TFT_YELLOW, TFT_BLACK, 2),
    tft.setTextColor(hl_c_color, hl_bk_color);
    tft.drawChar(c, x, y);   
  }

  //tft.drawFastHLine (0, Y_MIN_DRAW, display_WIDTH, TFT_WHITE);
  //tft.drawPixel(x, y + Y_MIN_DRAW, TFT_RED); 
  //tft.fillRect((bargraph_X + ((bargraph_dX + bargraph_dX_space) * 0)), bargraph_Y, bargraph_dX, bargraph_dY, Smeter_table_color[0]);
}



//============================================================================
void displayDrawKey(char c, uint16_t col, uint16_t lin, uint16_t highlight)
{
  displayDrawCharKey(Letters[lin][col], 12+((col_ini+col)*(Fonts[displayFont].width+11)), ((lin_ini+lin)*(Fonts[displayFont].height+4)), highlight);
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
  uint32_t bk_color = TFT_BLUE;
  uint32_t c_color = TFT_YELLOW;
  uint32_t hl_bk_color = TFT_YELLOW;
  uint32_t hl_c_color = TFT_BLUE;

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
  uint32_t bk_color = TFT_BLUE;
  uint32_t c_color = TFT_YELLOW;
  uint32_t hl_bk_color = TFT_YELLOW;
  uint32_t hl_c_color = TFT_BLUE;


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
  uint16_t x = col*(Fonts[displayFont].width);  //+11);
  uint16_t y = lin*(Fonts[displayFont].height);  //+ 4);

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

}






#define MAIN_OPTS  3
const char MainOpts[MAIN_OPTS][12]=  { "Camera",  "Words", "QR Code"};
const uint16_t MainOptsScr[MAIN_OPTS]={ SCR_CAM,  SCR_WORDS, SCR_QRCODE };
void (*MainOptsFunc[MAIN_OPTS])(void)={ scr_cam_setup,  scr_words_setup, scr_qrcode_setup };
                              

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
  tft.setRotation(0);           // Pode ser 0, 1, 2 ou 3, dependendo da orientação desejada
  tft.setFreeFont(Fonts[displayFont].font);      // Select the font
  tft.fillScreen(bk_color);            // Preenche a tela

  lin_selec = 0;
  col_selec = 0;
  lin_ini = 2;
  col_ini = 0;

  displayDrawOpt("PicoSigner", 0, 0, 0);

  for(uint16_t lin=0; lin<MAIN_OPTS; lin++)
      displayDrawOpt(MainOpts[lin], col_ini, lin_ini+lin, 0);

  displayDrawOpt(MainOpts[lin_selec], col_ini, lin_ini+lin_selec, 1);
}
//============================================================================
void scr_main_loop()
{
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

#define KEY_BACK  "Back"
#define KEY_OK    "OK"

//============================================================================
void displayDrawEdit()
{
  tft.fillRoundRect(12, 13, (Fonts[displayFont].width*10), (Fonts[displayFont].height*2)-3, 8, bk_color);
  tft.drawRoundRect(12, 13, (Fonts[displayFont].width*10), (Fonts[displayFont].height*2)-3, 8, c_color);

  displayDrawWord(Words[words_selec], 1, 1, 1);

  uint16_t BtcW_pos = SearchBtcWords(Words[words_selec]);
  if(BtcW_pos < BTCWORDS_NUM)  //found similar
  {
    uint16_t BtcW_len = str_len(BtcWords[BtcW_pos]);
    if(BtcW_len > words_len)
      displayDrawWord(&BtcWords[BtcW_pos][words_len], 1+words_len, 1, 0);
  }

  displayCursor(1+words_len, 1, 1); 
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
  tft.setRotation(0);           // Pode ser 0, 1, 2 ou 3, dependendo da orientação desejada
  tft.setFreeFont(Fonts[displayFont].font);      // Select the font
  tft.fillScreen(bk_color);            // Preenche a tela

  lin_selec = 2;
  col_selec = 4;
  lin_ini = 3;
  col_ini = 0;

  for(uint16_t col=0; col<KEYB_COLS; col++)
    for(uint16_t lin=0; lin<KEYB_LINES; lin++)
      displayDrawKey(Letters[lin][col], col, lin, 0);

  displayDrawKey(Letters[lin_selec][col_selec], col_selec, lin_selec, 1);

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

  words_len = str_len(Words[words_selec]);
/*
  displayDrawWord(Words[words_selec], 1, 1, 1);

  uint16_t BtcW_pos = SearchBtcWords(Words[words_selec]);
  if(BtcW_pos < BTCWORDS_NUM)  //found similar
  {
    uint16_t BtcW_len = str_len(BtcWords[BtcW_pos]);
    if(BtcW_len > words_len)
      displayDrawWord(&BtcWords[BtcW_pos][words_len], 1+words_len, 1, 0);
  }

  displayCursor(1+words_len, 1, 1);
*/
  displayDrawEdit();
  displayDrawWordKey(KEY_BACK, 1, lin_ini+KEYB_LINES, 0);
  displayDrawWordKey(KEY_OK, 5, lin_ini+KEYB_LINES, 0);


}
//============================================================================
void scr_keyboard_loop()
{
  uint16_t tec = trata_teclas();
  if(tec<NUM_SWITCHES)
  {
    switch(tec)  
    {
      case 0:   //"Left"
        if((lin_selec<KEYB_LINES) && (col_selec>0))
        {
          displayDrawKey(Letters[lin_selec][col_selec], col_selec, lin_selec, 0);
          col_selec--;
          displayDrawKey(Letters[lin_selec][col_selec], col_selec, lin_selec, 1);
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
          displayDrawKey(Letters[lin_selec][col_selec], col_selec, lin_selec, 0);
          lin_selec--;
          displayDrawKey(Letters[lin_selec][col_selec], col_selec, lin_selec, 1);
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
          displayDrawKey(Letters[lin_selec][col_selec], col_selec, lin_selec, 1);
        }   
        break;
      case 2:   //"Down"
        if(lin_selec<KEYB_LINES-1)
        {
          displayDrawKey(Letters[lin_selec][col_selec], col_selec, lin_selec, 0);
          lin_selec++;
          displayDrawKey(Letters[lin_selec][col_selec], col_selec, lin_selec, 1);
        }
        else if(lin_selec==KEYB_LINES-1)
        {
          displayDrawKey(Letters[lin_selec][col_selec], col_selec, lin_selec, 0);
          lin_selec++;
          if(col_selec < 4) col_selec = 0;  else col_selec = 1;
          if(col_selec==0)
            displayDrawWordKey(KEY_BACK, 1, lin_ini+KEYB_LINES, 1);
          else
            displayDrawWordKey(KEY_OK, 5, lin_ini+KEYB_LINES, 1);
        }
        break;
      case 3:   //"Right"
        if((lin_selec<KEYB_LINES) && (col_selec<KEYB_COLS-1))
        {
          displayDrawKey(Letters[lin_selec][col_selec], col_selec, lin_selec, 0);
          col_selec++;
          displayDrawKey(Letters[lin_selec][col_selec], col_selec, lin_selec, 1);
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
          if((lin_selec==KEYB_LINES-1) && (col_selec==KEYB_COLS-2))  //espace
          {
            //do not use space
          }
          else if((lin_selec==KEYB_LINES-1) && (col_selec==KEYB_COLS-1))  // Backspace  <
          {
            if(words_len > 0)
            {
              words_len--;
              Words[words_selec][words_len] = 0;
              displayDrawEdit();
            }
          }          
          else if(words_len < BTCWORDS_MAX_LEN)
          {
            Words[words_selec][words_len] = Letters[lin_selec][col_selec];
            words_len++;
            displayDrawEdit();        
          }
        }
        else if(lin_selec==KEYB_LINES)
        {
          if(col_selec==0)
            scr_main_setup();
          else
            scr_main_setup();
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
void drawWordLin(uint16_t hl)
{
  char s[5] = " 0-";
  if(lin_selec<9) { s[0]=' ';  s[1]='1'+lin_selec;  } 
  else            { s[0]='1';  s[1]='0'+lin_selec-9; }
  displayDrawOpt(s, 0, lin_ini+lin_selec, hl);
  displayDrawOpt(Words[lin_selec], col_ini, lin_ini+lin_selec, hl);
}

//============================================================================
void scr_words_setup()
{
/*
se tela = SCR_WORDS
   mostra palavras
   e BACK ou OK
   se OK, volta para tela 0
*/
  scr = SCR_WORDS;
  displayFont = 0;
  tft.setFreeFont(Fonts[displayFont].font);      // Select the font
  tft.fillScreen(bk_color);            // Preenche a tela

  col_selec = 0;
  lin_ini = 2;
  col_ini = 3;

  displayDrawOpt("PicoSigner", 3, 0, 0);
  displayDrawOpt("PassWords", 3, 1, 0);

  for(lin_selec=0; lin_selec<WORDS_NUM; lin_selec++)
  {
    drawWordLin(0);
  }
  lin_selec = 0;
  drawWordLin(1);

}
//============================================================================
void scr_words_loop()
{
  uint16_t tec = trata_teclas();
  if(tec<NUM_SWITCHES)
  {
    switch(tec)  
    {
      case 0:   //"Left"
          scr_main_setup();
        break;
      case 1:   //"Up"
        if(lin_selec>0)
        {
          drawWordLin(0);
          lin_selec--;
          drawWordLin(1);
          //displayDrawOpt(MainOpts[lin_selec], col_ini, lin_ini+lin_selec, 1);
        }      
        break;
      case 2:   //"Down"
        if(lin_selec<WORDS_NUM-1)
        {
          drawWordLin(0);
          //displayDrawOpt(MainOpts[lin_selec], col_ini, lin_ini+lin_selec, 0);
          lin_selec++;
          drawWordLin(1);
          //displayDrawOpt(MainOpts[lin_selec], col_ini, lin_ini+lin_selec, 1);
        }
        break;
      case 3:   //"Right"
        break;
      case 4:   //"Enter"
        //MainOptsFunc[lin_selec]();
        //scr_main_setup();
        words_selec = lin_selec;
        scr_keyboard_setup();
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
  tft.setRotation(3);           // Pode ser 0, 1, 2 ou 3, dependendo da orientação desejada
  scr = SCR_CAM;
  displayFont = 0;
  tft.setFreeFont(Fonts[displayFont].font);      // Select the font
  tft.setTextColor(TFT_YELLOW);        // one-arg => transparent background

  //tft.fillScreen(TFT_GREEN);     // Preenche a tela
  //delay(3000);

  //Cam_DrawImage(); //display test
  //delay(3000);
}
//============================================================================
void scr_cam_loop()
{
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
  tft.setRotation(0);           // Pode ser 0, 1, 2 ou 3, dependendo da orientação desejada
  //tft.setFreeFont(Fonts[displayFont].font);      // Select the font
  tft.fillScreen(c_color);            // Preenche a tela

  QRCode_gen_test();
}
//============================================================================
void scr_qrcode_loop()
{
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


  test_sha256();
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
    default:
      scr = SCR_MAIN;
      scr_main_loop();
      break;
  }

}






