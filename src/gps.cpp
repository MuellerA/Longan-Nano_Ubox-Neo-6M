//////////////////////////////////////////////////////////////////////////////
// gps.cpp
////////////////////////////////////////////////////////////////////////////////

#include "disp.h"
#include "ubx.h"
#include "file.h"
#include "Longan/fatfs.h"
#include "Longan/lcd.h"
#include "GD32VF103/gpio.h"

using ::RV::GD32VF103::Gpio ;
using ::RV::GD32VF103::GpioIrq ;
using ::RV::Longan::FatFs ;
using ::RV::Longan::Lcd ;

Gpio &button(Gpio::gpioA8()) ;
FatFs &fatfs{FatFs::fatfs()} ;
extern Lcd &lcd ;

volatile bool ubxAscFound{false} ;

////////////////////////////////////////////////////////////////////////////////

int _put_char(int ch) // used by printf
{
  lcd.put((char)ch) ;
  return ch ;
}

////////////////////////////////////////////////////////////////////////////////

uint8_t buttonPressed()
{
  struct State
  {
    State() : _value{0x00}, _tick{TickTimer::now()} {}
    uint8_t  _value ;
    uint64_t _tick ;
  } ;
  static State last ;
  static State current ;

  uint64_t now = TickTimer::now() ;
  if ((now - current._tick) < TickTimer::usToTick(2500))
    return 0 ;

  current._tick = now ;
  current._value = (current._value << 1) | button.get() ;
  if ((current._value == last._value) ||
      ((current._value != 0x00) && (current._value != 0xff)))
    return 0 ;

  uint32_t ms = TickTimer::tickToMs(current._tick - last._tick) ;
  last = current ;
  if (current._value)
    return 0 ;
  return (ms < 600) ? 1 : 2 ;
}

void getTimeUtc(const UbxNav &nav, FatFs::Time &time)
{
  if (!nav.timeUtcValid())
    return ;

  const UbxNavTimeUtc &timeUtc = nav.timeUtc() ;
  if (timeUtc.valid & 0b100)
  {
    time.year   = timeUtc.year - 2000 ;
    time.month  = timeUtc.month ;
    time.day    = timeUtc.day ;
    time.hour   = timeUtc.hour ;
    time.minute = timeUtc.min ;
    time.second = timeUtc.sec ;
  }
}

////////////////////////////////////////////////////////////////////////////////

int main()
{
  dispSetup() ;
  {
    TickTimer t(1500) ;
    
    lcd.color(0xffffffUL, 0xa00000UL) ;
    lcd.clear() ;
    lcd.font(&::RV::Longan::Roboto_Bold7pt7b) ;
    lcd.put("GPS RECEIVER 2.0-beta", 80, 32) ;
    
    LcdArea laUbx(Lcd::lcd(), 0, 160, 64, 16) ;
    laUbx.clear() ;
    ubxSetup(laUbx) ;

    while (!t()) ;
    lcd.clear(0x000000UL) ;
    lcd.put("   GPS RECEIVER   ") ;
  }
  
  LcdArea laData(Lcd::lcd(), 0, 150, 16, 64) ;
  
  button.setup(Gpio::Mode::IN_FL) ;

  UbxRx ubxRx ;

  DispGpsFix    dispGpsFix ;
  DispSats      dispSats   ;
  DispTow       dispTow    ;
  DispSvInfo    dispSvInfo ;
  DispTime      dispTime   ;
  DispPos       dispPos    ;
  DispFile      dispFile   ;
  DispAscFound  dispAscFound(1100) ;
  DispFileState dispFileState ;
  
  UbxNav ubxNav ;

  fatfs.setup([&ubxNav](FatFs::Time &time){ getTimeUtc(ubxNav, time) ; }) ;
  File logging(fatfs, ubxNav) ;

  enum class Page { main, sat, file } ;
  Page page{Page::main} ;
  
  bool force{true} ;
  uint8_t sched{0} ;

  dispPos.label() ;
  
  while (true)
  {
    while (ubxRx.poll())
    {
      if (ubxRx.valid())
      {
        if (ubxRx.clsId() == 0x01) // Nav
        {
          switch (ubxRx.msgId())
          {
          case 0x02: ubxNav.posllh (ubxRx.data()) ; break ;
          case 0x03: ubxNav.status (ubxRx.data()) ; break ;
          case 0x21: ubxNav.timeUtc(ubxRx.data()) ; break ;
          case 0x30: ubxNav.svinfo (ubxRx.data()) ; break ;
          }
        }
      }
      ubxRx.reset() ;
    }

    if (ubxAscFound)
    {
      ubxAscFound = false ;
      dispAscFound.set() ;
    }

    switch (sched)
    {
    case 0: // handle button
      switch (buttonPressed())
      {
      case 1: // short
        {
          laData.clear() ;
          force = true ;
          switch (page)
          {
          case Page::main:
            page = Page::sat ;
            break ;
          case Page::sat:
          case Page::file:
            page = Page::main ;
            dispPos.label() ;
            break ;
          }
        }
        break ;
      case 2: // long
        {
          switch (page)
          {
          case Page::main:
          case Page::sat:
            laData.clear() ;
            force = true ;
            page = Page::file ;
            break ;
          case Page::file:
            if (logging.state() == File::State::closed)
              logging.open() ;
            else
              logging.close() ;
            break ;
          }
        }
        break ;
      }
      break ;
    case 1: // update display
      {
        dispGpsFix  .display(ubxNav, false) ;
        dispSats    .display(ubxNav, false) ;
        dispTow     .display(ubxNav, false) ;
        dispFile    .display(logging, false) ;

        switch (page)
        {
        case Page::main:
          dispTime    .display(ubxNav, force) ;
          dispPos     .display(ubxNav, force) ;
          dispAscFound.display(force) ;
          break ;
        case Page::sat:
          dispSvInfo  .display(ubxNav, force) ;
          break ;
        case Page::file:
          if (force)
          {
            laData.txtPos(0) ; laData.put("press long to") ;
            laData.txtPos(1) ; laData.put("start/stop logging") ;
          }
          dispFileState.display(logging, force) ;
          break ;
        }

        force = false ;
      }
      break ;
    case 2: // update sd card
      {
        logging.loop() ;
      }
      break ;
    }
    sched = (sched + 1) % 3 ;
  }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
