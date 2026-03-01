#ifndef PS2MouseHandler_h

#define PS2MouseHandler_h

#define _ACK_ 0xFA
#define _NOACK_ 0xFC
#define _BAT_ 0xAA

class PS2MouseHandler
{
  private:
    bool _mouse_timeout;
    int _clock_pin;
    int _data_pin;
    uint8_t _last_status;
    uint8_t _status;
    int16_t _x_movement;
    int16_t _y_movement;
    int8_t _z_movement;
    uint8_t _device_id;
    uint8_t read_data();
    uint8_t read_byte();
    int read_bit();
    int16_t read_movement_xy(bool);
    int8_t read_movement_z();
    uint8_t get_button_mask(int);
    void pull_high(int);
    void pull_low(int);
    void write(int);
    int try_initialise();

  public:
    PS2MouseHandler(int, int);//, int mode = PS2_MOUSE_STREAM);
    int initialise();
    bool mouse_timeout();
    int clock_pin();
    int data_pin();
    uint8_t reset_mode();
    uint8_t get_device_id();
    uint8_t get_status();
    uint8_t get_resolution();
    uint8_t get_rate();
    //int device_id();
    int status();
    int x_movement();
    int y_movement();
    int z_movement();
    bool button(int);
    bool clicked(int);
    void get_data(bool);
    uint8_t enable_data_reporting();
    uint8_t disable_data_reporting();
    void hold_incoming_data();
    void release_incoming_data();
    uint8_t set_remote_mode();
    uint8_t set_stream_mode();
    uint8_t set_resolution(uint8_t);
    uint8_t set_scaling_2_1();
    uint8_t set_scaling_1_1();
    uint8_t set_sample_rate(uint8_t);
};

#endif
