#include <TimeLib.h>

//https://www.instructables.com/id/TESTED-Timekeeping-on-ESP8266-Arduino-Uno-WITHOUT-/

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  GetSetTime();

  Serial.print(month());Serial.print("/");
  Serial.print(day());Serial.print("/");
  Serial.print((year())%1000);Serial.print("\n");
  Serial.print("Weekday: ");Serial.print(weekday());Serial.print(". ");
  Serial.print("Daily Code: ");Serial.print(dailyCode());Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print(hour());Serial.print(":");
  Serial.print(minute());Serial.print(":");
  Serial.print(second());Serial.println();
  
  delay(1000);
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
