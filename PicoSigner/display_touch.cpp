/*
 * display_touch.cpp
 * 
 * Created: Aug 2025
 * Author: Klaus Fensterseifer

Touch handling for ILI9341 2.4" MSP2402 with XPT2046 touch controller.

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
#include "TFT_eSPI.h"   //Library  TFT_eSPI by Bodmer
#include "display_touch.h"
#include "display_tft.h"
#include "Cam_OV7670.h"

int16_t numTouch = 0;   //qdt opts touch nesta tela
bool    isTouch = false;   // 0 = não registra
stTouch optTouch[NUM_MAX_TOUCH_OPTIONS];     // setado antes da chamada de escrita das opções na tela (define area de touch)




//============================================================================
void touch_area()
{
  // desenha retangulos nas zonas de toque
  for(uint16_t i = 0; i < numTouch; i++)
  {
    tft.drawRoundRect(optTouch[i].x, optTouch[i].y, 
                      optTouch[i].w, optTouch[i].h, 
                      5, TFT_RED);
  }
}





//============================================================================
// Retorna true uma unica vez por pressionamento (borda de subida),
// ignorando enquanto o dedo permanecer pressionado.
bool touch_edge(int16_t *x, int16_t *y)
{
  static bool already_pressed = false;
  uint16_t tx, ty;

  if(tft.getTouch(&tx, &ty) == false)
  { 
    already_pressed = false;     // soltou
    return false; 
  }
  if(already_pressed == true)    // segurando (sem repetir)
    return false; 

  already_pressed = true;        // borda de subida

  *x = (int16_t)tx;  //usa int para x e y no geral, evitando  comparar uint com int
  *y = (int16_t)ty;
  return true;
}


//============================================================================
// Transforma coordenadas do touch conforme a rotacao do display.
// O XPT2046 sempre reporta na orientacao fisica; o setRotation() so
// inverte o display visualmente. Esta funcao alinha touch <-> visual.
//============================================================================
void touch_transform(int16_t *x, int16_t *y, uint8_t rotation)
{
  int16_t tx = *x;
  int16_t ty = *y;
  switch(rotation)
  {
    case 0:   // 0°   - sem transformacao
      break;
    case 1:   // 90°  - MV (sem MX/MY): colunas viram linhas
      *x = ty;
      *y = tx;
      break;
    case 2:   // 180° - invertido
      //*x = (TFT_WIDTH - 1) - tx;
      *x = tx;
      *y = (TFT_HEIGHT - 1) - ty;
      break;
    case 3:   // 270° - rotacionado 270° no sentido horario
      *x = (TFT_HEIGHT - 1) - ty;
      *y = tx;
      break;
  }
}


//============================================================================
// Stub handlers - logica de touch a ser implementada para cada tela
//============================================================================
void touch_main(int16_t x, int16_t y)
{
  touch_transform(&x, &y, displayRotation);

  for(uint16_t i = 0; i < numTouch; i++)
  {
    if((x >= optTouch[i].x) && (x <= (optTouch[i].x + optTouch[i].w)) &&
       (y >= optTouch[i].y) && (y <= (optTouch[i].y + optTouch[i].h)))
    {
      uint16_t lin = optTouch[i].lin - lin_ini;
      if(lin < MAIN_OPTS)
        MainOptsFunc[lin]();
      return;
    }
  }
}


//============================================================================
void touch_keyboard(int16_t x, int16_t y)
{
  touch_transform(&x, &y, displayRotation);

  for(uint16_t i = 0; i < numTouch; i++)
  {
    if((x >= optTouch[i].x) && (x <= (optTouch[i].x + optTouch[i].w)) &&
       (y >= optTouch[i].y) && (y <= (optTouch[i].y + optTouch[i].h)))
    {
      tft_keyboard_touch(optTouch[i].col, optTouch[i].lin);
      return;
    }
  }
}


//============================================================================
void touch_words(int16_t x, int16_t y)
{
  touch_transform(&x, &y, displayRotation);

  for(uint16_t i = 0; i < numTouch; i++)
  {
    if((x >= optTouch[i].x) && (x <= (optTouch[i].x + optTouch[i].w)) &&
       (y >= optTouch[i].y) && (y <= (optTouch[i].y + optTouch[i].h)))
    {
      //optTouch guarda a posicao de grade; converte para indice de conteudo
      uint16_t idx_lin = optTouch[i].lin - lin_ini;
      if(idx_lin == WORDS_NUM)      //botao Back
        scr_main_setup();
      else
      {
        words_selec_num = idx_lin;   //indice no Words[] -> edita no teclado
        scr_keyboard_setup();
      }
      return;
    }
  }
}

//============================================================================
void touch_search(int16_t x, int16_t y)
{
  touch_transform(&x, &y, displayRotation);

  for(uint16_t i = 0; i < numTouch; i++)
  {
    if((x >= optTouch[i].x) && (x <= (optTouch[i].x + optTouch[i].w)) &&
       (y >= optTouch[i].y) && (y <= (optTouch[i].y + optTouch[i].h)))
    {
      //optTouch guarda a posicao de grade; converte para indice de conteudo
      uint16_t idx_lin = optTouch[i].lin - lin_ini;
      if(idx_lin == SEARCH_RESULTS_MAX)   //botao Back
        scr_keyboard_setup();   //volta sem alterar nada
      else if(idx_lin < search_results_num)   //resultado
      {
        //grava a palavra selecionada; o setup do teclado copia para o campo
        strcpy(Words[words_selec_num], SearchResults[idx_lin]);
        scr_keyboard_setup();
      }
      return;
    }
  }
}

//============================================================================
void touch_cam(int16_t x, int16_t y)
{
  touch_transform(&x, &y, displayRotation);
  //debug temporario
  //Serial.print("CAM touch transform: x=");
  //Serial.print(x); Serial.print(" y=");
  //Serial.println(y);

  //A camera mostra a imagem 320x240 preenchendo a tela (rotacao 1), com a
  //dica desenhada por drawStringToImage no topo em x=0, y=1*height.
  //Tocar na dica equivale ao Enter (decodifica); tocar fora volta ao menu.
  const char *tip = "Press <Enter> to decode";
  uint16_t tip_w = (strlen(tip) + 2) * Fonts[displayFont].width;
  uint16_t tip_h = 3 * Fonts[displayFont].height;

  if((x >= 0) && (x <= tip_w) &&
     (y >= 0) && (y <= tip_h))
  {
    //debug temporario
    //Serial.println("CAM touch -> dentro da dica");
    Cam_OV7670_loop(4);   //igual teclar Enter: captura e decodifica
  }
  else
  {
    //debug temporario
    //Serial.println("CAM touch -> fora da dica (menu)");
    scr_main_setup();     //como outra tecla: volta ao menu
  }
}

//============================================================================
void touch_qrcode(int16_t x, int16_t y)
{
  touch_transform(&x, &y, displayRotation);
  scr_main_setup();   //igual qualquer tecla: volta ao menu
}

//============================================================================
void touch_addr(int16_t x, int16_t y)
{
  touch_transform(&x, &y, displayRotation);
  scr_main_setup();   //igual qualquer tecla: volta ao menu
}
