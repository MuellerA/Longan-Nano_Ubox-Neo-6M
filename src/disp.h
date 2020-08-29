////////////////////////////////////////////////////////////////////////////////
// disp.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

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

//////////////////////////////////////////////////////////////////////////
//  title 0, 110             | fix 115, 20 | nChan 135, 20 | tow 155, 5 //
//  time 0, 160                                                         //
//  lat lbl 0, 40     | lat val 40, 100                                 //
//  lon lbl 0, 40     | lon val 40, 100                                 //
//  alt lbl 0, 40     | alt val 40, 100                  | asc 150,10   //
//////////////////////////////////////////////////////////////////////////
//  title 0, 110             | fix 115, 20 | nChan 135, 20 | tow 155, 5 //
//  data 0, 150                                                         //
//  data 0, 150                                                         //
//  data 0, 150                                                         //
//  data 0, 150                                                         //
//////////////////////////////////////////////////////////////////////////

class DispTime
{
public:
  DispTime(uint32_t timeout) ;
  void set(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec, uint8_t valid) ;
  void display(uint8_t page, bool force=false) ;
private:
  TickTimer _t ;
  LcdArea   _la ;
  uint16_t _year ;
  uint8_t _month ;
  uint8_t _day ;
  uint8_t _hour ;
  uint8_t _min ;
  uint8_t _sec, _sec0 ;
  uint8_t _valid ;
} ;

class DispPos
{
public:
  DispPos(uint32_t timeout) ;
  void set(int32_t lat, int32_t lon, int32_t alt) ;
  void display(uint8_t page, bool force=false) ;
private:
  TickTimer _t ;
  LcdArea   _laLat ;
  LcdArea   _laLon ;
  LcdArea   _laAlt ;
  int32_t   _lat, _lat0 ;
  int32_t   _lon, _lon0 ;
  int32_t   _alt, _alt0 ;
} ;

class DispInd
{
public:
  DispInd(uint32_t x, uint32_t timeout) ;
  void set(int8_t ind) ;
  void display(uint8_t page, bool force=false) ;
private:
  TickTimer _t ;
  LcdArea   _la ;
  uint8_t   _ind, _ind0 ;
} ;

struct SvInfo
{
  uint8_t chn ;
  uint8_t svid ;
  uint8_t flags ;
  uint8_t quality ;
  uint8_t cno ;
} ;

class DispSvInfo
{
public:
  DispSvInfo(uint32_t x, uint32_t timeout) ;
  void set(std::vector<SvInfo> &&svInfos) ;
  void display(uint8_t page, bool force=false) ;
private:
  TickTimer _t ;
  LcdArea   _laSat ;
  LcdArea   _laSvInfos ;
  uint8_t   _sat, _sat0 ;
  std::vector<SvInfo> _svInfos ;
  uint8_t   _svInfosX, _svInfosX0 ;
} ;

class DispTow
{
public:
  DispTow(uint32_t timeout) ;
  void set(uint32_t tow) ;
  void display(uint8_t page, bool force=false) ;
private:
  TickTimer _t ;
  LcdArea   _la ;
  uint32_t  _tow, _tow0 ;
  bool      _toggle ;
} ;

class DispAscFound
{
public:
  DispAscFound(uint32_t timeout) ;
  void set(char c = '$') ;
  void display(uint8_t page, bool force=false) ;
private:
  TickTimer _t ;
  LcdArea   _la ;
  char      _c, _c0 ;
} ;

////////////////////////////////////////////////////////////////////////////////

void dispSetup() ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
