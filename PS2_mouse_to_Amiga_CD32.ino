//PS/2 to Amiga CD32 mouse translator v 4.0
//Tested on:
//Perixx PERIMICE-201 PS/2
//Logitech M-SBF96 PS/2
//Genius DX-110 PS/2
//NEW3P PS/2
//Seven levels of mouse speed
//DPI stored in EEPROM memory
//DPI change: Middle mouse button
#include <avr/wdt.h>
#include <EEPROM.h>
#include "CD32PS2MouseHandler.h"

//#define GENIUS
//#define MOUSEDEBUGGER

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

#define DPIMax 3
#define SkipMax 3

const char* firmwareRevision    = "4.1";
volatile uint16_t pinStateDelay = 3;   //2 us, half the length of one pulse
volatile int16_t  m_max         = 10;  //10 maximum number of pulses per cycle
bool speedState                 = 0;
byte xySkip                     = 0;
byte loopCounter                = 0;
byte speedDPI                   = 0;
volatile int16_t x_m            = 0;
volatile int16_t y_m            = 0;
volatile int16_t z_m            = 0;
bool reporting_mode_read_data   = true;

CD32PS2MouseHandler mouse(MOUSE_CLOCK, MOUSE_DATA);

void setup() {
  #if defined(MOUSEDEBUGGER)
    Serial.begin(2000000);
  #endif

  speedDPI = EEPROM.read(0);
  if (speedDPI > DPIMax) speedDPI = 0;
  xySkip = EEPROM.read(1);
  if (xySkip > SkipMax) xySkip = 0;
  pinMode(Vpin,    OUTPUT);
  pinMode(VQpin,   OUTPUT);
  pinMode(Hpin,    OUTPUT);
  pinMode(HQpin,   OUTPUT);
  pinMode(ButtonL, OUTPUT);
  pinMode(ButtonR, OUTPUT);
  pinMode(ButtonM, OUTPUT);
  digitalWrite(ButtonL, HIGH);
  digitalWrite(ButtonR, HIGH);
  digitalWrite(ButtonM, HIGH);
  digitalWrite(LED,     HIGH);

  int _mouse_Init = mouse.initialise();

  if (_mouse_Init != 0xFA) {

    #if defined(MOUSEDEBUGGER)
      Serial.print("MOUSE_ERROR:");
      Serial.print(_mouse_Init, HEX);
      Serial.print(";");
      Serial.println(mouse.mouse_timeout());
      Serial.flush();
    #endif

    digitalWrite(LED, LOW);
    wdt_reset();
    wdt_enable(WDTO_15MS);
    while (1) {;}
  }
  else {
    mouse.set_resolution(speedDPI);

    #if defined(MOUSEDEBUGGER)
      Serial.print("TYPE:");
      Serial.println(mouse.get_device_id());
      Serial.print("STATUS:");
      Serial.println(mouse.get_status());
      Serial.print("RESOLUTION:");
      Serial.println(mouse.get_resolution());
      Serial.print("RATE:");
      Serial.println(mouse.get_rate());
      Serial.print("LOOP_SKIP:");
      Serial.println(xySkip);
      Serial.print("FIRMWARE:");
      Serial.println(firmwareRevision);
      Serial.flush();
    #endif
  }

  digitalWrite(LED, LOW);

  #if defined(GENIUS)
    reporting_mode_read_data = false;
  #endif
}

void loop() {
  mouse.get_device_id(); //reset mouse counters 
  
  if (mouse.mouse_timeout()) { //check mouse
    wdt_reset();
    wdt_enable(WDTO_15MS);
    while (1) {;}
  }  
  mouse.get_data(reporting_mode_read_data);

  digitalWrite(ButtonL, !mouse.button(0));
  digitalWrite(ButtonR, !mouse.button(2));
  digitalWrite(ButtonM, !mouse.button(1));

  if (loopCounter > xySkip) {
    x_m = mouse.x_movement();
    y_m = mouse.y_movement();
    loopCounter = 0;
  }
  else {
    x_m = 0;
    y_m = 0;
  }

  z_m = mouse.z_movement();

  if (!speedState && mouse.button(1)) {
    if (speedDPI == 0 && xySkip > 0) {
      xySkip -= 1;
    }
    else {
      if (speedDPI < DPIMax) speedDPI += 1;
      else {
        speedDPI = 0;
        xySkip = SkipMax;
      }
    }
    EEPROM.write(0, speedDPI);
    delay(10);
    EEPROM.write(1, xySkip);
    delay(10);
    mouse.set_resolution(speedDPI);

    #if defined(MOUSEDEBUGGER)
      Serial.print("RESOLUTION:");
      Serial.println(mouse.get_resolution());
      Serial.print("LOOP_SKIP:");
      Serial.println(xySkip);
      Serial.flush();
    #endif
  }
  speedState = mouse.button(1); //before changing the speed again, you must release the middle mouse button

  #if defined(MOUSEDEBUGGER)
    if (x_m != 0 || y_m != 0 || z_m != 0) {
      Serial.print(x_m);
      Serial.print("*");
      Serial.print(y_m);
      Serial.print("*");
      Serial.println(z_m);
      Serial.flush();
    }
    else {
      Serial.print(mouse.get_status());
      Serial.print("*");
      Serial.print(mouse.get_resolution());
      Serial.print("*");
      Serial.println(mouse.get_rate());
      Serial.flush();
    }
  #endif
//  if (z_m == 0) {
    if (x_m != 0 || y_m != 0) {
      bool HcounterIsUp = (x_m >= 0) ? true : false;
      x_m = abs(x_m);
    
      if (x_m > m_max) x_m = m_max;

      bool VcounterIsUp = (y_m >= 0) ? false : true;
      y_m = abs(y_m);

      if (y_m > m_max) y_m = m_max;

      pulseGenerator(HcounterIsUp, VcounterIsUp, pinStateDelay, x_m, y_m);
    }
//  }
//  else {
//    digitalWrite(ButtonL, 0);
//    bool VcounterIsUp = (z_m >= 0) ? false : true;
//    pulseGenerator(false, VcounterIsUp, pinStateDelay, 0, 3);
// }
loopCounter++;
}

void pulseGenerator(bool HcounterIsUp, bool VcounterIsUp ,uint16_t pinStateDelay, uint8_t HPulsesPerStep, uint8_t VPulsesPerStep) {

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
}
