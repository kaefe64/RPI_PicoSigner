/*
 tft_setup_RP2040_ST7789_240x320.h   //** adapted from #include <User_Setups/Setup138_Pico_Explorer_Base_RP2040_ST7789.h>

>>Actual method:
>>Edit file:  .../Arduino/libraries/TFT_eSPI/User_Setup_Select.h
>>Include (with the path to this project):  #include ".../PicoCripto/tft_setup_RP2040_ST7789_240x320.h"
>>Comment other include's

>>Not working:
>>Use  #include "tft_setup_RP2040_ST7789_240x320.h"  just before the #include "TFT_eSPI"  on this project
>>It should skip any #include on User_Setup_Select.h  with the  #define USER_SETUP_LOADED  below
>>Obs.: The file .../Arduino/libraries/TFT_eSPI/User_Setup_Select.h  checks for USER_SETUP_LOADED to do not include any tft...h 

*/

#define USER_SETUP_LOADED    // usado em User_Setup_Select.h para nao carregar outro tft..h

                             
#define RP2040_PIO_SPI   //to make SPI with PIO

#define TFT_RGB_ORDER TFT_BGR  // Colour order Blue-Green-Red

// For the RP2040 processor define the SPI port channel used, default is 0
 #define TFT_SPI_PORT 1 // Set to 0 if SPI0 pins are used, or 1 if spi1 pins used

// Pico Explorer Base by Pimoroni (RP2040) (ST7789 on SPI bus with 240x240 TFT)
#define USER_SETUP_ID 138

#define ST7789_DRIVER     // Configure all registers

#define TFT_WIDTH  240
#define TFT_HEIGHT 320  //240


/*
Mapeamento dos Pinos do Display para o Raspberry Pi Pico (SPI1)

Aqui está a relação e o porquê de cada conexão:
Pino no Display	Função	Pino Sugerido no Raspberry Pi Pico	Observações
BL	Backlight (Luz de Fundo)  GP12 (ou outro GPIO livre)	Controla a iluminação da tela. Você pode ligá-lo diretamente ao 3.3V do Pico para manter a luz sempre acesa, ou conectá-lo a um GPIO para controlá-lo via software (ligar/desligar, ou até mesmo PWM para ajustar o brilho, se o display suportar).
CS	Chip Select (SPI)	        GP9	O pino CS (Chip Select) é usado para selecionar o dispositivo SPI específico com o qual o Raspberry Pi Pico vai se comunicar. Ele "ativa" a comunicação com o display quando em nível baixo.
DC	Data/Command (SPI)	      GP14	Este pino é crucial para o SPI. Ele informa ao display se os dados que estão sendo enviados no momento são Dados (pixels, configurações) ou Comandos (instruções para o controlador).
RST	Reset	                    GP15	O pino RST (Reset) é usado para reiniciar o controlador do display. Geralmente é pulsado brevemente no início da comunicação para garantir que o display esteja em um estado conhecido.
SDA	SPI Data In/Out	          GP11 (MOSI)	Para displays SPI, SDA geralmente significa MOSI (Master Out, Slave In - o Raspberry Pi Pico envia dados para o display). Seu display ST7789 é primariamente de escrita, então este é o pino de dados principal.
SCL	SPI Clock	                GP10	O pino SCL (Serial Clock) ou SCK (Serial Clock) é o sinal de clock que sincroniza a transferência de dados entre o Raspberry Pi Pico e o display.
VCC	Power (Alimentação)	      3.3V (do Raspberry Pi Pico)	Este é o pino de alimentação do display. O ST7789 geralmente opera com 3.3V, o que é perfeito, pois o Raspberry Pi Pico também é um dispositivo de 3.3V (não precisa de conversores de nível para a comunicação SPI).
GND	Ground (Terra)	          GND (do Raspberry Pi Pico)
*/
// For Pico Explorer Base (PR2040)
#define TFT_CS     1  //13  //9     //GPIO9    17   // Chip Select pin
#define TFT_DC     0  //9  //14    //GPIO14   16   // Data Command control pin 
#define TFT_RST    4  //8  //15    //GPIO15   -1   // No Reset pin
#define TFT_MOSI   3  //11  //11    //GPIO11   19
#define TFT_SCLK   2  //10  //10    //GPIO10   18

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
// #define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT

#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000



//******************
//** extras
//******************
//#define TFT_BL              14  //12 // GPIO12 para o controle do backlight
//#define TFT_BACKLIGHT_ON    HIGH // Ou LOW, dependendo da sua fiação

// DMA settings for RP2040 (these are generally already correctly set for RP2040 setups)
#define SUPPORT_TRANSACTIONS
#define HAS_SPI_DMA
// #define ESP32_DMA // This is often defined in RP2040 configs, despite the name.

//#define TOUCH_CS   -1   // No touch screen controller
#undef TOUCH_CS
//#undef TFT_MISO
#define TFT_MISO   -1
