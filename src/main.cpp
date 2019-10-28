#include "commandrouting.hpp"
#include <Arduino.h>
#include <i2c_t3.h>

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
}

void i2c_init() {
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, 400000);
  Wire.setDefaultTimeout(200000); // 200ms
  Wire.resetBus();
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);
}

uint8_t write_uint16(uint8_t slave_address, uint16_t register_address,
                     uint16_t data) {
  // Ensure the address is in write mode
  uint8_t slave = slave_address & (0xFF ^ 0x01);
  Wire.beginTransmission(slave); // slave addr
  Wire.write((register_address >> 0) & 0xFF);
  Wire.write((register_address >> 8) & 0xFF);
  Wire.write((data >> 0) & 0xFF);
  Wire.write((data >> 8) & 0xFF);
  Wire.endTransmission(
      I2C_STOP); // blocking write (when not specified I2C_STOP is implicit)

  return Wire.getError();
}

uint8_t read_uint16(uint8_t slave_address, uint16_t register_address,
                    uint16_t &data) {
  // Ensure the address is in write mode
  uint8_t slave = slave_address & (0xFF ^ 0x01);
  uint8_t err;
  data = 0;

  Wire.beginTransmission(slave); // slave addr
  Wire.write((register_address >> 0) & 0xFF);
  Wire.write((register_address >> 8) & 0xFF);
  Wire.endTransmission(
      I2C_NOSTOP); // blocking write (when not specified I2C_STOP is implicit)
  err = Wire.getError();
  if (err) {
    goto handle_error;
  }

  Wire.requestFrom(slave | 0x01, 2);
  Wire.finish();
  err = Wire.getError();
  if (err) {
    goto handle_error;
  }

  if (Wire.available() != 2) {
    err = 3;
    goto handle_error;
  }

  data = Wire.readByte();
  data = data | (Wire.readByte() << 8);

  return 0;

handle_error:
  while (Wire.available() != 0) {
    Wire.readByte();
  }
  return err;
}

void _loop() {
  if (Serial.available()) {
    cmd.processSerialStream();
  }
}