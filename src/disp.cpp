////////////////////////////////////////////////////////////////////////////////
// disp.cpp
////////////////////////////////////////////////////////////////////////////////

#include "disp.h"
#include "ubx.h"
#include "file.h"

////////////////////////////////////////////////////////////////////////////////

Lcd &lcd(Lcd::lcd()) ;

////////////////////////////////////////////////////////////////////////////////

DispTime::DispTime() :
  _la(lcd), _iTOW(~0UL)
{
  _la.area(0, 160, 16, 16) ;
  _la.clear() ;
}

void DispTime::display(const UbxNav &nav, bool force)
{
  if (nav.timeUtcValid() && (nav.timeUtc().valid & 0b100))
  {
    const UbxNavTimeUtc &timeUtc = nav.timeUtc() ;
    if ((_iTOW != timeUtc.iTOW) || force)
    {
      _iTOW = timeUtc.iTOW ;
      _la.txtPos(0) ;
      std::string time = nav.timeUtcStr() ;
      _la.put(time.data(), time.size()) ;
      _la.clearEOL() ;
    }
  }
  else
  {
    if ((_iTOW != ~0UL) || force)
    {
      _iTOW = ~0UL ;
      _la.clear() ;
    }
  }
}
      
////////////////////////////////////////////////////////////////////////////////

DispPos::DispPos() :
  _laLat(lcd), _laLon(lcd), _laAlt(lcd), _iTOW(~0UL)
{
  _laLat.area(40, 100, 32, 16) ; _laLat.clear() ;
  _laLon.area(40, 100, 48, 16) ; _laLon.clear() ;
  _laAlt.area(40, 100, 64, 16) ; _laAlt.clear() ;
}

void DispPos::label()
{
  LcdArea laLbl(Lcd::lcd()) ;
  laLbl.area( 0,  40, 32, 16) ; laLbl.clear() ; laLbl.put("Lat") ;
  laLbl.area( 0,  40, 48, 16) ; laLbl.clear() ; laLbl.put("Lon") ;
  laLbl.area( 0,  40, 64, 16) ; laLbl.clear() ; laLbl.put("Alt") ;
}

void DispPos::display(const UbxNav &nav, bool force)
{
  uint8_t fix = 0 ;
  if (nav.statusValid())
  {
    fix = nav.status().gpsFix ;
    if ((fix != 2) && (fix != 3))
      fix = 0 ;
  }
    
  if (fix && nav.posllhValid())
  {
    const UbxNavPosllh &posllh = nav.posllh() ;
    if ((_iTOW != posllh.iTOW) || force)
    {
      _iTOW = posllh.iTOW ;
      {
        std::string lat = nav.latStr() ;
        _laLat.txtPos(0) ;
        _laLat.put(lat.data(), lat.size()) ;
        _laLat.clearEOL() ;
      }
      {
        std::string lon = nav.lonStr() ;
        _laLon.txtPos(0) ;
        _laLon.put(lon.data(), lon.size()) ;
        _laLon.clearEOL() ;
      }
      if (fix == 3)
      {
        std::string alt = nav.altStr() ;
        _laAlt.txtPos(0) ;
        _laAlt.put(alt.data(), alt.size()) ;
        _laAlt.clearEOL() ;
      }
      else
      {
        _laAlt.clear() ;
      }
    }
  }
  else
  {
    if ((_iTOW != ~0UL) || force)
    {
      _iTOW = ~0UL ;
      _laLat.clear() ;
      _laLon.clear() ;
      _laAlt.clear() ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

DispGpsFix::DispGpsFix() :
  _la(lcd), _iTOW{~0UL}
{
  _la.area(115, 20, 0, 16) ;
  _la.clear() ;
}

void DispGpsFix::display(const UbxNav &nav, bool force)
{
  if (nav.statusValid())
  {
    const UbxNavStatus &status = nav.status() ;
    if ((_iTOW != status.iTOW) || force)
    {
      _iTOW = status.iTOW ;
      _la.txtPos(0) ;
      _la.put(status.gpsFix) ;
      _la.clearEOL() ;
    }
  }
  else
  {
    if ((_iTOW != ~0UL) || force)
    {
      _iTOW = ~0UL ;
      _la.clear() ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

DispSats::DispSats() :
  _la(lcd), _iTOW{~0UL}
{
  _la.area(135, 20, 0, 16) ;
  _la.clear() ;
}

void DispSats::display(const UbxNav &nav, bool force)
{
  if (nav.svinfoValid())
  {
    const UbxNavSvinfo &svinfo = nav.svinfo() ;
    if ((_iTOW != svinfo.iTOW) || force)
    {
      _iTOW = svinfo.iTOW ;

      uint32_t sat = 0 ;
      for (const UbxNavSvinfoRep &svInfoRep : nav.svinfoRep())
        if (svInfoRep.flags & 1)
          sat += 1 ;
      
      _la.txtPos(0) ;
      _la.put(sat) ;
      _la.clearEOL() ;
    }
  }
  else
  {
    if ((_iTOW != ~0UL) || force)
    {
      _iTOW = ~0UL ;
      _la.clear() ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

DispSvInfo::DispSvInfo() :
  _la(lcd), _iTOW{~0UL}
{
  _la.area(0, 150, 16, 64) ;
  _la.font(&::RV::Longan::RobotoMono_Light6pt7b) ;
  _la.clear() ;
}

void DispSvInfo::display(const UbxNav &nav, bool force)
{
  if (nav.svinfoValid())
  {
    const UbxNavSvinfo &svinfo = nav.svinfo() ;
    if ((_iTOW != svinfo.iTOW) || force)
    {
      _iTOW = svinfo.iTOW ;

      uint8_t r{0} ;
      uint8_t c{0} ;
      for (const UbxNavSvinfoRep &svInfoRep : nav.svinfoRep())
      {
        _la.txtPos(r, c*4) ;
        if (svInfoRep.chn != 255)
        {
          if (svInfoRep.flags & 1)
            _la.color(0xffffff, 0x008080) ;
          else
            _la.color(0xffffff, 0x000000) ;
          _la.put(svInfoRep.cno, 2, ' ') ;
        }
        else
        {
          _la.color(0xffffff, 0x000000) ;
          _la.put(" -") ;
        }
        
        r += 1 ;
        if (r > 3)
        {
          r = 0 ;
          c += 1 ;
        }
        if (c > 4)
          break ;
      }
      _la.color(0xffffff, 0x000000) ;
      while (c < 5)
      {
        _la.txtPos(r, c*4) ;
        _la.put("  ") ;
        r += 1 ;
        if (r > 3)
        {
          r = 0 ;
          c += 1 ;
        }
      }
    }
  }
  else
  {
    if ((_iTOW != ~0UL) || force)
    {
      _iTOW = ~0UL ;
      _la.clear() ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

DispTow::DispTow() :
  _la(lcd), _iTOW{~0UL}, _toggle{false}, _t(1000)
{
  _la.area(155, 5, 0, 5) ;
  _la.clear() ;
}

void DispTow::display(const UbxNav &nav, bool force)
{
  if (nav.valid())
  {
    if (_t() || force)
    {
      _iTOW = nav.lastTow() ;
      if (force)
        _toggle = true ;
      _t.restart() ;
      _la.clear(_toggle ? 0x00ff00 : 0x000000) ;
      _toggle = !_toggle ;
    }
  }
  else
  {
    if ((_iTOW != ~0UL) || force)
    {
      _iTOW = ~0UL ;
      _la.clear(0xff0000) ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

DispFile::DispFile() :
  _la(lcd), _state(~0UL)
{
  _la.area(155, 5, 10, 5) ;
  _la.clear() ;
}

void DispFile::display(const UbxNav &nav, const File &file, bool force)
{
  File::State state = file.state() ;
  
  if ((_state != (uint32_t) state) || force)
  {
    _state = (uint32_t) state ;
    switch (state)
    {
    case File::State::closed:  _la.clear(0xff0000) ; break ;
    case File::State::pending: _la.clear(0xe2b007) ; break ;
    case File::State::open:    _la.clear(0xe2b007) ; break ;
    case File::State::writing: _la.clear(0x00ff00) ; break ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

DispFileState::DispFileState() :
  _la(lcd), _state(~0UL)
{
  _la.area(0, 160, 48, 16) ;
  _la.clear() ;
}

void DispFileState::display(const File &file, bool force)
{
  File::State state = file.state() ;
  
  if ((_state != (uint32_t) state) || force)
  {
    _state = (uint32_t) state ;
    _la.txtPos(0) ;
    switch (state)
    {
    case File::State::closed:  _la.put("not logging")          ; break ;
    case File::State::pending: _la.put("waiting for UTC time") ; break ;
    case File::State::open:    _la.put("waiting for GPS fix")  ; break ;
    case File::State::writing: _la.put("logging")              ; break ;
    }
    _la.clearEOL() ;
  }
}

////////////////////////////////////////////////////////////////////////////////

DispAscFound::DispAscFound(uint32_t timeout) :
  _t(timeout), _la(lcd), _c(0x00), _c0{0x00}
{
  _la.area(150, 10, 64, 16) ;
  _la.clear() ;
}

void DispAscFound::set(char c)
{
  _t.restart() ;
  _c = c ;
}

void DispAscFound::display(bool force)
{
  if (_t())
    _c = 0x00 ;

  if ((_c != _c0) || force)
  {
    _c0 = _c ;

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
  }
}

////////////////////////////////////////////////////////////////////////////////

void dispSetup()
{
  lcd.setup() ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
