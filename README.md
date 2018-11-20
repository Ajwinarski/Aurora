# Arduino RFID Door Lock V2
Arduino Uno R3 RFID and Keypad Door Lock Code and Setup

## Parts List

## Overall wiring (use http://fritzing.org/home/)

## ID-20LA Guide [Link](https://www.sparkfun.com/products/11828)
![RFID Reader ID-20LA](https://cdn.sparkfun.com//assets/parts/8/1/8/8/11828-01.jpg)

### Wiring
| ID-20LA | -> | Arduino Uno R3 |
| :---    | :---: | :---|
| Pin 1  (GND)  | -> | Ground (GND) |
| Pin 2  (RES)  | -> | Digital Pin 7 (D7)  |
| Pin 7  (FORM) | -> | Ground (GND)  |
| Pin 9  (D0)   | -> | Digital Pin 0 (D0)  |
| Pin 11 (VCC)  | -> | 3.3 Volts (3.3V)  |

### Code
```C++
void checkReader();          // Checks the reader for tags
void checkTag(char tag[]);   // Checks given tag to the other predfined valid tags
void resetReader();          // Resets the reader to get ready for another read
```

## Qwiic Keypad [Link](https://www.sparkfun.com/products/14836)
![Qwiic Keypad](https://cdn.sparkfun.com//assets/parts/1/3/1/0/6/14836-Qwiic_Keypad-01.jpg)

### Wiring

### Code
```C++
void checkKeypad();          // Reads the input from the keypad
void checkCode(String code); // Checks to see if the 4 digit code entered is correct
void dailyCode();            // Generates the 4 digit code based on the current date
```

## NeoPixel LED Ring [Link](https://www.adafruit.com/product/1463)
![NeoPixel Ring - 16 x 5050 RGB LED with Integrated Drivers](https://cdn-shop.adafruit.com/1200x900/1463-03.jpg)

### Wiring

### Code
```C++
void rainbowCycle();         // Updates a single pixel every call to simulate a spinning rainbow
void keypadFeedback();       // Lights up the LEDring to give feedback on how many digits you have entered
void flash(uint32_t c);      // Takes a color and flashes the LEDring with it 3 times
void allSet(uint32_t c);     // Takes a color and sets all the LEDring pixels to the color
```

## Servo Motor [Link](https://www.thingbits.net/products/servo-motor-standard-size-sg5010)
![Servo Motor - Standard Size - SG5010](https://d2drzakx2pq6fl.cloudfront.net/production/products/399/large/servo-motor-sg5010.jpg?1451678104)

### Wiring

### Code
```C++
void lock();                 // Sets the lock position to locked
void unlock();               // Sets the lock position to unlocked
void changeLock();           // Changes the current lock position
```
