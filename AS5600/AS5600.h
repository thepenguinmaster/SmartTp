#ifndef AMS_5600_h
#define AMS_5600_h

typedef unsigned char   uint8_t;
typedef unsigned short  uint16_t;


typedef uint8_t byte;
typedef uint16_t word;

void AS5600_open(int ic2Id);
uint8_t getAddress(void);

word setMaxAngle(word newMaxAngle);
word getMaxAngle(void);

word setStartPosition(word startAngle);
word getStartPosition(void);

word setEndPosition(word endAngle);
word getEndPosition(void);

word getRawAngle(void);
word getScaledAngle(void);

uint8_t detectMagnet(void);
uint8_t getMagnetStrength(void);
uint8_t getAgc(void);
word getMagnitude(void);

uint8_t getBurnCount(void);
int8_t burnAngle(void);
int8_t burnMaxAngleAndConfig(void);
void setOutPut(uint8_t mode);

//private:

static const uint8_t _ams5600_Address =0x36;
word _rawStartAngle;
word _zPosition;
word _rawEndAngle;
word _mPosition;
word _maxAngle;

/* Registers */
uint8_t _zmco;
uint8_t _zpos_hi; /*zpos[11:8] high nibble  START POSITION */
uint8_t _zpos_lo; /*zpos[7:0] */
uint8_t _mpos_hi; /*mpos[11:8] high nibble  STOP POSITION */
uint8_t _mpos_lo; /*mpos[7:0] */
uint8_t _mang_hi; /*mang[11:8] high nibble  MAXIMUM ANGLE */
uint8_t _mang_lo; /*mang[7:0] */
uint8_t _conf_hi;
uint8_t _conf_lo;
uint8_t _raw_ang_hi;
uint8_t _raw_ang_lo;
uint8_t _ang_hi;
uint8_t _ang_lo;
uint8_t _stat;
uint8_t _agc;
uint8_t _mag_hi;
uint8_t _mag_lo;
uint8_t _burn;

uint8_t readOneByte(uint8_t in_adr);
word readTwoBytes(uint8_t in_adr_hi, uint8_t in_adr_lo);
void writeOneByte(uint8_t adr_in, uint8_t dat_in);
float convertRawAngleToDegrees(word newAngle);
#endif