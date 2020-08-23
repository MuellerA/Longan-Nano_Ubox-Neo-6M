////////////////////////////////////////////////////////////////////////////////
// disp.h
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
  #include "gd32vf103.h"
}

#include "GD32VF103/time.h"
#include "Longan/lcd.h"
#include "Longan/fonts.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::Longan::Lcd ;
using ::RV::Longan::LcdArea ;

////////////////////////////////////////////////////////////////////////////////
//   |  title 0, 110             | fix 115, 20 | nChan 135, 20 | tow 155, 5 |
//   |  time 0, 160                                                         |
//   |  lat lbl 0, 40     | lat val 40, 120                                 |
//   |  lon lbl 0, 40     | lon val 40, 120                                 |
//   |  alt lbl 0, 40     | alt val 40, 120                                 |
////////////////////////////////////////////////////////////////////////////////

class DispTime
{
public:
  DispTime(uint32_t timeout) ;
  void set(uint32_t iTOW, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec, uint8_t valid) ;
  void expire() ;
private:
  TickTimer _t ;
  LcdArea   _la ;
  uint32_t  _iTOW ;
} ;

class DispPos
{
public:
  DispPos(uint32_t timeout) ;
  void set(int32_t lat, int32_t lon, int32_t alt) ;
  void expire() ;
private:
  TickTimer _t ;
  LcdArea   _laLat ;
  LcdArea   _laLon ;
  LcdArea   _laAlt ;
  int32_t   _lat ;
  int32_t   _lon ;
  int32_t   _alt ;
} ;

class DispInd
{
public:
  DispInd(uint32_t x, uint32_t timeout) ;
  void set(int8_t ind) ;
  void expire() ;
private:
  TickTimer _t ;
  LcdArea   _la ;
  uint8_t   _ind ;
} ;

class DispTow
{
public:
  DispTow(uint32_t timeout) ;
  void set(uint32_t tow) ;
  void expire() ;
private:
  TickTimer _t ;
  LcdArea   _la ;
  uint32_t  _tow ;
  bool      _toggle ;
} ;

////////////////////////////////////////////////////////////////////////////////

void dispAscFound() ;
void dispSetup() ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
