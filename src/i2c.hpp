#pragma once
// I2C support for Teensy 4.x (i.MX RT1062).
// See list of microcontroller units
// https://docs.platformio.org/en/latest/platforms/teensy.html
#if defined(TEENSYDUINO) && defined(__IMXRT1062__)
#define TEENSY_TO_ANY_HAS_I2C_T4 1
#else
#define TEENSY_TO_ANY_HAS_I2C_T4 0
#endif

#if !TEENSY_TO_ANY_HAS_I2C_T4
#error I2C library not available for this build.
#endif

#include <unistd.h>

#if TEENSY_TO_ANY_HAS_I2C_T4

#include <i2c_driver_wire.h>

// Make this a template class that takes in a WIRE object
class I2CMaster_T4{
public:
  I2CMaster_T4(I2CDriverWire* wire_instance) : wire(wire_instance) {}
  int init(int baudrate, int timeout_ms, int address_size,
    int address_msb_first);
  int reset();
  int begin_transaction(int slave_address);
  int write(uint8_t *data, int num_bytes);
  int end_transaction(bool stop = true);
  int write_uint16(int slave_address, int register_address, uint16_t data);
  int read_uint16(int slave_address, int register_address, uint16_t &data);
  int write_uint8(int slave_address, int register_address, uint8_t data);
  int read_uint8(int slave_address, int register_address, uint8_t &data);
  int read_no_register_uint8(int slave_address, uint8_t &data);
  int write_no_register_uint8(int slave_address, uint8_t data);
  int write_payload(int slave_address, int register_address, uint8_t *data,
                    int num_bytes);
  int read_payload(int slave_address, int register_address, uint8_t *data,
                   int num_bytes);
  int read_payload_no_register(int slave_address, uint8_t *data, int num_bytes);
  int read_payload_uint16(int slave_address, int register_address,
                          uint8_t *data, int num_bytes);
  int ping(int slave_address);

private:
  void _write_register_address(int register_address);
  int address_msb_first = 0;
  int address_size = 0;
  bool is_initialized = false;
  bool slave_8bit_address = true;
  I2CDriverWire *wire;
};
#endif
