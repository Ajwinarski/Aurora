#include <Wire.h>
#include <Servo.h>

#define PIEZO_PIN 11

int device = 0;         // Used to hold the keypad address
Servo lockServo;        // Create servo object to control a servo

void setup() {
  Wire.begin();         // Join i2c bus (address optional for master)
  Serial.begin(9600);   // Start serial for output

  byte error, address;

  Serial.println("Scanning...");

  for (address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address ");
      if (address < 16)
        Serial.print("0");
      Serial.print(address);
      device = address;           // Set the device value to be used later
      break;
    }
  }
}

// Reads from the keypad every half a second
void loop() {
  Wire.requestFrom(device, 1);    // Request 1 byte from slave device

  while (Wire.available()) {      // Slave may send less than requested
    char c = Wire.read();         // Receive a byte as character
    if (c != NULL) {
      Serial.println(c);          // Print the character
      tone(PIEZO_PIN, 2000);
      delay(150);
      noTone(PIEZO_PIN);
    }
  }
  delay(500);
}
