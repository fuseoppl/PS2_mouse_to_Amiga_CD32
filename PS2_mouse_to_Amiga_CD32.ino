//PS2 to Amiga mouse translator v 2.1 (tested on CD32)
//Optical mouse Logitech M-SBF96 PS2
#include <avr/wdt.h>
#include <EEPROM.h>
//PS2MouseHandler.h modified library! - do not use the original one!
//keep the library files directly in the sketch directory
#include "PS2MouseHandler.h"

//====================================//
//   DFRobot Beetle Board
//     compatible with
//    Arduino Leonardo  <-> Amiga DB9
//------------------------------------//
                      // +vcc <-  pin 7
                      // gnd  <-> pin 8
#define Vpin       A0 // A0    -> pin 1
#define VQpin      A1 // A1    -> pin 3
#define Hpin        9 // D9    -> pin 2
#define HQpin      10 //D10    -> pin 4
#define ButtonL     2 //SDA    -> pin 6
#define ButtonR    A2 // A2    -> pin 9
#define ButtonM     0 // RX    -> pin 5
//------------------------------------//
#define MOUSE_DATA 11 //D11
#define MOUSE_CLOCK 3 //SCL
//------------------------------------//
#define LED        13 //internal LED
//====================================//

#define resolutionHighDiv 3
#define resolutionLowDiv  7

const char* firmwareRevision    = "2.1";
volatile uint16_t pinStateDelay = 20;   //us, 1/2 of the one pulse
volatile int16_t  m_max         = 15;   //maximum number of pulses per cycle
bool resolutionState            = 0;
byte resolutionDiv              = resolutionLowDiv;

PS2MouseHandler mouse(MOUSE_CLOCK, MOUSE_DATA, PS2_MOUSE_STREAM);

void setup() {
  resolutionDiv      = EEPROM.read(0);
  if (resolutionDiv < resolutionHighDiv || resolutionDiv > resolutionLowDiv) resolutionDiv = resolutionLowDiv;
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

Serial.begin(250000);

  digitalWrite(LED, HIGH);

  if (mouse.initialise() != 0) {
    wdt_reset();
    wdt_enable(WDTO_1S);
    Serial.println("MOUSE_ERROR");
    Serial.flush();
    digitalWrite(LED, LOW);
    while (1) {}
  }

  digitalWrite(LED, LOW);

  mouse.set_resolution(8);
  mouse.set_scaling_2_1(); //2_1 == acceleration on
  mouse.set_sample_rate(200, false); //max 200
  mouse.set_stream_mode();
}

void loop() {
  delay(5); 
  //mouse.enable_data_reporting();
  mouse.get_data();

  digitalWrite(ButtonL, !mouse.button(0));
  digitalWrite(ButtonR, !mouse.button(2));
  digitalWrite(ButtonM, !mouse.button(1));

  int16_t x_m = mouse.x_movement();
  int16_t y_m = mouse.y_movement();
  int16_t z_m = mouse.z_movement();

  if (!resolutionState && mouse.button(1)) {
    resolutionDiv = (resolutionDiv == resolutionHighDiv) ? resolutionLowDiv : resolutionHighDiv;
    EEPROM.write(0, resolutionDiv);
    delay(10);
  }

  resolutionState = mouse.button(1);

/*
  if (x_m != 0 || y_m != 0 || z_m != 0) {
    Serial.print(x_m);
    Serial.print("*");
    Serial.print(y_m);
    Serial.print("*");
    Serial.println(z_m);
    Serial.flush();
  }
*/

  //mouse.disable_data_reporting();

  if (x_m != 0 || y_m != 0) {
    bool HcounterIsUp = (x_m >= 0) ? true : false;
    x_m = abs(x_m);
   
    if (x_m > 0) {
      double _x_m = x_m / (double)resolutionDiv;
      x_m = _x_m + 0.5;
      
      if (x_m < 1) x_m = 1;
      if (x_m > m_max) x_m = m_max;
    }

    bool VcounterIsUp = (y_m >= 0) ? false : true;
    y_m = abs(y_m);

    if (y_m > 0) {
      double _y_m = y_m / (double)resolutionDiv;
      y_m = _y_m + 0.5;

      if (y_m < 1) y_m = 1;
      if (y_m > m_max) y_m = m_max;
    }

    pulseGenerator(HcounterIsUp, VcounterIsUp, pinStateDelay, x_m, y_m); 
  }
}

void pulseGenerator(bool HcounterIsUp, bool VcounterIsUp ,uint16_t pinStateDelay, uint8_t HPulsesPerStep, uint8_t VPulsesPerStep) {
  digitalWrite(LED, HIGH);
  uint8_t _maxPulsesPerStep = (HPulsesPerStep > VPulsesPerStep) ? HPulsesPerStep : VPulsesPerStep; 

  for (int pulses = 0; _maxPulsesPerStep > pulses ; pulses++) {    

    delayMicroseconds(pinStateDelay);

    if (HPulsesPerStep > pulses) {
      if (HcounterIsUp) digitalWrite(Hpin,  !digitalRead(Hpin));
      else              digitalWrite(HQpin, !digitalRead(HQpin));
    }

    if (VPulsesPerStep > pulses) {
      if (VcounterIsUp) digitalWrite(Vpin,  !digitalRead(Vpin));
      else              digitalWrite(VQpin, !digitalRead(VQpin));
    }

    delayMicroseconds(pinStateDelay);

    if (HPulsesPerStep > pulses) {
      if (HcounterIsUp) digitalWrite(HQpin, !digitalRead(HQpin));
      else              digitalWrite(Hpin,  !digitalRead(Hpin));
    }

    if (VPulsesPerStep > pulses) {
      if (VcounterIsUp) digitalWrite(VQpin, !digitalRead(VQpin));
      else              digitalWrite(Vpin,  !digitalRead(Vpin));
    }
  }

  digitalWrite(LED, LOW);
}
