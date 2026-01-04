//PS/2 to Amiga mouse translator v 2.2 (tested on CD32)
//Optical mouse Genius M/N:DX-110 GM-150014 PS/2
//Nine speed levels, speed stored in EEPROM memory
//Increase speed: Middle mouse button
//Decrease speed: Right mouse button + Middle mouse button
#include <avr/wdt.h>
#include <EEPROM.h>
//PS2MouseHandler.h modified library! - do not use the original one!
//keep the modified library files directly in the sketch directory
#include "PS2MouseHandler.h"

//=======================================//
//   DFRobot Beetle Board
//     compatible with
//    Arduino Leonardo  <-> Amiga DB9
                        // +vcc <-  pin 7 & pin 4 (Mouse mini DIN plug)
                        // gnd  <-> pin 8 & pin 3 (Mouse mini DIN plug)
#define Vpin         A0 // A0    -> pin 1
#define VQpin        A1 // A1    -> pin 3
#define Hpin          9 // D9    -> pin 2
#define HQpin        10 //D10    -> pin 4
#define ButtonL       2 //SDA    -> pin 6
#define ButtonR      A2 // A2    -> pin 9
#define ButtonM       0 // RX    -> pin 5
//-------------------------------------------------------------------------//
#define MOUSE_DATA   11 //D11 (Beetle Board) -> pin 1 (Mouse mini DIN plug)
#define MOUSE_CLOCK   3 //SCL (Beetle Board) -> pin 5 (Mouse mini DIN plug)
//-------------------------------------------------------------------------//
#define LED          13 //internal LED
//=======================================//

#define speedHighDiv 2
#define speedLowDiv  10

const char* firmwareRevision    = "2.2";
volatile uint16_t pinStateDelay = 20;   //us, half the length of one pulse
volatile int16_t  m_max         = 15;   //maximum number of pulses per cycle
bool speedState                 = 0;
byte speedDiv                   = speedLowDiv;

PS2MouseHandler mouse(MOUSE_CLOCK, MOUSE_DATA, PS2_MOUSE_STREAM);

void setup() {
  speedDiv      = EEPROM.read(0);
  if (speedDiv < speedHighDiv || speedDiv > speedLowDiv) speedDiv = speedLowDiv;
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
  //mouse.enable_data_reporting(); //for test purpose only
  mouse.get_data();

  digitalWrite(ButtonL, !mouse.button(0));
  digitalWrite(ButtonR, !mouse.button(2));
  digitalWrite(ButtonM, !mouse.button(1));

  int16_t x_m = mouse.x_movement();
  int16_t y_m = mouse.y_movement();
  int16_t z_m = mouse.z_movement();

  if (!speedState && mouse.button(1)) {
    speedDiv = (mouse.button(2)) ?  speedDiv + 1 : speedDiv - 1;
    if (speedDiv < speedHighDiv) speedDiv = speedHighDiv;
    if (speedDiv > speedLowDiv) speedDiv = speedLowDiv;
    EEPROM.write(0, speedDiv);
    delay(10);
  }

  speedState = mouse.button(1); //before changing the speed again, you must release the middle mouse button

//debbuging
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

  //mouse.disable_data_reporting(); //for test purpose only

  if (x_m != 0 || y_m != 0) {
    bool HcounterIsUp = (x_m >= 0) ? true : false;
    x_m = abs(x_m);
   
    if (x_m > 0) {
      double _x_m = x_m / (double)speedDiv;
      x_m = _x_m + 0.5; //rounding
      
      if (x_m < 1) x_m = 1;
      if (x_m > m_max) x_m = m_max; //too high a value will overclock the Amiga counter
    }

    bool VcounterIsUp = (y_m >= 0) ? false : true;
    y_m = abs(y_m);

    if (y_m > 0) {
      double _y_m = y_m / (double)speedDiv;
      y_m = _y_m + 0.5; //rounding

      if (y_m < 1) y_m = 1;
      if (y_m > m_max) y_m = m_max; //too high a value will overclock the Amiga counter
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
