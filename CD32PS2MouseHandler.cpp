#include <Arduino.h>
#include "HardwareSerial.h"
#include "CD32PS2MouseHandler.h"

CD32PS2MouseHandler::CD32PS2MouseHandler(int clock_pin, int data_pin) {
  _mouse_timeout = false;
  _clock_pin = clock_pin;
  _data_pin = data_pin;
  _device_id = 0x0;
  _last_status = 0x0;
}

bool CD32PS2MouseHandler::mouse_timeout() {
  return _mouse_timeout;
}

int CD32PS2MouseHandler::clock_pin() {
  return _clock_pin;
}

int CD32PS2MouseHandler::data_pin() {
  return _data_pin;
}

int CD32PS2MouseHandler::status () {
  return _status;
}

int CD32PS2MouseHandler::x_movement () {
  return _x_movement;
}

int CD32PS2MouseHandler::y_movement () {
  return _y_movement;
}

int CD32PS2MouseHandler::z_movement () {
  return _z_movement;
}

bool CD32PS2MouseHandler::button(int button_num){
  return (bool)( _status & get_button_mask(button_num));
}

bool CD32PS2MouseHandler::clicked(int button_num){
  if (button(button_num) && !(bool)( _last_status & get_button_mask(button_num))) {
    return true;
  }
  else {
    return false;
  }
}

uint8_t CD32PS2MouseHandler::get_button_mask(int button){
  uint8_t mask = 0;
  switch(button){
    case 0: // left
      mask = 0x01;
      break;
    case 1: // middle
      mask = 0x04;
      break;
    case 2: // right
      mask = 0x02;
      break;
    default:
      mask = 0;
      break;
  }
  return mask;
}

int CD32PS2MouseHandler::initialise() {

  int counter = 0;
  int return_value = 0;
  // poll mouse to get a connection
  do {
    return_value = try_initialise();
    counter ++;
  } while ((return_value != 0) && (counter < 10));

  return return_value;
}

int CD32PS2MouseHandler::try_initialise() {
  pull_high(_clock_pin); // idle state
  pull_high(_data_pin);

  if (reset_mode() != _ACK_) return _NOACK_; //COMMUNICATION ERROR
  if (read_byte()  != _BAT_) return _NOACK_; //BAT ERROR
  read_byte(); //read dummy byte

  // set scroll wheel mode if available
  if (set_sample_rate(200) != _ACK_) return _NOACK_;
  if (set_sample_rate(100) != _ACK_) return _NOACK_;
  if (set_sample_rate(80)  != _ACK_) return _NOACK_; 
  _device_id = get_device_id(); // 0x00 = no scroll wheel

  if (set_resolution(0)    != _ACK_) return _NOACK_;
  if (set_scaling_1_1()    != _ACK_) return _NOACK_;
  if (set_sample_rate(200) != _ACK_) return _NOACK_; //10, 20, 40, 60, 80, 100, 200

  return _ACK_; // OK
}

uint8_t CD32PS2MouseHandler::reset_mode() {
  write(0xff);
  uint8_t ack = read_byte(); // Read Ack Byte
  return ack;
}

uint8_t CD32PS2MouseHandler::read_data() {
  write(0xeb);
  uint8_t ack = read_byte(); // Read Ack Byte
  return ack;
}

uint8_t CD32PS2MouseHandler:: get_device_id(){
  write(0xf2); // Ask mouse for device ID.
  read_byte(); // // Read Ack Byte
  uint8_t id =  read_byte(); // Read second byte - gives device id
  return id;
}

uint8_t CD32PS2MouseHandler::set_remote_mode() {
  write(0xf0); //set MOUSE_REMOTE
  uint8_t ack = read_byte(); // Read Ack Byte
  return ack;
}

uint8_t CD32PS2MouseHandler::set_stream_mode() {
  write(0xea); //set MOUSE_STREAM
  uint8_t ack = read_byte(); // Read Ack Byte
  return ack;
}

uint8_t CD32PS2MouseHandler::set_sample_rate(uint8_t rate) {
  write(0xf3); // Tell the mouse we are going to set the sample rate.
  uint8_t ack = read_byte(); // Read Ack Byte
  write(rate); // Send Set Sample Rate
  if (read_byte() == _ACK_ && ack == _ACK_) return ack; // Read ack byte
  else return 0;
}

uint8_t CD32PS2MouseHandler::set_scaling_2_1() {
  write(0xe7); // Set the scaling to 2:1
  uint8_t ack = read_byte(); // Read Ack Byte
  return ack;
}

uint8_t CD32PS2MouseHandler::set_scaling_1_1() {
  write(0xe6); // set the scaling to 1:1
  uint8_t ack = read_byte(); // Read Ack Byte
  return ack;
}

// This only effects data reporting in Stream mode.
uint8_t CD32PS2MouseHandler::enable_data_reporting() {
    write(0xf4); // Send enable data reporting
    uint8_t ack = read_byte(); // Read Ack Byte
    return ack;
}

// Disabling data reporting in Stream Mode will make it behave like Remote Mode
uint8_t CD32PS2MouseHandler::disable_data_reporting() {
    write(0xf5); // Send disable data reporting
    uint8_t ack = read_byte(); // Read Ack Byte
    return ack;
}

uint8_t CD32PS2MouseHandler::set_resolution(uint8_t resolution) {
  write(0xe8); // Send Set Resolution
  uint8_t ack = read_byte(); // Read ack Byte
  write(resolution); // Send resolution setting
  if (read_byte() == _ACK_ && ack == _ACK_) return ack; // Read ack byte
  else return 0;
}

void CD32PS2MouseHandler::write(int data) {
  char i;
  char parity = 1;
  unsigned long start_time = millis();
  pull_high(_data_pin);
  pull_high(_clock_pin);
  delayMicroseconds(300);
  pull_low(_clock_pin);
  delayMicroseconds(300);
  pull_low(_data_pin);
  delayMicroseconds(10);
  pull_high(_clock_pin); // Start Bit
  // wait for mouse to take control of clock
  while (digitalRead(_clock_pin)) {
    if (millis() - start_time >= 100) {
      // no connection to mouse
      pull_high(_data_pin); // back to waiting
      _mouse_timeout = true;
      return;
    }
  } 
  // clock is low, and we are clear to send data
  for (i=0; i < 8; i++) {
    if (data & 0x01) {
      pull_high(_data_pin);
    } else {
      pull_low(_data_pin);
    }
    // wait for clock cycle
    while (!digitalRead(_clock_pin)) {;}
    while (digitalRead(_clock_pin)) {;}
    parity = parity ^ (data & 0x01);
    data = data >> 1;
  }
  // parity
  if (parity) {
    pull_high(_data_pin);
  } else {
    pull_low(_data_pin);
  }
  // wait for clock cycle
  while (!digitalRead(_clock_pin)) {;}
  while (digitalRead(_clock_pin)) {;}
  pull_high(_data_pin); // release data line
  while (digitalRead(_data_pin)) {;} // wait for mouse to take over data line
  while (digitalRead(_clock_pin)) {;} // wait for mouse to take over clock
  while ((!digitalRead(_clock_pin)) && (!digitalRead(_data_pin))) {;} // wait for mouse to release clock and data
  _mouse_timeout = false;
}

void CD32PS2MouseHandler::hold_incoming_data() {
  pull_low(_clock_pin);
}

void CD32PS2MouseHandler::release_incoming_data() {
  pull_high(_clock_pin);
}

uint8_t CD32PS2MouseHandler::get_status() {
  write(0xe9);
  int ack = read_byte(); // Read Ack Byte
  uint8_t _statusRequested = read_byte();
  uint8_t _resolutionRequested = read_byte();
  uint8_t _rateRequested = read_byte();
  return _statusRequested;
}

uint8_t CD32PS2MouseHandler::get_resolution() {
  write(0xe9);
  int ack = read_byte(); // Read Ack Byte
  uint8_t _statusRequested = read_byte();
  uint8_t _resolutionRequested = read_byte();
  uint8_t _rateRequested = read_byte();
  return _resolutionRequested;
}

uint8_t CD32PS2MouseHandler::get_rate() {
  write(0xe9);
  int ack = read_byte(); // Read Ack Byte
  uint8_t _statusRequested = read_byte();
  uint8_t _resolutionRequested = read_byte();
  uint8_t _rateRequested = read_byte();
  return _rateRequested;
}

void CD32PS2MouseHandler::get_data(bool reporting_mode) {
  _last_status = _status; // save copy of status byte

  int ack = _NOACK_;

  if (reporting_mode) {
    ack = read_data(); // Send Read Data
  }
  else {
    ack = enable_data_reporting();
  }
  
  if (ack == _ACK_) {
    _status = read_byte(); // Status byte
    _x_movement = read_movement_xy(bitRead(_status, 4)); // X Movement Packet
    _y_movement = read_movement_xy(bitRead(_status, 5)); // Y Movement Packet

    if (_device_id > 0) {
      // read scroll wheel
      _z_movement = read_movement_z(); // Z Movement Packet
    }
    else {
      _z_movement = 0;
    }
  }
  else {
    ;
  }

  if (!reporting_mode) {
    disable_data_reporting();
  } 
}

int16_t CD32PS2MouseHandler::read_movement_xy(bool sign_bit) {
  // movement data is a 9 bit signed int using status bit and data reading
  int16_t value = read_byte();
  // use status bit to get sign of reading
  if (sign_bit) {
    // fill upper byte with 1's for negative number
    value |= 0xFF00;
  }

  return value;
}

int8_t CD32PS2MouseHandler::read_movement_z() {
  // z data can be mixed with extra button data so only the lower nibble is movement
  // 4 bit signed
  uint8_t value = read_byte(); // an 8 bit unsigned value
  // test bit 3 for sign
  if(bitRead(value, 3)) {
    // negative - set upper nibble to 1's
    value |= 0xF0;
  }
  else {
    // positive - set upper nibble to 0's
    value &= 0x0F;
  }
  return value;
}

uint8_t CD32PS2MouseHandler::read_byte() {
  uint8_t data = 0;
  unsigned long start_time = millis();
  pull_high(_clock_pin);
  pull_high(_data_pin);
  //delayMicroseconds(50);

  // read start bit but check for timeout
  while (digitalRead(_clock_pin)) {
    if (millis() - start_time > 600) {
      // timeout
      _mouse_timeout = true;
      return 0;
    }
  }

  while (!digitalRead(_clock_pin)) {;}
  // read data bits
  for (int i = 0; i < 8; i++) {
    bitWrite(data, i, read_bit());
  }
  read_bit(); // Partiy Bit
  read_bit(); // Stop bit should be 1
  _mouse_timeout = false;
  return data;
}

int CD32PS2MouseHandler::read_bit() {
  while (digitalRead(_clock_pin)) {;}
  int bit = digitalRead(_data_pin);
  while (!digitalRead(_clock_pin)) {;}
  return bit;
}

void CD32PS2MouseHandler::pull_low(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void CD32PS2MouseHandler::pull_high(int pin) {
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
}
