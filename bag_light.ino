#include <Wire.h>

void setup() {
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  Serial.println(millis());
  Wire.beginTransmission(0x13);
  Wire.write(0x81);
  Wire.endTransmission();
  Wire.requestFrom(0x13, 1);

  while(Wire.available()) {
    char c = Wire.read();
    Serial.print(">>");
    Serial.println(c, HEX);
  }
  
  delay(500);
}
