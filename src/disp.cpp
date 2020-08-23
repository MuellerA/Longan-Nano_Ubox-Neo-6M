////////////////////////////////////////////////////////////////////////////////
// disp.cpp
////////////////////////////////////////////////////////////////////////////////

#include "disp.h"

////////////////////////////////////////////////////////////////////////////////

Lcd &lcd(Lcd::lcd()) ;

////////////////////////////////////////////////////////////////////////////////

DispTime::DispTime(uint32_t timeout) :
  _t(timeout), _la(lcd), _iTOW{0xffffffff}
{
  _la.area(0, 160, 16, 16) ; _la.clear() ;
}

void DispTime::set(uint32_t iTOW, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec, uint8_t valid)
{
  _t.restart() ;
  iTOW /= 1000 ;
  if ((iTOW == _iTOW) || !(valid & 0b100))
    return ;
  _iTOW = iTOW ;
  _la.txtPos(0) ;
  _la.put(year, 4, '0') ;
  _la.put('-') ;
  _la.put(month, 2, '0') ;
  _la.put('-') ;
  _la.put(day, 2, '0') ;
  _la.put(' ') ;
  _la.put(hour, 2, '0') ;
  _la.put(':') ;
  _la.put(min, 2, '0') ;
  _la.put(':') ;
  _la.put(sec, 2, '0') ;
  _la.clearEOL() ;
}

void DispTime::expire()
{
  if (_t())
  {
    _la.clear() ;
    _iTOW = 0xffffffff; 
  }
}

////////////////////////////////////////////////////////////////////////////////

DispPos::DispPos(uint32_t timeout) :
  _t(timeout), _laLat(lcd), _laLon(lcd), _laAlt(lcd), _lat{0x7fffffff}, _lon{0x7fffffff}, _alt{0x7fffffff}
{
  _laLat.area( 0,  40, 32, 16) ; _laLat.clear() ; _laLat.put("Lat") ;
  _laLat.area(40, 100, 32, 16) ; _laLat.clear() ;

  _laLon.area( 0,  40, 48, 16) ; _laLon.clear() ; _laLon.put("Lon") ;
  _laLon.area(40, 100, 48, 16) ; _laLon.clear() ;

  _laAlt.area( 0,  40, 64, 16) ; _laAlt.clear() ; _laAlt.put("Alt") ;
  _laAlt.area(40, 100, 64, 16) ; _laAlt.clear() ;
}

void DispPos::set(int32_t lat, int32_t lon, int32_t alt)
{
  _t.restart() ;
  {
    lat /= 1000 ;
    if (lat == _lat)
      return ;
    _lat = lat ;
    int32_t  g = lat / 10000 ;
    int32_t t = lat - g*10000 ;
    if (t < 0) t = -t ;
    _laLat.txtPos(0) ;
    _laLat.put(g) ;
    _laLat.put('.') ;
    _laLat.put(t, 4, '0') ;
    _laLat.clearEOL() ;
  }
  {
    lon /= 1000 ;
    if (lon == _lon)
      return ;
    _lon = lon ;
    int32_t  g = lon / 10000 ;
    int32_t t = lon - g*10000 ;
    if (t < 0) t = -t ;
    _laLon.txtPos(0) ;
    _laLon.put(g) ;
    _laLon.put('.') ;
    _laLon.put(t, 4, '0') ;
    _laLon.clearEOL() ;
  }
  {
    alt /= 100 ;
    if (alt == _alt)
      return ;
    _alt = alt ;
    int32_t  g = alt / 10 ;
    int32_t t = alt - g*10 ;
    if (t < 0) t = -t ;

    _laAlt.txtPos(0) ;
    _laAlt.put(g) ;
    _laAlt.put('.') ;
    _laAlt.put(t) ;
    _laAlt.clearEOL() ;
  }
}

void DispPos::expire()
{
  if (_t())
  {
    _laLat.clear() ;
    _laLon.clear() ;
    _laAlt.clear() ;
    _lat = 0x7fffffff; 
    _lon = 0x7fffffff; 
    _alt = 0x7fffffff; 
  }
}

////////////////////////////////////////////////////////////////////////////////

DispInd::DispInd(uint32_t x, uint32_t timeout) :
  _t(timeout), _la(lcd), _ind{0xff}
{
  _la.area(x, 20, 0, 16) ; _la.clear() ;
}

void DispInd::set(int8_t ind)
{
  _t.restart() ;
  if (ind == _ind)
    return ;
  _ind = ind ;
  _la.txtPos(0) ;
  _la.put(ind) ;
  _la.clearEOL() ;
}

void DispInd::expire()
{
  if (_t())
  {
    _la.clear() ;
    _ind = 0xff ; 
  }
}

////////////////////////////////////////////////////////////////////////////////

DispTow::DispTow(uint32_t timeout) :
  _t(timeout), _la(lcd), _toggle{false}
{
  _la.area(155, 5, 0, 5) ; _la.clear() ;
}

void DispTow::set(uint32_t tow)
{
  _t.restart() ;
  tow /= 1000 ;
  if (tow == _tow)
    return ;
  _tow = tow ;
  _la.clear(_toggle ? 0x00ff00 : 0x000000) ;
  _toggle = !_toggle ;
}

void DispTow::expire()
{
  if (_t())
    _la.clear(0xff0000) ;
}

////////////////////////////////////////////////////////////////////////////////

DispAscFound::DispAscFound(uint32_t timeout) :
  _t(timeout), _la(lcd)
{
  _la.area(150, 10, 64, 16) ; _la.clear() ;
}

void DispAscFound::set()
{
  _t.restart() ;
  _la.txtPos(0) ;
  _la.put('$') ;
  _la.clearEOL() ;
}

void DispAscFound::expire()
{
  if (_t())
    _la.clear() ;
}

////////////////////////////////////////////////////////////////////////////////

void dispSetup()
{
  lcd.setup() ;

  LcdArea laTitle (lcd,   0, 110,  0, 16, &::RV::Longan::Roboto_Bold7pt7b     , 0xffffffUL, 0xa00000UL) ;

  laTitle.clear() ;
  laTitle.put("  GPS RECEIVER  ") ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
