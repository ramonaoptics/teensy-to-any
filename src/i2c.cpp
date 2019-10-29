#include "i2c.hpp"
#include <errno.h>
#include <i2c_t3.h>
int I2CMaster::init(int baudrate, int timeout_ms, int address_size,
                    int address_msb_first) {
  if (address_size != 2)
    return ENOSYS;
  if (address_msb_first != 0)
    return ENOSYS;
  Wire.begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, baudrate);
  Wire.setDefaultTimeout(timeout_ms);
  this->address_msb_first = address_msb_first;
  this->address_size = address_size;
  this->is_initialized = true;
  return 0;
}

int I2CMaster::reset() {
  Wire.resetBus();
  return 0;
}

int I2CMaster::write_uint16(int slave_address, int register_address,
                            uint16_t data) {
  // Ensure the address is in write mode
  uint8_t slave = slave_address & (0xFF ^ 0x01);
  Wire.beginTransmission(slave); // slave addr
  Wire.write((register_address >> 8) & 0xFF);
  Wire.write((register_address >> 0) & 0xFF);
  Wire.write((data >> 0) & 0xFF);
  Wire.write((data >> 8) & 0xFF);
  Wire.endTransmission(
      I2C_STOP); // blocking write (when not specified I2C_STOP is implicit)

  return Wire.getError();
}

int I2CMaster::read_uint16(int slave_address, int register_address,
                           uint16_t &data) {
  // Ensure the address is in write mode
  uint8_t slave = slave_address & (0xFF ^ 0x01);
  uint8_t err;
  data = 0;

  Wire.beginTransmission(slave); // slave addr
  // Write the MSB of the address first
  Wire.write((register_address >> 8) & 0xFF);
  Wire.write((register_address >> 0) & 0xFF);
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