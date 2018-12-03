# Aurora
A Radio Frequency Identification (RFID) and Keypad Door Lock for the Arduino Uno R3
[Template](https://gist.github.com/fvcproductions/1bfc2d4aecb01a834b46)

## Table of Contents
- [Installation](#installation)
- [Wiring Diagram](#wiring-diagram)
- [Software](#software)
- [Hardware](#hardware)
  - [Parts List](#parts-list)
  - [RFID Reader](#rfid-reader)
    - [Wiring](#wiring)
    - [Code](#code)
  - [Qwiic Keypad](#qwiic-keypad)
    - [Wiring](#wiring-1)
    - [Code](#code-1)
  - [NeoPixel LED Ring](#neopixel-led-ring)
    - [Wiring](#wiring-2)
    - [Code](#code-2)
  - [Servo Motor](#servo-motor)
    - [Wiring](#wiring-3)
    - [Code](#code-3)

## Installation
#### 1. Clone or download the zip folder
#### 2. Open Aurora-master.ino in the Arduino IDE
#### 3. After connecting your Arduino device, choose it through *Tools->Board->...* and then selecting the proper COM Port for your device through *Tools->Port->...*
#### 4. Click on verify to make sure that you have the necessary libraries included and that there are no compilation errors (If any libraries are missing, you can search for them through *Sketch->Include Library->Manage Libraries...*)
#### 5. Before you click upload, make sure that the Digital Pin 0 (D0) is disconnected from the ID-20LA RFID Reader either from a switch or by pulling the pin out from the Arduino
#### 6. Upload to your board and upon completion, make sure to reconnect the D0 pin to the ID-20LA RFID Reader

## Wiring Diagram
(use http://fritzing.org/home/)

## Software

## Hardware

### Parts List
| Product | Model # | Price ($) | Links |
|  :---   |  :---   | :--- |  :---  |
|  Arduino Uno R3   |  A000066   | 22.00 | [Link](https://store.arduino.cc/usa/arduino-uno-rev3) |
|  RFID Reader   |  ID-20LA   | 34.95 | [Link](https://www.sparkfun.com/products/11828) |
|  RFID Reader Breakout   |  SEN-13030   | 1.95 | [Link](https://www.sparkfun.com/products/13030) |
|  Qwiic Keypad   |  SPX-14836  | 9.95 | [Link](https://www.sparkfun.com/products/14836) |
|  NeoPixel LED Ring   |  -----   | 9.95 | [Link](https://www.adafruit.com/product/1463) |
|  Servo Motor   |  SG5010  | 7.99 | [Link](https://www.thingbits.net/products/servo-motor-standard-size-sg5010) |
|  Magnetic Contact Switch  |  -----  | 3.95 | [Link](https://www.adafruit.com/product/375) |
|  Piezo Buzzer   |  -----  | 0.95 | [Link](https://www.adafruit.com/product/1536) |

### RFID Reader
125 KHz reader used to scan tags in order to change the lock position on the door

<img src="https://cdn.sparkfun.com//assets/parts/8/1/8/8/11828-01.jpg" height="350" width="350">

#### Wiring
| ID-20LA | **-->** | Arduino Uno R3 |
|  :---  | :---: | :--- |
| Pin 1  (GND)  | **-->** | Ground (GND) |
| Pin 2  (RES)  | **-->** | Digital Pin 7 (D7)  |
| Pin 7  (FORM) | **-->** | Ground (GND)  |
| Pin 9  (D0)   | **-->** | Digital Pin 0 (D0)  |
| Pin 11 (VCC)  | **-->** | 3.3 Volts (3.3V)  |

#### Code
```C++
void checkReader();          // Checks the reader for tags
void checkTag(char tag[]);   // Checks given tag to the other predfined valid tags
void resetReader();          // Resets the reader to get ready for another read
```

## Qwiic Keypad
Keypad used to enter a 4-digit code in order to change the lock position on the door

<img src="https://cdn.sparkfun.com//assets/parts/1/3/1/0/6/14836-Qwiic_Keypad-01.jpg" height="350" width="350">

### Wiring
| Qwiic Keypad | **-->** | Arduino Uno R3 |
|  :---  | :---: | :--- |
| Ground  (GND)  | **-->** | Ground (GND) |
| 3.3 Volts  (3.3V)  | **-->** |  3.3 Volts (3.3V)  |
| SDA  (SDA) | **-->** | Analog In 4 (A4)  |
| SCL (SCL)   | **-->** | Analog In 5 (A5)  |

### Code
```C++
void checkKeypad();          // Reads the input from the keypad
void checkCode(String code); // Checks to see if the 4 digit code entered is correct
void dailyCode();            // Generates the 4 digit code based on the current date
```

## NeoPixel LED Ring
Gives the user feedback on things like lock countdown, number of pins digits entered, and more

<img src="https://cdn-shop.adafruit.com/1200x900/1463-03.jpg" height="320" width="400">

### Wiring
| NeoPixel LED Ring | **-->** | Arduino Uno R3 |
|  :---  | :---: | :--- |
| Ground  (GND)  | **-->** | Ground (GND) |
| V+  (V+)  | **-->** |  5 Volts (5V)  |
| IN (IN)  | **-->** | Digital Pin 6 (D6)  |

### Code
```C++
void rainbowCycle();         // Updates a single pixel every call to simulate a spinning rainbow
void keypadFeedback();       // Lights up the LEDring to give feedback on how many digits you have entered
void flash(uint32_t c);      // Takes a color and flashes the LEDring with it 3 times
void allSet(uint32_t c);     // Takes a color and sets all the LEDring pixels to the color
```

## Servo Motor
Used to rotate the lock on the door to adjust the lock state (locked/unlock)

<img src="https://futabausa.com/wp-content/uploads/2018/06/mxs-601017.jpg" height="300" width="400">

### Wiring
| Servo Motor | **-->** | Arduino Uno R3 |
|  :---  | :---: | :--- |
| Ground  (GND)  | **-->** | Ground (GND) |
| 3.3 Volts  (3.3V)  | **-->** |  3.3 Volts (3.3V)  |
| Source (Signal)  | **-->** | Digital Pin 9 (D9)  |

### Code
```C++
void lock();                 // Sets the lock position to locked
void unlock();               // Sets the lock position to unlocked
void changeLock();           // Changes the current lock position
```
