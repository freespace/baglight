#include <Wire.h>

#define VNCL4000_ID         (0x13)
#define PROD_ADDR           (0x81)
#define COMMAND_ADDR        (0x80)
#define PROX_HIGH_ADDR      (0x87)
#define PROX_LOW_ADDR       (0x88)
#define PROX_MOD_ADDR       (0x8A)
#define IR_CURRENT_ADDR     (0x83)

#define COMMAND_PROX_OD     (1<<3)
#define COMMAND_ALS_OD      (1<<4)
#define COMMAND_PROX_RDY    (1<<5)

/**
  Asks the VNCL4000 to perform an on-demand proximity reading, returning
  the proximity value
*/
unsigned int readProximity() {
  // ask to a proximity reading to be done
  writeReg(COMMAND_ADDR, COMMAND_PROX_OD);

  delay(100);

  // wait for the data to be ready
  byte r = 0;
  while (r & COMMAND_PROX_RDY == 0) {
    r = readReg1(COMMAND_ADDR);
  }

  unsigned int data = readReg1(PROX_HIGH_ADDR);
  data <<= 8;
  data |= readReg1(PROX_LOW_ADDR);
  return data;
};

/**
  Writes the byte b to register r
  */
void writeReg(byte r, byte b) {
  Wire.beginTransmission(VNCL4000_ID);
  Wire.write(r);
  Wire.write(b);
  Wire.endTransmission();
}
/**
Reads 1 byte from register r
*/
byte readReg1(byte r) {
  Wire.beginTransmission(VNCL4000_ID);
  Wire.write(r);
  Wire.endTransmission();
  Wire.requestFrom(VNCL4000_ID, 1);

  while(!Wire.available());
  return Wire.read();
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  byte prodid = readReg1(PROD_ADDR);
  if (prodid != 0x11) {
    Serial.println("!! Product ID mistmatch !!");
  }

  Serial.println("VNCL4000 found");
  writeReg(PROX_MOD_ADDR, 0x81);
  writeReg(IR_CURRENT_ADDR, 10);

  pinMode(5, OUTPUT);
}

void loop() {
  Serial.print("Proximity data:");
  unsigned int prox = readProximity();
  Serial.print(prox, DEC);
  Serial.print(" ");
  Serial.print(prox);
  Serial.print(" ");
  Serial.println();

  analogWrite(5, map(prox, 2500, 30000, 0, 255));
}
