//----------      RFID_CODE      ------------
//--------  Author: Austin Winarski  --------

#include <Adafruit_NeoPixel.h>
#include <Servo.h>
#include <TimeLib.h>
#include <Wire.h>

// Analog Pins
// SDA_PIN 4            These are purely here to
// SCL_PIN 5            remember which pins are used

// Digital Pins
#define D0_PIN              0
#define CONTACT_SWITCH      4
#define LED_DATA_PIN        6
#define RESET_PIN           7
#define LOCK_BUTTON_PIN     8
#define SERVO_DATA_PIN      9
#define PIEZO_PIN           11
#define KEYPAD_ADDRESS      75

// Other
#define LED_NUM 16          // 0-15
#define AUSTIN_CARD         "0E001E9E1698"
//#define MOM ""
//#define DAD ""
#define AUSTIN_PIN          "1130"
#define STEFANOS_PIN        ""

// TODO: * Make a 2D char array[13] for each valid tagString
//       * Look into optimizing each loop cycle when checking keypad and reader
//       * Make an easy way to get the days keypad codes
//       * Change the rainbowCycle to be less of an eye sore
//       * Do the keypad buffer to get only 4 digits per check


// Global Variable Definition
bool buttonState = false;       // Im assuming this is for the switch button?
bool buttonOld = false;         // Used to see if the buttonState changed
bool state = false;             // Used for the contact switch state (Needs a name change)
bool ignoreReset = false;       // Look into this var
bool doorLocked = false;        // True if door is locked
bool reading = false;           // True if the RFID reader is in a reading state
char tagString[13];             // May want to use byte instead of char
char keypad_buffer[32];         // Holds the 4 keypad digits
Servo lockServo;                // Create servo object to control a servo
uint8_t readByte = 0;           // The byte that holds the current keypad entry
uint16_t pixel = 0, color = 0;  // Inits the pixel and color data for the rainbow cycling
Adafruit_NeoPixel LEDring = Adafruit_NeoPixel(LED_NUM, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);

// Run once as the initial setup
void setup() {
  Serial.begin(9600);                         // Set the baud rate
  pinMode(RESET_PIN, OUTPUT);                 // Init the RFID reset pin
  digitalWrite(RESET_PIN, HIGH);              // Write the reset pin
  pinMode(LOCK_BUTTON_PIN, INPUT_PULLUP);     // Init the lock button
  pinMode(CONTACT_SWITCH, INPUT_PULLUP);      // Init the contact switch
  lock();                                     // Yeah
  LEDring.begin();                            // Init the LED Ring pin
  LEDring.show();                             // Set the LEDs to off
  LEDring.setBrightness(20);                  // ~1/12.75 the max brightness
  Wire.begin();                               // Init the I2C communications (keypad)
  buttonOld = digitalRead(LOCK_BUTTON_PIN);   // CHECK IF THIS CALL IS NECESSARY
  //Serial.println(dailyCode());              // Shows the dailyCode (testing purposes)
  GetSetTime();                               // Do this last to get the most recent time?
}

// Run indefinitely where all of the important functionaly takes place
void loop() {

  // These statements are weird...
  // if(doorLocked) {
  //   rainbowCycle();
  // } else {
  //   allSet(LEDring.Color(0,0,0));
  // }

  rainbowCycleIf();

  // Look into this
  if(!ignoreReset) {
    buttonState = digitalRead(LOCK_BUTTON_PIN);
    if(buttonState != buttonOld) {
     Serial.println("in reset function");
     if(!doorLocked) {
       flash(LEDring.Color(255,0,0));
     } else {
       flash(LEDring.Color(0,255,0));
     }
     buttonOld = buttonState;
     changeLock();
     Serial.println("Reset Button");
    }
  }
}

// Checks the reader for tags
void checkReader() {
  uint8_t index = 0;

  // Instead of a while loop, use an if statement with a nested for loop
  while(Serial.available()) {
    readByte = Serial.read();   // Read next available byte

    if(readByte == 2) reading = true;   // Begining of tag
    if(readByte == 3) reading = false;  // End of tag

    if(reading && readByte != 2 && readByte != 10 && readByte != 13) {
      tagString[index] = readByte;      // Store the tag
      index ++;
    }
  }
  checkTag(tagString);          // Check if it is a match
  // reset the tagString here
  memset(tagString, NULL, 13);  // Reset the tagString
  resetReader();                // Reset the RFID reader
}

// Checks the reader for tags
void checkReaderIf() {
  if (Serial.available() >= 13) {
    if (Serial.read() == 0x02) {
      reading = true;
      for (uint8_t i = 0; i < sizeof(tagString); i++) {
        readByte = Serial.read();

        if (readByte >= '0' && readByte <= '9')
          readByte = readByte - '0';
        else if (readByte >= 'A' && readByte <= 'F')
          readByte = 10 + readByte - 'A';

        tagString[i] = readByte;
      }
    } else reading = false;
  }

  if (reading == true) {
    checkTag(tagString);          // Check if it is a match
    //resetTag
    resetReader();                // Reset the RFID reader
    reading = false;
  }
  //delay(200);
}

// Reads the char(s) from the keypad
void checkKeypad() {
  while (Wire.available()) {
    char c = Wire.read();         // Receive a byte as character
    if (c != NULL) {
      Serial.println(c);          // Print the character
      strcat(keypad_buffer, c);   // Append the char to 4 digit array
      // If you have captured a full 4-digit pin...
      if (strlen(keypad_buffer) == 4) {
        checkCode(keypad_buffer);           // Check database for matching code
        memset(keypad_buffer, NULL, 64);    // Reset the keypad_buffer
      } else {
        tone(PIEZO_PIN, 2000);
        delay(150);
        noTone(PIEZO_PIN);
      }
    }
  }
}

// Reads the char(s) from the keypad
void checkKeypadIf() {
  if (Wire.available()) {
    char c = Wire.read();         // Receive a byte as character
    if (c != NULL) {
      Serial.println(c);          // Print the character
      strcat(keypad_buffer, c);   // Append the char to 4 digit array
      // If you have captured a full 4-digit pin...
      if (strlen(keypad_buffer) == 4) {
        checkCode(keypad_buffer);           // Check database for matching code
        memset(keypad_buffer, NULL, 64);    // Reset the keypad_buffer
      } else {
        tone(PIEZO_PIN, 2000);
        delay(150);
        noTone(PIEZO_PIN);
      }
    }
  }
}

// Plays a tone signifying your tag was accepted
void acceptTone() {
  tone(PIEZO_PIN, 2610);
  delay(200);
  tone(PIEZO_PIN, 4150);
  delay(200);
  noTone(PIEZO_PIN);
}

// Plays a tone signifying your tag was rejected
void rejectTone() {
  tone(PIEZO_PIN, 1000);
  delay(200);
  tone(PIEZO_PIN, 300);
  delay(200);
  noTone(PIEZO_PIN);
}

// Takes a char array and checks with the database to see if the tag is accepted
void checkTag(char tag[]) {
  if (strlen(tag) == 0) return;        // empty, no need to continue

  // If the given tag is(n't) in the database, take the proper steps
  if (compareTag(tag, AUSTIN_CARD)) {
    Serial.print("Tag match for Austin's card: ");
    Serial.print(AUSTIN_CARD);
    flash(LEDring.Color(255,0,0));
    LEDring.show();
    acceptTone();
    changeLock();
  }
  // else if(compareTag(tag, Ross)) {
  //   Serial.println("Tag Match - Ross");
  //   flash(LEDring.Color(0,255,0));
  //   LEDring.show();
  //   acceptTone();
  //   changeLock();
  // }
  else {
    Serial.print("No match for tag: ");
    Serial.print(tag); //read out the unknown tag
    flash(LEDring.Color(255,0,0));
    LEDring.show();
    rejectTone();
    lock();
  }
}

// Clears the tag from the previous read
void clearTag() {
  for (uint8_t i = 0; i < sizeof(tagString); i++) {
    tagString[i] = NULL;
  }
}

// Reset the reader to be read from again
void resetReader() {
  digitalWrite(RESET_PIN, LOW);
  digitalWrite(RESET_PIN, HIGH);
  delay(80);
}

// Compare the given tag to the given valid tag
bool compareTag(char tag[], char database[]) {
  if(strlen(tag) == 0) return false; //empty

  for(int i = 0; i < 12; i++) {
    // may want to use strstr(tag, database) to compare two char arrays
    if(tag[i] != database[i]) return false;
  }

  return true; //no mismatches
}

// Compare the given tag to all database tags
bool compareDatabase(char tag[]) {
  // Cycle through the database of acceptable tags
}

// THIS NEEDS MAJOR EDITING
// wait is never even used...
void lockCountdown(uint8_t wait) {
  allSet(LEDring.Color(255,0,0));
  doorLocked = false;
  Serial.println("Lock Countdown...");
  bool buttonTrack = digitalRead(LOCK_BUTTON_PIN);

  // For each led in the ring, countdown and break if the lock button is pressed
  for (uint16_t i=0; i<LEDring.numPixels(); i++) {
    // If button is pressed, countdown aborts
    if (digitalRead(LOCK_BUTTON_PIN) != buttonTrack) {
      //Serial.println("In break lock");
      buttonTrack = digitalRead(LOCK_BUTTON_PIN);
      doorLocked = false;
      changeLock();
      return;
    }
    //Serial.println("After break");
    LEDring.setPixelColor(i, LEDring.Color(255,0,0));
    if (i==0) {
      LEDring.setPixelColor(LEDring.numPixels()-1, LEDring.Color(0,0,0));
    } else {
      LEDring.setPixelColor(i-1, LEDring.Color(0,0,0));
    }
    LEDring.show();
    delay(1000);
    //delay((wait/16)*1000);      // wait is he number of seconds passed
  }
  doorLocked = false;
  //Serial.println("calling change from countdown");
  changeLock();

  tone(PIEZO_PIN, 4150);
  delay(200);
  tone(PIEZO_PIN, 2160);
  delay(200);
  noTone(PIEZO_PIN);
}

// Sets the lock position to locked
void lock() {
  lockServo.attach(9);
  lockServo.write(15);
  delay(100);
  while(lockServo.read() != 15) {
    //lockServo.write(15);
    delay(100);
  }
  Serial.println("Locked");
  doorLocked = true;
  lockServo.detach();
}

// Sets the lock position to unlocked
void unlock() {
  lockServo.attach(9);
  lockServo.write(110);
  delay(100);
  while(lockServo.read() != 110) {
    //lockServo.write(110);
    delay(100);
  }
  Serial.println("Unlocked");
  doorLocked = false;
  lockServo.detach();
}

// Changes the current lock position
void changeLock() {
  Serial.print("Changing Lock... ");
  lockServo.attach(9);

  // If the door is locked and needs to be unlocked...
  if(doorLocked) {
    unlock();
    lockCountdown(15);          // Countdown will last 15 seconds
    buttonOld = digitalRead(LOCK_BUTTON_PIN);   // Update prev button position
    Serial.print("door unlocked.");
  }
  // If the door is unlocked and needs to be locked...
  else {
    while (state == HIGH) {
      // Door is open... wait for it to close before locking
      delay(1000);
    }
    lock();
    Serial.print("door locked.");
  }
  lockServo.detach();
}

// Generates the 4 digit code
// TODO: Convert from string to char[] for efficiency
String dailyCode() {
  // day() = 1-31
  // month() = 1-12
  // weekday() = 1-7 (Sunday = 1)
  // year() = 2018 (year() % 100 = 18, i.e. last 2 digits)
  // Example1: Tuesday, November 13th, 2018 = 13*11*3*18 = 7722
  // Example1: Friday, November 16th, 2018 = 16*11*6*18 = 19008 -> 9008
  uint32_t code = day() * month() * weekday() * (year() % 100);

  String code_str = String(code);

  if (code_str.length() > 4) {
    code_str = code_str.substring(code_str.length() - 4);
  } else while (code_str.length() < 4) {
    for (uint8_t i=0; i < (4-code_str.length()); i++) {
      code_str = "0" + code_str;
    }
  }
  return code_str;
}

// Checks to see if the 4 digit code entered is correct
void checkCode(String code) {
  if (code == AUSTIN_PIN || code == dailyCode()) {
    flash(LEDring.Color(255,0,0));
    LEDring.show();
    acceptTone();
    changeLock();
  } else {
    flash(LEDring.Color(255,0,0));
    LEDring.show();
    rejectTone();
    lock();
  }
}

// Gets the current time from your machine and sets the internal clock
void GetSetTime() {
  char const date[] = __DATE__;
  char const t[] = __TIME__;
  uint16_t month, day, year, hour, min, second;
  char month_str[5];
  static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  sscanf(t, "%02d:%02d:%02d", &hour, &min, &second);            // Set the current hour, minute, and second
  sscanf(date, "%s %d %d", month_str, &day, &year);             // Get the month string and set the current month, day, and year
  month = (strstr(month_names, month_str)-month_names)/3+1;     // Set the current month as an int

  setTime(hour, min, second, day, month, year);                 // Set the internal clock to the current time
}

/***********************************************/
/************* LED Ring Code Below *************/
/***********************************************/

// INCORPORATE THIS FUNCTION TO BE COMPLETELY RUN IN THE LOOP FUNCTION
// AND EDIT TO BE MORE USER FRIENDLY...
void rainbowCycle() {
  // Cycles of all colors on wheel
  for(color=0; color<256; color++) {
    for(pixel=0;pixel<LEDring.numPixels(); pixel++) {
      LEDring.setPixelColor(pixel, Wheel(((pixel * 256 / LEDring.numPixels()) + color) & 255));
    }
    LEDring.show();
  }
}

void rainbowCycleIf() {
  if (pixel == LEDring.numPixels()) {
    pixel = 0;
    color++;
    if (color == 256)
      color = 0;
  }
  LEDring.setPixelColor(pixel, Wheel(((pixel * 256 / LEDring.numPixels()) + color) & 255));
  LEDring.show();
  pixel++;
}

// Takes a color and flashes the LEDring with it 3 times?
void flash(uint32_t c) {
  allSet(c); delay(50);
  allSet(LEDring.Color(0, 0, 0)); delay(50);
  allSet(c); delay(50);
  allSet(LEDring.Color(0, 0, 0)); delay(50);
  allSet(c); delay(50);
  allSet(LEDring.Color(0, 0, 0)); delay(50);
}

// Takes a color and set all the LEDring pixels to the color
void allSet(uint32_t c) {
  for(uint16_t i=0; i<LEDring.numPixels(); i++)
      LEDring.setPixelColor(i, c);
  LEDring.show();
}

// Makes sure that the last pixel and first pixel connect in a loop
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return LEDring.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return LEDring.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return LEDring.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
