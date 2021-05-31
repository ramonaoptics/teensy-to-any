#include "i2c.hpp"
#include <errno.h>
#if TEENSY_TO_ANY_HAS_I2C
#include <i2c_t3.h>
int I2CMaster::init(int baudrate, int timeout_ms, int address_size,
                    int address_msb_first) {
  if (address_size != 2 && address_size != 1)
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
  if (!this->is_initialized)
    return ENODEV;
  Wire.resetBus();
  return 0;
}

void I2CMaster::_write_register_address(int register_address) {
  if (this->address_size == 2) {
    Wire.write((register_address >> 8) & 0xFF);
    Wire.write((register_address >> 0) & 0xFF);
  } else if (this->address_size == 1) {
    Wire.write((register_address >> 0) & 0xFF);
  }
}

int I2CMaster::write_uint16(int slave_address, int register_address,
                            uint16_t data) {
  if (!this->is_initialized)
    return ENODEV;
  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;
  // Ensure the address is in write mode
  Wire.beginTransmission(slave_address); // slave addr

  this->_write_register_address(register_address);
  // Assume MSB first for data as well
  Wire.write((data >> 8) & 0xFF);
  Wire.write((data >> 0) & 0xFF);
  // blocking write (when not specified I2C_STOP is implicit)
  Wire.endTransmission(I2C_STOP);

  return Wire.getError();
}

int I2CMaster::write_uint8(int slave_address, int register_address,
                           uint8_t data) {
  if (!this->is_initialized)
    return ENODEV;
  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;
  // Ensure the address is in write mode
  Wire.beginTransmission(slave_address); // slave addr
  this->_write_register_address(register_address);
  Wire.write((data >> 0) & 0xFF);
  // blocking write (when not specified I2C_STOP is implicit)
  Wire.endTransmission(I2C_STOP);

  return Wire.getError();
}

int I2CMaster::write_no_register_uint8(int slave_address, uint8_t data) {
  if (!this->is_initialized)
    return ENODEV;

  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;

  Wire.beginTransmission(slave_address); // slave addr
  Wire.write(data);
  // blocking write (when not specified I2C_STOP is implicit)
  Wire.endTransmission(I2C_STOP);

  return Wire.getError();
}

int I2CMaster::read_uint16(int slave_address, int register_address,
                           uint16_t &data) {
  if (!this->is_initialized)
    return ENODEV;
  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;
  // Ensure the address is in write mode
  uint8_t err;

  Wire.beginTransmission(slave_address); // slave addr
  // Write the MSB of the address first
  this->_write_register_address(register_address);
  Wire.endTransmission(
      I2C_NOSTOP); // blocking write (when not specified I2C_STOP is implicit)
  err = Wire.getError();
  if (err) {
    goto handle_error;
  }

  Wire.requestFrom(slave_address, 2);
  Wire.finish();
  err = Wire.getError();
  if (err) {
    goto handle_error;
  }

  if (Wire.available() != 2) {
    err = 3;
    goto handle_error;
  }

  // Assume MSB is transfered first
  data = Wire.readByte() << 8;
  data = data | Wire.readByte();

  return 0;

handle_error:
  while (Wire.available() != 0) {
    Wire.readByte();
  }
  return err;
}

int I2CMaster::read_uint8(int slave_address, int register_address,
                          uint8_t &data) {
  if (!this->is_initialized)
    return ENODEV;
  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;
  // Ensure the address is in write mode
  uint8_t err;

  Wire.beginTransmission(slave_address); // slave addr
  // Write the MSB of the address first
  this->_write_register_address(register_address);
  Wire.endTransmission(
      I2C_NOSTOP); // blocking write (when not specified I2C_STOP is implicit)
  err = Wire.getError();
  if (err) {
    goto handle_error;
  }

  Wire.requestFrom(slave_address, 1);
  Wire.finish();
  err = Wire.getError();
  if (err) {
    goto handle_error;
  }

  if (Wire.available() != 1) {
    err = 3;
    goto handle_error;
  }

  data = Wire.readByte();

  return 0;

handle_error:
  while (Wire.available() != 0) {
    Wire.readByte();
  }
  return err;
}

int I2CMaster::read_no_register_uint8(int slave_address, uint8_t &data) {
  if (!this->is_initialized)
    return ENODEV;
  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;
  uint8_t err;
  data = 0;

  Wire.requestFrom(slave_address, 1);
  Wire.finish();
  err = Wire.getError();
  if (err) {
    goto handle_error;
  }

  if (Wire.available() != 1) {
    err = 3;
    goto handle_error;
  }

  data = Wire.readByte();
  return 0;

handle_error:
  while (Wire.available() != 0) {
    Wire.readByte();
  }
  return err;
}
#endif
