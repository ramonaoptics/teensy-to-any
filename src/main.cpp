#include "commandconstants.hpp"
#include "commandrouting.hpp"
#include "i2c.hpp"
#include <Arduino.h>
#include <errno.h>
#include <i2c_t3.h>
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  cmd.init(command_list, 10, 1024);
}

I2CMaster i2c;

int info_func(CommandRouter *cmd, int argc, const char **argv) {
  (void)cmd;
  (void)argc;
  (void)argv;
  Serial.print("Ramona Optics I2C Debugger\n");
  return 0;
}

int reboot_func(CommandRouter *cmd, int argc, const char **argv) {
  (void)cmd;
  (void)argc;
  (void)argv;
  _reboot_Teensyduino_();
  // Does nto get here
  return 0;
}

int version_func(CommandRouter *cmd, int argc, const char **argv) {
  (void)cmd;
  (void)argc;
  (void)argv;
  Serial.print("0.0.0\n");
  return 0;
}

int i2c_init(CommandRouter *cmd, int argc, const char **argv) {
  (void)cmd;
  int baudrate = 100000;
  int timeout_ms = 200000; // 200ms
  int address_size = 2;
  int address_msb_first = false;
  if (argc >= 2) {
    baudrate = atoi(argv[1]);
  }
  if (argc >= 3) {
    timeout_ms = atoi(argv[2]);
  }
  if (argc >= 3) {
    address_size = atoi(argv[3]);
  }
  if (argc >= 4) {
    address_msb_first = atoi(argv[4]);
  }

  return i2c.init(baudrate, timeout_ms, address_size, address_msb_first);
}

int i2c_reset(CommandRouter *cmd, int argc, const char **argv) {
  return i2c.reset();
}

int i2c_write_uint16(CommandRouter *cmd, int argc, const char **argv) {
  if (argc < 4)
    return EINVAL;

  int slave_address = atoi(argv[1]);
  int register_address = atoi(argv[2]);
  uint16_t data = atoi(argv[3]);

  return i2c.write_uint16(slave_address, register_address, data);
}

int i2c_read_uint16(CommandRouter *cmd, int argc, const char **argv) {
  if (argc < 3)
    return EINVAL;

  int slave_address = atoi(argv[1]);
  int register_address = atoi(argv[2]);
  uint16_t data;
  int result;

  result = i2c.read_uint16(slave_address, register_address, data);
  if (result == 0) {
    Serial.printf("0x%40x\n", data);
  }
  return result;
}

int gpio_pin_mode(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 3)
    return EINVAL;

  uint8_t pin = atoi(argv[1]);
  uint8_t mode = atoi(argv[2]);
  if (mode != OUTPUT && mode != INPUT)
    return EINVAL;

  pinMode(pin, mode);
  return 0;
}
int gpio_digital_write(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 3)
    return EINVAL;

  uint8_t pin = atoi(argv[1]);
  uint8_t value = atoi(argv[2]);
  if (value != HIGH && value != LOW)
    return EINVAL;

  digitalWrite(pin, value);
  return 0;
}

int gpio_digital_read(CommandRouter *cmd, int argc, const char **argv) {
  if (argc < 2)
    return EINVAL;

  uint8_t pin = atoi(argv[1]);

  uint8_t value = digitalRead(pin);
  Serial.print(value);
  Serial.print("\n");
  return 0;
}

void loop_() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_BUILTIN, HIGH);
  delay(250);
  digitalWrite(LED_BUILTIN, LOW);
  delay(250);
}

void loop() {
  if (Serial.available()) {
    cmd.processSerialStream();
  }
}