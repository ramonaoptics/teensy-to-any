#include "commandconstants.hpp"
#include "commandrouting.hpp"
#include "i2c.hpp"
#include "startup_commands.hpp"
#include <Arduino.h>
#include <SPI.h>
#include <errno.h>
#include <usb_names.h>
#include <EEPROM.h>

// TODO: this isn't exactly correct since this main file won't get
// regenerated if it wasn't touched.
#ifndef GIT_DESCRIBE
#define GIT_DESCRIBE "0.0.0-unknown"
#endif


#define USE_STATIC_ALLOCATION 1
#if USE_STATIC_ALLOCATION
#define BUFFER_SIZE 1024 * 16
#define ARGV_MAX 300
char serial_buffer[BUFFER_SIZE];
const char *argv_buffer[ARGV_MAX];
#endif
// Default SPI Settings
uint32_t spi_baudrate = 4'000'000;
uint8_t spi_bit_order = MSBFIRST;
uint8_t spi_data_mode = SPI_MODE0;

inline SPISettings my_spi_settings() {
  // Gotta hate this really weird API
  return SPISettings(spi_baudrate, spi_bit_order, spi_data_mode);
}

#include <Adafruit_NeoPixel.h>
#define NEOPIXEL_PIN     19
#define NEOPIXEL_NUMLEDS     5
#define NEOPIXEL_BRIGHTNESS  50 // Set BRIGHTNESS to about 1/5 (max = 255)

Adafruit_NeoPixel neopixel_strip(
    NEOPIXEL_NUMLEDS,
    NEOPIXEL_PIN,
    NEO_GRBW + NEO_KHZ800
);

#if TEENSY_TO_ANY_HAS_I2C_T3
I2CMaster i2c;
#endif
#if TEENSY_TO_ANY_HAS_I2C_T4
I2CMaster_T4 i2c;
#endif

// Teensy4 has 1080 bytes of EEPROM, use the last byte to store the
// demo command enabled flag
#define DEMO_COMMAND_ENABLED_ADDRESS 1079


int len_startup_commands;
int len_demo_commands;

void setup_startup_and_demo_commands() {
  len_startup_commands = 0;
  for (int i = 0; teensy_to_any_startup_commands[i] != nullptr; i++) {
    len_startup_commands++;
  }
  len_demo_commands = 0;
  for (int i = 0; teensy_to_any_demo_commands[i] != nullptr; i++) {
    len_demo_commands++;
  }
}

void execute_startup_commands() {
  for (int i = 0; teensy_to_any_startup_commands[i] != nullptr; i++) {
    cmd.processString(teensy_to_any_startup_commands[i]);
  }
}

void execute_demo_commands() {
  int demo_enabled = EEPROM.read(DEMO_COMMAND_ENABLED_ADDRESS) == 0xFF;
  if (!demo_enabled) {
    return;
  }
  // No demo programs defined
  if (teensy_to_any_demo_commands[0] == nullptr) {
    return;
  }
  while (true) {
    for (int i = 0; teensy_to_any_demo_commands[i] != nullptr; i++) {
      cmd.processString(teensy_to_any_demo_commands[i]);
      if (Serial.available()) {
        return;
      }
    }
  }
}

int disable_demo_commands(CommandRouter *cmd, int argc, const char **argv) {
  (void)argv;
  if (argc != 1) {
    return EINVAL;
  }
  EEPROM.write(DEMO_COMMAND_ENABLED_ADDRESS, 0);
  // Mark - 2024/11/10
  // I've noticed that if we don't write the EEPROM twice the value
  // isn't guaranteed to be written....
  EEPROM.write(DEMO_COMMAND_ENABLED_ADDRESS, 0);
  return 0;
}

int enable_demo_commands(CommandRouter *cmd, int argc, const char **argv) {
  (void)argv;
  if (argc != 1) {
    return EINVAL;
  }
  EEPROM.write(DEMO_COMMAND_ENABLED_ADDRESS, 0xFF);
  // Mark - 2024/11/10
  // I've noticed that if we don't write the EEPROM twice the value
  // isn't guaranteed to be written....
  EEPROM.write(DEMO_COMMAND_ENABLED_ADDRESS, 0xFF);
  return 0;
}
int demo_commands_enabled(CommandRouter *cmd, int argc, const char **argv) {
  (void)argv;
  if (argc != 1) {
    return EINVAL;
  }
  int demo_enabled = EEPROM.read(DEMO_COMMAND_ENABLED_ADDRESS) == 0xFF;
  snprintf(cmd->buffer, cmd->buffer_size, "%d", demo_enabled);
  return 0;
}

void setup() {
  // Pause for 100 MS in order to debounce the power supply getting
  // plugged in.
  delay(100);

#if USE_STATIC_ALLOCATION
  cmd.init_no_malloc(command_list, BUFFER_SIZE, serial_buffer, ARGV_MAX,
                     argv_buffer);
#else
  cmd.init(command_list, 1024, 10);
#endif
  setup_startup_and_demo_commands();
  execute_startup_commands();

  // Starting serial seems to be slow, so do it at the end
  // See Delays section in
  // https://www.pjrc.com/teensy/td_startup.html
  Serial.begin(115'200);

  execute_demo_commands();
}

int startup_commands_available(CommandRouter *cmd, int argc, const char **argv){
  snprintf(cmd->buffer, cmd->buffer_size, "%d", len_startup_commands);
  return 0;
}

int read_startup_command(CommandRouter *cmd, int argc, const char **argv){
  if (argc != 2)
    return EINVAL;
  int index = strtol(argv[1], nullptr, 0);
  if (index < 0)
    return EINVAL;
  if (index >= len_startup_commands)
    return EINVAL;
  const char *command = teensy_to_any_startup_commands[index];
  if (command == nullptr)
    return EINVAL;
  snprintf(cmd->buffer, cmd->buffer_size, "%s", command);
  return 0;
}

int demo_commands_available(CommandRouter *cmd, int argc, const char **argv){
  snprintf(cmd->buffer, cmd->buffer_size, "%d", len_demo_commands);
  return 0;
}

int read_demo_command(CommandRouter *cmd, int argc, const char **argv){
  if (argc != 2)
    return EINVAL;
  int index = strtol(argv[1], nullptr, 0);
  if (index < 0)
    return EINVAL;
  if (index >= len_demo_commands)
    return EINVAL;
  const char *command = teensy_to_any_demo_commands[index];
  if (command == nullptr)
    return EINVAL;
  snprintf(cmd->buffer, cmd->buffer_size, "%s", command);
  return 0;
}

int info_func(CommandRouter *cmd, int argc, const char **argv) {
  (void)argc;
  (void)argv;
  snprintf(cmd->buffer, cmd->buffer_size, "Ramona Optics Teensy Debugger");
  return 0;
}

int reboot_func(CommandRouter *cmd, int argc, const char **argv) {
  (void)argc;
  (void)argv;
  // This function was never tested to work....
  return ENOSYS;

  // Maybe look into
  // https://forum.pjrc.com/index.php?threads/soft-reboot-on-teensy4-0.57810/
  // or
  // https://forum.pjrc.com/index.php?threads/wdt_t4-watchdog-library-for-teensy-4.59257/
  // for the teensy 4
  // This reboots you into the programming mode not the normal mode which we care about...
  _reboot_Teensyduino_();
  // Does not get here
  return 0;
}

int version_func(CommandRouter *cmd, int argc, const char **argv) {
  (void)argc;
  (void)argv;
  snprintf(cmd->buffer, cmd->buffer_size, GIT_DESCRIBE);
  return 0;
}

int serialnumber_func(CommandRouter *cmd, int argc, const char **argv) {
  (void)argc;
  (void)argv;
  uint8_t i;
  // https://github.com/PaulStoffregen/cores/pull/722
#pragma GCC diagnostic push
  // https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
#pragma GCC diagnostic ignored "-Warray-bounds"
  for (i=0; i < (usb_string_serial_number.bLength - 2) / sizeof(uint16_t); i++) {
    cmd->buffer[i] = (char)usb_string_serial_number.wString[i];
  }
#pragma GCC diagnostic pop
  cmd->buffer[i] = '\0';
  return 0;
}


int mcu_func(CommandRouter *cmd, int argc, const char **argv) {
  (void)argc;
  (void)argv;

#if (TEENSYDUINO) && defined(__IMXRT1062__)
  snprintf(cmd->buffer, cmd->buffer_size, "TEENSY40");
#elif defined(TEENSYDUINO) && (defined(__MK20DX256__) ||  \
                               defined(__MK64FX512__) ||  \
                               defined(__MK66FX1M0__))
  snprintf(cmd->buffer, cmd->buffer_size, "TEENSY32");
#else
  snprintf(cmd->buffer, cmd->buffer_size, "UNKNOWN");
#endif
  return 0;
}


int i2c_init(CommandRouter *cmd, int argc, const char **argv) {
#if TEENSY_TO_ANY_HAS_I2C
  int baudrate = 100'000;
  int timeout_ms = 200'000; // 200ms
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
#else
  return -1;
#endif
}

int i2c_reset(CommandRouter *cmd, int argc, const char **argv) {
#if TEENSY_TO_ANY_HAS_I2C
  return i2c.reset();
#else
  return -1;
#endif
}

int i2c_write_uint16(CommandRouter *cmd, int argc, const char **argv) {
#if TEENSY_TO_ANY_HAS_I2C
  if (argc != 4)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);
  uint16_t data = strtol(argv[3], nullptr, 0);
  return i2c.write_uint16(slave_address, register_address, data);
#else
  return -1;
#endif
}

int i2c_write_uint8(CommandRouter *cmd, int argc, const char **argv) {
#if TEENSY_TO_ANY_HAS_I2C
  if (argc != 4)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);
  uint8_t data = strtol(argv[3], nullptr, 0);

  return i2c.write_uint8(slave_address, register_address, data);
#else
  return -1;
#endif
}

int i2c_write_payload(CommandRouter *cmd, int argc, const char **argv) {
#if TEENSY_TO_ANY_HAS_I2C
  const int num_bytes_max = 8;
  uint8_t data[num_bytes_max];
  if (argc < 4)
    return EINVAL;

  int num_bytes = argc - 3;
  if (num_bytes > num_bytes_max)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);

  for (int i = 0; i < num_bytes; i++) {
    data[i] = strtol(argv[i + 3], nullptr, 0);
  }

  return i2c.write_payload(slave_address, register_address, data, num_bytes);

#else
  return -1;
#endif
}

int i2c_read_payload(CommandRouter *cmd, int argc, const char **argv) {
#if TEENSY_TO_ANY_HAS_I2C
  const int num_bytes_max = 8;
  if (argc != 4)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);
  int num_bytes = strtol(argv[3], nullptr, 0);
  if (num_bytes > num_bytes_max)
    return EINVAL;

  uint8_t data[8];
  int result;
  result = i2c.read_payload(slave_address, register_address, data, num_bytes);

  if (result == 0) {
    for (int i = 0; i < num_bytes; i++) {
      if (i == 0) {
        snprintf(cmd->buffer, cmd->buffer_size, "0x%02X", data[i]);
      } else {
        snprintf(cmd->buffer + strlen(cmd->buffer), cmd->buffer_size, "0x%02X",
                 data[i]);
      }
      if (i != num_bytes - 1) {
        snprintf(cmd->buffer + strlen(cmd->buffer), cmd->buffer_size, "%c",
                 ' ');
      }
    }
  }
  return result;
#else
  return -1;
#endif
}

int i2c_read_payload_no_register(CommandRouter *cmd, int argc, const char **argv) {
#if TEENSY_TO_ANY_HAS_I2C
  const int num_bytes_max = 16;
  if (argc != 3)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int num_bytes = strtol(argv[2], nullptr, 0);
  if (num_bytes > num_bytes_max)
    return EINVAL;

  uint8_t data[16];
  int result;
  result = i2c.read_payload_no_register(slave_address, data, num_bytes);

  if (result == 0) {
    for (int i = 0; i < num_bytes; i++) {
      if (i == 0) {
        snprintf(cmd->buffer, cmd->buffer_size, "0x%02X", data[i]);
      } else {
        snprintf(cmd->buffer + strlen(cmd->buffer), cmd->buffer_size, "0x%02X",
                 data[i]);
      }
      if (i != num_bytes - 1) {
        snprintf(cmd->buffer + strlen(cmd->buffer), cmd->buffer_size, "%c",
                 ' ');
      }
    }
  }
  return result;
#else
  return -1;
#endif
}

int i2c_read_payload_uint16(CommandRouter *cmd, int argc, const char **argv) {
#if TEENSY_TO_ANY_HAS_I2C
  const int num_bytes_max = 16;
  if (argc != 4)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address;
  register_address = strtol(argv[2], nullptr, 0);
  int num_bytes = strtol(argv[3], nullptr, 0);
  if (num_bytes > num_bytes_max)
    return EINVAL;

  uint8_t data[16];
  int result;
  result = i2c.read_payload_uint16(slave_address, register_address,
                                   data, num_bytes);

  if (result == 0) {
    for (int i = 0; i < num_bytes; i++) {
      if (i == 0) {
        snprintf(cmd->buffer, cmd->buffer_size, "0x%02X", data[i]);
      } else {
        snprintf(cmd->buffer + strlen(cmd->buffer), cmd->buffer_size, "0x%02X",
                 data[i]);
      }
      if (i != num_bytes - 1) {
        snprintf(cmd->buffer + strlen(cmd->buffer), cmd->buffer_size, "%c",
                 ' ');
      }
    }
  }
  return result;
#else
  return -1;
#endif
}

int i2c_read_uint16(CommandRouter *cmd, int argc, const char **argv) {
#if TEENSY_TO_ANY_HAS_I2C
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
#else
  return -1;
#endif
}

int i2c_read_uint8(CommandRouter *cmd, int argc, const char **argv) {
#if TEENSY_TO_ANY_HAS_I2C
  if (argc != 3)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);
  uint8_t data;
  int result;
  result = i2c.read_uint8(slave_address, register_address, data);
  if (result == 0) {
    snprintf(cmd->buffer, cmd->buffer_size, "0x%02X", data);
  }
  return result;
#else
  return -1;
#endif
}

int i2c_read_no_register_uint8(CommandRouter *cmd, int argc,
                               const char **argv) {
#if TEENSY_TO_ANY_HAS_I2C
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
#else
  return -1;
#endif
}

int i2c_write_no_register_uint8(CommandRouter *cmd, int argc,
                                const char **argv) {
#if TEENSY_TO_ANY_HAS_I2C
  if (argc != 3)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  uint8_t data = strtol(argv[2], nullptr, 0);
  return i2c.write_no_register_uint8(slave_address, data);
#else
  return -1;
#endif
}

int gpio_pin_mode(CommandRouter *cmd, int argc, const char **argv) {
  if (argc < 3 || argc > 4)
    return EINVAL;

  uint8_t pin = strtol(argv[1], nullptr, 0);
  uint8_t mode;
  bool write_after_mode_set = argc >= 4;
  uint8_t value = LOW;

  if (strcmp("OUTPUT", argv[2]) == 0) {
    mode = OUTPUT;
  } else if (strcmp("INPUT", argv[2]) == 0) {
    mode = INPUT;
  } else {
    mode = strtol(argv[2], nullptr, 0);
  }

  if ((mode != OUTPUT) && (mode != INPUT))
    return EINVAL;

  if (write_after_mode_set) {
    if (strcmp("HIGH", argv[3]) == 0) {
      value = HIGH;
    } else if (strcmp("LOW", argv[3]) == 0) {
      value = LOW;
    } else {
      value = strtol(argv[3], nullptr, 0);
    }
    if ((value != HIGH) && (value != LOW))
      return EINVAL;
  }

  pinMode(pin, mode);
  if (write_after_mode_set){
    digitalWrite(pin, value);
  }
  return 0;
}
int gpio_digital_write(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 3)
    return EINVAL;

  uint8_t value;

  uint8_t pin = strtol(argv[1], nullptr, 0);
  if (strcmp("HIGH", argv[2]) == 0) {
    value = HIGH;
  } else if (strcmp("LOW", argv[2]) == 0) {
    value = LOW;
  } else {
    value = strtol(argv[2], nullptr, 0);
  }
  if ((value != HIGH) && (value != LOW))
    return EINVAL;

  digitalWrite(pin, value);
  return 0;
}

int gpio_digital_read(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 2)
    return EINVAL;

  uint8_t pin = strtol(argv[1], nullptr, 0);
  uint8_t value = digitalRead(pin);

  snprintf(cmd->buffer, cmd->buffer_size, "%u", value);
  return 0;
}

int gpio_digital_pulse(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 5)
    return EINVAL;

  uint8_t value;
  uint8_t value_end;
  double duration;

  uint8_t pin = strtol(argv[1], nullptr, 0);

  if (strcmp("HIGH", argv[2]) == 0) {
    value = HIGH;
  } else if (strcmp("LOW", argv[2]) == 0) {
    value = LOW;
  } else {
    value = strtol(argv[2], nullptr, 0);
  }
  if ((value != HIGH) && (value != LOW))
    return EINVAL;

  if (strcmp("HIGH", argv[3]) == 0) {
    value_end = HIGH;
  } else if (strcmp("LOW", argv[3]) == 0) {
    value_end = LOW;
  } else {
    value_end = strtol(argv[3], nullptr, 0);
  }
  if ((value_end != HIGH) && (value_end != LOW))
    return EINVAL;

  duration = strtod(argv[4], nullptr);

  // allow for 3 mins which should accomodate
  // any reasonable use case
  if (duration < 0 || duration > 180.1) {
    return EINVAL;
  }

  if (duration < 16E-6) {
    int duration_ns = (int)(duration * 1E9);
    digitalWrite(pin, value);
    delayNanoseconds(duration_ns);
    digitalWrite(pin, value_end);
  } else if (duration < 16E-3) {
    int duration_us = (int)(duration * 1E6);
    digitalWrite(pin, value);
    delayMicroseconds(duration_us);
    digitalWrite(pin, value_end);
  } else {
    int duration_ms = (int)(duration * 1E3);
    digitalWrite(pin, value);
    delay(duration_ms);
    digitalWrite(pin, value_end);
  }

  return 0;
}

int sleep_seconds(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 2)
    return EINVAL;
  double duration;

  duration = strtod(argv[1], nullptr);

  // allow for 3 mins which should accomodate
  // any reasonable use case
  if (duration < 0 || duration > 180.1) {
    return EINVAL;
  }

  if (duration < 16E-6) {
    int duration_ns = (int)(duration * 1E9);
    delayNanoseconds(duration_ns);
  } else if (duration < 16E-3) {
    int duration_us = (int)(duration * 1E6);
    delayMicroseconds(duration_us);
  } else {
    int duration_ms = (int)(duration * 1E3);
    delay(duration_ms);
  }
  return 0;
}

int analog_write(CommandRouter *cmd, int argc, const char **argv) {
  int pin;
  int dutycycle;
  if (argc != 3) {
    return EINVAL;
  }

  pin = strtol(argv[1], nullptr, 0);
  dutycycle = strtol(argv[2], nullptr, 0);
  analogWrite(pin, dutycycle);
  return 0;
}

int analog_read(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 2)
    return EINVAL;

  uint8_t pin = strtol(argv[1], nullptr, 0);
  uint8_t value = analogRead(pin);

  snprintf(cmd->buffer, cmd->buffer_size, "%u", value);
  return 0;
}

int analog_write_frequency(CommandRouter *cmd, int argc, const char **argv) {
  int pin;
  int frequency;
  if (argc != 3) {
    return EINVAL;
  }

  pin = strtol(argv[1], nullptr, 0);
  frequency = strtol(argv[2], nullptr, 0);
  analogWriteFrequency(pin, frequency);
  return 0;
}

int analog_write_resolution(CommandRouter *cmd, int argc, const char **argv) {
  int resolution;
  if (argc != 2) {
    return EINVAL;
  }

  resolution = strtol(argv[1], nullptr, 0);
  analogWriteResolution(resolution);
  return 0;
}

int analog_pulse(CommandRouter *cmd, int argc, const char **argv) {
  int pin;
  int dutycycle, dutycycle_end;
  double duration;
  if (argc != 5) {
    return EINVAL;
  }

  pin = strtol(argv[1], nullptr, 0);
  dutycycle = strtol(argv[2], nullptr, 0);
  dutycycle_end = strtol(argv[3], nullptr, 0);

  duration = strtod(argv[4], nullptr);
  // allow for 3 mins which should accomodate
  // any reasonable use case
  if (duration < 0 || duration > 180.1) {
    return EINVAL;
  }

  if (duration < 16E-6) {
    int duration_ns = (int)(duration * 1E9);
    analogWrite(pin, dutycycle);
    delayNanoseconds(duration_ns);
    analogWrite(pin, dutycycle_end);
  } else if (duration < 16E-3) {
    int duration_us = (int)(duration * 1E6);
    analogWrite(pin, dutycycle);
    delayMicroseconds(duration_us);
    analogWrite(pin, dutycycle_end);
  } else {
    int duration_ms = (int)(duration * 1E3);
    analogWrite(pin, dutycycle);
    delay(duration_ms);
    analogWrite(pin, dutycycle_end);
  }

  return 0;
}

int spi_begin(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 1) {
    return EINVAL;
  }
  SPI.begin();
  return 0;
}
int spi_end(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 1) {
    return EINVAL;
  }
  SPI.end();
  return 0;
}

int spi_set_mosi(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 2) {
    return EINVAL;
  }
  uint8_t mosi = strtol(argv[1], nullptr, 0);
  SPI.setMOSI(mosi);
  return 0;
}

int spi_set_miso(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 2) {
    return EINVAL;
  }
  uint8_t miso = strtol(argv[1], nullptr, 0);
  SPI.setMISO(miso);
  return 0;
}

int spi_set_sck(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 2) {
    return EINVAL;
  }
  uint8_t sck = strtol(argv[1], nullptr, 0);
  SPI.setSCK(sck);
  return 0;
}

int spi_settings(CommandRouter *cmd, int argc, const char **argv) {
  int frequency;
  int bitOrder;
  int dataMode;

  if (argc < 4) {
    return EINVAL;
  }

  frequency = strtol(argv[1], nullptr, 0);

  if (strcmp("MSBFIRST", argv[2]) == 0) {
    bitOrder = MSBFIRST;
  } else if (strcmp("LSBFIRST", argv[2]) == 0) {
    bitOrder = LSBFIRST;
  } else {
    return EINVAL;
  }

  if (strcmp("SPI_MODE0", argv[3]) == 0) {
    dataMode = SPI_MODE0;
  } else if (strcmp("SPI_MODE1", argv[3]) == 0) {
    dataMode = SPI_MODE1;
  } else if (strcmp("SPI_MODE2", argv[3]) == 0) {
    dataMode = SPI_MODE2;
  } else if (strcmp("SPI_MODE3", argv[3]) == 0) {
    dataMode = SPI_MODE3;
  } else {
    return EINVAL;
  }

  spi_baudrate = frequency;
  spi_bit_order = bitOrder;
  spi_data_mode = dataMode;

  return 0;
}

int spi_begin_transaction(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 1) {
    return EINVAL;
  }

  SPI.beginTransaction(my_spi_settings());
  return 0;
}

int spi_end_transaction(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 1) {
    return EINVAL;
  }
  SPI.endTransaction();
  return 0;
}

int spi_transfer(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 2) {
    return EINVAL;
  }
  uint8_t data = strtol(argv[1], nullptr, 0);
  SPI.transfer(data);
  return 0;
}

int spi_read_byte(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 2) {
    return EINVAL;
  }
  uint8_t data = strtol(argv[1], nullptr, 0);
  SPI.beginTransaction(my_spi_settings());
  SPI.transfer(data);
  uint8_t received_byte = SPI.transfer(0xff);
  SPI.endTransaction();
  snprintf(cmd->buffer, cmd->buffer_size, "0x%02X", received_byte);
  return 0;
}

int spi_transfer_bulk(CommandRouter *cmd, int argc, const char **argv) {
  const int MAX_UINT8_TO_SEND = 100;
  uint8_t data[MAX_UINT8_TO_SEND];
  int transfer_count;
  size_t output_buffer_remaining = cmd->buffer_size;
  int bytes_written;
  char *output_pointer;

  if (argc <= 1) {
    return EINVAL;
  }

  transfer_count = argc - 1;
  if (transfer_count > MAX_UINT8_TO_SEND) {
    return E2BIG;
  }

  // Each returned value is at least 4 bytes. For the last value
  // a '\0' is required instead of a ' ' <- space
  if (output_buffer_remaining < transfer_count * sizeof("0xFF ")) {
    return E2BIG;
  }

  // We are storing the data in a temporary pre-allocated buffer
  // to avoid any kind of hardware failure during transfer due to string
  // buffer overflow
  SPI.beginTransaction(my_spi_settings());
  for (int i = 0; i < transfer_count; i++) {
    uint8_t d = strtol(argv[i + 1], nullptr, 0);
    data[i] = SPI.transfer(d);
  }
  SPI.endTransaction();

  output_pointer = cmd->buffer;
  bytes_written = snprintf(
    output_pointer,
    output_buffer_remaining,
    "0x%02X",
    data[0]
  );
  output_buffer_remaining -= bytes_written;
  output_pointer = output_pointer + bytes_written;
  for (int i = 1; i < transfer_count; i++) {
    bytes_written = snprintf(
      output_pointer,
      output_buffer_remaining,
      " 0x%02X",  // notice the extra space at the beginning
      data[i]
    );
    output_buffer_remaining -= bytes_written;
    output_pointer = output_pointer + bytes_written;
  }
  return 0;
}

int register_read_uint8(CommandRouter *cmd, int argc, const char **argv) {
  uint8_t data;
  // We declare the pointer as volatile since it is expected that the user
  // will read a hardware register and we do not want the compiler to optimize
  // the "invalid" read away.
  volatile uint8_t *ptr;
  if (argc != 2) {
    return EINVAL;
  }
  ptr = (volatile uint8_t *)strtol(argv[1], nullptr, 0);
  if (ptr == 0){
    return EINVAL;
  }
  data = ptr[0];
  snprintf(cmd->buffer, cmd->buffer_size, "0x%02X", data);
  return 0;
}

int register_write_uint8(CommandRouter *cmd, int argc, const char **argv) {
  uint8_t data;
  // We declare the pointer as volatile since it is expected that the user
  // will write a hardware register and we do not want the compiler to optimize
  // the "invalid" write away.
  volatile uint8_t *ptr;
  if (argc != 3) {
    return EINVAL;
  }
  ptr = (volatile uint8_t *)strtol(argv[1], nullptr, 0);
  if (ptr == 0) {
    return EINVAL;
  }
  data = (uint8_t)strtol(argv[2], nullptr, 0);
  *ptr = data;
  return 0;
}

int register_write_uint16(CommandRouter *cmd, int argc, const char **argv) {
  uint16_t data;
  // We declare the pointer as volatile since it is expected that the user
  // will write a hardware register and we do not want the compiler to optimize
  // the "invalid" write away.
  volatile uint16_t *ptr;
  if (argc != 3) {
    return EINVAL;
  }
  ptr = (volatile uint16_t *)strtol(argv[1], nullptr, 0);
  if (ptr == 0) {
    return EINVAL;
  }
  data = (uint16_t)strtol(argv[2], nullptr, 0);
  *ptr = data;
  return 0;
}


int register_read_uint16(CommandRouter *cmd, int argc, const char **argv) {
  uint16_t data;
  // We declare the pointer as volatile since it is expected that the user
  // will read a hardware register and we do not want the compiler to optimize
  // the "invalid" read away.
  volatile uint16_t *ptr;
  if (argc != 2) {
    return EINVAL;
  }
  ptr = (volatile uint16_t *)strtol(argv[1], nullptr, 0);
  if (ptr == 0){
    return EINVAL;
  }
  data = ptr[0];
  snprintf(cmd->buffer, cmd->buffer_size, "0x%04X", data);
  return 0;
}


int register_write_uint32(CommandRouter *cmd, int argc, const char **argv) {
  uint32_t data;
  // We declare the pointer as volatile since it is expected that the user
  // will write a hardware register and we do not want the compiler to optimize
  // the "invalid" write away.
  volatile uint32_t *ptr;
  if (argc != 3) {
    return EINVAL;
  }
  ptr = (volatile uint32_t *)strtol(argv[1], nullptr, 0);
  if (ptr == 0) {
    return EINVAL;
  }
  data = (uint32_t)strtol(argv[2], nullptr, 0);
  *ptr = data;
  return 0;
}


int register_read_uint32(CommandRouter *cmd, int argc, const char **argv) {
  uint32_t data;
  // We declare the pointer as volatile since it is expected that the user
  // will read a hardware register and we do not want the compiler to optimize
  // the "invalid" read away.
  volatile uint32_t *ptr;
  if (argc != 2) {
    return EINVAL;
  }
  ptr = (volatile uint32_t *)strtol(argv[1], nullptr, 0);
  if (ptr == 0){
    return EINVAL;
  }
  data = ptr[0];
  snprintf(cmd->buffer, cmd->buffer_size, "0x%08lX", data);
  return 0;
}

int eeprom_read_uint8(CommandRouter *cmd, int argc, const char **argv) {
  if (argc < 2) {
    return EINVAL;
  }
  int index;
  int length = 1;

  index = strtol(argv[1], nullptr, 0);
  if (argc == 3) {
    length = strtol(argv[2], nullptr, 0);
  }

  auto buffer = cmd->buffer;
  auto buffer_size = cmd->buffer_size;
  int written = 0;
  const char * fmt;
  for (int i = 0; i < length; i++) {
    if (index + i > EEPROM.length()) {
      return EINVAL;
    }
    auto data = EEPROM.read(index + i);
    if (i == 0) {
      fmt = "0x%02X";
    } else {
      fmt = " 0x%02X";
    }
    written = snprintf(buffer, buffer_size, fmt, data);
    buffer += written;
    buffer_size -= written;
    index++;
  }

  return 0;
}

int eeprom_write_uint8(CommandRouter *cmd, int argc, const char **argv) {
  if (argc < 3) {
    return EINVAL;
  }
  int index;
  uint8_t data;

  index = strtol(argv[1], nullptr, 0);

  for (int i = 2; i < argc; i++) {
    if (index > EEPROM.length()) {
      return EINVAL;
    }

    data = strtol(argv[i], nullptr, 0);
    /*
    The function EEPROM.update(address, val) is equivalent to the following:

    if( EEPROM.read(address) != val ){
      EEPROM.write(address, val);
    }
    */
    EEPROM.update(index, data);
    ++index;
  }

  return 0;
}


void loop() {
  // TODO: remove this check on if Serial is available.
  // I don't think we need it since it is already in a loop
  if (Serial.available()) {
    cmd.processSerialStream();
  }
}
