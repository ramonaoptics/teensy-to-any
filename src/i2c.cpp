#include "i2c.hpp"
#include <errno.h>
#if TEENSY_TO_ANY_HAS_I2C_T3
#include <i2c_t3.h>
#endif
#if TEENSY_TO_ANY_HAS_I2C_T4
#include <i2c_driver_wire.h>
#endif


#if TEENSY_TO_ANY_HAS_I2C_T3
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

int I2CMaster::write_payload(int slave_address, int register_address,
                             uint8_t *data, int num_bytes) {
  if (!this->is_initialized)
    return ENODEV;

  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;

  Wire.beginTransmission(slave_address); // slave addr
  this->_write_register_address(register_address);
  Wire.write(data, num_bytes);
  // blocking write (when not specified I2C_STOP is implicit)
  Wire.endTransmission(I2C_STOP);

  return Wire.getError();
}

int I2CMaster::read_payload(int slave_address, int register_address,
                            uint8_t *data, int num_bytes) {
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

  Wire.requestFrom(slave_address, num_bytes);
  Wire.finish();
  err = Wire.getError();
  if (err) {
    goto handle_error;
  }

  if (Wire.available() != num_bytes) {
    err = 3;
    goto handle_error;
  }

  for (int i = 0; i < num_bytes; i++) {
    data[i] = Wire.readByte();
  }

  return 0;

handle_error:
  while (Wire.available() != 0) {
    Wire.readByte();
  }
  return err;
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

int I2CMaster::read_payload_no_register(int slave_address,
                                        uint8_t *data,
                                        int num_bytes) {
  if (!this->is_initialized)
    return ENODEV;
  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;
  uint8_t err;
  memset(data, 0, num_bytes);

  Wire.requestFrom(slave_address, num_bytes);
  Wire.finish();
  err = Wire.getError();
  if (err) {
    goto handle_error;
  }

  if (Wire.available() != num_bytes) {
    err = 3;
    goto handle_error;
  }

  for (int i=0; i<num_bytes; i++) {
    data[i] = Wire.readByte();
  }

  return 0;

handle_error:
  while (Wire.available() != 0) {
    Wire.readByte();
  }
  return err;
}

int I2CMaster::read_payload_uint16(int slave_address, int register_address,
                                   uint8_t *data, int num_bytes) {
  if (!this->is_initialized)
    return ENODEV;
  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;
  // Ensure the address is in write mode
  uint8_t err;

  Wire.beginTransmission(slave_address); // slave addr
  // Write the MSB of the address first
  Wire.write((register_address >> 8) & 0xFF);
  Wire.write((register_address >> 0) & 0xFF);
  Wire.endTransmission(
      I2C_NOSTOP); // blocking write (when not specified I2C_STOP is implicit)
  err = Wire.getError();
  if (err) {
    goto handle_error;
  }

  Wire.requestFrom(slave_address, num_bytes);
  Wire.finish();
  err = Wire.getError();
  if (err) {
    goto handle_error;
  }

  if (Wire.available() != num_bytes) {
    err = 3;
    goto handle_error;
  }

  for (int i = 0; i < num_bytes; i++) {
    data[i] = Wire.readByte();
  }

  return 0;

handle_error:
  while (Wire.available() != 0) {
    Wire.readByte();
  }
  return err;
}
#endif


#if TEENSY_TO_ANY_HAS_I2C_T4
const bool I2C_NOSTOP = false;
const bool I2C_STOP = true;
int I2CMaster_T4::init(int baudrate, int timeout_ms, int address_size,
                    int address_msb_first) {
  if (address_size != 2 && address_size != 1)
    return ENOSYS;
  if (address_msb_first != 0)
    return ENOSYS;
  // For the teensy4 library, the timeout is constant to 200 ms
  // Wire.timeout_millis = timeout_ms;
  // Wire.setDefaultTimeout(timeout_ms);
  Wire.setClock(baudrate);
  Wire.begin();

  this->address_msb_first = address_msb_first;
  this->address_size = address_size;
  this->is_initialized = true;
  return 0;
}

int I2CMaster_T4::reset() {
  if (!this->is_initialized)
    return ENODEV;

  // Doesn't seem to exist. but maybe we can recreate with end/begin
  // Wire.resetBus()
  Wire.end();
  Wire.begin();

  return 0;
}

void I2CMaster_T4::_write_register_address(int register_address) {
  if (this->address_size == 2) {
    Wire.write((register_address >> 8) & 0xFF);
    Wire.write((register_address >> 0) & 0xFF);
  } else if (this->address_size == 1) {
    Wire.write((register_address >> 0) & 0xFF);
  }
}

int I2CMaster_T4::write_uint16(int slave_address, int register_address,
                            uint16_t data) {
  uint8_t error;
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
  error = Wire.endTransmission(I2C_STOP);

  return (int)(error);
}

int I2CMaster_T4::write_uint8(int slave_address, int register_address,
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
  return Wire.endTransmission(I2C_STOP);
}

int I2CMaster_T4::write_no_register_uint8(int slave_address, uint8_t data) {
  if (!this->is_initialized)
    return ENODEV;

  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;

  Wire.beginTransmission(slave_address); // slave addr
  Wire.write(data);
  // blocking write (when not specified I2C_STOP is implicit)
  return Wire.endTransmission(I2C_STOP);
}

int I2CMaster_T4::write_payload(int slave_address, int register_address,
                             uint8_t *data, int num_bytes) {
  if (!this->is_initialized)
    return ENODEV;

  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;

  Wire.beginTransmission(slave_address); // slave addr
  this->_write_register_address(register_address);
  Wire.write(data, num_bytes);
  // blocking write (when not specified I2C_STOP is implicit)
  return Wire.endTransmission(I2C_STOP);
}

int I2CMaster_T4::read_payload(int slave_address, int register_address,
                            uint8_t *data, int num_bytes) {
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
  // blocking write (when not specified I2C_STOP is implicit)
  err = Wire.endTransmission(I2C_NOSTOP);
  if (err) {
    goto handle_error;
  }

  Wire.requestFrom(slave_address, num_bytes);

  if (Wire.available() != num_bytes) {
    err = 3;
    goto handle_error;
  }

  for (int i = 0; i < num_bytes; i++) {
    data[i] = Wire.read();
  }

  return 0;

handle_error:
  while (Wire.available() != 0) {
    Wire.read();
  }
  return err;
}

int I2CMaster_T4::read_uint16(int slave_address, int register_address,
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
  // blocking write (when not specified I2C_STOP is implicit)
  err = Wire.endTransmission(I2C_NOSTOP);
  if (err) {
    goto handle_error;
  }

  Wire.requestFrom(slave_address, 2);

  if (Wire.available() != 2) {
    err = 3;
    goto handle_error;
  }

  // Assume MSB is transfered first
  data = Wire.read() << 8;
  data = data | Wire.read();

  return 0;

handle_error:
  while (Wire.available() != 0) {
    Wire.read();
  }
  return err;
}

int I2CMaster_T4::read_uint8(int slave_address, int register_address,
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
  // blocking write (when not specified I2C_STOP is implicit)
  err = Wire.endTransmission(I2C_NOSTOP);
  if (err) {
    goto handle_error;
  }

  Wire.requestFrom(slave_address, 1);

  if (Wire.available() != 1) {
    err = 3;
    goto handle_error;
  }

  data = Wire.read();

  return 0;

handle_error:
  while (Wire.available() != 0) {
    Wire.read();
  }
  return err;
}

int I2CMaster_T4::read_no_register_uint8(int slave_address, uint8_t &data) {
  if (!this->is_initialized)
    return ENODEV;
  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;
  uint8_t err;
  data = 0;

  Wire.requestFrom(slave_address, 1);

  if (Wire.available() != 1) {
    err = 3;
    goto handle_error;
  }

  data = Wire.read();
  return 0;

handle_error:
  while (Wire.available() != 0) {
    Wire.read();
  }
  return err;
}

int I2CMaster_T4::read_payload_no_register(int slave_address,
                                           uint8_t *data,
                                           int num_bytes) {
  if (!this->is_initialized)
    return ENODEV;
  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;
  uint8_t err;
  memset(data, 0, num_bytes);

  Wire.requestFrom(slave_address, num_bytes);

  if (Wire.available() != num_bytes) {
    err = 3;
    goto handle_error;
  }

  for (int i=0; i<num_bytes; i++) {
    data[i] = Wire.read();
  }

  return 0;

handle_error:
  while (Wire.available() != 0) {
    Wire.read();
  }
  return err;
}

int I2CMaster_T4::read_payload_uint16(int slave_address, int register_address,
                                      uint8_t *data, int num_bytes) {
  if (!this->is_initialized)
    return ENODEV;
  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;
  // Ensure the address is in write mode
  uint8_t err;

  Wire.beginTransmission(slave_address); // slave addr
  // Write the MSB of the address first
  Wire.write((register_address >> 8) & 0xFF);
  Wire.write((register_address >> 0) & 0xFF);
  // blocking write (when not specified I2C_STOP is implicit)
  err = Wire.endTransmission(I2C_NOSTOP);

  Wire.requestFrom(slave_address, num_bytes);

  if (Wire.available() != num_bytes) {
    err = 3;
    goto handle_error;
  }

  for (int i = 0; i < num_bytes; i++) {
    data[i] = Wire.read();
  }

  return 0;

handle_error:
  while (Wire.available() != 0) {
    Wire.read();
  }
  return err;
}
#endif
