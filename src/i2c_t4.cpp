#include "i2c_t4.hpp"

//
// I2C data structures - these need to be global so "C" ISRs can use them
//
struct i2cStruct i2cData[I2C_BUS_NUM];

i2c_t4 Wire  = i2c_t4(0);       // I2C0
i2c_t4 Wire1 = i2c_t4(1);       // I2C1
i2c_t4 Wire2 = i2c_t4(2);       // I2C2

i2c_t4::i2c_t4(uint8_t i2c_bus){
    bus = i2c_bus;
    i2c = &i2cData[bus];
    i2c->opMode = I2C_OP_MODE_IMM;

}

i2c_t4::~i2c_t4()
{
#if 0
    // we still don't support the DMA mode.
    // if DMA active, delete DMA object
    if(i2c->opMode == I2C_OP_MODE_DMA)
        delete i2c->DMA;
#endif
}

static void i2c_t4::begin_(
    struct i2cStruct* i2c, uint8_t bus, i2c_mode mode, uint8_t address1, uint8_t address2,
    uint8_t pinSCL, uint8_t pinSDA, i2c_pullup pullup, uint32_t rate, i2c_op_mode opMode
) {
    // TODO: do this...
}



static uint8_t i2c_t4::finish_(struct i2cStruct* i2c, uint8_t bus, uint32_t timeout)
{
    // TODO: do this
}

static void i2c_t4::resetBus_(struct i2cStruct* i2c, uint8_t bus)
{
    // TODO: do this
}
