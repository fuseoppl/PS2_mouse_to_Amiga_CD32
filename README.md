# PS/2 mouse to Amiga CD32

Five levels of mouse speed:
* Speed stored in EEPROM memory.
* Increase speed: Middle mouse button
* Decrease speed: Right mouse button + Middle mouse button

PS/2 to Amiga mouse translator tested on:
* Amiga CD32
* optical mouse Genius M/N:DX-110 GM-150014 PS/2
* DFRobot Beetle Board SKU: DFR0282, compatible with Arduino Leonardo

The project used a modified library: https://github.com/getis/Arduino-PS2-Mouse-Handler
* PS2MouseHandler library is modified, so do not use the original one!
* Keep the modified library files directly in the sketch directory.

DFRobot Beetle Board <-> Amiga DB9  
* '+' <-  pin 7 & pin 4 (Mouse mini DIN plug)
* '-' <-> pin 8 & pin 3 (Mouse mini DIN plug)
* A0  ->  pin 1
* A1  ->  pin 3
* D9  ->  pin 2
* D10 ->  pin 4
* A2  ->  pin 6
* SDA ->  pin 9
* RX  ->  pin 5
* D11 ->  pin 1 (Mouse mini DIN plug)
* SCL ->  pin 5 (Mouse mini DIN plug)
