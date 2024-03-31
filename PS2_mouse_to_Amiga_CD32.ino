//PS2 to Amiga mouse translator v 1.0 (tested on CD32)
//Optical mouse Logitech M-SBF96 PS2
#include <avr/wdt.h>

//PS2MouseHandler.h modified library! - do not use the original one!
//keep the library files directly in the sketch directory
#include "PS2MouseHandler.h"
#define MOUSE_DATA 11
#define MOUSE_CLOCK 3

//   DFRobot Beetle Board <-> Amiga DB9
//     compatible with
//    Arduino Leonardo
                    // +   <- pin 7
                    // -  <-> pin 8
#define Vpin     A0 // A0  -> pin 1
#define VQpin    A1 // A1  -> pin 3
#define Hpin      9 // D9  -> pin 2
#define HQpin    10 //D10  -> pin 4
#define ButtonL  A2 // A2  -> pin 6
#define ButtonR   2 //SDA  -> pin 9
#define ButtonM   0 // RX  -> pin 5

#define LED      13 //internal LED

#define DelayLoop 3 //protection against overturning Amiga counters

volatile uint16_t pinStateDelay = 100; //us, 1/4 of the full period
volatile int16_t  m_max         = 8;   //maximum number of pulses per cycle

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

  //Serial.begin(115200);
  
  if (mouse.initialise() != 0)
  {
    //Serial.println("MOUSE_ERROR");
    //Serial.flush();
    digitalWrite(LED, HIGH);
    delay(1000);
    digitalWrite(LED, LOW);
    wdt_reset();
    wdt_enable(WDTO_1S);
    while (1) {}
  }
  
  mouse.set_resolution(1);
  mouse.set_scaling_1_1();
  mouse.set_sample_rate(80, false);
}

void loop()
{
  mouse.get_data();

  digitalWrite(ButtonL, !mouse.button(0));
  digitalWrite(ButtonR, !mouse.button(2));
  digitalWrite(ButtonM, !mouse.button(1));

  int16_t x_m = mouse.x_movement();
  int16_t y_m = mouse.y_movement();

  if (x_m != 0 || y_m != 0) 
  {
    bool HcounterIsUp = (x_m > 0) ? true : false;
    x_m = abs(x_m);
    if (x_m > m_max) x_m = m_max;

    bool VcounterIsUp = (y_m > 0) ? false : true;
    y_m = abs(y_m);
    if (y_m > m_max) y_m = m_max;

    pulseGenerator(HcounterIsUp, VcounterIsUp, pinStateDelay, x_m, y_m);
  }
  delay(DelayLoop);
}

void pulseGenerator(bool HcounterIsUp, bool VcounterIsUp ,uint16_t pinStateDelay, uint8_t HPulsesPerStep, uint8_t VPulsesPerStep)
{
  digitalWrite(LED, HIGH);
  uint8_t _HcounterIsUp = HcounterIsUp ? 255 : 0;
  uint8_t _VcounterIsUp = VcounterIsUp ? 255 : 0;
  uint8_t _maxPulsesPerStep = (HPulsesPerStep > VPulsesPerStep) ? HPulsesPerStep : VPulsesPerStep;

  for (int pulses = 0; pulses < _maxPulsesPerStep; pulses++)
  {
    if (HPulsesPerStep > pulses)
    {
      digitalWrite(Hpin, 0);
      digitalWrite(HQpin, _HcounterIsUp ^ 0);
    } 

    if (VPulsesPerStep > pulses)
    {
      digitalWrite(Vpin, 0);
      digitalWrite(VQpin, _VcounterIsUp ^ 0);
    }

    delayMicroseconds(pinStateDelay);

    if (HPulsesPerStep > pulses) digitalWrite(HQpin, _HcounterIsUp ^ 255);

    if (VPulsesPerStep > pulses) digitalWrite(VQpin, _VcounterIsUp ^ 255);

    delayMicroseconds(pinStateDelay);
    
    if (HPulsesPerStep > pulses) digitalWrite(Hpin, 255);

    if (VPulsesPerStep > pulses) digitalWrite(Vpin, 255);

    delayMicroseconds(pinStateDelay);

    if (HPulsesPerStep > pulses) digitalWrite(HQpin, _HcounterIsUp ^ 0);

    if (VPulsesPerStep > pulses) digitalWrite(VQpin, _VcounterIsUp ^ 0);

    delayMicroseconds(pinStateDelay);
  }

  digitalWrite(LED, LOW);
}
