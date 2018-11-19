#include <Time.h>
#include <TimeLib.h>
#include <Wire.h>

#define PIEZO_PIN           11
#define KEYPAD_ADDRESS      75
#define AUSTIN_PIN          "1130"

char keypad_buffer[64];

void setup() {
  Serial.begin(9600);             // Set the baud rate
  Wire.begin();                   // Init the data "wire" for the keypad
  checkCode("1111");
}

void loop() {
  checkKeypadIf();
  delay(200);
}

// Reads the char(s) from the keypad
//void checkKeypad() {
//  while (Wire.available()) {
//    char c = Wire.read();         // Receive a byte as character
//    if (c != NULL) {
//      //Serial.println(c);          // Print the character
//      strcat(keypad_buffer, c);   // Append the char to 4 digit array
//      // If you have captured a full 4-digit pin...
//      if (strlen(keypad_buffer) == 4) {
//        checkCode(keypad_buffer);           // Check database for matching code
//        memset(keypad_buffer, NULL, 64);    // Reset the keypad_buffer
//      } else {
//        tone(PIEZO_PIN, 2000);
//        delay(150);
//        noTone(PIEZO_PIN);
//      }
//    }
//  }
//}

// Reads the char(s) from the keypad
void checkKeypadIf() {
  if (Wire.available()) {
    byte c = Wire.read();                   // Receive a byte as character
    if (c != NULL) {
      Serial.println(c);                    // Print the character
      strcat(keypad_buffer, c);             // Append the char to 4 digit array
      // If you have captured a full 4-digit pin...
      if (strlen(keypad_buffer) == 4) {
        //Serial.print("Current keypad buffer: ");Serial.print(keypad_buffer);Serial.print("\n");
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
      code_str = "0" + code_str;   // Pad codes that are less than 4 digits with 0s
    }
  }
  return code_str;
}

// Checks to see if the 4 digit code entered is correct
void checkCode(char code[]) {
//  if (code == AUSTIN_PIN || code == dailyCode()) {
//    flash(LEDring.Color(255,0,0));
//    LEDring.show();
//    acceptTone();
//    changeLock();
//  } else {
//    flash(LEDring.Color(255,0,0));
//    LEDring.show();
//    rejectTone();
//    lock();
//  }
  code = "9008";
  if (strcmp(code, AUSTIN_PIN) == 0) {
    Serial.println("HERE");
  } else if (strcmp(code, dailyCode()) == 0) {
    Serial.println("HERE TOO");
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
