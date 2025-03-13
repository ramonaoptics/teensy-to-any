#pragma once
// only support teensies 3.1, 3.2, 3.5, and 3.6
// See list of microcontroller units
// https://docs.platformio.org/en/latest/platforms/teensy.html
#if defined(TEENSYDUINO) && (defined(__MK20DX256__) ||                         \
                             defined(__MK64FX512__) || defined(__MK66FX1M0__))
#define TEENSY_TO_ANY_HAS_I2C_T3 1
#else
#define TEENSY_TO_ANY_HAS_I2C_T3 0
#endif

#if defined(TEENSYDUINO) && defined(__IMXRT1062__)
#define TEENSY_TO_ANY_HAS_I2C_T4 1
#else
#define TEENSY_TO_ANY_HAS_I2C_T4 0
#endif

#if !(TEENSY_TO_ANY_HAS_I2C_T3 || TEENSY_TO_ANY_HAS_I2C_T4)
#error I2C library not available for this build.
#endif

#include <unistd.h>

#if TEENSY_TO_ANY_HAS_I2C_T3

#include <i2c_t3.h>
class I2CMaster {
public:
  I2CMaster(i2c_t3* wire_instance) : wire(wire_instance) {}

  int init(int baudrate, int timeout_ms, int address_size,
           int address_msb_first);
  int reset();
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

private:
  void _write_register_address(int register_address);
  int address_msb_first = 0;
  int address_size = 0;
  bool is_initialized = false;
  bool slave_8bit_address = true;
  i2c_t3 *wire;
};
#endif

#if TEENSY_TO_ANY_HAS_I2C_T4

#include <i2c_driver_wire.h>

// Make this a template class that takes in a WIRE object
class I2CMaster_T4{
public:
  I2CMaster_T4(I2CDriverWire* wire_instance) : wire(wire_instance) {}
  int init(int baudrate, int timeout_ms, int address_size,
    int address_msb_first);
  int reset();
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

private:
  void _write_register_address(int register_address);
  int address_msb_first = 0;
  int address_size = 0;
  bool is_initialized = false;
  bool slave_8bit_address = true;
  I2CDriverWire *wire;
};
#endif
