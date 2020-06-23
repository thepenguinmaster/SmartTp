

#include <Arduino.h>
#include "AS5600.h"
#include <applibs/i2c.h>
#include "../wire/Wire.h"
static int i2cFd = -1;
int _ic2Id;
int ang, lang = 0;

void AS5600_open(int ic2Id)
{
  /* set i2c address */
  _ic2Id = ic2Id;
TwoWire(ic2Id);
wire_begin2();
  //_ams5600_Address = 0x36;
  /* load register values*/
  /* c++ class forbids pre loading of variables */
  _zmco = 0x00;
  _zpos_hi = 0x01;
  _zpos_lo = 0x02;
  _mpos_hi = 0x03;
  _mpos_lo = 0x04;
  _mang_hi = 0x05;
  _mang_lo = 0x06;
  _conf_hi = 0x07;
  _conf_lo = 0x08;
  _raw_ang_hi = 0x0c;
  _raw_ang_lo = 0x0d;
  _ang_hi = 0x0e;
  _ang_lo = 0x0f;
  _stat = 0x0b;
  _agc = 0x1a;
  _mag_hi = 0x1b;
  _mag_lo = 0x1c;
  _burn = 0xff;
  // i2cFd = I2CMaster_Open(_ams5600_Address);
  //i2cFd = I2CMaster_Open(AVNET_AESMS_ISU0_I2C);
  i2cFd = I2CMaster_Open(_ic2Id);
  if (i2cFd == -1)
  {
    Log_Debug("ERROR: I2CMaster_Open: errno=%d (%s)\n", errno, strerror(errno));
    return;
  }

  int result;
  result = I2CMaster_SetBusSpeed(i2cFd, I2C_BUS_SPEED_STANDARD);

  //result = I2CMaster_SetTimeout(i2cFd, 100);
  //result = I2CMaster_SetDefaultTargetAddress(i2cFd, _ams5600_Address);
  Log_Debug("I2C Result %s\n", result);
}

/* mode = 0, output PWM, mode = 1 output analog (full range from 0% to 100% between GND and VDD*/
void setOutPut(uint8_t mode)
{
  uint8_t config_status;
  config_status = readOneByte(_conf_lo);
  if (mode == 1)
  {
    config_status = config_status & 0xcf;
  }
  else
  {
    config_status = config_status & 0xef;
  }
  writeOneByte(_conf_lo, lowByte(config_status));
}

uint8_t getAddress()
{
  return _ams5600_Address;
}

word setMaxAngle(word newMaxAngle)
{
  word retVal;
  if (newMaxAngle == -1)
  {
    _maxAngle = getRawAngle();
  }
  else
    _maxAngle = newMaxAngle;

  writeOneByte(_mang_hi, highByte(_maxAngle));
  delay(2);
  writeOneByte(_mang_lo, lowByte(_maxAngle));
  delay(2);

  retVal = readTwoBytes(_mang_hi, _mang_lo);
  return retVal;
}

word getMaxAngle()
{
  return readTwoBytes(_mang_hi, _mang_lo);
}

word setStartPosition(word startAngle)
{
  if (startAngle == -1)
  {
    _rawStartAngle = getRawAngle();
  }
  else
    _rawStartAngle = startAngle;

  writeOneByte(_zpos_hi, highByte(_rawStartAngle));
  delay(2);
  writeOneByte(_zpos_lo, lowByte(_rawStartAngle));
  delay(2);
  _zPosition = readTwoBytes(_zpos_hi, _zpos_lo);

  return (_zPosition);
}

word getStartPosition()
{
  return readTwoBytes(_zpos_hi, _zpos_lo);
}

word setEndPosition(word endAngle)
{
  if (endAngle == -1)
    _rawEndAngle = getRawAngle();
  else
    _rawEndAngle = endAngle;

  writeOneByte(_mpos_hi, highByte(_rawEndAngle));
  delay(2);
  writeOneByte(_mpos_lo, lowByte(_rawEndAngle));
  delay(2);
  _mPosition = readTwoBytes(_mpos_hi, _mpos_lo);

  return (_mPosition);
}

word getEndPosition()
{
  word retVal = readTwoBytes(_mpos_hi, _mpos_lo);
  return retVal;
}

word getRawAngle()
{
  return readTwoBytes(_raw_ang_hi, _raw_ang_lo);
}

word getScaledAngle()
{
  return readTwoBytes(_ang_hi, _ang_lo);
}

uint8_t detectMagnet()
{
  uint8_t magStatus;
  uint8_t retVal = 0;

  magStatus = readOneByte(_stat);

  if (magStatus & 0x20)
    retVal = 1;

  return retVal;
}

uint8_t getMagnetStrength()
{
  uint8_t magStatus;
  uint8_t retVal = 0;

  magStatus = readOneByte(_stat);
  if (detectMagnet() == 1)
  {
    retVal = 2; /*just right */
    if (magStatus & 0x10)
      retVal = 1; /*to weak */
    else if (magStatus & 0x08)
      retVal = 3; /*to strong */
  }

  return retVal;
}

uint8_t getAgc()
{
  return readOneByte(_agc);
}

word getMagnitude()
{
  return readTwoBytes(_mag_hi, _mag_lo);
}

uint8_t getBurnCount()
{
  return readOneByte(_zmco);
}

int8_t burnAngle()
{
  int8_t retVal = 1;
  _zPosition = getStartPosition();
  _mPosition = getEndPosition();
  _maxAngle = getMaxAngle();

  if (detectMagnet() == 1)
  {
    if (getBurnCount() < 3)
    {
      if ((_zPosition == 0) && (_mPosition == 0))
        retVal = -3;
      else
        writeOneByte(_burn, 0x80);
    }
    else
      retVal = -2;
  }
  else
    retVal = -1;

  return retVal;
}

int8_t burnMaxAngleAndConfig()
{
  int8_t retVal = 1;
  _maxAngle = getMaxAngle();

  if (getBurnCount() == 0)
  {
    if (_maxAngle * 0.087 < 18)
      retVal = -2;
    else
      writeOneByte(_burn, 0x40);
  }
  else
    retVal = -1;

  return retVal;
}

uint8_t readOneByte(uint8_t in_adr)
{
  uint8_t retVal;
  //ssize_t transferredBytes =
  I2CMaster_WriteThenRead(i2cFd, _ams5600_Address, &in_adr, sizeof(in_adr), &retVal, sizeof(retVal));
  return retVal;
}


word readTwoBytes(uint8_t in_adr_hi, uint8_t in_adr_lo)
{
  word retVal = 0;
 
  /* Read Low Byte */
  wire_beginTransmission(_ams5600_Address);
  wire_write(in_adr_lo);
  wire_endTransmission(true);
  wire_requestFrom(_ams5600_Address, 1);
  while(wire_available() == 0);
  int low = wire_read();
 
  /* Read High Byte */  
  wire_beginTransmission(_ams5600_Address);
  wire_write(in_adr_hi);
  wire_endTransmission(true);
  wire_requestFrom(_ams5600_Address, 1);
  
  while(wire_available() == 0);
  
  word high = wire_read();
  
  high = high << 8;
  retVal = high | low;
  
  return retVal;
}


void writeOneByte(uint8_t adr_in, uint8_t dat_in)
{
  wire_beginTransmission(_ams5600_Address);
  wire_write(adr_in); 
  wire_write(dat_in);
  wire_endTransmission(true);
}

float convertRawAngleToDegrees(word newAngle)
{
    /* Raw data reports 0 - 4095 segments, which is 0.087 of a degree */
    float retVal = newAngle * 0.087;
    ang = retVal;
    return ang;
}