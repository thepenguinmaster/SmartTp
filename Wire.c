/*
    Created on: 01.01.2019
    Author: Georgi Angelov
        http://www.wizio.eu/
        https://github.com/Wiz-IO

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
  
  Modified: Junxiao Shi
 */

//#include <alltypes.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "avnet_aesms_mt3620.h"
#include <Arduino.h>

//#define min(a, b) ((a) < (b) ? (a) : (b))
//#define max(a, b) ((a) > (b) ? (a) : (b))

//#include "Arduino.h"
#include "Wire.h"
#include <applibs/i2c.h>

//#include <variant.h>
#define DEBUG_I2C
//Serial.printf

static const size_t DEFAULT_BUFFER_LIMIT = 32;

TwoWire(int port_id)
{
  fd = -1;
  id = port_id;
  rxBuffer = NULL;
  rxBufferLength = 0;
  rxBufferLimit = 0;
  txAddress = 0;
  txBuffer = NULL;
  txBufferLength = 0;
  txBufferLimit = 0;
}

TwoWireCtor()
{
  end();
}

void begin2()
{
  if (fd >= 0)
  {
    DEBUG_I2C("[I2C] already open\n");
    return;
  }
  changeBufferLimits(DEFAULT_BUFFER_LIMIT, DEFAULT_BUFFER_LIMIT);
  fd = I2CMaster_Open(id);
  DEBUG_I2C("[I2C] I2CMaster_Open = %d\n", fd);
}

void end()
{
  if (fd >= 0)
  {
    //close(fd);
    fd = -1;
  }
  //  if (rxBuffer != nullptr) {
  ////    delete[] rxBuffer;
  //    rxBufferLimit = 0;
  //  }
  //  if (txBuffer != nullptr) {
  //    delete[] txBuffer;
  //   txBufferLimit = 0;
  // }
}

void setClock(uint32_t frequency)
{
  int rc = I2CMaster_SetBusSpeed(fd, I2C_BUS_SPEED_STANDARD);
  if (rc != 0)
  {
    DEBUG_I2C("[ERROR] I2CMaster_SetBusSpeed\n");
  }
}

uint8_t requestFrom2(uint8_t address, uint8_t quantity, uint8_t sendStop)
{
  if (txBufferLength > 0 && address != txAddress)
  {
    endTransmission(true);
  }

  quantity = min(quantity, rxBufferLimit);
  size_t res = -1;
  if (txBufferLength > 0)
  {
    res = I2CMaster_WriteThenRead(fd, address,
                                  txBuffer, txBufferLength, rxBuffer, quantity);
    DEBUG_I2C("[I2C] I2CMaster_WriteThenRead = %d, errno = %d\n", res, errno);
  }
  else
  {
    res = I2CMaster_Read(fd, address, rxBuffer, quantity);
    DEBUG_I2C("[I2C] I2CMaster_Read = %d, errno = %d\n", res, errno);
  }

  rxBufferIndex = 0;
  rxBufferLength = res < 0 ? 0 : res - txBufferLength;
  txBufferLength = 0;
  return rxBufferLength;
}

void beginTransmission(uint8_t address)
{
  txAddress = address;
  txBufferLength = 0;
}

uint8_t endTransmission(uint8_t sendStop)
{
  if (!sendStop)
  {
    return 0;
  }

  int res = I2CMaster_Write(fd, txAddress,
                            txBuffer, txBufferLength);
  DEBUG_I2C("[I2C] I2CMaster_Write = %d, errno = %d\n", res, errno);
  txBufferLength = 0;

  if (res < 0)
  {
    switch (errno)
    {
    case ENXIO:
      return 3;
    default:
      return 4;
    }
  }
  return 0;
}

uint8_t endTransmission2(void)
{
  return endTransmission(true);
}

size_t wire_write(uint8_t data)
{
  if (txBufferLength >= txBufferLimit)
  {
    return 0;
  }
  txBuffer[txBufferLength++] = data;
  return 1;
}

size_t wire_write2(const uint8_t *data, size_t quantity)
{
  size_t count = 0;
  for (size_t i = 0; i < quantity; ++i)
  {
    count += wire_write(data[i]);
  }
  return count;
}

int wire_available(void)
{
  return rxBufferLength - rxBufferIndex;
}

int wire_read(void)
{
  int value = -1;
  if (rxBufferIndex < rxBufferLength)
  {
    value = rxBuffer[rxBufferIndex];
    ++rxBufferIndex;
  }
  return value;
}

int peek(void)
{
  int value = -1;
  if (rxBufferIndex < rxBufferLength)
  {
    value = rxBuffer[rxBufferIndex];
  }
  return value;
}

void begin(uint8_t address)
{
  begin2();
}

uint8_t requestFrom(uint8_t address, uint8_t quantity)
{
  return requestFrom2(address, quantity, (uint8_t) true);
}

void changeBufferLimits(size_t rxLimit, size_t txLimit)
{
  rxLimit = max(1, rxLimit);
  txLimit = max(1, txLimit);

  if (rxBufferLimit != rxLimit)
  {
     if (rxBuffer != NULL) {
     //delete[] rxBuffer;
     free(rxBuffer);
       }
    rxBufferLimit = rxLimit;
  // rxBuffer = new uint8_t[rxBufferLimit];
   rxBuffer= malloc(rxBufferLimit);
    rxBufferIndex = 0;
    rxBufferLength = 0;
  }

  if (txBufferLimit != txLimit)
  {
    if (txBuffer != NULL) {
      //delete[] txBuffer;
        free(txBuffer);
     }
    txBufferLimit = txLimit;
      //txBuffer = new uint8_t[txBufferLimit];
     txBuffer = malloc(txBufferLimit);
    txBufferLength = 0;
  }
}
