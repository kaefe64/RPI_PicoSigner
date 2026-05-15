/*
 * Cam_OV7670.cpp
 * 
 * Created: May 2025
 * Author: Klaus Fensterseifer
 

Based on:
https://github.com/mxyxbb/rp2040_ov7670_usb_camera/tree/main

YUV422

*/

#include "Arduino.h"
#include <Wire.h>

#include "TFT_eSPI.h"
#include "display_tft.h"
#include "Cam_OV7670.h"

#include "ov7670.h"
#include "hardware/vreg.h"
#include "qr_code.h"



const uint8_t CMD_REG_WRITE = 0xAA;
const uint8_t CMD_REG_READ = 0xBB;
const uint8_t CMD_CAPTURE = 0xCC;
const uint8_t CMD_SET_WINDOW = 0xDD;
#define DUMMY_BYTE (2*4)  //2*4
#define FRAME_OV7670_SIZE   (FRAME_WIDTH * FRAME_HEIGHT)  //320 x 240 = 76800 x 2 = 153600 (RP2040 max RAM = 262K)
//uint16_t *frame_ov7670;
uint16_t frame_ov7670[FRAME_OV7670_SIZE + DUMMY_BYTE];
struct ov2640_config config;



//============================================================================
void Cam_DrawImage(void) 
{
  uint32_t pos = 0;
  Serial.println("Cam_DrawImage 1");
  Serial.println(FRAME_OV7670_SIZE + DUMMY_BYTE); // tamanho em bytes

  for(uint16_t y=0; y<FRAME_HEIGHT; y++)
  {
    for(uint16_t x=0; x<FRAME_WIDTH; x++)
    {
      uint16_t c = tft_color565((x>255 ? (255-(x-255)) : x&0xff), y, 255-y);
      frame_ov7670[pos++] = c;
    }
  }
  Serial.println("Cam_DrawImage 2");
  displayDrawImage(frame_ov7670, FRAME_WIDTH, FRAME_HEIGHT);
  Serial.println("Cam_DrawImage 3");
}


//============================================================================
void Cam_OV7670_setup(void) 
{
  //frame_ov7670 = (uint16_t*)malloc(FRAME_OV7670_SIZE + DUMMY_BYTE);   //usar free(frame_ov7670); para liberar a memória

  vreg_set_voltage(VREG_VOLTAGE_MAX);
  set_sys_clock_khz(120UL*1000UL, true);

	config.sccb = i2c0;
	config.pin_sioc = CAM_SCL;
	config.pin_siod = CAM_SDA;

	config.pin_resetb = CAM_RET;
	config.pin_xclk = CAM_XCLK;
	config.pin_vsync = CAM_VSYNC;
	config.pin_y2_pio_base = CAM_D0;

	config.pio = pio1;    //pio used 0-1
	config.pio_sm = 0;    //pio state machine 0-3

//	config.dma_channel = 0;
	config.dma_channel = dma_claim_unused_channel(true);
	config.image_buf = frame_ov7670;
	config.image_buf_size = FRAME_OV7670_SIZE;  //sizeof(frame_ov7670);

	ov2640_init(&config);
	uint8_t midh = ov2640_reg_read(&config, 0x1C);
	uint8_t midl = ov2640_reg_read(&config, 0x1D);
  Serial.print(midh);  //127
  Serial.print(" "); 
  Serial.print(midl);  //162
  Serial.println("  Camera OV7670 setup");

  //OV7670_Window_Set(&config, 0, 0, FRAME_WIDTH, FRAME_HEIGHT);

  //Cam_DrawImage(); //display test
  //delay(3000);


  QRCode_init((uint8_t*)frame_ov7670);
}



//#define CamTime    1000

//============================================================================
void Cam_OV7670_loop(uint16_t tec) 
{

  ov2640_capture_frame(&config);
/*
  //to see on display the grayscaled image delivered to quirc
  for (int n = 0; n < FRAME_WIDTH*FRAME_HEIGHT; n++) 
  {
    uint8_t pixel_gray  = RGB2Gray(frame_ov7670[n]);

    uint16_t R_gray = pixel_gray >> 3;  // (valor de 0 a 31)
    uint16_t G_gray = pixel_gray >> 2;  // (valor de 0 a 63)
    uint16_t B_gray = pixel_gray >> 3;  // (valor de 0 a 31)

    //RGB565 byte swapped = gggbbbbb rrrrrggg
    uint16_t pixel_rgb565 = (R_gray << 11) | (G_gray << 5) | B_gray;
    frame_ov7670[n] = (pixel_rgb565 << 8) | (pixel_rgb565 >> 8);
  }
*/
  //writes over image before sending to display (avoid fliking)
  drawStringToImage(frame_ov7670, FRAME_WIDTH, FRAME_HEIGHT, 
                    (0*Fonts[displayFont].width), (1*Fonts[displayFont].height), 
                    "Press <Enter> to decode", TFT_YELLOW);

  //shwos image on display
  displayDrawImage(frame_ov7670, FRAME_WIDTH, FRAME_HEIGHT);

  if(tec == 4)  //enter
    QRCode_decode(frame_ov7670);  //it will use the frame buffer for decoding

}




