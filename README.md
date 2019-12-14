# matrixdisplay
## Parts:
- 64x32 LED Matrix (i got this one of [amazon](https://www.amazon.de/gp/product/B06XDQYV7V))
- ESP8266
- ESP8266 Development Board (makes it easier to use one power supply for the board and the led matrix)
- Power Supply (mine provided 12V - 4A)
- Jumper Cables (i only needed female-to-female ones)

## Software:
- [PxMatrix](https://github.com/2dom/PxMatrix)

## Cabling:
LED Matrix Model: P4-256*128-2121-A2

Pin Layout:

|Left|Right|
|-|-|
|R1|G1|
|B1|GND|
|R2|G2|
|B2|GND|
|A|B|
|C|D|
|CLK|LAT|
|P_OE|GND|


PI --> PO

|PI|PO|
|-|-|
|R2|R1|
|G1|R2|
|G2|G1|
|B1|G2|
|B2|B1|


PI --> ESP8266

|PI|ESP8266|
|-|-|
|LAT|D0|
|A|D1|
|B|D2|
|C|D8|
|D|D6|
|P_OE|D4|
|CLK|D5|
|R1|D7|
