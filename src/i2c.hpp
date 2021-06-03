#pragma once
// only support teensies 3.1, 3.2, 3.5, and 3.6
// See list of microcontroller units
// https://docs.platformio.org/en/latest/platforms/teensy.html
#if defined(TEENSYDUINO) && (defined(__MK20DX256__) ||                         \
                             defined(__MK64FX512__) || defined(__MK66FX1M0__))
#define TEENSY_TO_ANY_HAS_I2C 1
#else
#define TEENSY_TO_ANY_HAS_I2C 0
#warning I2C library not available for this build.
#endif

#if TEENSY_TO_ANY_HAS_I2C
#include <unistd.h>

class I2CMaster {
public:
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

private:
  void _write_register_address(int register_address);
  int address_msb_first = 0;
  int address_size = 0;
  bool is_initialized = false;
  bool slave_8bit_address = true;
};
#endif
