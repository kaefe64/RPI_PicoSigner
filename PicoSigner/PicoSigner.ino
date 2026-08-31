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

Serial:
/dev/ttyACM0



Warning must be ignored (display without touch):
#warning >>>>------>> TOUCH_CS pin not defined, TFT_eSPI touch functions will not be available!



Obs.:
to clean the compilation:
File > Preferences
Show verbose output during: compile
Compile it and look where the files where generated.
Erase them if necessary


============================================================================
mbedTLS for BIP-84 address derivation (bip84.cpp / "Address" menu option)
============================================================================
The Philhower RP2040 core does NOT link mbedTLS by default, and neither
arduino-cli (IDE 2.x) nor the core adds an -I include path for the sketch
folder or sketch/src/. Because mbedTLS headers include each other as
"mbedtls/xxx.h" (which needs a real include path), the minimal mbedTLS subset
lives instead in a local Arduino library so arduino-cli adds -I<lib>/src
automatically.

Library: ~/Arduino/libraries/MBedTLS_Btc84/
  src/mbedtls/      74 public headers  (include/mbedtls) - via -I<lib>/src
  src/psa/          23 public headers  (include/psa)     - via -I<lib>/src
  src/library/      15 .c + 64 internal .h (library/*)   - compiled as C

Source/version: Pico SDK rp2040 6.0.0 -> pico-sdk/lib/mbedtls
               (mbedTLS 3.x public headers + library sources).

Only the modules actually used by bip84.cpp are included:
  md, sha256, sha512, ripemd160, pkcs5, bignum(+bignum_core/mod/mod_raw),
  constant_time, platform, platform_util, error, ecp, ecp_curves.

Configuration: src/mbedtls/mbedtls_config.h was REPLACED by a minimal standalone
config (see file). It keeps everything needed for BIP-84 (SHA-256/SHA-512,
RIPEMD-160, MD/HMAC, PBKDF2, ECP secp256k1, bignum) and disables everything
that needs POSIX on bare-metal RP2040 (no NET, TIMING, HAVE_TIME, FS_IO,
X.509/PEM, ENTROPY/DRBG, all SSL key exchanges). Because the config is
self-contained (does not include the default mbedtls_config.h), mbedtls'
internal consistency check (check_config.h) runs only once and passes.

bip84.cpp provides its own dummy RNG for ECP blinding, so no entropy/DRBG
is required. The ECP code is MODP-based and allocation-heavy (calloc/free);
use "Suspend, resume all cores" build flags as usual and keep plenty of RAM
headroom. Flash: 2 MB default is fine; gc-sections drops unused code.
============================================================================
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


