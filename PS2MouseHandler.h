#ifndef PS2MouseHandler_h

#define PS2MouseHandler_h

class PS2MouseHandler
{
  private:
    int _clock_pin;
    int _data_pin;
    bool _no_mouse;

    uint8_t _last_status;
    uint8_t _status;
    int16_t _x_movement;
    int16_t _y_movement;
    int8_t _z_movement;

    uint8_t _device_id;
    uint8_t read_byte();
    int read_bit();
    int16_t read_movement_9(bool);
    int8_t read_movement_z();
    uint8_t get_button_mask(int);
    void pull_high(int);
    void pull_low(int);
    void write(int);
    int try_initialise();

  public:
    PS2MouseHandler(int, int);//, int mode = PS2_MOUSE_STREAM);
    int initialise();
    int clock_pin();
    int data_pin();
    int get_device_id();
    int device_id();
    int status();
    int x_movement();
    int y_movement();
    int z_movement();
    bool button(int);
    bool clicked(int);
    void get_data();
    int enable_data_reporting();
    int disable_data_reporting();
    void hold_incoming_data();
    void release_incoming_data();
    int set_remote_mode();
    int set_stream_mode();
    int set_resolution(int);
    int set_scaling_2_1();
    int set_scaling_1_1();
    int set_sample_rate(int);
};

#endif
