#include "commandconstants.hpp"
#include "commandrouting.hpp"
#include "i2c.hpp"
#include <Arduino.h>
#include <errno.h>

I2CMaster i2c;

void setup() {
  // Pause for 100 MS in order to debounce the power supply getting
  // plugged in.
  delay(100);
  Serial.begin(115200);
  cmd.init(command_list, 10, 1024);
}

int info_func(CommandRouter *cmd, int argc, const char **argv) {
  (void)argc;
  (void)argv;
  snprintf(cmd->buffer, cmd->buffer_size, "Ramona Optics I2C Debugger");
  return 0;
}

int reboot_func(CommandRouter *cmd, int argc, const char **argv) {
  cmd->buffer[0] = '\0';
  (void)argc;
  (void)argv;
  return ENOSYS;

  // This reboots you into the programming mode
  _reboot_Teensyduino_();
  // Does nto get here
  return 0;
}

int version_func(CommandRouter *cmd, int argc, const char **argv) {
  (void)argc;
  (void)argv;
  snprintf(cmd->buffer, cmd->buffer_size, "0.0.0");
  return 0;
}

int i2c_init(CommandRouter *cmd, int argc, const char **argv) {
  cmd->buffer[0] = '\0';
  ;
  int baudrate = 100000;
  int timeout_ms = 200000; // 200ms
  int address_size = 2;
  int address_msb_first = false;
  if (argc >= 2) {
    baudrate = strtol(argv[1], nullptr, 0);
  }
  if (argc >= 3) {
    timeout_ms = strtol(argv[2], nullptr, 0);
  }
  if (argc >= 4) {
    address_size = strtol(argv[3], nullptr, 0);
  }
  if (argc >= 5) {
    address_msb_first = strtol(argv[4], nullptr, 0);
  }

  return i2c.init(baudrate, timeout_ms, address_size, address_msb_first);
}

int i2c_reset(CommandRouter *cmd, int argc, const char **argv) {
  cmd->buffer[0] = '\0';
  return i2c.reset();
}

int i2c_write_uint16(CommandRouter *cmd, int argc, const char **argv) {
  cmd->buffer[0] = '\0';
  if (argc != 4)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);
  uint16_t data = strtol(argv[3], nullptr, 0);

  return i2c.write_uint16(slave_address, register_address, data);
}

int i2c_read_uint16(CommandRouter *cmd, int argc, const char **argv) {
  cmd->buffer[0] = '\0';
  if (argc != 3)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);
  uint16_t data;
  int result;

  result = i2c.read_uint16(slave_address, register_address, data);

  if (result == 0) {
    snprintf(cmd->buffer, cmd->buffer_size, "0x%04X", data);
  }
  return result;
}

int i2c_read_no_register_uint8(CommandRouter *cmd, int argc,
                               const char **argv) {
  cmd->buffer[0] = '\0';
  if (argc != 2)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  uint8_t data;
  int result;
  result = i2c.read_no_register_uint8(slave_address, data);
  if (result == 0) {
    snprintf(cmd->buffer, cmd->buffer_size, "0x%02X", data);
  }
  return result;
}

int i2c_write_no_register_uint8(CommandRouter *cmd, int argc,
                                const char **argv) {
  cmd->buffer[0] = '\0';
  if (argc != 3)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  uint8_t data = strtol(argv[2], nullptr, 0);
  return i2c.write_no_register_uint8(slave_address, data);
}

int gpio_pin_mode(CommandRouter *cmd, int argc, const char **argv) {
  cmd->buffer[0] = '\0';
  if (argc != 3)
    return EINVAL;

  uint8_t pin = strtol(argv[1], nullptr, 0);
  uint8_t mode = strtol(argv[2], nullptr, 0);
  if ((mode != OUTPUT) && (mode != INPUT))
    return EINVAL;

  pinMode(pin, mode);
  return 0;
}
int gpio_digital_write(CommandRouter *cmd, int argc, const char **argv) {
  cmd->buffer[0] = '\0';
  if (argc != 3)
    return EINVAL;

  uint8_t pin = strtol(argv[1], nullptr, 0);
  uint8_t value = strtol(argv[2], nullptr, 0);
  if ((value != HIGH) && (value != LOW))
    return EINVAL;

  digitalWrite(pin, value);
  return 0;
}

int gpio_digital_read(CommandRouter *cmd, int argc, const char **argv) {
  cmd->buffer[0] = '\0';
  if (argc != 2)
    return EINVAL;

  uint8_t pin = strtol(argv[1], nullptr, 0);
  uint8_t value = digitalRead(pin);

  snprintf(cmd->buffer, cmd->buffer_size, "%d", value);
  return 0;
}

void loop() {
  // TODO: remove this check on if Serial is available.
  // I don't think we need it since it is already in a loop
  if (Serial.available()) {
    cmd.processSerialStream();
  }
}