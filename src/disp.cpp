////////////////////////////////////////////////////////////////////////////////
// disp.cpp
////////////////////////////////////////////////////////////////////////////////

#include "disp.h"

////////////////////////////////////////////////////////////////////////////////

Lcd &lcd(Lcd::lcd()) ;

////////////////////////////////////////////////////////////////////////////////

DispTime::DispTime(uint32_t timeout) :
  _t(timeout), _la(lcd), _sec{0xff}, _sec0{0xff}
{
  _la.area(0, 160, 16, 16) ; _la.clear() ;
}

void DispTime::set(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec, uint8_t valid)
{
  _t.restart() ;
  if (valid & 0b100)
  {
    _year  = year ;
    _month = month ;
    _day   = day ;
    _hour  = hour ;
    _min   = min ;
    _sec   = sec ;
  }
  else
  {
    _sec = 0xff ;
  }
}

void DispTime::display(uint8_t page, bool force)
{
  if (_t())
    _sec = 0xff ;

  if ((_sec != _sec0) || force)
  {
    _sec0 = _sec ;
    
    switch (page)
    {
    case 0:
      if (_sec != 0xff)
      {
        _la.txtPos(0) ;
        _la.put(_year, 4, '0') ;
        _la.put('-') ;
        _la.put(_month, 2, '0') ;
        _la.put('-') ;
        _la.put(_day, 2, '0') ;
        _la.put(' ') ;
        _la.put(_hour, 2, '0') ;
        _la.put(':') ;
        _la.put(_min, 2, '0') ;
        _la.put(':') ;
        _la.put(_sec, 2, '0') ;
        _la.clearEOL() ;
      }
      else
      {
        _la.clear() ;
      }
      break ;
    }
  }
}
      
////////////////////////////////////////////////////////////////////////////////

DispPos::DispPos(uint32_t timeout) :
  _t(timeout), _laLat(lcd), _laLon(lcd), _laAlt(lcd),
  _lat{0x7fffffff}, _lat0{0x7fffffff}, _lon{0x7fffffff}, _lon0{0x7fffffff}, _alt{0x7fffffff}, _alt0{0x7fffffff}
{
  _laLat.area(40, 100, 32, 16) ; _laLat.clear() ;
  _laLon.area(40, 100, 48, 16) ; _laLon.clear() ;
  _laAlt.area(40, 100, 64, 16) ; _laAlt.clear() ;
}

void DispPos::set(int32_t lat, int32_t lon, int32_t alt)
{
  _t.restart() ;
  _lat = lat / 1000 ;
  _lon = lon / 1000 ;
  _alt = alt /  100 ;
}

void DispPos::display(uint8_t page, bool force)
{
  if (_t())
  {
    _lat = 0x7fffffff ;
    _lon = 0x7fffffff ;
    _alt = 0x7fffffff ;
  }

  if ((_lat != _lat0) || force)
  {
    _lat0 = _lat ;

    switch (page)
    {
    case 0:
      if (force)
      {
        LcdArea laLbl(Lcd::lcd()) ;
        laLbl.area( 0,  40, 32, 16) ; laLbl.clear() ; laLbl.put("Lat") ;
        laLbl.area( 0,  40, 48, 16) ; laLbl.clear() ; laLbl.put("Lon") ;
        laLbl.area( 0,  40, 64, 16) ; laLbl.clear() ; laLbl.put("Alt") ;
      }
      
      if (_lat != 0x7fffffff)
      {
        int32_t g = _lat / 10000 ;
        int32_t t = _lat - g*10000 ;
        if (t < 0) t = -t ;
        _laLat.txtPos(0) ;
        _laLat.put(g) ;
        _laLat.put('.') ;
        _laLat.put(t, 4, '0') ;
        _laLat.clearEOL() ;
      }
      else
      {
        _laLat.clear() ;
      }
      break ;
    }
  }

  if ((_lon != _lon0) || force)
  {
    _lon0 = _lon ;

    switch (page)
    {
    case 0:
      if (_lon != 0x7fffffff)
      {
        int32_t g = _lon / 10000 ;
        int32_t t = _lon - g*10000 ;
        if (t < 0) t = -t ;
        _laLon.txtPos(0) ;
        _laLon.put(g) ;
        _laLon.put('.') ;
        _laLon.put(t, 4, '0') ;
        _laLon.clearEOL() ;
      }
      else
      {
        _laLon.clear() ;
      }
      break ;
    }
  }

  if ((_alt != _alt0) || force)
  {
    _alt0 = _alt ;

    switch (page)
    {
    case 0:
      if (_alt != 0x7fffffff)
      {
        int32_t g = _alt / 10 ;
        int32_t t = _alt - g*10 ;
        if (t < 0) t = -t ;
        _laAlt.txtPos(0) ;
        _laAlt.put(g) ;
        _laAlt.put('.') ;
        _laAlt.put(t) ;
        _laAlt.clearEOL() ;
      }
      else
      {
        _laAlt.clear() ;
      }
      break ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

DispInd::DispInd(uint32_t x, uint32_t timeout) :
  _t(timeout), _la(lcd), _ind{0xff}, _ind0{0xff}
{
  _la.area(x, 20, 0, 16) ; _la.clear() ;
}

void DispInd::set(int8_t ind)
{
  _t.restart() ;
  _ind = ind ;
}

void DispInd::display(uint8_t page, bool force)
{
  if (_t())
    _ind = 0xff ;

  if ((_ind != _ind0) || force)
  {
    _ind0 = _ind ;

    if (_ind != 0xff)
    {
      _la.txtPos(0) ;
      _la.put(_ind) ;
      _la.clearEOL() ;
    }
    else
    {
      _la.clear() ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

DispSvInfo::DispSvInfo(uint32_t x, uint32_t timeout) :
  _t(timeout), _laSat(lcd), _laSvInfos(lcd), _sat{0xff}, _sat0{0xff}, _svInfosX{0xff}, _svInfosX0{0xff}
{
  _laSat.area(x, 20, 0, 16) ; _laSat.clear() ;
  _laSvInfos.area(0, 150, 16, 64) ; _laSvInfos.clear() ;
}

void DispSvInfo::set(std::vector<SvInfo> &&svInfos)
{
  _t.restart() ;
  _svInfos = std::move(svInfos) ;
  _sat = 0 ;
  _svInfosX = (_svInfosX + 1) & 0x0f ;
  for (const SvInfo &svInfo : _svInfos)
    if (svInfo.flags & 1)
      _sat += 1 ;
}

void DispSvInfo::display(uint8_t page, bool force)
{
  if (_t())
  {
    _sat = 0xff ;
    _svInfosX = 0xff ;
  }
  
  if ((_sat != _sat0) || force)
  {
    _sat0 = _sat ;

    if (_sat != 0xff)
    {
      _laSat.txtPos(0) ;
      _laSat.put(_sat) ;
      _laSat.clearEOL() ;
    }
    else
    {
      _laSat.clear() ;
    }
  }

  if ((_svInfosX != _svInfosX0) || force)
  {
    _svInfosX0 = _svInfosX ;
    
    switch (page)
    {
    case 1:
      _laSvInfos.clear() ;

      if (_svInfosX != 0xff)
      {
        uint8_t r{0} ;
        uint8_t c{0} ;
        for (const SvInfo &svInfo : _svInfos)
        {
          for (uint8_t i = 0 ; i < _svInfos.size() ; ++i)
          {
            _laSvInfos.txtPos(r, c*12) ;
            if (svInfo.chn == 255)
            {
              _laSvInfos.put("---") ;
            }
            else
            {
              _laSvInfos.put(svInfo.cno) ;
              if (svInfo.flags & 1)
                _laSvInfos.put('*') ;
            }
          }
          r += 1 ;
          if (r > 3)
          {
            r = 0 ;
            c += 1 ;
          }
        }
      }
      break ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

DispTow::DispTow(uint32_t timeout) :
  _t(timeout), _la(lcd), _tow{0xffffffff}, _tow0{0xffffffff}, _toggle{false}
{
  _la.area(155, 5, 0, 5) ; _la.clear() ;
}

void DispTow::set(uint32_t tow)
{
  _t.restart() ;
  _tow = tow / 1000 ;
}

void DispTow::display(uint8_t page, bool force)
{
  if (_t())
    _tow = 0xffffffff ;

  if ((_tow != _tow0) || force)
  {
    _tow0 = _tow ;

    if (_tow != 0xffffffff)
    {
      _la.clear(_toggle ? 0x00ff00 : 0x000000) ;
      _toggle = !_toggle ;      
    }
    else
    {
      _la.clear(0xff0000) ;      
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

DispAscFound::DispAscFound(uint32_t timeout) :
  _t(timeout), _la(lcd), _c(0x00), _c0{0x00}
{
  _la.area(150, 10, 64, 16) ; _la.clear() ;
}

void DispAscFound::set(char c)
{
  _t.restart() ;
  _c = c ;
}

void DispAscFound::display(uint8_t page, bool force)
{
  if (_t())
    _c = 0x00 ;

  if ((_c != _c0) || force)
  {
    _c0 = _c ;

    switch (page)
    {
    case 0:
      if (_c != 0x00)
      {
        _la.txtPos(0) ;
        _la.put(_c) ;
        _la.clearEOL() ;
      }
      else
      {
        _la.clear() ;
      }
      break ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void dispSetup()
{
  lcd.setup() ;

  LcdArea laTitle (lcd, 0, 110,  0, 16, &::RV::Longan::Roboto_Bold7pt7b , 0xffffffUL, 0xa00000UL) ;

  laTitle.clear() ;
  laTitle.put("  GPS RECEIVER  ") ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
