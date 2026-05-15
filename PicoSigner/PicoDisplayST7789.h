#ifndef __PicoDisplayST7789_H__
#define __PicoDisplayST7789_H__

#ifdef __cplusplus
extern "C" {
#endif


#define PY2KLA_setup     1       //setup for PY2KLA hardware   (comment this line for default setup)
#define SW_VERSION    "Dec24"    //software version

//choose the serial to be used (names come from MBed library, look at "pins_arduino.h" and comments at .ino file)
//#define Serialx   Serial  //USB    //USB virtual serial  /dev/ttyACM0
//#define Serialx   Serial1   //UART0  /dev/ttyUSB0

//#define LOOP_MS    100  //100 miliseconds




#ifdef __cplusplus
}
#endif
#endif
