#ifndef __CAM_OV7670_H__
#define __CAM_OV7670_H__

#ifdef __cplusplus
extern "C" {
#endif



// =========================================================
// --- Definições dos Pinos da Câmera ---
// same as https://learn.adafruit.com/capturing-camera-images-with-circuitpython/raspberry-pi-pico-wiring
// and https://helloraspberrypi.blogspot.com/2021/08/ov7670-camera-st7789-ips-display-on.html
// =========================================================
#define CAM_D0    12 // GP12 cam output
#define CAM_D1    13 // GP13 cam output
#define CAM_D2    14 // GP14 cam output
#define CAM_D3    15 // GP15 cam output
#define CAM_D4    16 // GP16 cam output
#define CAM_D5    17 // GP17 cam output
#define CAM_D6    18 // GP18 cam output
#define CAM_D7    19 // GP19 cam output

#define CAM_PCLK  26 // GP26 (Pixel Clock) cam output
#define CAM_XCLK  20 // GP20 XCLK (Master/System Clock) cam input

#define CAM_VSYNC 22  //GP22 (Vertical Sync) cam output
#define CAM_HREF  21  // GP21 (Horizontal Reference) cam output

#define CAM_RET   10  //Reset (active low) cam input
#define CAM_PWDN  11  //Power down (active high) cam input  

// Configuração I2C para a câmera
#define CAM_SDA 8  //pullup 1K8 to 3V3
#define CAM_SCL 9  //pullup 1K8 to 3V3



#define FRAME_WIDTH   320
#define FRAME_HEIGHT  240
//#define FRAME_RATE    30



void Cam_DrawImage(void); //display test

void Cam_OV7670_setup(void);
void Cam_OV7670_loop(uint16_t tec);






#ifdef __cplusplus
}
#endif
#endif
