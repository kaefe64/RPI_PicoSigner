#ifndef __DISPLAY_TOUCH_H__
#define __DISPLAY_TOUCH_H__

#ifdef __cplusplus
extern "C" {
#endif

//#include <stdint.h>


#define NUM_MAX_TOUCH_OPTIONS 40

typedef struct {
  int16_t     x;
  int16_t     y;
  int16_t     w;
  int16_t     h;
  uint16_t    col;      // posicao da grade na tela (absoluta, ja com col_ini/lin_ini)
  uint16_t    lin;
} stTouch;

extern int16_t numTouch;   //qdt opts touch nesta tela
extern bool    isTouch;   // 0 = não registra
extern stTouch optTouch[NUM_MAX_TOUCH_OPTIONS];     // setado antes da chamada de escrita

void touch_area();
bool touch_edge(int16_t *x, int16_t *y);

// Transforma coordenadas do touch conforme a rotacao do display.
// Chamar antes de repassar x,y para os handlers.
void touch_transform(int16_t *x, int16_t *y, uint8_t rotation);

// Handlers de touch por tela (stubs - sem logica ainda)
void touch_main(int16_t x, int16_t y);
void touch_keyboard(int16_t x, int16_t y);
void touch_words(int16_t x, int16_t y);
void touch_search(int16_t x, int16_t y);
void touch_cam(int16_t x, int16_t y);
void touch_qrcode(int16_t x, int16_t y);
void touch_addr(int16_t x, int16_t y);

#ifdef __cplusplus
}
#endif
#endif
