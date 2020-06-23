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


#include "Wire.h"
#include <applibs/i2c.h>

#define DEBUG_I2C


static const size_t DEFAULT_BUFFER_LIMIT = 32;

void TwoWire(int port_id)
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

void wire_create(void)
{
  wire_end();
}

void wire_begin2()
{
  if (fd >= 0)
  {
    DEBUG_I2C("[I2C] already open\n");
    return;
  }
  wire_changeBufferLimits(DEFAULT_BUFFER_LIMIT, DEFAULT_BUFFER_LIMIT);
  fd = I2CMaster_Open(id);
  DEBUG_I2C("[I2C] I2CMaster_Open = %d\n", fd);
}

void wire_end()
{
  if (fd >= 0)
  {
    fd = -1;
  }
}

void wire_setClock(uint32_t frequency)
{
  int rc = I2CMaster_SetBusSpeed(fd, I2C_BUS_SPEED_STANDARD);
  if (rc != 0)
  {
    DEBUG_I2C("[ERROR] I2CMaster_SetBusSpeed\n");
  }
}

uint8_t wire_requestFrom2(uint8_t address, uint8_t quantity, uint8_t sendStop)
{
  if (txBufferLength > 0 && address != txAddress)
  {
    wire_endTransmission(true);
  }

  quantity = min(quantity, rxBufferLimit);
  ssize_t res = -1;
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

void wire_beginTransmission(uint8_t address)
{
  txAddress = address;
  txBufferLength = 0;
}

uint8_t wire_endTransmission(uint8_t sendStop)
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

uint8_t wire_endTransmission2(void)
{
  return wire_endTransmission(true);
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

int wire_peek(void)
{
  int value = -1;
  if (rxBufferIndex < rxBufferLength)
  {
    value = rxBuffer[rxBufferIndex];
  }
  return value;
}

void wire_begin(uint8_t address)
{
  wire_begin2();
}

uint8_t wire_requestFrom(uint8_t address, uint8_t quantity)
{
  return wire_requestFrom2(address, quantity, (uint8_t) true);
}

void wire_changeBufferLimits(size_t rxLimit, size_t txLimit)
{
  rxLimit = max(1, rxLimit);
  txLimit = max(1, txLimit);

  if (rxBufferLimit != rxLimit)
  {
    if (rxBuffer != NULL)
    {
      free(rxBuffer);
    }
    rxBufferLimit = rxLimit;
    rxBuffer = malloc(rxBufferLimit);
    rxBufferIndex = 0;
    rxBufferLength = 0;
  }

  if (txBufferLimit != txLimit)
  {
    if (txBuffer != NULL)
    {
      free(txBuffer);
    }
    txBufferLimit = txLimit;
    txBuffer = malloc(txBufferLimit);
    txBufferLength = 0;
  }
}
