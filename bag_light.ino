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

#define PIN_LED             (11)
#define PIN_DIAG_LED        (13)

#define PIN_BTN_CLOSED      (4)
#define PIN_BTN_OPENED      (5)

typedef struct {
  unsigned int openedthres;
  unsigned int closedthres;
} State;

State _state;

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
  _state.openedthres = 0;
  _state.closedthres = 0;

  Serial.begin(9600);
  Wire.begin();

  byte prodid = readReg1(PROD_ADDR);
  if (prodid != 0x11) {
    Serial.println("!! Product ID mistmatch !!");
  }

  Serial.println("VNCL4000 found");
  writeReg(PROX_MOD_ADDR, 0x81);
  writeReg(IR_CURRENT_ADDR, 10);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_DIAG_LED, OUTPUT);

  // input button, pulled high
  pinMode(PIN_BTN_CLOSED, INPUT);
  digitalWrite(PIN_BTN_CLOSED, HIGH);

  pinMode(PIN_BTN_OPENED, INPUT);
  digitalWrite(PIN_BTN_OPENED, HIGH);
}

void loop() {
  Serial.print("Thresholds: opened=");
  Serial.print(_state.openedthres);
  Serial.print(" closed=");
  Serial.print(_state.closedthres);
  Serial.println();
  Serial.print("Proximity data:");
  unsigned int prox = readProximity();
  Serial.print(prox, DEC);
  Serial.println();
  Serial.println();

  bool opendown = digitalRead(PIN_BTN_OPENED) == 0;
  bool closedown = digitalRead(PIN_BTN_CLOSED) == 0;

  if (opendown || closedown) {
    digitalWrite(PIN_DIAG_LED, HIGH);

    /**
      When opened btn is pressed the proximity value is read into openedthres
      When closed btn is pressed the proximity value is read after 3 seconds
      into closedthres
      */
    if (opendown) _state.openedthres = prox;
    if (closedown) {
      delay(3000);
      _state.closedthres = readProximity();
    }

    // debounce
    delay(100);
  } else digitalWrite(PIN_DIAG_LED, LOW);

  if (_state.openedthres > 0 && _state.closedthres > 0) {
    unsigned int range = _state.closedthres - _state.openedthres;
    if (prox < 0.7*range+_state.openedthres) {
      digitalWrite(PIN_LED, HIGH);
      // if it is on, make it stay on for at least 3s to avoid flashing
      delay(200);
    } if (prox > 0.75*range+_state.openedthres) {
      digitalWrite(PIN_LED, LOW);
      delay(200);
    }
  }
}
