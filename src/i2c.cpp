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
  this->wire->begin(I2C_MASTER, 0x00, I2C_PINS_18_19, I2C_PULLUP_EXT, baudrate);
  this->wire->setDefaultTimeout(timeout_ms);
  this->address_msb_first = address_msb_first;
  this->address_size = address_size;
  this->is_initialized = true;
  return 0;
}

int I2CMaster::reset() {
  if (!this->is_initialized)
    return ENODEV;
  this->wire->resetBus();
  return 0;
}

void I2CMaster::_write_register_address(int register_address) {
  if (this->address_size == 2) {
    this->wire->write((register_address >> 8) & 0xFF);
    this->wire->write((register_address >> 0) & 0xFF);
  } else if (this->address_size == 1) {
    this->wire->write((register_address >> 0) & 0xFF);
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
  this->wire->beginTransmission(slave_address); // slave addr

  this->_write_register_address(register_address);
  // Assume MSB first for data as well
  this->wire->write((data >> 8) & 0xFF);
  this->wire->write((data >> 0) & 0xFF);
  // blocking write (when not specified I2C_STOP is implicit)
  this->wire->endTransmission(I2C_STOP);

  return this->wire->getError();
}

int I2CMaster::write_uint8(int slave_address, int register_address,
                           uint8_t data) {
  if (!this->is_initialized)
    return ENODEV;
  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;
  // Ensure the address is in write mode
  this->wire->beginTransmission(slave_address); // slave addr
  this->_write_register_address(register_address);
  this->wire->write((data >> 0) & 0xFF);
  // blocking write (when not specified I2C_STOP is implicit)
  this->wire->endTransmission(I2C_STOP);

  return this->wire->getError();
}

int I2CMaster::write_no_register_uint8(int slave_address, uint8_t data) {
  if (!this->is_initialized)
    return ENODEV;

  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;

  this->wire->beginTransmission(slave_address); // slave addr
  this->wire->write(data);
  // blocking write (when not specified I2C_STOP is implicit)
  this->wire->endTransmission(I2C_STOP);

  return this->wire->getError();
}

int I2CMaster::write_payload(int slave_address, int register_address,
                             uint8_t *data, int num_bytes) {
  if (!this->is_initialized)
    return ENODEV;

  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;

  this->wire->beginTransmission(slave_address); // slave addr
  this->_write_register_address(register_address);
  this->wire->write(data, num_bytes);
  // blocking write (when not specified I2C_STOP is implicit)
  this->wire->endTransmission(I2C_STOP);

  return this->wire->getError();
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

  this->wire->beginTransmission(slave_address); // slave addr
  // Write the MSB of the address first
  this->_write_register_address(register_address);
  this->wire->endTransmission(
      I2C_NOSTOP); // blocking write (when not specified I2C_STOP is implicit)
  err = this->wire->getError();
  if (err) {
    goto handle_error;
  }

  this->wire->requestFrom(slave_address, num_bytes);
  this->wire->finish();
  err = this->wire->getError();
  if (err) {
    goto handle_error;
  }

  if (this->wire->available() != num_bytes) {
    err = 3;
    goto handle_error;
  }

  for (int i = 0; i < num_bytes; i++) {
    data[i] = this->wire->readByte();
  }

  return 0;

handle_error:
  while (this->wire->available() != 0) {
    this->wire->readByte();
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

  this->wire->beginTransmission(slave_address); // slave addr
  // Write the MSB of the address first
  this->_write_register_address(register_address);
  this->wire->endTransmission(
      I2C_NOSTOP); // blocking write (when not specified I2C_STOP is implicit)
  err = this->wire->getError();
  if (err) {
    goto handle_error;
  }

  this->wire->requestFrom(slave_address, 2);
  this->wire->finish();
  err = this->wire->getError();
  if (err) {
    goto handle_error;
  }

  if (this->wire->available() != 2) {
    err = 3;
    goto handle_error;
  }

  // Assume MSB is transfered first
  data = this->wire->readByte() << 8;
  data = data | this->wire->readByte();

  return 0;

handle_error:
  while (this->wire->available() != 0) {
    this->wire->readByte();
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

  this->wire->beginTransmission(slave_address); // slave addr
  // Write the MSB of the address first
  this->_write_register_address(register_address);
  // blocking write (when not specified I2C_STOP is implicit)
  this->wire->endTransmission(I2C_NOSTOP);
  err = this->wire->getError();
  if (err) {
    goto handle_error;
  }

  this->wire->requestFrom(slave_address, 1);
  this->wire->finish();
  err = this->wire->getError();
  if (err) {
    goto handle_error;
  }

  if (this->wire->available() != 1) {
    err = 3;
    goto handle_error;
  }

  data = this->wire->readByte();

  return 0;

handle_error:
  while (this->wire->available() != 0) {
    this->wire->readByte();
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

  this->wire->requestFrom(slave_address, 1);
  this->wire->finish();
  err = this->wire->getError();
  if (err) {
    goto handle_error;
  }

  if (this->wire->available() != 1) {
    err = 3;
    goto handle_error;
  }

  data = this->wire->readByte();
  return 0;

handle_error:
  while (this->wire->available() != 0) {
    this->wire->readByte();
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

  this->wire->requestFrom(slave_address, num_bytes);
  this->wire->finish();
  err = this->wire->getError();
  if (err) {
    goto handle_error;
  }

  if (this->wire->available() != num_bytes) {
    err = 3;
    goto handle_error;
  }

  for (int i=0; i<num_bytes; i++) {
    data[i] = this->wire->readByte();
  }

  return 0;

handle_error:
  while (this->wire->available() != 0) {
    this->wire->readByte();
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

  this->wire->beginTransmission(slave_address); // slave addr
  // Write the MSB of the address first
  this->wire->write((register_address >> 8) & 0xFF);
  this->wire->write((register_address >> 0) & 0xFF);
  // blocking write (when not specified I2C_STOP is implicit)
  this->wire->endTransmission(I2C_NOSTOP);
  err = this->wire->getError();
  if (err) {
    goto handle_error;
  }

  this->wire->requestFrom(slave_address, num_bytes);
  this->wire->finish();
  err = this->wire->getError();
  if (err) {
    goto handle_error;
  }

  if (this->wire->available() != num_bytes) {
    err = 3;
    goto handle_error;
  }

  for (int i = 0; i < num_bytes; i++) {
    data[i] = this->wire->readByte();
  }

  return 0;

handle_error:
  while (this->wire->available() != 0) {
    this->wire->readByte();
  }
  return err;
}

int I2CMaster::ping(int slave_address) {
  if (!this->is_initialized)
    return ENODEV;
  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;
  // Ensure the address is in write mode
  uint8_t err;

  this->wire->beginTransmission(slave_address); // slave addr
  // Write the MSB of the address first
  // blocking write (when not specified I2C_STOP is implicit)
  err = this->wire->endTransmission(I2C_STOP);
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
  // this->wire->timeout_millis = timeout_ms;
  // this->wire->setDefaultTimeout(timeout_ms);
  this->wire->setClock(baudrate);
  this->wire->begin();

  this->address_msb_first = address_msb_first;
  this->address_size = address_size;
  this->is_initialized = true;
  return 0;
}

int I2CMaster_T4::reset() {
  if (!this->is_initialized)
    return ENODEV;

  // Doesn't seem to exist. but maybe we can recreate with end/begin
  // this->wire->resetBus()
  this->wire->end();
  this->wire->begin();

  return 0;
}

void I2CMaster_T4::_write_register_address(int register_address) {
  if (this->address_size == 2) {
    this->wire->write((register_address >> 8) & 0xFF);
    this->wire->write((register_address >> 0) & 0xFF);
  } else if (this->address_size == 1) {
    this->wire->write((register_address >> 0) & 0xFF);
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
  this->wire->beginTransmission(slave_address); // slave addr

  this->_write_register_address(register_address);
  // Assume MSB first for data as well
  this->wire->write((data >> 8) & 0xFF);
  this->wire->write((data >> 0) & 0xFF);
  // blocking write (when not specified I2C_STOP is implicit)
  error = this->wire->endTransmission(I2C_STOP);

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
  this->wire->beginTransmission(slave_address); // slave addr
  this->_write_register_address(register_address);
  this->wire->write((data >> 0) & 0xFF);
  // blocking write (when not specified I2C_STOP is implicit)
  return this->wire->endTransmission(I2C_STOP);
}

int I2CMaster_T4::write_no_register_uint8(int slave_address, uint8_t data) {
  if (!this->is_initialized)
    return ENODEV;

  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;

  this->wire->beginTransmission(slave_address); // slave addr
  this->wire->write(data);
  // blocking write (when not specified I2C_STOP is implicit)
  return this->wire->endTransmission(I2C_STOP);
}

int I2CMaster_T4::write_payload(int slave_address, int register_address,
                             uint8_t *data, int num_bytes) {
  if (!this->is_initialized)
    return ENODEV;

  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;

  this->wire->beginTransmission(slave_address); // slave addr
  this->_write_register_address(register_address);
  this->wire->write(data, num_bytes);
  // blocking write (when not specified I2C_STOP is implicit)
  return this->wire->endTransmission(I2C_STOP);
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

  this->wire->beginTransmission(slave_address); // slave addr
  // Write the MSB of the address first
  this->_write_register_address(register_address);
  // blocking write (when not specified I2C_STOP is implicit)
  err = this->wire->endTransmission(I2C_NOSTOP);
  if (err) {
    goto handle_error;
  }

  this->wire->requestFrom(slave_address, num_bytes);

  if (this->wire->available() != num_bytes) {
    err = 3;
    goto handle_error;
  }

  for (int i = 0; i < num_bytes; i++) {
    data[i] = this->wire->read();
  }

  return 0;

handle_error:
  while (this->wire->available() != 0) {
    this->wire->read();
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

  this->wire->beginTransmission(slave_address); // slave addr
  // Write the MSB of the address first
  this->_write_register_address(register_address);
  // blocking write (when not specified I2C_STOP is implicit)
  err = this->wire->endTransmission(I2C_NOSTOP);
  if (err) {
    goto handle_error;
  }

  this->wire->requestFrom(slave_address, 2);

  if (this->wire->available() != 2) {
    err = 3;
    goto handle_error;
  }

  // Assume MSB is transfered first
  data = this->wire->read() << 8;
  data = data | this->wire->read();

  return 0;

handle_error:
  while (this->wire->available() != 0) {
    this->wire->read();
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

  this->wire->beginTransmission(slave_address); // slave addr
  // Write the MSB of the address first
  this->_write_register_address(register_address);
  // blocking write (when not specified I2C_STOP is implicit)
  err = this->wire->endTransmission(I2C_NOSTOP);
  if (err) {
    goto handle_error;
  }

  this->wire->requestFrom(slave_address, 1);

  if (this->wire->available() != 1) {
    err = 3;
    goto handle_error;
  }

  data = this->wire->read();

  return 0;

handle_error:
  while (this->wire->available() != 0) {
    this->wire->read();
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

  this->wire->requestFrom(slave_address, 1);

  if (this->wire->available() != 1) {
    err = 3;
    goto handle_error;
  }

  data = this->wire->read();
  return 0;

handle_error:
  while (this->wire->available() != 0) {
    this->wire->read();
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

  this->wire->requestFrom(slave_address, num_bytes);

  if (this->wire->available() != num_bytes) {
    err = 3;
    goto handle_error;
  }

  for (int i=0; i<num_bytes; i++) {
    data[i] = this->wire->read();
  }

  return 0;

handle_error:
  while (this->wire->available() != 0) {
    this->wire->read();
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

  this->wire->beginTransmission(slave_address); // slave addr
  // Write the MSB of the address first
  this->wire->write((register_address >> 8) & 0xFF);
  this->wire->write((register_address >> 0) & 0xFF);
  // blocking write (when not specified I2C_STOP is implicit)
  err = this->wire->endTransmission(I2C_NOSTOP);

  this->wire->requestFrom(slave_address, num_bytes);

  if (this->wire->available() != num_bytes) {
    err = 3;
    goto handle_error;
  }

  for (int i = 0; i < num_bytes; i++) {
    data[i] = this->wire->read();
  }

  return 0;

handle_error:
  while (this->wire->available() != 0) {
    this->wire->read();
  }
  return err;
}

int I2CMaster_T4::ping(int slave_address) {
  if (!this->is_initialized)
    return ENODEV;
  // Wire library uses 7 bit addresses
  if (slave_8bit_address)
    slave_address = slave_address >> 1;
  // Ensure the address is in write mode
  uint8_t err;

  this->wire->beginTransmission(slave_address); // slave addr
  // Write the MSB of the address first
  // blocking write (when not specified I2C_STOP is implicit)
  err = this->wire->endTransmission(I2C_STOP);
  return err;
}

#endif
