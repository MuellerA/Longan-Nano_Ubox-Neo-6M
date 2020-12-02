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

GpioIrq &button(GpioIrq::gpioA8()) ;
FatFs &fatfs{FatFs::fatfs()} ;
extern Lcd &lcd ;

volatile bool ubxAscFound{false} ;
volatile uint8_t buttonPressed{0} ; // 1 short, 2 long

////////////////////////////////////////////////////////////////////////////////

void buttonIrqHdl(bool rising)
{
  static const uint64_t minTime{TickTimer::msToTick(5)} ;
  static uint64_t lastChange{0} ;

  uint64_t now = TickTimer::now() ;
  uint64_t delta = now - lastChange ;

  if (delta < minTime)
    return ;

  lastChange = now ;

  if (rising)
    return ;

  buttonPressed = (delta < TickTimer::msToTick(500)) ? 1 : 2 ;
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

////////////////////////////////////////////////////////////////////////////////

LcdArea *DbgArea ;
void Dbg(char c)
{
  if (DbgArea)
    DbgArea->put(c) ;
}

int main()
{
  dispSetup() ;
  {
    TickTimer t(1500) ;
    
    lcd.color(0xffffffUL, 0xa00000UL) ;
    lcd.clear() ;
    lcd.font(&::RV::Longan::Roboto_Bold7pt7b) ;
    lcd.put("GPS RECEIVER 2.0-pre", 80, 32) ;
    
    LcdArea laUbx(Lcd::lcd(), 0, 160, 64, 16) ;
    laUbx.clear() ;
    ubxSetup(laUbx) ;

    while (!t()) ;
    lcd.clear(0x000000UL) ;
    lcd.put("   GPS RECEIVER   ") ;
  }
  
  LcdArea laData(Lcd::lcd(), 0, 150, 16, 64) ;
  DbgArea = &laData ;
  
  button.setup(GpioIrq::Mode::IN_FL, buttonIrqHdl) ;

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
  File logging(ubxNav) ;

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
      if (buttonPressed)
      {
        laData.clear() ;
        switch (buttonPressed)
        {
        case 1:
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
          break ;
        case 2:
          switch (page)
          {
          case Page::main:
          case Page::sat:
            page = Page::file ;
            break ;
          case Page::file:
            if (logging.state() == File::State::closed)
            {
              if (fatfs.mount() == RV::Longan::FF::FR_OK)
                logging.open() ;
            }
            else
            {
              logging.close() ;
              fatfs.unmount() ;
            }
            break ;
          }
          break ;
        }
        force = true ;
        buttonPressed = 0 ;
      }
      break ;
    case 1: // update display
      {
        dispGpsFix  .display(ubxNav, false) ;
        dispSats    .display(ubxNav, false) ;
        dispTow     .display(ubxNav, false) ;
        dispFile    .display(ubxNav, logging, false) ;

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
