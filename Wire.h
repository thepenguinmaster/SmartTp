#ifndef TwoWire_h
#define TwoWire_h

#include <inttypes.h>
#include <Arduino.h>

extern uint8_t TWBR;

#define BUFFER_LENGTH 32

int id;
int fd;
uint8_t *rxBuffer;
size_t rxBufferIndex;
size_t rxBufferLength;
size_t rxBufferLimit;
uint8_t txAddress;
uint8_t *txBuffer;
size_t txBufferLength;
size_t txBufferLimit;

TwoWire(int port_id);
TwoWireCtor();

void end();
void begin2();
void begin(uint8_t);
void setClock(uint32_t);
void beginTransmission(uint8_t);
uint8_t endTransmission2(void);
uint8_t endTransmission(uint8_t);
uint8_t requestFrom(uint8_t, uint8_t);
uint8_t requestFrom2(uint8_t, uint8_t, uint8_t);
size_t wire_write(uint8_t);
size_t wire_write2(const uint8_t *data, size_t quantity);
int wire_available(void);
int wire_read(void);
int peek(void);
void flush(void) ;

/** \brief Change buffer size limits.
     *  \param rxLimit read buffer size.
     *  \param txLimit write buffer size.
     */
void changeBufferLimits(size_t rxLimit, size_t txLimit);

//extern TwoWire Wire;
//extern TwoWire Wire1;

#endif
