#include "commandconstants.hpp"
#include "commandrouting.hpp"
#include "i2c.hpp"
#include "startup_commands.hpp"
#include <Arduino.h>
#include <SPI.h>
#include <errno.h>
#include <new>
#include <usb_names.h>
#include <EEPROM.h>

// TODO: this isn't exactly correct since this main file won't get
// regenerated if it wasn't touched.
#ifndef GIT_DESCRIBE
#define GIT_DESCRIBE "0.0.0-unknown"
#endif


#define USE_STATIC_ALLOCATION 1
#if USE_STATIC_ALLOCATION
#define BUFFER_SIZE 1024 * 2
#define ARGV_MAX 300
char serial_buffer[BUFFER_SIZE];
const char *argv_buffer[ARGV_MAX];
#endif

// Buffer sizes - smaller for Teensy 3.2, larger for Teensy 4.0
#if defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)
#define I2C_BUFFER_SIZE 32   // Teensy 3.2/3.5/3.6
#define SPI_BUFFER_SIZE 32   // Teensy 3.2/3.5/3.6
#else
#define I2C_BUFFER_SIZE 256  // Teensy 4.0/4.1
#define SPI_BUFFER_SIZE 256  // Teensy 4.0/4.1
#endif
// Default SPI Settings
uint32_t spi_baudrate = 4'000'000;
uint8_t spi_bit_order = MSBFIRST;
uint8_t spi_data_mode = SPI_MODE0;

inline SPISettings my_spi_settings() {
  // Gotta hate this really weird API
  return SPISettings(spi_baudrate, spi_bit_order, spi_data_mode);
}

/* The following are weak aliases for the USB string descriptors
   We use this to override the default USB string descriptors
   With our own to describe the TeensyToAny device.


extern struct usb_string_descriptor_struct usb_string_manufacturer_name
        __attribute__ ((weak, alias("usb_string_manufacturer_name_default")));
extern struct usb_string_descriptor_struct usb_string_product_name
        __attribute__ ((weak, alias("usb_string_product_name_default")));
extern struct usb_string_descriptor_struct usb_string_serial_number
        __attribute__ ((weak, alias("usb_string_serial_number_default")));
*/
#ifndef TEENSY_TO_ANY_MANUFACTURER_NAME
#define TEENSY_TO_ANY_MANUFACTURER_NAME {'T', 'e', 'e', 'n', 's', 'y', 'T', 'o', 'A', 'n', 'y'}
#endif
#ifndef TEENSY_TO_ANY_MANUFACTURER_NAME_LEN
#define TEENSY_TO_ANY_MANUFACTURER_NAME_LEN 11
#endif

PROGMEM usb_string_descriptor_struct usb_string_manufacturer_name = {
    .bLength = 2 + 2 * TEENSY_TO_ANY_MANUFACTURER_NAME_LEN,
    .bDescriptorType = 3,
    .wString = TEENSY_TO_ANY_MANUFACTURER_NAME,
};

#include "FastLED.h"
CRGB * fastled_leds = nullptr;
CLEDController * fastled_controller = nullptr;
int fastled_num_leds = 0;
int fastled_has_white = 0;

// ---------------------------------------------------------------------------
// Dynamic FastLED chain definition
// ---------------------------------------------------------------------------
//
// FastLED takes the chipset, *data pin* and color order as C++ *template*
// parameters, so they are baked in at compile time.  FastLED.addLeds<...>()
// for clockless chipsets internally creates ONE static controller object per
// unique (chipset, pin, order) template instantiation and registers it in a
// global, singly-linked list of CLEDController objects.  There is no public
// API to remove a controller from that list, and the controller objects are
// `static` (not heap allocated), so they cannot be freed.
//
// What this means in practice:
//   * "Runtime" pin / order / chipset selection is implemented by switching
//     over the supported values and calling the matching template at compile
//     time (the canonical FastLED pattern).  Every (pin, order) combination
//     we want to support has to be instantiated, which costs flash.
//   * A chain can be (re)defined at runtime: we reallocate the CRGB buffer and
//     point the (possibly new) controller at it via addLeds<>(buf, n).
//   * A chain can NOT be torn down in the sense of removing the controller
//     object from FastLED's list.  The honest "teardown" we can offer is to
//     disable the active controller (setEnabled(false) so FastLED.show()
//     skips it) and free the buffer.  Re-defining simply disables whatever was
//     active and activates the new controller.
//
// To keep flash usage bounded (Teensy 3.2 in particular is very tight) we
// support the common WS2812/NEOPIXEL-family 800kHz clockless chipsets, the
// standard color orders, RGB vs RGBW, and data pins 0..23.
//
// Every (chipset, pin, order) combination is a separate template instantiation
// and each clockless controller is a few KB of flash, so the full matrix
// (chipsets x pins x orders) is large.  Teensy 4.0 has ~2 MB of flash and can
// afford the whole matrix.  Teensy 3.2 has only 256 KB, so there we keep the
// full pin range (the most useful "dynamic" axis) and the WS2812 family, but
// instantiate only the two most common color orders (GRB and RGB) and fold the
// distinct chipset names onto the WS2812 800 kHz template.  Requesting an
// unsupported order/chipset on Teensy 3.2 returns EINVAL.
#if defined(__MK20DX256__)
// Teensy 3.2 (Kinetis K20, 256 KB flash): reduced matrix.
#define FASTLED_DYNAMIC_FULL_ORDERS 0
#define FASTLED_DYNAMIC_FULL_CHIPSETS 0
#else
// Teensy 4.x and anything else with generous flash: full matrix.
#define FASTLED_DYNAMIC_FULL_ORDERS 1
#define FASTLED_DYNAMIC_FULL_CHIPSETS 1
#endif

// Supported chipset families.  They all share the WS2812 800kHz clockless
// timing in FastLED 3.10.3 except SK6812 (slightly different reset timing);
// they differ mainly in their default color order, which the caller overrides
// explicitly via the rgb_order argument.
enum fastled_chipset_t {
  FASTLED_CHIPSET_WS2812 = 0,
  FASTLED_CHIPSET_WS2812B,
  FASTLED_CHIPSET_WS2811,
  FASTLED_CHIPSET_WS2813,
  FASTLED_CHIPSET_SK6812,
  FASTLED_CHIPSET_NEOPIXEL,
};

static int fastled_parse_chipset(const char *s, fastled_chipset_t *out) {
  if (strcmp(s, "WS2812") == 0)        { *out = FASTLED_CHIPSET_WS2812;   return 0; }
  if (strcmp(s, "WS2812B") == 0)       { *out = FASTLED_CHIPSET_WS2812B;  return 0; }
  if (strcmp(s, "WS2811") == 0)        { *out = FASTLED_CHIPSET_WS2811;   return 0; }
  if (strcmp(s, "WS2813") == 0)        { *out = FASTLED_CHIPSET_WS2813;   return 0; }
  if (strcmp(s, "SK6812") == 0)        { *out = FASTLED_CHIPSET_SK6812;   return 0; }
  if (strcmp(s, "NEOPIXEL") == 0)      { *out = FASTLED_CHIPSET_NEOPIXEL; return 0; }
  return EINVAL;
}

static int fastled_parse_order(const char *s, EOrder *out) {
  if (strcmp(s, "RGB") == 0) { *out = RGB; return 0; }
  if (strcmp(s, "RBG") == 0) { *out = RBG; return 0; }
  if (strcmp(s, "GRB") == 0) { *out = GRB; return 0; }
  if (strcmp(s, "GBR") == 0) { *out = GBR; return 0; }
  if (strcmp(s, "BRG") == 0) { *out = BRG; return 0; }
  if (strcmp(s, "BGR") == 0) { *out = BGR; return 0; }
  return EINVAL;
}

// Dispatch helpers.  Because the chipset, pin and color order are all template
// parameters we have to enumerate every combination we want to support.  These
// macros keep the explosion readable.  Only the WS2812 800kHz family is wired
// up; the distinct chipset names all map onto WS2812Controller800Khz timing in
// FastLED 3.10.3 (NEOPIXEL/WS2812/WS2812B/WS2813/GS1903 are identical there),
// with SK6812 the lone exception (handled via its own template).

// One color-order switch for a given (CHIPSET template, PIN).  On
// flash-constrained targets only GRB and RGB are instantiated.
#if FASTLED_DYNAMIC_FULL_ORDERS
#define _FASTLED_ORDER_CASES(CHIPSET, PIN, buf, n, order, ctrl)                 \
  switch (order) {                                                              \
    case RGB: ctrl = &FastLED.addLeds<CHIPSET, PIN, RGB>(buf, n); break;        \
    case RBG: ctrl = &FastLED.addLeds<CHIPSET, PIN, RBG>(buf, n); break;        \
    case GRB: ctrl = &FastLED.addLeds<CHIPSET, PIN, GRB>(buf, n); break;        \
    case GBR: ctrl = &FastLED.addLeds<CHIPSET, PIN, GBR>(buf, n); break;        \
    case BRG: ctrl = &FastLED.addLeds<CHIPSET, PIN, BRG>(buf, n); break;        \
    case BGR: ctrl = &FastLED.addLeds<CHIPSET, PIN, BGR>(buf, n); break;        \
    default: ctrl = nullptr; break;                                            \
  }
#else
#define _FASTLED_ORDER_CASES(CHIPSET, PIN, buf, n, order, ctrl)                 \
  switch (order) {                                                              \
    case RGB: ctrl = &FastLED.addLeds<CHIPSET, PIN, RGB>(buf, n); break;        \
    case GRB: ctrl = &FastLED.addLeds<CHIPSET, PIN, GRB>(buf, n); break;        \
    default: ctrl = nullptr; break;                                            \
  }
#endif

// One pin case for a given chipset template.
#define _FASTLED_PIN_CASE(CHIPSET, PIN, buf, n, order, ctrl)                    \
  case PIN: _FASTLED_ORDER_CASES(CHIPSET, PIN, buf, n, order, ctrl); break;

// Switch over the supported data pins (0..23) for a given chipset template.
#define _FASTLED_PIN_SWITCH(CHIPSET, pin, buf, n, order, ctrl)                  \
  switch (pin) {                                                                \
    _FASTLED_PIN_CASE(CHIPSET, 0,  buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 1,  buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 2,  buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 3,  buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 4,  buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 5,  buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 6,  buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 7,  buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 8,  buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 9,  buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 10, buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 11, buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 12, buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 13, buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 14, buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 15, buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 16, buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 17, buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 18, buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 19, buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 20, buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 21, buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 22, buf, n, order, ctrl)                         \
    _FASTLED_PIN_CASE(CHIPSET, 23, buf, n, order, ctrl)                         \
    default: ctrl = nullptr; break;                                            \
  }

// Disable every controller FastLED currently knows about.  This is the closest
// thing to "teardown" that the FastLED API permits: the controller objects
// stay in the static linked list (and any heap/DMA buffers they own stay
// allocated), but FastLED.show() will skip disabled controllers, so a stale
// controller will not try to clock data out of a buffer we are about to free
// or re-point.
static void fastled_disable_all_controllers() {
  for (CLEDController *c = CLEDController::head(); c != nullptr; c = c->next()) {
    c->setEnabled(false);
  }
}

// Core (re)definition routine shared by fastled_define and the legacy
// fastled_add_leds command.  Returns an errno-style int.
static int fastled_define_core(int pin, int num_leds, fastled_chipset_t chipset,
                               EOrder order, int has_white) {
  if (num_leds <= 0) {
    return EINVAL;
  }
  if (pin < 0 || pin > 23) {
    return EINVAL;
  }

  // Round up to a multiple of 8.  Historically exact odd counts (e.g. 5)
  // produced crashes; the clockless drivers appear happiest with a padded
  // buffer, so we keep that behaviour.
  int alloc_leds = int((num_leds + 7) / 8) * 8;

  CRGB *new_buf = new (std::nothrow) CRGB[alloc_leds];
  if (new_buf == nullptr) {
    return ENOMEM;
  }
  for (int i = 0; i < alloc_leds; ++i) {
    new_buf[i] = CRGB::Black;
  }

  // Activate the controller for the requested (chipset, pin, order).  This may
  // re-use an existing static controller singleton (if we have defined this
  // exact combination before) or register a new one.
  CLEDController *ctrl = nullptr;
  switch (chipset) {
    case FASTLED_CHIPSET_WS2812:
    case FASTLED_CHIPSET_WS2812B:
    case FASTLED_CHIPSET_WS2811:
    case FASTLED_CHIPSET_WS2813:
    case FASTLED_CHIPSET_NEOPIXEL:
      // All of these share WS2812 800kHz timing in FastLED 3.10.3.  We let the
      // caller pick the color order explicitly, so a single template family is
      // enough and keeps flash usage manageable.
      _FASTLED_PIN_SWITCH(WS2812, pin, new_buf, alloc_leds, order, ctrl);
      break;
    case FASTLED_CHIPSET_SK6812:
#if FASTLED_DYNAMIC_FULL_CHIPSETS
      _FASTLED_PIN_SWITCH(SK6812, pin, new_buf, alloc_leds, order, ctrl);
#else
      // On flash-constrained targets SK6812 is not instantiated separately; it
      // shares the WS2812 800 kHz timing closely enough for indicator use.
      _FASTLED_PIN_SWITCH(WS2812, pin, new_buf, alloc_leds, order, ctrl);
#endif
      break;
    default:
      ctrl = nullptr;
      break;
  }

  if (ctrl == nullptr) {
    delete[] new_buf;
    return EINVAL;
  }

  // Disable any previously-active controllers so they don't clock data out of
  // the buffer we are about to free, then make the new controller the active
  // one.
  CRGB *old_buf = fastled_leds;
  for (CLEDController *c = CLEDController::head(); c != nullptr; c = c->next()) {
    if (c != ctrl) {
      c->setEnabled(false);
    }
  }
  ctrl->setEnabled(true);
  ctrl->setLeds(new_buf, alloc_leds);
  ctrl->setRgbw(has_white ? RgbwDefault::value() : RgbwInvalid::value());

  fastled_leds = new_buf;
  fastled_num_leds = alloc_leds;
  fastled_has_white = has_white;
  fastled_controller = ctrl;

  // Now that nothing points at the old buffer, free it.
  if (old_buf != nullptr && old_buf != new_buf) {
    delete[] old_buf;
  }

  return 0;
}

#if TEENSY_TO_ANY_HAS_I2C_T3
I2CMaster i2c(&Wire);
I2CMaster i2c_1(&Wire1);
#endif
#if TEENSY_TO_ANY_HAS_I2C_T4
I2CMaster_T4 i2c(&Wire);
I2CMaster_T4 i2c_1(&Wire1);
#endif

// Teensy4 has 1080 bytes of EEPROM, use the last byte to store the
// demo command enabled flag
#define DEMO_COMMAND_ENABLED_ADDRESS 1079


int len_startup_commands;
int len_post_serial_startup_commands;
int len_demo_commands;

void setup_startup_and_demo_commands() {
  len_startup_commands = 0;
  for (int i = 0; teensy_to_any_startup_commands[i] != nullptr; i++) {
    len_startup_commands++;
  }
  len_post_serial_startup_commands = 0;
  for (int i = 0; teensy_to_any_post_serial_startup_commands[i] != nullptr; i++) {
    len_post_serial_startup_commands++;
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
void execute_post_serial_startup_commands() {
  for (int i = 0; teensy_to_any_post_serial_startup_commands[i] != nullptr; i++) {
    cmd.processString(teensy_to_any_post_serial_startup_commands[i]);
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

int nop_func(CommandRouter *cmd, int argc, const char **argv) {
  (void)argc;
  (void)argv;
  (void)cmd;
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
  execute_post_serial_startup_commands();

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

int post_startup_commands_available(CommandRouter *cmd, int argc, const char **argv){
  snprintf(cmd->buffer, cmd->buffer_size, "%d", len_post_serial_startup_commands);
  return 0;
}
int read_post_startup_command(CommandRouter *cmd, int argc, const char **argv){
  if (argc != 2)
    return EINVAL;
  int index = strtol(argv[1], nullptr, 0);
  if (index < 0)
    return EINVAL;
  if (index >= len_post_serial_startup_commands)
    return EINVAL;
  const char *command = teensy_to_any_post_serial_startup_commands[index];
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
}

int i2c_reset(CommandRouter *cmd, int argc, const char **argv) {
  return i2c.reset();
}

int i2c_begin_transaction(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 2)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  return i2c.begin_transaction(slave_address);
}

int i2c_write(CommandRouter *cmd, int argc, const char **argv) {
  const int num_bytes_max = I2C_BUFFER_SIZE;
  uint8_t data[num_bytes_max];
  if (argc < 3)
    return EINVAL;

  int num_bytes = argc - 1;
  if (num_bytes > num_bytes_max)
    return EINVAL;

  for (int i = 0; i < num_bytes; i++) {
    data[i] = strtol(argv[i + 1], nullptr, 0);
  }

  return i2c.write(data, num_bytes);
}

int i2c_end_transaction(CommandRouter *cmd, int argc, const char **argv) {
  bool stop = true;
  if (argc >= 2) {
    if (strcmp(argv[1], "false") == 0) {
      stop = false;
    } else if (strcmp(argv[1], "true") == 0) {
      stop = true;
    } else {
      stop = (bool)strtol(argv[1], nullptr, 0);
    }
  }
  return i2c.end_transaction(stop);
}

int i2c_write_uint16(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 4)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);
  uint16_t data = strtol(argv[3], nullptr, 0);
  return i2c.write_uint16(slave_address, register_address, data);
}

int i2c_write_uint8(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 4)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);
  uint8_t data = strtol(argv[3], nullptr, 0);

  return i2c.write_uint8(slave_address, register_address, data);
}

int i2c_write_payload(CommandRouter *cmd, int argc, const char **argv) {
  const int num_bytes_max = I2C_BUFFER_SIZE;
  uint8_t data[num_bytes_max];
  if (argc < 4)
    return EINVAL;

  int num_bytes = argc - 3;
  if (num_bytes > num_bytes_max)
    return E2BIG;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);

  for (int i = 0; i < num_bytes; i++) {
    data[i] = strtol(argv[i + 3], nullptr, 0);
  }

  return i2c.write_payload(slave_address, register_address, data, num_bytes);

}

int i2c_read_payload(CommandRouter *cmd, int argc, const char **argv) {
  const int num_bytes_max = I2C_BUFFER_SIZE;
  if (argc != 4)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);
  int num_bytes = strtol(argv[3], nullptr, 0);
  if (num_bytes > num_bytes_max)
    return E2BIG;

  uint8_t data[num_bytes_max];
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
}

int i2c_read_payload_no_register(CommandRouter *cmd, int argc, const char **argv) {
  const int num_bytes_max = I2C_BUFFER_SIZE;
  if (argc != 3)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int num_bytes = strtol(argv[2], nullptr, 0);
  if (num_bytes > num_bytes_max)
    return E2BIG;

  uint8_t data[num_bytes_max];
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
}

int i2c_read_payload_uint16(CommandRouter *cmd, int argc, const char **argv) {
  const int num_bytes_max = I2C_BUFFER_SIZE;
  if (argc != 4)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address;
  register_address = strtol(argv[2], nullptr, 0);
  int num_bytes = strtol(argv[3], nullptr, 0);
  if (num_bytes > num_bytes_max)
    return EINVAL;

  uint8_t data[num_bytes_max];
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
}

int i2c_read_uint16(CommandRouter *cmd, int argc, const char **argv) {
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

int i2c_read_uint8(CommandRouter *cmd, int argc, const char **argv) {
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
}

int i2c_read_no_register_uint8(CommandRouter *cmd, int argc,
                               const char **argv) {
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
  if (argc != 3)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  uint8_t data = strtol(argv[2], nullptr, 0);
  return i2c.write_no_register_uint8(slave_address, data);
}


int i2c_ping(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 2)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  return i2c.ping(slave_address);
}


int i2c_1_init(CommandRouter *cmd, int argc, const char **argv) {
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

  return i2c_1.init(baudrate, timeout_ms, address_size, address_msb_first);
}

int i2c_1_reset(CommandRouter *cmd, int argc, const char **argv) {
  return i2c_1.reset();
}

int i2c_1_begin_transaction(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 2)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  return i2c_1.begin_transaction(slave_address);
}

int i2c_1_write(CommandRouter *cmd, int argc, const char **argv) {
  const int num_bytes_max = I2C_BUFFER_SIZE;
  uint8_t data[num_bytes_max];
  if (argc < 3)
    return EINVAL;

  int num_bytes = argc - 1;
  if (num_bytes > num_bytes_max)
    return EINVAL;

  for (int i = 0; i < num_bytes; i++) {
    data[i] = strtol(argv[i + 1], nullptr, 0);
  }

  return i2c_1.write(data, num_bytes);
}

int i2c_1_end_transaction(CommandRouter *cmd, int argc, const char **argv) {
    bool stop = true;
  if (argc >= 2) {
    if (strcmp(argv[1], "false") == 0) {
      stop = false;
    } else if (strcmp(argv[1], "true") == 0) {
      stop = true;
    } else {
      stop = (bool)strtol(argv[1], nullptr, 0);
    }
  }
  return i2c_1.end_transaction(stop);
}

int i2c_1_write_uint16(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 4)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);
  uint16_t data = strtol(argv[3], nullptr, 0);
  return i2c_1.write_uint16(slave_address, register_address, data);
}

int i2c_1_write_uint8(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 4)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);
  uint8_t data = strtol(argv[3], nullptr, 0);

  return i2c_1.write_uint8(slave_address, register_address, data);
}

int i2c_1_write_payload(CommandRouter *cmd, int argc, const char **argv) {
  const int num_bytes_max = I2C_BUFFER_SIZE;
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

  return i2c_1.write_payload(slave_address, register_address, data, num_bytes);

}

int i2c_1_read_payload(CommandRouter *cmd, int argc, const char **argv) {
  const int num_bytes_max = I2C_BUFFER_SIZE;
  if (argc != 4)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);
  int num_bytes = strtol(argv[3], nullptr, 0);
  if (num_bytes > num_bytes_max)
    return EINVAL;

  uint8_t data[num_bytes_max];
  int result;
  result = i2c_1.read_payload(slave_address, register_address, data, num_bytes);

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
}

int i2c_1_read_payload_no_register(CommandRouter *cmd, int argc, const char **argv) {
  const int num_bytes_max = I2C_BUFFER_SIZE;
  if (argc != 3)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int num_bytes = strtol(argv[2], nullptr, 0);
  if (num_bytes > num_bytes_max)
    return EINVAL;

  uint8_t data[num_bytes_max];
  int result;
  result = i2c_1.read_payload_no_register(slave_address, data, num_bytes);

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
}

int i2c_1_read_payload_uint16(CommandRouter *cmd, int argc, const char **argv) {
  const int num_bytes_max = I2C_BUFFER_SIZE;
  if (argc != 4)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address;
  register_address = strtol(argv[2], nullptr, 0);
  int num_bytes = strtol(argv[3], nullptr, 0);
  if (num_bytes > num_bytes_max)
    return EINVAL;

  uint8_t data[num_bytes_max];
  int result;
  result = i2c_1.read_payload_uint16(slave_address, register_address,
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
}

int i2c_1_read_uint16(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 3)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);
  uint16_t data;
  int result;
  result = i2c_1.read_uint16(slave_address, register_address, data);

  if (result == 0) {
    snprintf(cmd->buffer, cmd->buffer_size, "0x%04X", data);
  }
  return result;
}

int i2c_1_read_uint8(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 3)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  int register_address = strtol(argv[2], nullptr, 0);
  uint8_t data;
  int result;
  result = i2c_1.read_uint8(slave_address, register_address, data);
  if (result == 0) {
    snprintf(cmd->buffer, cmd->buffer_size, "0x%02X", data);
  }
  return result;
}

int i2c_1_read_no_register_uint8(CommandRouter *cmd, int argc,
                               const char **argv) {
  if (argc != 2)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  uint8_t data;
  int result;

  result = i2c_1.read_no_register_uint8(slave_address, data);
  if (result == 0) {
    snprintf(cmd->buffer, cmd->buffer_size, "0x%02X", data);
  }
  return result;
}

int i2c_1_write_no_register_uint8(CommandRouter *cmd, int argc,
                                const char **argv) {
  if (argc != 3)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  uint8_t data = strtol(argv[2], nullptr, 0);
  return i2c_1.write_no_register_uint8(slave_address, data);
}

int i2c_1_ping(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 2)
    return EINVAL;

  int slave_address = strtol(argv[1], nullptr, 0);
  return i2c_1.ping(slave_address);
}

int i2c_buffer_size(CommandRouter *cmd, int argc, const char **argv) {
  (void)argc;
  (void)argv;
  snprintf(cmd->buffer, cmd->buffer_size, "%d", I2C_BUFFER_SIZE);
  return 0;
}

int i2c_1_buffer_size(CommandRouter *cmd, int argc, const char **argv) {
  (void)argc;
  (void)argv;
  snprintf(cmd->buffer, cmd->buffer_size, "%d", I2C_BUFFER_SIZE);
  return 0;
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
  uint16_t value = SPI.transfer(data);
  snprintf(cmd->buffer, cmd->buffer_size, "0x%02X", value);
  return 0;
}

int spi_transfer16(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 2) {
    return EINVAL;
  }
  uint16_t data = strtol(argv[1], nullptr, 0);
  uint16_t value = SPI.transfer16(data);
  snprintf(cmd->buffer, cmd->buffer_size, "0x%04X", value);
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
  const int MAX_UINT8_TO_SEND = SPI_BUFFER_SIZE;
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

int spi_buffer_size(CommandRouter *cmd, int argc, const char **argv) {
  (void)argc;
  (void)argv;
  snprintf(cmd->buffer, cmd->buffer_size, "%d", SPI_BUFFER_SIZE);
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

int fastled_add_leds(CommandRouter *cmd, int argc, const char **argv) {
  // Legacy command, kept for backwards compatibility.
  // Arguments are:
  //   fastled_add_leds <chipset> <has_white> <pin> <num_leds>
  // Historically only NEOPIXEL was supported and a chain could only be
  // defined once.  It now delegates to the dynamic core, so it can be called
  // repeatedly to redefine the chain.
  if (argc != 5) {
    return EINVAL;
  }
  const char *led_class = argv[1];

  fastled_chipset_t chipset;
  if (fastled_parse_chipset(led_class, &chipset) != 0) {
    return EINVAL;
  }

  int has_white = strtol(argv[2], nullptr, 0) != 0;
  int pin = strtol(argv[3], nullptr, 0);
  int num_leds = strtol(argv[4], nullptr, 0);

  // NEOPIXEL implies the GRB color order; the other (explicit) chipsets in
  // this legacy path also default to GRB for the WS2812 family.
  return fastled_define_core(pin, num_leds, chipset, GRB, has_white);
}

int fastled_define(CommandRouter *cmd, int argc, const char **argv) {
  // fastled_define <pin> <num_leds> <chipset> <rgb_order> [is_rgbw]
  //
  // Dynamically (re)define the LED chain at runtime.  Calling it again
  // redefines the chain: the previously active controller is disabled and the
  // CRGB buffer is reallocated for the new configuration.
  if (argc < 5 || argc > 6) {
    return EINVAL;
  }

  int pin = strtol(argv[1], nullptr, 0);
  int num_leds = strtol(argv[2], nullptr, 0);

  fastled_chipset_t chipset;
  if (fastled_parse_chipset(argv[3], &chipset) != 0) {
    return EINVAL;
  }

  EOrder order;
  if (fastled_parse_order(argv[4], &order) != 0) {
    return EINVAL;
  }

  int has_white = 0;
  if (argc == 6) {
    has_white = strtol(argv[5], nullptr, 0) != 0;
  }

  return fastled_define_core(pin, num_leds, chipset, order, has_white);
}

int fastled_clear(CommandRouter *cmd, int argc, const char **argv) {
  // Tear down the active chain as far as FastLED allows.  The controller
  // object cannot be removed from FastLED's static list, so we blank the LEDs,
  // disable every controller, and free our CRGB buffer.  After this a fresh
  // fastled_define is required before any other fastled_* command will work.
  if (argc != 1) {
    return EINVAL;
  }

  if (fastled_controller != nullptr && fastled_leds != nullptr) {
    // Push an all-black frame out so the strip actually turns off.
    for (int i = 0; i < fastled_num_leds; ++i) {
      fastled_leds[i] = CRGB::Black;
    }
    FastLED.show();
  }

  fastled_disable_all_controllers();

  if (fastled_leds != nullptr) {
    delete[] fastled_leds;
    fastled_leds = nullptr;
  }
  fastled_controller = nullptr;
  fastled_num_leds = 0;
  fastled_has_white = 0;

  return 0;
}
int fastled_show(CommandRouter *cmd, int argc, const char **argv) {
  if (argc > 2) {
    return EINVAL;
  }
  if (argc == 2) {
    uint8_t scale = (uint8_t)strtol(argv[1], nullptr, 0);
    FastLED.show(scale);
  } else {
    FastLED.show();
  }
  return 0;
}
int fastled_set_rgb(CommandRouter *cmd, int argc, const char **argv) {
  if (argc < 5) {
    return EINVAL;
  }
  if (fastled_leds == nullptr) {
    return EINVAL;
  }

  int index = strtol(argv[1], nullptr, 0);
  if (index >= fastled_num_leds) {
    return EINVAL;
  }

  uint8_t r = (uint8_t)strtol(argv[2], nullptr, 0);
  uint8_t g = (uint8_t)strtol(argv[3], nullptr, 0);
  uint8_t b = (uint8_t)strtol(argv[4], nullptr, 0);

  fastled_leds[index].setRGB(r, g, b);

  return 0;
}


int fastled_set_hsv(CommandRouter *cmd, int argc, const char **argv) {
  if (argc < 5) {
    return EINVAL;
  }
  if (fastled_leds == nullptr) {
    return EINVAL;
  }

  int index = strtol(argv[1], nullptr, 0);
  if (index >= fastled_num_leds) {
    return EINVAL;
  }

  uint8_t h = (uint8_t)strtol(argv[2], nullptr, 0);
  uint8_t s = (uint8_t)strtol(argv[3], nullptr, 0);
  uint8_t v = (uint8_t)strtol(argv[4], nullptr, 0);

  fastled_leds[index].setHSV(h, s, v);

  return 0;
}

int fastled_set_hue(CommandRouter *cmd, int argc, const char **argv) {
  if (argc < 3) {
    return EINVAL;
  }
  if (fastled_leds == nullptr) {
    return EINVAL;
  }

  int index = strtol(argv[1], nullptr, 0);
  if (index >= fastled_num_leds) {
    return EINVAL;
  }

  uint8_t hue = (uint8_t)strtol(argv[2], nullptr, 0);

  fastled_leds[index].setHue(hue);

  return 0;
}


int fastled_set_brightness(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 2) {
    return EINVAL;
  }
  if (fastled_leds == nullptr) {
    return EINVAL;
  }
  int brightness = strtol(argv[1], nullptr, 0);
  FastLED.setBrightness(brightness);
  return 0;
}

int fastled_get_brightness(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 1) {
    return EINVAL;
  }
  if (fastled_leds == nullptr) {
    return EINVAL;
  }
  snprintf(cmd->buffer, cmd->buffer_size, "%u", FastLED.getBrightness());
  return 0;
}

int fastled_set_max_refresh_rate(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 2) {
    return EINVAL;
  }
  uint16_t rate = (uint16_t) strtol(argv[1], nullptr, 0);
  FastLED.setMaxRefreshRate(rate);
  return 0;
}


void loop() {
  // TODO: remove this check on if Serial is available.
  // I don't think we need it since it is already in a loop
  if (Serial.available()) {
    cmd.processSerialStream();
  }
}
