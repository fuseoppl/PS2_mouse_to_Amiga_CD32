# PS/2 mouse to Amiga CD32

Seven levels of mouse speed:

* DPI stored in EEPROM memory.
* DPI change: hold down the middle mouse button for at least 2 seconds

Hardware:

* Tested on:
* Perixx PERIMICE-201 PS/2
* Logitech M-SBF96 PS/2
* Genius DX-110 PS/2
* NEW3P PS/2

DFRobot Beetle Board <-> Amiga DB9
(#define ATMEGA32 //uncomment for ATmega32)

* '+' <-  pin 7 \& pin 4 (Mouse mini DIN plug)
* '-' <-> pin 8 \& pin 3 (Mouse mini DIN plug)
* &#x20;A0  ->  pin 1
* &#x20;A1  ->  pin 3
* &#x20;D9  ->  pin 2
* D10  ->  pin 4
* &#x20;A2  ->  pin 6
* SDA  ->  pin 9
* &#x20;RX  ->  pin 5
* D11  ->  pin 1 (Mouse mini DIN plug)
* SCL  ->  pin 5 (Mouse mini DIN plug)

mini ultra, china Arduino UNO nano clone https://pl.aliexpress.com/item/1005007492500542.html
To avoid problems with the watchdog, it is best to flash a new bootloader or disable the watchdog.

(//#define ATMEGA32 //comment for ATmega328P)

* &#x20;5V <-  pin 7 \& pin 4 (Mouse mini DIN plug)
* gnd <-> pin 8 \& pin 3 (Mouse mini DIN plug)
* &#x20;D3  -> pin 1
* &#x20;D5  -> pin 3
* &#x20;D4  -> pin 2
* &#x20;D6  -> pin 4
* &#x20;D7  -> pin 6
* &#x20;D8  -> pin 9
* &#x20;D9  -> pin 5
* D12  -> pin 1 (Mouse mini DIN plug)
* D11  -> pin 5 (Mouse mini DIN plug)

