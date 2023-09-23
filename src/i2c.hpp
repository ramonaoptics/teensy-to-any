#pragma once
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
