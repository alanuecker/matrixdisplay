# matrixdisplay

## Parts:

- 64x32 LED Matrix (i got this one of [amazon](https://www.amazon.de/gp/product/B06XDQYV7V))
- ESP8266 (V3 LoLin, NodeMCU v0.9)
- ESP8266 Development Board (makes it easier to use one power supply for the board and the led matrix)
- Power Supply (mine provided 12V - 4A)
- Jumper Cables (female-to-female)

## Cabling:

LED Matrix Model: `P4-256*128-2121-A2`

### Pin Layout:

| Left | Right |
| ---- | ----- |
| R1   | G1    |
| B1   | GND   |
| R2   | G2    |
| B2   | GND   |
| A    | B     |
| C    | D     |
| CLK  | LAT   |
| P_OE | GND   |

### PI --> PO

| PI  | PO  |
| --- | --- |
| R2  | R1  |
| G1  | R2  |
| G2  | G1  |
| B1  | G2  |
| B2  | B1  |

### PI --> ESP8266

| PI   | ESP8266 |
| ---- | ------- |
| LAT  | D0      |
| A    | D1      |
| B    | D2      |
| C    | D8      |
| D    | D6      |
| P_OE | D4      |
| CLK  | D5      |
| R1   | D7      |

## Software:

### Setup Arduino
1. rename `secrets.cpp.template` to `secrets.cpp`
2. enter your wifi information
3. setup all relevant `Arduino Libraries`
4. upload files to `ESP8266`
5. upload the website via the `Arduino IDE`

### Setup Website Development
1. run `yarn`
2. run `yarn watch` for local development
3. run `yarn build` to minify and gzip the html file
4. upload the website via the `Arduino IDE`

### Libraries

#### Arduino Library Manager

- [PxMatrix](https://github.com/2dom/PxMatrix) (by Dominic Buchstaller)
- ArduinoJson (by Benoit Blanchon)
- Adafruit GFX Library (by Adafruit)
- NTPClient (by Fabrice Weinberg)

#### External

- WebSocketsServer (by [arduinoWebSockets](https://github.com/Links2004/arduinoWebSockets))
  - install manually to `<home_dir>/Documents/Arduino/libraries/`

### File System

- setup `ESP8266 Sketch Data Upload` [docs](https://arduino-esp8266.readthedocs.io/en/latest/filesystem.html#uploading-files-to-file-system)

  - extract [ESP8266FS](https://github.com/esp8266/arduino-esp8266fs-plugin/releases/download/0.5.0/ESP8266FS-0.5.0.zip) to `<home_dir>/Documents/Arduino/tools/`
  - reload `Arduino IDE`
  - create a `data` folder
  - select `Tools > ESP8266 Sketch Data Upload`
