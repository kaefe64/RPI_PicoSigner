/*
PicoSigner.ino

Software for a Bitcoin transaction signing device
Using RPI Pico RP2040, OV7670 and ST7789

Created: Aug 2025
Author: Klaus Fensterseifer


Arduino IDE setup:
Board: Arduino Raspberry Pi Pico/RP2040/RP2350 by  Earle F. Philhower, III
       Raspberry Pi Pico
Arduino Library: 
Crypto by Rhys Weatherley
QRCode by Richard Moore


Warning must be ignored (display without touch):
#warning >>>>------>> TOUCH_CS pin not defined, TFT_eSPI touch functions will not be available!



Obs.:
to clean the compilation:
File > Preferences
Show verbose output during: compile
Compile it and look where the files where generated.
Erase them if necessary


*/

#include "PicoDisplayST7789.h" 
#include "TFT_eSPI.h"
#include "display_tft.h" 
#include "Cam_OV7670.h" 
#include "key_input.h"



#define LED_pin LED_BUILTIN // Using LED_BUILTIN for clarity, which is usually the on-board LED






//============================================================================
//*** Core1
//============================================================================
void core1_setup_and_loop()
{
  //**************
  //Core1 setup
  //**************



  //**************
	//Core1 loop
  //**************
  while(1) 
	{
    display_tft_loop();  //main process on Core1
    //text_input_loop();

  }
}


//============================================================================
void Core1_init(void)
{
  //delay(500);  //required to run core1 - after tests
  multicore_launch_core1(core1_setup_and_loop);        // Start processing the function on Core1
  //delay(5);  
}


//============================================================================
//*** Core0
//============================================================================
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.begin(115200);  
  //time for serial to connect
  for (int i = 0; i < 50; i++)
  {
    digitalWrite(LED_BUILTIN, (digitalRead(LED_BUILTIN) == HIGH ? LOW : HIGH));
    delay(100);
    if(Serial) break;
  }
  delay(10);
  Serial.println("===========================");
  Serial.println("Display Main setup");


  display_tft_setup();
  Cam_OV7670_setup();  //escreve no display, deve ser chamado após inicializar o display
  text_input_setup();
  
  Core1_init();
}


#define BlinkLedTime    500
//============================================================================
//*** Core0
//============================================================================
void loop() 
{
/*  
  static unsigned long lastLedTime = millis();
  if (millis() - lastLedTime > BlinkLedTime)
  {
    lastLedTime += BlinkLedTime;
    //LED blink
    //digitalWrite(LED_BUILTIN, (digitalRead(LED_BUILTIN) == HIGH ? LOW : HIGH));
    //Serial.print(">");
  }
*/

  //display_tft_loop();
  text_input_loop();   //switch inputs on Core0
  //Core0 loop() makes more than this call
  //I will use it to less demanding processing

}


