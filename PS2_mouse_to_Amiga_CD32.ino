//PS/2 to Amiga CD32 mouse translator v5.0
//Tested on:
//Perixx PERIMICE-201 PS/2
//Logitech M-SBF96 PS/2
//Genius DX-110 PS/2
//NEW3P PS/2
//seven levels of mouse speed
//DPI stored in EEPROM memory
//DPI change: Middle mouse button
#include <avr/wdt.h>
#include <EEPROM.h>
#include "CD32PS2MouseHandler.h"

//#define GENIUS
//#define MOUSEDEBUGGER
//#define COUNTERDEBUGGER
#define INITDEBUGGER
//#define ATMEGA32 //uncomment for ATmega32, comment for ATmega328P

//=======================================//
//   DFRobot Beetle Board
//     compatible with
//    Arduino Leonardo  <-> Amiga DB9
                          // +  <-  pin 7 & pin 4 (Mouse mini DIN plug)
                          // -  <-> pin 8 & pin 3 (Mouse mini DIN plug)
#if defined(ATMEGA32)
  #define Vpin         A0 // A0  -> pin 1
  #define VQpin        A1 // A1  -> pin 3
  #define Hpin          9 // D9  -> pin 2
  #define HQpin        10 //D10  -> pin 4
  #define ButtonL       2 //SDA  -> pin 6
  #define ButtonR      A2 // A2  -> pin 9
  #define ButtonM       0 // RX  -> pin 5
  //-------------------------------------------------------------------------//
  #define MOUSE_DATA   11 //D11  -> pin 1 (Mouse mini DIN plug)
  #define MOUSE_CLOCK   3 //SCL  -> pin 5 (Mouse mini DIN plug)
  //-------------------------------------------------------------------------//
  #define LED          13 //internal LED
#else
//mini ultra, china Arduino UNO nano clone https://pl.aliexpress.com/item/1005007492500542.html
//select Tools->Processor->ATmega328P (Old Bootloader)
//To avoid problems with the watchdog, it is best to flash a new bootloader or disable the watchdog.
                          // 5V <-  pin 7 & pin 4 (Mouse mini DIN plug)
                          //gnd <-> pin 8 & pin 3 (Mouse mini DIN plug)
  #define Vpin          3 // D3  -> pin 1
  #define VQpin         5 // D5  -> pin 3
  #define Hpin          4 // D4  -> pin 2
  #define HQpin         6 // D6  -> pin 4
  #define ButtonL       7 // D7  -> pin 6
  #define ButtonR       8 // D8  -> pin 9
  #define ButtonM       9 // D9  -> pin 5
  //-------------------------------------------------------------------------//
  #define MOUSE_CLOCK  11 // D11 -> pin 5 (Mouse mini DIN plug)
  #define MOUSE_DATA   12 // D12 -> pin 1 (Mouse mini DIN plug)
  //-------------------------------------------------------------------------//
  #define LED          13 // internal D2 LED
#endif
//=======================================//

#define DPIMax  3
#define xyDividerMax 4
#define xyDividerMin 1
#define mMax 20

const char* firmwareRevision    = "5.0";
volatile uint16_t pinStateDelay = 3;   // 3 us, half the length of one pulse
volatile int16_t  m_max         = mMax;  // 10 maximum number of pulses per cycle
bool speedState                 = 0;
byte xyDivider                     = xyDividerMin;
byte speedDPI                   = 0;
volatile int16_t x_m            = 0;
volatile int16_t y_m            = 0;
volatile int16_t z_m            = 0;
bool reporting_mode_read_data   = true;
bool loopSkip                   = false;

unsigned long _millis           = 0;

CD32PS2MouseHandler mouse(MOUSE_CLOCK, MOUSE_DATA);

void setup() {
  #if defined(MOUSEDEBUGGER) || defined(COUNTERDEBUGGER) || defined(INITDEBUGGER)
    Serial.begin(2000000);
  #endif

  speedDPI = EEPROM.read(0);
  if (speedDPI > DPIMax) {
    speedDPI = 0;
    EEPROM.write(0, speedDPI);
    delay(10);
  }

  xyDivider = EEPROM.read(1);
  if (xyDivider > xyDividerMax) {
    xyDivider = xyDividerMax;
    EEPROM.write(1, xyDivider);
    delay(10);
  }

  if (xyDivider < xyDividerMin) {
    xyDivider = xyDividerMin;
    EEPROM.write(1, xyDivider);
    delay(10);
  }

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
    #if defined(INITDEBUGGER)
      Serial.print("MOUSE_ERROR:");
      Serial.print(_mouse_Init, HEX);
      Serial.print(";");
      Serial.println(mouse.mouse_timeout());
      Serial.flush();
    #endif

    digitalWrite(LED, LOW);
    wdt_enable(WDTO_15MS);
    while (1) {delayMicroseconds(10);} 
  }
  
  mouse.set_resolution(speedDPI);

  #if defined(INITDEBUGGER)
    Serial.print("FIRMWARE:");
    Serial.println(firmwareRevision);
    Serial.print("TYPE:");
    Serial.println(mouse.get_device_id());
    Serial.print("STATUS:");
    Serial.println(mouse.get_status());
    Serial.print("RATE:");
    Serial.println(mouse.get_rate());
    Serial.print("RESOLUTION:");
    Serial.println(mouse.get_resolution());
    Serial.print("XY_DIVIDER:");
    Serial.println(xyDivider);
    Serial.flush();
  #endif

  digitalWrite(LED, LOW);

  #if defined(GENIUS)
    reporting_mode_read_data = false;
  #endif
}

void loop() {
  mouse.get_device_id(); // reset mouse counters
  
  if (mouse.mouse_timeout()) { // check mouse
    wdt_enable(WDTO_15MS);
    while (1) {delayMicroseconds(10);}
  }

  mouse.get_data(reporting_mode_read_data);

  digitalWrite(ButtonL, !mouse.button(0));
  digitalWrite(ButtonR, !mouse.button(2));
  digitalWrite(ButtonM, !mouse.button(1));

  if (xyDivider < 4) loopSkip = false;

  if (!loopSkip) {
    x_m = mouse.x_movement();
    y_m = mouse.y_movement();
  }
  else {
    x_m = 0;
    y_m = 0;
  }

  loopSkip = !loopSkip;

  z_m = mouse.z_movement();

  if (!mouse.button(1)) {
    _millis = millis();
    speedState = false;
  }

  if (mouse.button(1) && !speedState && millis() - _millis > 1000) {
    speedState = true;
    if (speedDPI == 0 && xyDivider > 1) {
      xyDivider -= 1;
    }
    else {
      if (speedDPI < DPIMax) speedDPI += 1;
      else {
        speedDPI = 0;
        xyDivider = xyDividerMax;
      }
    }
    EEPROM.write(0, speedDPI);
    delay(10);
    EEPROM.write(1, xyDivider);
    delay(10);
    mouse.set_resolution(speedDPI);

    #if defined(INITDEBUGGER)
      Serial.print("RATE:");
      Serial.println(mouse.get_rate());
      Serial.print("RESOLUTION:");
      Serial.println(mouse.get_resolution());
      Serial.print("XY_DIVIDER:");
      Serial.println(xyDivider);
      Serial.flush();
    #endif
  }

  #if defined(MOUSEDEBUGGER)
    if (x_m != 0 || y_m != 0 || z_m != 0) {
      Serial.print(x_m);
      Serial.print("*");
      Serial.print(y_m);
      Serial.print("*");
      Serial.print(z_m);
      Serial.print("*");
  #if defined(COUNTERDEBUGGER)
      Serial.print(mouse.get_status());
      Serial.print("*");
  #else
      Serial.println(mouse.get_status());
      //Serial.print("*");
  #endif
      Serial.flush();
    }
  #endif
//  if (z_m == 0) {
    if (x_m != 0 || y_m != 0) {

      bool HcounterIsUp = (x_m >= 0) ? true : false;
      x_m = abs(x_m);

      if (x_m > 0) {
        x_m = (double)x_m / (double)xyDivider + 0.5;
        if (x_m == 0) x_m = 1;
        else if (x_m > m_max) x_m = m_max;
      }

      bool VcounterIsUp = (y_m >= 0) ? false : true;
      y_m = abs(y_m);

      if (y_m > 0) {
        y_m = (double)y_m / (double)xyDivider + 0.5;
        if (y_m == 0) y_m = 1;
        else if (y_m > m_max) y_m = m_max;
      }

      pulseGenerator(HcounterIsUp, VcounterIsUp, pinStateDelay, x_m, y_m);
    }
//  }
//  else {
//    digitalWrite(ButtonL, 0);
//    bool VcounterIsUp = (z_m >= 0) ? false : true;
//    pulseGenerator(false, VcounterIsUp, pinStateDelay, 0, 3);
// }
}

void pulseGenerator(bool HcounterIsUp, bool VcounterIsUp ,uint16_t _pinStateDelay, uint8_t HPulsesPerStep, uint8_t VPulsesPerStep) {

  uint8_t _maxPulsesPerStep = (HPulsesPerStep > VPulsesPerStep) ? HPulsesPerStep : VPulsesPerStep; 

  _pinStateDelay = _pinStateDelay * ((1.0 / (double)_maxPulsesPerStep) * (double)mMax);

  #if defined(COUNTERDEBUGGER)
    Serial.print(HPulsesPerStep);
    Serial.print("*");
    Serial.print(VPulsesPerStep);
    Serial.print("*");
    Serial.println(_pinStateDelay);
  #endif

  for (int pulses = 0; _maxPulsesPerStep > pulses ; pulses++) {    

    delayMicroseconds(_pinStateDelay);

    if (HPulsesPerStep > pulses) {
      if (HcounterIsUp) digitalWrite(Hpin,  !digitalRead(Hpin));
      else              digitalWrite(HQpin, !digitalRead(HQpin));
    }

    if (VPulsesPerStep > pulses) {
      if (VcounterIsUp) digitalWrite(Vpin,  !digitalRead(Vpin));
      else              digitalWrite(VQpin, !digitalRead(VQpin));
    }

    delayMicroseconds(_pinStateDelay);

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
