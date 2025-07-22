//PS2 to Amiga mouse translator v 1.1 (tested on CD32)
//Optical mouse Logitech M-SBF96 PS2
#include <avr/wdt.h>

//PS2MouseHandler.h modified library! - do not use the original one!
//keep the library files directly in the sketch directory
#include "PS2MouseHandler.h"

//====================================//
//   DFRobot Beetle Board
//     compatible with
//    Arduino Leonardo  <-> Amiga DB9
//------------------------------------//
                      // +   <- pin 7
                      // -  <-> pin 8
#define Vpin       A0 // A0  -> pin 1
#define VQpin      A1 // A1  -> pin 3
#define Hpin        9 // D9  -> pin 2
#define HQpin      10 //D10  -> pin 4
#define ButtonL     2 //SDA  -> pin 6
#define ButtonR    A2 // A2  -> pin 9
#define ButtonM     0 // RX  -> pin 5
//------------------------------------//
#define MOUSE_DATA 11 //D11
#define MOUSE_CLOCK 3 //SCL
//====================================//

#define LED      13 //internal LED

volatile uint16_t pinStateDelay = 25; //100 us, 1/4 of the full period
volatile int16_t  m_max         = 15;   //10 maximum number of pulses per cycle
bool resolutionState            = 0;
double resolution               = 2.0;

PS2MouseHandler mouse(MOUSE_CLOCK, MOUSE_DATA, PS2_MOUSE_STREAM);

void setup()
{
  pinMode(Vpin,  OUTPUT);
  pinMode(VQpin, OUTPUT);
  pinMode(Hpin,  OUTPUT);
  pinMode(HQpin, OUTPUT);
  pinMode(ButtonL, OUTPUT);
  pinMode(ButtonR, OUTPUT);
  pinMode(ButtonM, OUTPUT);
  digitalWrite(ButtonL, HIGH);
  digitalWrite(ButtonR, HIGH);
  digitalWrite(ButtonM, HIGH);
  digitalWrite(LED, LOW);

Serial.begin(115200);

  digitalWrite(LED, HIGH);
  if (mouse.initialise() != 0)
  {
    wdt_reset();
    wdt_enable(WDTO_1S);
    Serial.println("MOUSE_ERROR");
    Serial.flush();
    digitalWrite(LED, LOW);
    while (1) {}
  }
  digitalWrite(LED, LOW);

  mouse.set_resolution(1);
  mouse.set_scaling_2_1();
  mouse.set_sample_rate(200, false); //max 200
  mouse.set_stream_mode();
}

void loop()
{
  delay(10);
  mouse.get_data();

  digitalWrite(ButtonL, !mouse.button(0));
  digitalWrite(ButtonR, !mouse.button(2));
  digitalWrite(ButtonM, !mouse.button(1));

  int16_t x_m = mouse.x_movement();
  int16_t y_m = mouse.y_movement();
  int16_t z_m = mouse.z_movement();

/*
if (x_m != 0 || y_m != 0 || z_m != 0){
  Serial.print(x_m);
  Serial.print("*");
  Serial.print(y_m);
  Serial.print("*");
  Serial.print(z_m);
  Serial.println("");
  Serial.flush(); 
}
*/

  if (!resolutionState && mouse.button(1))
  {
    resolution = (resolution < 10) ? 5.0 : 2.0;
  }

  resolutionState = mouse.button(1);

  if (x_m != 0 || y_m != 0)
  {
    bool HcounterIsUp = (x_m >= 0) ? true : false;
    x_m = abs(x_m);
   
    if (x_m > 0)
    {
      double _x_m = x_m / resolution;
      x_m = _x_m + 0.5;
      
      if (x_m < 1) x_m = 1;
      if (x_m > m_max) x_m = m_max;
    }

    bool VcounterIsUp = (y_m >= 0) ? false : true;
    y_m = abs(y_m);

    if (y_m > 0)
    {
      double _y_m = y_m / resolution;
      y_m = _y_m + 0.5;

      if (y_m < 1) y_m = 1;
      if (y_m > m_max) y_m = m_max;
    }
/*
Serial.print(x_m);
Serial.print(";");
Serial.print(y_m);
Serial.println("");
Serial.flush(); 
*/
    pulseGenerator(HcounterIsUp, VcounterIsUp, pinStateDelay, x_m, y_m);
  }
}

void pulseGenerator(bool HcounterIsUp, bool VcounterIsUp ,uint16_t pinStateDelay, uint8_t HPulsesPerStep, uint8_t VPulsesPerStep)
{

  digitalWrite(LED, HIGH);
  uint8_t _maxPulsesPerStep = (HPulsesPerStep > VPulsesPerStep) ? HPulsesPerStep : VPulsesPerStep; 

  for (int pulses = 0; _maxPulsesPerStep > pulses ; pulses++)
  {    

    delayMicroseconds(pinStateDelay);
    if (HcounterIsUp) {  
      if (HPulsesPerStep > pulses) digitalWrite(Hpin, !digitalRead(Hpin));
    }
    else {
      if (HPulsesPerStep > pulses) digitalWrite(HQpin, !digitalRead(HQpin));
    }

    if (VcounterIsUp) {  
      if (VPulsesPerStep > pulses) digitalWrite(Vpin, !digitalRead(Vpin));
    }
    else {
      if (VPulsesPerStep > pulses) digitalWrite(VQpin, !digitalRead(VQpin));
    }

    delayMicroseconds(pinStateDelay);
    if (HcounterIsUp) { 
      if (HPulsesPerStep > pulses) digitalWrite(HQpin, !digitalRead(HQpin));
    }
    else {
      if (HPulsesPerStep > pulses) digitalWrite(Hpin, !digitalRead(Hpin));
    }

    if (VcounterIsUp) { 
      if (VPulsesPerStep > pulses) digitalWrite(VQpin, !digitalRead(VQpin));
    }
    else {
      if (VPulsesPerStep > pulses) digitalWrite(Vpin, !digitalRead(Vpin));
    }
  }

  digitalWrite(LED, LOW);
}
