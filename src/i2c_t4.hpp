#pragma once

#include <unistd.h>

// Tx/Rx buffer sizes - modify these as needed.  Buffers should be large enough to hold:
//                      Target Addr + Data payload.  Default is: 1byte Addr + 258byte Data
//                      (this can be substantially reduced if working with sensors or small data packets)
//
#define I2C_TX_BUFFER_LENGTH 259
#define I2C_RX_BUFFER_LENGTH 259

enum i2c_op_mode  {I2C_OP_MODE_IMM, I2C_OP_MODE_ISR, I2C_OP_MODE_DMA};
enum i2c_mode     {I2C_MASTER, I2C_SLAVE};
enum i2c_pullup   {I2C_PULLUP_EXT, I2C_PULLUP_INT};

enum i2c_pins {
    I2C_PINS_16_17 = 0,      // 16 SCL1  17 SDA1
    I2C_PINS_18_19,          // 19 SCL0  18 SDA0
    I2C_PINS_25_24,          // 24 SCL2  25 SDA2
    I2C_PINS_DEFAULT,
    I2C_PINS_COUNT,
};
#define I2C_BUS_NUM 3
enum i2c_stop     {I2C_NOSTOP, I2C_STOP};
const uint8_t i2c_valid_pins[] = { 0, 16, 17, 2,  // bus, scl, sda, alt
                                   0, 19, 18, 2,
                                   0, 25, 24, 2,
                                   0,  0,  0, 0 };
struct i2cStruct
{
    uint8_t  rxBuffer[I2C_RX_BUFFER_LENGTH]; // Rx Buffer                         (ISR)
    volatile size_t   rxBufferIndex;         // Rx Index                          (User&ISR)
    volatile size_t   rxBufferLength;        // Rx Length                         (ISR)
    uint8_t  txBuffer[I2C_TX_BUFFER_LENGTH]; // Tx Buffer                         (User)
    volatile size_t   txBufferIndex;         // Tx Index                          (User&ISR)
    volatile size_t   txBufferLength;        // Tx Length                         (User&ISR)
    i2c_op_mode opMode;                      // Operating Mode                    (User)
    i2c_mode currentMode;                    // Current Mode                      (User)
    uint32_t defTimeout;                     // Default Timeout                   (User)
    i2c_stop currentStop;                    // Current Stop                      (User)
};
// class i2c_t4 : public Stream{
class i2c_t4 {
public:
    // ------------------------------------------------------------------------------------------------------
    // Constructor
    //
    i2c_t4(uint8_t i2c_bus);
    ~i2c_t4();

    //
    // Initialize I2C (Master) - initializes I2C as Master mode, external pullups, 100kHz rate,
    //                           and default pin setting
    // default pin setting SCL/SDA:
    //      Wire:   19/18
    // return: none
    //
    inline void begin(void)
        { begin_(i2c, bus, I2C_MASTER, 0, 0, 0, 0, I2C_PULLUP_EXT, 100000, I2C_OP_MODE_ISR); }

    inline void begin(i2c_mode mode, uint8_t address1, i2c_pins pins=I2C_PINS_DEFAULT,
                      i2c_pullup pullup=I2C_PULLUP_EXT, uint32_t rate=400000)
        { begin_(i2c, bus, mode, address1, 0, mapSCL(pins), mapSDA(pins), pullup, rate, I2C_OP_MODE_ISR); }

    inline void begin(i2c_mode mode, uint8_t address1, i2c_pins pins=I2C_PINS_DEFAULT,
                      i2c_pullup pullup=I2C_PULLUP_EXT, uint32_t rate=400000, i2c_op_mode opMode=I2C_OP_MODE_ISR)
        { begin_(i2c, bus, mode, address1, 0, mapSCL(pins), mapSDA(pins), pullup, rate, opMode); }

    // ------------------------------------------------------------------------------------------------------
    // Set Default Timeout - sets the default timeout to be applied to all functions called with a timeout of
    //                       zero (the default in cases where timeout is not specified).  The default is
    //                       initially zero (infinite wait).
    // return: none
    // parameters:
    //      timeout = timeout in microseconds
    inline void setDefaultTimeout(uint32_t timeout) { i2c->defTimeout = timeout; }

    static void resetBus_(struct i2cStruct* i2c, uint8_t bus);
    inline void resetBus(void) { resetBus_(i2c, bus); }

    // ------------------------------------------------------------------------------------------------------
    // Setup Master Transmit - initialize Tx buffer for transmit to slave at address
    // return: none
    // parameters:
    //      address = target 7bit slave address
    //
    void beginTransmission(uint8_t address);

    // ------------------------------------------------------------------------------------------------------
    // Master Transmit (base routine) - cannot be static due to call to getError() and in turn getWriteError()
    //
    uint8_t endTransmission(struct i2cStruct* i2c, uint8_t bus, i2c_stop sendStop, uint32_t timeout);
    //
    // Master Transmit - blocking routine, transmits Tx buffer to slave. i2c_stop parameter can be used
    //                   to indicate if command should end with a STOP(I2C_STOP) or not (I2C_NOSTOP).
    //                   Timeout parameter can be optionally specified.
    // return: 0=success, 1=data too long, 2=recv addr NACK, 3=recv data NACK, 4=other error
    // parameters:
    //      ^i2c_stop = I2C_NOSTOP, I2C_STOP (default STOP)
    //      ^timeout = timeout in microseconds (default 0 = infinite wait)
    //
    inline uint8_t endTransmission(i2c_stop sendStop=I2C_STOP, uint32_t timeout=0) { return endTransmission(i2c, bus, sendStop, timeout); }
    inline uint8_t endTransmission(uint8_t sendStop) { return endTransmission(i2c, bus, (i2c_stop)sendStop, 0); } // Wire compatibility

    // ------------------------------------------------------------------------------------------------------
    // Get Wire Error - returns "Wire" error code from a failed Tx/Rx command
    // return: 0=success, 1=data too long, 2=recv addr NACK, 3=recv data NACK, 4=other error
    //
    uint8_t getError(void);


    // ------------------------------------------------------------------------------------------------------
    // Read Byte (base routine)
    //
    static uint8_t readByte_(struct i2cStruct* i2c);
    //
    // Read Byte - returns next data byte (uint8_t) from Rx buffer
    // return: data, 0 if buffer empty
    //
    inline uint8_t readByte(void) { return readByte_(i2c); }

    // ------------------------------------------------------------------------------------------------------
    // Write - write data byte to Tx buffer
    // return: #bytes written = success, 0=fail
    // parameters:
    //      data = data byte
    //
    size_t write(uint8_t data);
        // ------------------------------------------------------------------------------------------------------
    // Write Array - write count number of bytes from data array to Tx buffer
    // return: #bytes written = success, 0=fail
    // parameters:
    //      data = pointer to uint8_t (or char) array of data
    //      count = number of bytes to write
    //
    size_t write(const uint8_t* data, size_t count);

    // ------------------------------------------------------------------------------------------------------
    // Master Receive (base routine)
    //
    static size_t requestFrom_(struct i2cStruct* i2c, uint8_t bus, uint8_t addr, size_t len, i2c_stop sendStop, uint32_t timeout);
    //
    // Master Receive - Requests length bytes from slave at address. Receive data will be placed in the Rx buffer.
    //                  i2c_stop parameter can be used to indicate if command should end with a STOP (I2C_STOP) or
    //                  not (I2C_NOSTOP).  Timeout parameter can be optionally specified.
    // return: #bytes received = success, 0=fail (0 length request, NAK, timeout, or bus error)
    // parameters:
    //      address = target 7bit slave address
    //      length = number of bytes requested
    //     ^i2c_stop = I2C_NOSTOP, I2C_STOP (default STOP)
    //     ^timeout = timeout in microseconds (default 0 = infinite wait)
    //
    inline size_t requestFrom(uint8_t addr, size_t len, i2c_stop sendStop=I2C_STOP, uint32_t timeout=0)
        { return requestFrom_(i2c, bus, addr, len, sendStop, timeout); }

    // ------------------------------------------------------------------------------------------------------
    // Return Status (base routine)
    //
    static uint8_t finish_(struct i2cStruct* i2c, uint8_t bus, uint32_t timeout);
    //
    // Finish - blocking routine, loops until Tx/Rx is complete.  Timeout parameter can be optionally specified.
    // return: 1=success (Tx or Rx completed, no error), 0=fail (NAK, timeout or Arb Lost)
    // parameters:
    //      ^timeout = timeout in microseconds (default 0 = infinite wait)
    //
    inline uint8_t finish(uint32_t timeout=0) { return finish_(i2c, bus, timeout); }

    // ------------------------------------------------------------------------------------------------------
    // Available - returns number of remaining available bytes in Rx buffer
    // return: #bytes available
    //
    inline int available(void) { return i2c->rxBufferLength - i2c->rxBufferIndex; }

    // ------------------------------------------------------------------------------------------------------
    // Initialize I2C (base routine)
    //
    static void begin_(struct i2cStruct* i2c, uint8_t bus, i2c_mode mode, uint8_t address1, uint8_t address2,
                       uint8_t pinSCL, uint8_t pinSDA, i2c_pullup pullup, uint32_t rate, i2c_op_mode opMode);

private:
    // ------------------------------------------------------------------------------------------------------
    // Pin Mapping - convert i2c_pins enum into pin values, intended for internal use only
    //
    inline uint8_t mapSCL(i2c_pins pins) { return i2c_valid_pins[pins*4+1]; }
    inline uint8_t mapSDA(i2c_pins pins) { return i2c_valid_pins[pins*4+2]; }

    //
    // I2C bus number - this is a local, passed as an argument to base functions
    //                  since static functions cannot see it.
    uint8_t bus;

    //
    // I2C structure pointer - this is a local, passed as an argument to base functions
    //                         since static functions cannot see it.
    struct i2cStruct* i2c;
};

extern i2c_t4 Wire;
extern i2c_t4 Wire1;
extern i2c_t4 Wire2;
