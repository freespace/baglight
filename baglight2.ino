#include <Wire.h>

#define VCNL4000_ADDRESS 0x13  // I2C Address of the board
#define PROBES = 5

const int redLed = 12;
const int greenLed = 11;
const int blueLed = 10;

const int openTreshold = 2200;



void setup(){
  Serial.begin(9600);
  Wire.begin();
  initVCNL4000();
  setupLight();
}

void loop(){
  unsigned int proximityValue = readProximity();
  Serial.println(proximityValue);
  if (proximityValue <= openTreshold) {
    lightOn();
  } else { 
    lightOff();
  }
  delay(1000 / PROBES);
}


void initVCNL4000(){
  byte temp = readVCNLByte(0x81);

  if (temp != 0x11){  // Product ID Should be 0x11
    Serial.print("initVCNL4000 failed to initialize");
    Serial.println(temp, HEX);
  }else{
    Serial.println("VNCL4000 Online...");
  } 

  /*VNCL400 init params
   Feel free to play with any of these values, but check the datasheet first!*/
  writeVCNLByte(0x84, 0x0F);  // Configures ambient light measures - Single conversion mode, 128 averages
  writeVCNLByte(0x83, 15);  // sets IR current in steps of 10mA 0-200mA --> 200mA
  writeVCNLByte(0x89, 2);  // Proximity IR test signal freq, 0-3 - 781.25 kHz
  writeVCNLByte(0x8A, 0x81);  // proximity modulator timing - 129, recommended by Vishay 
}


unsigned int readProximity(){
  // readProximity() returns a 16-bit value from the VCNL4000's proximity data registers
  byte temp = readVCNLByte(0x80);
  writeVCNLByte(0x80, temp | 0x08);  // command the sensor to perform a proximity measure
  byte readyBit = readVCNLByte(0x80);
  while(!(readyBit&0x20)) {
    readyBit = readVCNLByte(0x80);  
  }
  unsigned int data = readVCNLByte(0x87) << 8;
  data |= readVCNLByte(0x88);

  return data;
}


unsigned int readAmbient(){
  // readAmbient() returns a 16-bit value from the VCNL4000's ambient light data registers
  byte temp = readVCNLByte(0x80);
  writeVCNLByte(0x80, temp | 0x10);  // command the sensor to perform ambient measure

  while(!(readVCNLByte(0x80)&0x40));  // wait for the proximity data ready bit to be set
  unsigned int data = readVCNLByte(0x85) << 8;
  data |= readVCNLByte(0x86);

  return data;
}


void writeVCNLByte(byte address, byte data){
  // writeVCNLByte(address, data) writes a single byte of data to address
  Wire.beginTransmission(VCNL4000_ADDRESS);
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
  delayMicroseconds(170);
}


byte readVCNLByte(byte address){
  // readByte(address) reads a single byte of data from address
  Wire.beginTransmission(VCNL4000_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  delayMicroseconds(170);
  Wire.requestFrom(VCNL4000_ADDRESS, 1);
  while(!Wire.available()) {
      // nothing
  };
  byte data = Wire.read();

  return data;
}



void setupLight() {                
  pinMode(redLed, OUTPUT);  
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);  
}

void lightOn() {
  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, HIGH);
  digitalWrite(blueLed, HIGH);
}

void lightOff() {
  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, LOW);
  digitalWrite(blueLed, LOW);
}
