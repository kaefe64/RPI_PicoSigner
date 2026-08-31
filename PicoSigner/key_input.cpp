#include "Arduino.h"
#include "key_input.h"
#include "TFT_eSPI.h"
#include "display_tft.h"



#define switch_ADC  28  //GPIO28
//divisor resistivo pullup=2K7 + 0R, 220R, 470R, 1K, 2K7
//const uint16_t switches_ADC[NUM_SWITCHES] = {0, 308, 607, 1106, 2047};   //ADC 12 bits  4095
//const int16_t switches_ADC[NUM_SWITCHES] = {2, 80, 150, 274, 510};   //ADC 10 bits  1023
//const int16_t switches_ADC_range[NUM_SWITCHES] = {50, 50, 50, 50, 50};   //ADC 10 bits  1023

//divisor resistivo pullup=2K7   +         0R, 470R, 1K5, 3K3, 8K2
const int16_t switches_ADC[NUM_SWITCHES] = {0,  150, 365, 562, 770};   //ADC 10 bits  1023
const char switches_func[NUM_SWITCHES][8] = {"Left", "Up", "Down", "Right", "Enter"};
#define SWITCHES_ADC_RANGE    35
//#define AVG_SHIFT   2
#define TEXT_INPUTTIME    4

volatile uint16_t out_teclas = NUM_SWITCHES;


//============================================================================
uint16_t trata_teclas()
{
  uint16_t ret = out_teclas;
  if(ret != NUM_SWITCHES)
  {
    out_teclas = NUM_SWITCHES; //send out only once and try to no write to out_teclas at the same time as Core1
    //Serial.println(switches_func[ret]);  //debug: send pressed switch to serial
  }
  return ret;
}



//============================================================================
//*** Core1
//============================================================================
uint16_t read_teclas()  //return switch num once when switch change
{
  uint16_t sw = NUM_SWITCHES;
  static uint16_t sw_pressed = NUM_SWITCHES;
  static int16_t valorADC_old = 0;
//  static uint8_t n=0;
//  static int16_t valorADC_avg = 0;
  //static int16_t valorADC_acc = 0;


  static unsigned long lastText_inputTime = millis();
  //the switch must stay on its value for some time
  if ((millis() - lastText_inputTime) > TEXT_INPUTTIME)  //switch RC time = aprox 1ms
  {
    lastText_inputTime += TEXT_INPUTTIME;


    // Realiza a leitura do valor analógico do pino
    int16_t valorADC = analogRead(switch_ADC);
    //Serial.print("\r");
    //Serial.println(valorADC);

    for(sw=0; sw<NUM_SWITCHES; sw++)
    {
      if(((valorADC>(switches_ADC[sw]-SWITCHES_ADC_RANGE))&&(valorADC<(switches_ADC[sw]+SWITCHES_ADC_RANGE))) &&
          ((valorADC_old>(switches_ADC[sw]-SWITCHES_ADC_RANGE))&&(valorADC_old<(switches_ADC[sw]+SWITCHES_ADC_RANGE))))
      {
        break;
      }
    }

    if(sw<NUM_SWITCHES)
    {
      if(sw!=sw_pressed)  //if changed
      {
        sw_pressed = sw;
        // Exibe o valor lido no Monitor Serial
/*      //serial write on Core1  !!!!
        Serial.print("Valor do ADC2 (GPIO28): ");
        Serial.print(valorADC); 
        Serial.print("  ");
        Serial.println(i);     
        Serial.print("  ");
*/
        //Serial.println(switches_func[sw]);
      }
      else
      {
        //Serial.println("  ");
        sw = NUM_SWITCHES;  //no switch pressed
      }
    }
    else
    {
      //Serial.println("  ");
      sw_pressed = NUM_SWITCHES;  //no switch pressed
    }

    valorADC_old = valorADC;
  }
  return sw;  
}




//============================================================================
void text_input_setup(void)
{
  //pinMode(switch_ADC, INPUT);
  //analogReadResolution(12);

  //keyboard_init();
    
}


#define BlinkLedTimeCore1    500
//============================================================================
//*** Core1
//============================================================================
void text_input_loop()
{
  static unsigned long lastLedTime = millis();
  if (millis() - lastLedTime > BlinkLedTimeCore1)
  {
    lastLedTime += BlinkLedTimeCore1;
    //LED blink
    digitalWrite(LED_BUILTIN, (digitalRead(LED_BUILTIN) == HIGH ? LOW : HIGH));
    //Serial.print(">");
  }

  uint16_t in_teclas = read_teclas();
  if(in_teclas<NUM_SWITCHES)
  {
    out_teclas = in_teclas;
  }
}




