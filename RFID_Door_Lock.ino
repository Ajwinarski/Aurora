/***************************************************/
/*************        RFID_CODE        *************/
/***********   Author: Austin Winarski   ***********/
/*************    <> for the Uno R3    *************/
/***************************************************/

#include <Adafruit_NeoPixel.h>
#include <Servo.h>
#include <TimeLib.h>
#include <Wire.h>

// Analog Pins
// KEYPAD_SDA_PIN 4                    These are purely here to
// KEYPAD_SCL_PIN 5                    document which pins are used

// Digital Pins
#define RFID_DATA_PIN       0
#define CONTACT_SWITCH_PIN  4
#define LED_DATA_PIN        6       // 5V
#define RESET_PIN           7
#define LOCK_BUTTON_PIN     8       // 5V
#define SERVO_DATA_PIN      9       // 3.3V
#define PIEZO_PIN           11
#define KEYPAD_ADDRESS      75      // 3.3V

// Other
#define LED_NUM 16                  // 0-15
#define AUSTIN_CARD         "0E001E9E1698"
#define SPARE_CARD          ""
#define AUSTIN_PIN          "1130"

// TODO: * Make a 2D char array[13] for each valid tagString
//       * ...

// Global Variable Definition
bool buttonState = false;       // Holds the state of the change lock push button
bool buttonOld = false;         // Used to validate if the buttonState changed
bool doorLocked = false;        // True if door is locked
char tagString[13];             // May want to use byte instead of char
char keypadBuffer[32];          // Holds the 4 keypad digits
Servo lockServo;                // Create servo object to control a servo
String dayCode = "";            // Holds the days keypad code
uint8_t counter = 0;            // Keeps track of position in the keypadBuffer
uint16_t pixel = 0, color = 0;  // Inits the pixel and color data for the rainbow cycling
Adafruit_NeoPixel LEDring = Adafruit_NeoPixel(LED_NUM, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);

// Run once as the initial setup
void setup() {
  Serial.begin(9600);                         // Set the baud rate
  pinMode(RESET_PIN, OUTPUT);                 // Init the RFID reset pin
  digitalWrite(RESET_PIN, HIGH);              // Write the reset pin
  pinMode(LOCK_BUTTON_PIN, INPUT_PULLUP);     // Init the lock button
  pinMode(CONTACT_SWITCH_PIN, INPUT_PULLUP);  // Init the contact switch
  LEDring.begin();                            // Init the LED Ring pin
  LEDring.show();                             // Set the LEDs to off
  LEDring.setBrightness(20);                  // ~1/12.75 the max brightness (0-255)
  Wire.begin();                               // Init the I2C communications (keypad)
  buttonOld = digitalRead(LOCK_BUTTON_PIN);   // CHECK IF THIS CALL IS NECESSARY
  lock();                                     // Locks the door
  GetSetTime();                               // Call the getter/setter for time
  dailyCode();                                // Calculates and sets the dayCode
}

// Run indefinitely where all of the important functionaly takes place
void loop() {
  if (strlen(keypadBuffer) == 0)
    rainbowCycle();

  // Check for button push every second
  if ((millis() % 500) == 0)
    // Check if the change lock button was pressed
    buttonState = digitalRead(LOCK_BUTTON_PIN);

  // Check the keypad every quarter of a second
  if ((millis() % 250) == 0) {
    // Check the reader every half second
    if ((millis() % 500) == 0)
      checkReader();
    checkKeypad();
  }

  // Prints the new daily code (testing purposes)
  if (hour() == 0 && minute() == 0 && second() == 0) {
    dailyCode();           // Set the new daily code
    delay(1000);           // Wait for second to no longer == 0
  }
}

/***********************************************/
/*************** RFID Code Below ***************/
/***********************************************/

// Checks the reader for tags
void checkReader() {
  // If the reader has captured the correct number of bytes
  if (Serial.available() >= 13) {
    // If the first byte is a 2, begin to read the input
    if (Serial.read() == 2) {
      // Read in each byte from the tag
      for (uint8_t i=0; i<sizeof(tagString); i++)
        tagString[i] = Serial.read();
      Serial.print("Tag scanned: ");
      Serial.println(tagString);
      checkTag(tagString);                // Check if it is a match
      resetReader();                      // Reset the RFID reader
    }
  }
}

// Takes a char array and checks with the database to see if the tag is accepted
void checkTag(char tag[]) {
  if (strlen(tag) == 0) return;           // Empty, no need to continue

  // If the given tag is in the database, take the proper steps
  if (compareTag(tag, AUSTIN_CARD)) {
    Serial.print("Tag match for Austin's card: ");
    Serial.println(AUSTIN_CARD);
    flash(LEDring.Color(0,255,0));        // Flash green to show valid tag
    acceptTone();                         // Play the acceptance tone
    changeLock();                         // Change the current lock position
  }
  // else if(compareTag(tag, SPARE_CARD)) {
  //   Serial.println("Tag Match - Spare card: ");
  //   Serial.println(SPARE_CARD);
  //   flash(LEDring.Color(0,255,0));
  //   acceptTone();
  //   changeLock();
  // }
  // If the given tag isn't in the database, take the proper steps
  else {
    Serial.print("No match for tag: ");
    Serial.println(tag);                  // Read out the unknown tag
    flash(LEDring.Color(255,0,0));        // Flash red to show invalid tag
    rejectTone();                         // Play the rejection tone
    lock();                               // Lock the door (just in case)
  }
}

// Reset the reader to be read from again
void resetReader() {
  digitalWrite(RESET_PIN, LOW);
  digitalWrite(RESET_PIN, HIGH);
  delay(50);
}

// Compare the given tag to the given valid tag
bool compareTag(char tag[], char database[]) {
  if(strlen(tag) == 0) return false;      //empty
  for(int i=0; i<12; i++) {
    // may want to use strstr(tag, database) to compare two char arrays
    if(tag[i] != database[i]) return false;
  }
  return true;                            //no mismatches
}

// Compare the given tag to all database tags
// TODO
bool compareDatabase(char tag[]) {
  // Cycle through the database of acceptable tags
}

/***********************************************/
/************** Keypad Code Below **************/
/***********************************************/

// Reads the input from the keypad
void checkKeypad() {
  Wire.requestFrom(KEYPAD_ADDRESS, 1);        // Request 1 byte from the keypad
  if (Wire.available()) {
    char c = Wire.read();                     // Receive a byte as character
    if (c != NULL) {
      //Serial.println(c);                    // Print the character
      keypadBuffer[counter] = c;
      // If you have captured a full 4-digit pin...
      if (strlen(keypadBuffer) == 4) {
        checkCode(keypadBuffer);              // Check database for matching code
        memset(keypadBuffer, NULL, 4);        // Reset the keypadBuffer
        counter = 0;
        delay(1000);
      } else {
        pinPressedTone();                     // Plays a quick tone
        counter++;
        if (counter == 1)
          allSet(LEDring.Color(0, 0, 0));     // Turn off all the pixels
        keypadFeedback();                     // Feedback on how many digits are entered
      }
    }
  }
}

// Checks to see if the 4 digit code entered is correct
void checkCode(String code) {
  if (code == AUSTIN_PIN || code == dayCode) {
    flash(LEDring.Color(0,255,0));
    Serial.print("Code: ");Serial.print(code);
    Serial.println(" accepted.");
    acceptTone();
    changeLock();
  } else {
    flash(LEDring.Color(255,0,0));
    Serial.print("Code: ");Serial.print(code);
    Serial.println(" rejected.");
    rejectTone();
    if (!doorLocked)
      lock();
  }
}

/***********************************************/
/*************** Lock Code Below ***************/
/***********************************************/

// Sets the lock position to locked
void lock() {
  // Door is open... wait for it to close before locking
  while (digitalRead(CONTACT_SWITCH_PIN) == HIGH)
    delay(1000);
  lockServo.attach(SERVO_DATA_PIN);
  lockServo.write(15);
  delay(1000);
  lockServo.detach();
  doorLocked = true;
  Serial.println("Locked.");
}

// Sets the lock position to unlocked
void unlock() {
  lockServo.attach(SERVO_DATA_PIN);
  lockServo.write(110);
  delay(1000);
  lockServo.detach();
  doorLocked = false;
  Serial.println("Unlocked.");
}

// Changes the current lock position
void changeLock() {
  Serial.print("Changing Lock... ");
  // If the door is locked and needs to be unlocked...
  if(doorLocked) {
    unlock();
    lockCountdown(15);                          // Countdown will last 15 seconds
    buttonOld = digitalRead(LOCK_BUTTON_PIN);   // Update prev button position
  } else
    lock();                                     // Locks if the door is open
}

// Counts down in seconds depending on the given wait time
void lockCountdown(uint8_t wait) {
  allSet(LEDring.Color(0,255,0));                   // Turn all LEDs green
  Serial.println("Begin lock Countdown...");        // Used to check if lock button is pressed
  bool buttonTrack = digitalRead(LOCK_BUTTON_PIN);  // Used to abort the countdown

  // For each led in the ring, countdown and break if the lock button is pressed
  for (uint16_t i=0; i<LEDring.numPixels(); i++) {
    // If the lock button is pressed, countdown aborts
    if (digitalRead(LOCK_BUTTON_PIN) != buttonTrack) {
      Serial.println("In break lock.");
      changeLock();
      return;
    }
    //LEDring.setPixelColor(i, LEDring.Color(0,255,0));
    if (i == 0)
      LEDring.setPixelColor(LEDring.numPixels()-1, LEDring.Color(0,255,0));
    else
      LEDring.setPixelColor(i-1, LEDring.Color(0,0,0));
    LEDring.show();
    delay((wait * 1000) / 16);         // Delays the number of seconds given as wait
  }
  changeLock();
  closeTone();
}

/***********************************************/
/************** Time Code Below ****************/
/***********************************************/

// Generates the 4 digit code based on the current date
void dailyCode() {
  // day() = 1-31
  // month() = 1-12
  // weekday() = 1-7 (Sunday = 1)
  // year() = 2018 (year() % 100 = 18, i.e. last 2 digits)
  // Example1: Tuesday, November 13th, 2018 = 13*11*3*18 = 7722
  // Example1: Friday, November 16th, 2018 = 16*11*6*18 = 19008 -> 9008
  //uint32_t code = (day() * month() * weekday() * (year() % 100));
  //String code_str = String(code);

  String code_str = String((day() * month() * weekday() * (year() % 100)));
  // If the code is longer than 4 digits, truncate
  if (code_str.length() > 4)
    code_str = code_str.substring(code_str.length() - 4);
  // While the code is less than 4 digits, pad with 0's
  else
    while (code_str.length() < 4)
      for (uint8_t i=0; i<(4-code_str.length()); i++)
        code_str = "0" + code_str;
  dayCode = code_str;
  Serial.print("Daily Code: ");
  Serial.println(dayCode);             // Shows the dailyCode (testing purposes)
}

// Gets and sets the current time from your machine (ONLY RUN ONCE)
void GetSetTime() {
  char const d[] = __DATE__;       // Gets the date from your system: "Mmm dd yyyy"
  char const t[] = __TIME__;       // Gets the time from your system: "hh:mm:ss"
  uint16_t month, day, year, hour, min, second;
  char month_str[5];
  static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  sscanf(t, "%02d:%02d:%02d", &hour, &min, &second);         // Set the current hour, minute, and second
  sscanf(d, "%s %d %d", month_str, &day, &year);             // Get the month string and set the current month, day, and year
  month = (strstr(month_names, month_str)-month_names)/3+1;  // Set the current month as an int

  setTime(hour, min, second, day, month, year);              // Set the internal clock to the current time
}

/***********************************************/
/************** Piezo Code Below ***************/
/***********************************************/

// Plays a tone signifying your tag was accepted
void acceptTone() {
  tone(PIEZO_PIN, 2610);
  delay(200);
  tone(PIEZO_PIN, 4150);
  delay(200);
  noTone(PIEZO_PIN);
}

// Plays a tone signifying your door was re-locked
void closeTone() {
  tone(PIEZO_PIN, 4150);
  delay(200);
  tone(PIEZO_PIN, 2610);
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

// Plays a tone when a keypad button is pressed
void pinPressedTone() {
  tone(PIEZO_PIN, 2400);
  delay(120);
  noTone(PIEZO_PIN);
}

/***********************************************/
/************* LED Ring Code Below *************/
/***********************************************/
// https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/#LEDStripEffectRainbowCycle
// https://www.adafruit.com/product/1260

// Updates a single pixel every call to simulate a spinning rainbow
void rainbowCycle() {
  // If you have reached the end of the ring, go to the beginning
  if (pixel == LEDring.numPixels()) {
    pixel = 0;
    color++;
    // If you have reached the max color code, reset
    if (color == 256)
      color = 0;
  }
  LEDring.setPixelColor(pixel, Wheel(((pixel * 256 / LEDring.numPixels()) + color) & 255));
  LEDring.show();
  pixel++;
}

// Lights up the LEDring to give feedback on how many digits you have entered
void keypadFeedback() {
  for (uint8_t i=0; i<(counter*4); i++)
    LEDring.setPixelColor(i, LEDring.Color(255, 255, 255));
  LEDring.show();
}

// Takes a color and flashes the LEDring with it 3 times
void flash(uint32_t c) {
  allSet(c); delay(50);
  allSet(LEDring.Color(0, 0, 0)); delay(50);
  allSet(c); delay(50);
  allSet(LEDring.Color(0, 0, 0)); delay(50);
  allSet(c); delay(50);
  allSet(LEDring.Color(0, 0, 0)); delay(50);
}

// Takes a color and sets all the LEDring pixels to the color
void allSet(uint32_t c) {
  for(uint16_t i=0; i<LEDring.numPixels(); i++)
      LEDring.setPixelColor(i, c);
  LEDring.show();
}

// Makes sure that the last pixel and first pixel connect in a loop
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
   return LEDring.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
   return LEDring.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return LEDring.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

// OLD rainbowCycle function (changed from a for loop to an if statement)
// void rainbowCycle() {
//   // Cycles of all colors on wheel
//   for(color=0; color<256; color++) {
//     for(pixel=0;pixel<LEDring.numPixels(); pixel++) {
//       LEDring.setPixelColor(pixel, Wheel(((pixel * 256 / LEDring.numPixels()) + color) & 255));
//     }
//     LEDring.show();
//   }
// }
