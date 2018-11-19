//#include <stdio.h>
//https://tutorial.cytron.io/2015/02/17/rfid-reader-id-20la-125khz-with-arduino/

#define RESET_PIN           7


// Run once as the initial setup
void setup() {
  Serial.begin(9600);                         // Set the baud rate
  pinMode(RESET_PIN, OUTPUT);                 // Init the RFID reset pin
  digitalWrite(RESET_PIN, HIGH);              // Write the reset pin
}

// Run indefinitely where all of the important functionaly takes place
void loop() {
  checkReaderIf();
  //checkReader();
  delay(250);
}

// Checks the reader for tags
void checkReaderIf() {
  bool reading = false;
  //byte readByte = NULL;
  uint8_t readByte = NULL;

  if (Serial.available() >= 13) {
    if (Serial.read() == 0x02) {
      reading = true;
      for (uint8_t i = 0; i < sizeof(tagString); i++) {
        readByte = Serial.read();
        // Do more research into the following statements...
        if (readByte >= '0' && readByte <= '9')
          readByte = readByte - '0';
        else if (readByte >= 'A' && readByte <= 'F')
          readByte = 10 + readByte - 'A';

        tagString[i] = readByte;
        Serial.print(readByte);
      }
    } else reading = false;
  }

  // If something was scanned...
  if (reading == true) {
    Serial.print("Tag scanned: ")
    memset(tagString, NULL, 13);  // Reset the tagString
    reading = false;
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
  memset(tagString, NULL, 13);  // Reset the tagString
  resetReader();                // Reset the RFID reader
}

// Reset the reader to be read from again
void resetReader() {
  digitalWrite(RESET_PIN, LOW);
  digitalWrite(RESET_PIN, HIGH);
  delay(50);
}
