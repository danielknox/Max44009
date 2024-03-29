//
//    FILE: Max44009.cpp
//  AUTHOR: Dan Knox + Rob Tillaart
// VERSION: 0.1.11
// PURPOSE: library for MAX44009 lux sensor Arduino
//     URL: https://github.com/RobTillaart/Arduino/tree/master/libraries
//
// Released to the public domain
// 0.1.11 - Play nice with other i2c devices (by passing a wire object)
// 0.1.10 - 2018-12-08 issue #118 Fix constructor esp8266
//          (thanks to Bolukan)
// 0.1.9  - 2018-07-01 issue #108 Fix shift math
//          (thanks Roland vandecook)
// 0.1.8  - 2018-05-13 issue #105 Fix read register
//          (thanks morxGrillmeister)
// 0.1.7  - 2018-04-02 issue #98 extend constructor for ESP8266
// 0.1.6  - 2017-07-26 revert double to float 
// 0.1.5  - updated history
// 0.1.4  - added setAutomaticMode() to max44009.h (thanks debsahu)
// 0.1.03 - added configuration
// 0.1.02 - added threshold code
// 0.1.01 - added interrupt code
// 0.1.00 - initial version

#include "Max44009.h"

Max44009::Max44009()
{
}

void Max44009::begin(uint8_t addr, TwoWire *theWire){
    _address = addr;
    _wire = theWire;
    init();
}

void Max44009::begin(uint8_t addr){
    _address = addr;
    _wire = &Wire;
    init();
}

void Max44009::begin(){
    _address = MAX44009_ADDRESS;
    _wire = &Wire;
    init();
}

void Max44009::init(){
    _data = 0;
    _error = 0;
    _wire->begin();
}

float Max44009::getLux(void)
{
    uint8_t dhi = read(MAX44009_LUX_READING_HIGH);
    uint8_t dlo = read(MAX44009_LUX_READING_LOW);
    uint8_t e = dhi >> 4;
    uint32_t m = ((dhi & 0x0F) << 4) + (dlo & 0x0F);
    m <<= e;
    float val = m * 0.045;
    return val;
}

int Max44009::getError()
{
    int e = _error;
    _error = 0;
    return e;
}

void Max44009::setHighThreshold(const float value)
{
    setThreshold(MAX44009_THRESHOLD_HIGH, value);
}

float Max44009::getHighThreshold(void)
{
    return getThreshold(MAX44009_THRESHOLD_HIGH);
}

void Max44009::setLowThreshold(const float value)
{
    setThreshold(MAX44009_THRESHOLD_LOW, value);
}

float Max44009::getLowThreshold(void)
{
    return getThreshold(MAX44009_THRESHOLD_LOW);
}

void Max44009::setThresholdTimer(const uint8_t value)
{
    write(MAX44009_THRESHOLD_TIMER, value);
}

uint8_t Max44009::getThresholdTimer()
{
    return read(MAX44009_THRESHOLD_TIMER);
}

void Max44009::setConfiguration(const uint8_t value)
{
    write(MAX44009_CONFIGURATION, value);
}

uint8_t Max44009::getConfiguration()
{
    return read(MAX44009_CONFIGURATION);
}

void Max44009::setAutomaticMode()
{
    uint8_t config = read(MAX44009_CONFIGURATION);
    config &= ~MAX44009_CFG_CONTINUOUS; // off
    config &= ~MAX44009_CFG_MANUAL;     // off
    write(MAX44009_CONFIGURATION, config);
}

void Max44009::setContinuousMode()
{
    uint8_t config = read(MAX44009_CONFIGURATION);
    config |= MAX44009_CFG_CONTINUOUS; // on
    config &= ~MAX44009_CFG_MANUAL;    // off
    write(MAX44009_CONFIGURATION, config);
}

void Max44009::setManualMode(uint8_t CDR, uint8_t TIM)
{
    if (CDR !=0) CDR = 1;
    if (TIM > 7) TIM = 7;
    uint8_t config = read(MAX44009_CONFIGURATION);
    config &= ~MAX44009_CFG_CONTINUOUS; // off
    config |= MAX44009_CFG_MANUAL;      // on
    config &= 0xF0; // clear CDR & TIM bits
    config |= CDR << 3 | TIM;
    write(MAX44009_CONFIGURATION, config);
}

///////////////////////////////////////////////////////////
//
// PRIVATE
//
void Max44009::setThreshold(const uint8_t reg, const float value)
{
    // TODO CHECK RANGE
    uint32_t m = round(value / 0.045);  // mulitply * 22.22222222 is faster.
    uint8_t e = 0;
    while (m > 255)
    {
        m >>= 1;
        e++;
    };
    m = (m >> 4) & 0x0F;
    e <<= 4;
    write(reg, e | m);
}

float Max44009::getThreshold(uint8_t reg)
{
    uint8_t data = read(reg);
    uint8_t e = (data & 0xF0) >> 4;
    uint32_t m = ((data & 0x0F) << 4) + 0x0F;
    m <<= e;
    float val = m * 0.045;
    return val;
}

uint8_t Max44009::read(uint8_t reg)
{
    _wire->beginTransmission(_address);
    _wire->write(reg);
    _error = _wire->endTransmission();
    if (_error != 0)
    {
        return _data; // last value
    }
    if (_wire->requestFrom(_address, (uint8_t) 1) != 1)
    {
        _error = 10;
        return _data; // last value
    }
#if (ARDUINO <  100)
    _data = _wire->receive();
#else
    _data = _wire->read();
#endif
    return _data;
}

void Max44009::write(uint8_t reg, uint8_t value)
{
    _wire->beginTransmission(_address);
    _wire->write(reg);
    _wire->write(value);
    _error = _wire->endTransmission();
}

// --- END OF FILE ---
