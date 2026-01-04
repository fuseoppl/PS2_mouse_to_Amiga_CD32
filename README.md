# PS2 mouse to Amiga CD32

Nine speed levels, speed stored in EEPROM memory
Increase speed: Middle mouse button
Decrease speed: Right mouse button + Middle mouse button

PS2 to Amiga mouse translator tested on:
- Amiga CD32
- optical mouse Logitech M-SBF96 PS2
- DFRobot Beetle Board compatible with Arduino Leonardo

The project used a modified library: https://github.com/getis/Arduino-PS2-Mouse-Handler
PS2MouseHandler library is modified, so do not use the original one!
Keep the modified library files directly in the sketch directory.

DFRobot Beetle Board <-> Amiga DB9  
'+' <-  pin 7 & pin 4 (Mouse mini DIN plug)
'-' <-> pin 8 & pin 3 (Mouse mini DIN plug)
A0  ->  pin 1
A1  ->  pin 3
D9  ->  pin 2
D10 ->  pin 4
A2  ->  pin 6
SDA ->  pin 9
RX  ->  pin 5
D11 ->  pin 1 (Mouse mini DIN plug)
SCL ->  pin 5 (Mouse mini DIN plug)
