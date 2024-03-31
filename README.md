# PS2 mouse to Amiga CD32

PS2 to Amiga mouse translator tested on:
- Amiga CD32
- optical mouse Logitech M-SBF96 PS2
- DFRobot Beetle Board compatible with Arduino Leonardo

The project used a modified library: https://github.com/getis/Arduino-PS2-Mouse-Handler

PS2MouseHandler library is modified, so do not use the original one!
Keep the library files directly in the sketch directory.

DFRobot Beetle Board <-> Amiga DB9  
- '+' <-  pin 7
- '-' <-> pin 8
- A0  ->  pin 1
- A1  ->  pin 3
- D9  ->  pin 2
- D10 ->  pin 4
- A2  ->  pin 6
- SDA ->  pin 9
- RX  ->  pin 5
