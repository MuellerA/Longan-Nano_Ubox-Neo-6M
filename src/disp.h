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

class UbxNav ;

//////////////////////////////////////////////////////////////////////////
//  title 0, 110             | fix 115, 20 | sats 135, 20  | tow 155, 5 //
//  time 0, 160                                                         //
//  lat lbl 0, 40     | lat val 40, 100                                 //
//  lon lbl 0, 40     | lon val 40, 100                                 //
//  alt lbl 0, 40     | alt val 40, 100                  | asc 150,10   //
//////////////////////////////////////////////////////////////////////////
//  title 0, 110             | fix 115, 20 | sats 135, 20  | tow 155, 5 //
//  data 0, 150                                                         //
//  data 0, 150                                                         //
//  data 0, 150                                                         //
//  data 0, 150                                                         //
//////////////////////////////////////////////////////////////////////////

class DispTime
{
public:
  DispTime() ;
  void display(const UbxNav &nav, bool force=false) ;
private:
  LcdArea   _la ;
  uint32_t  _iTOW ;
} ;

class DispPos
{
public:
  DispPos() ;
  void label() ;
  void display(const UbxNav &nav, bool force=false) ;
private:
  LcdArea   _laLat ;
  LcdArea   _laLon ;
  LcdArea   _laAlt ;
  uint32_t  _iTOW ;
} ;

class DispGpsFix
{
public:
  DispGpsFix() ;
  void display(const UbxNav &nav, bool force=false) ;
private:
  LcdArea   _la ;
  uint32_t  _iTOW ;
} ;

class DispSats
{
public:
  DispSats() ;
  void display(const UbxNav &nav, bool force=false) ;
private:
  LcdArea   _la ;
  uint32_t  _iTOW ;
} ;

class DispSvInfo
{
public:
  DispSvInfo() ;
  void display(const UbxNav &nav, bool force=false) ;
private:
  LcdArea   _la ;
  uint32_t  _iTOW ;
} ;

class DispTow
{
public:
  DispTow() ;
  void display(const UbxNav &nav, bool force=false) ;
private:
  LcdArea   _la ;
  uint32_t  _iTOW ;
} ;

class DispAscFound
{
public:
  DispAscFound(uint32_t timeout) ;
  void set(char c = '$') ;
  void display(bool force=false) ;
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
