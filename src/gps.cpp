//////////////////////////////////////////////////////////////////////////////
// gps.cpp
////////////////////////////////////////////////////////////////////////////////

#include "disp.h"
#include "ubx.h"
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

bool ubxAscFound{false} ;
bool buttonPressed{false} ;

////////////////////////////////////////////////////////////////////////////////

void buttonIrqHdl(bool rising)
{
  if (rising)
    buttonPressed = true ;
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
    lcd.put("GPS RECEIVER 2.0", 80, 32) ;
    
    LcdArea laUbx(Lcd::lcd(), 0, 160, 64, 16) ;
    laUbx.clear() ;
    ubxSetup(laUbx) ;

    while (!t()) ;
    lcd.clear(0x000000UL) ;
    lcd.put("   GPS RECEIVER   ") ;
  }
  
  LcdArea laData(Lcd::lcd(), 0, 150, 16, 64) ;
  
  button.setup(GpioIrq::Mode::IN_FL, buttonIrqHdl) ;

  UbxRx ubxRx ;

  DispGpsFix dispGpsFix ;
  DispSats   dispSats   ;
  DispTow    dispTow    ;
  DispSvInfo dispSvInfo ;
  DispTime   dispTime   ;
  DispPos    dispPos    ;
  DispAscFound dispAscFound(1100) ;
  
  UbxNav ubxNav ;

  fatfs.setup([&ubxNav](FatFs::Time &time){ getTimeUtc(ubxNav, time) ; }) ;
  
  uint32_t page{0} ;
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
    case 0:
      if (buttonPressed)
      {
        static TickTimer t(500, (uint32_t)0) ; // ignore multi presses within 500ms
        
        buttonPressed = false ;
      
        if (t())
        {
          t.restart() ;
          laData.clear() ;
          page = page ^ 0x01 ;

          if (page == 0)
            dispPos.label() ;
            
          force = true ;
        }
      }
      break ;
    case 1:
      {
        dispGpsFix  .display(ubxNav, force) ;
        dispSats    .display(ubxNav, force) ;
        dispTow     .display(ubxNav, force) ;

        switch (page)
        {
        case 0:
          dispTime    .display(ubxNav, force) ;
          dispPos     .display(ubxNav, force) ;
          dispAscFound.display(force) ;
          break ;
        case 1:
          dispSvInfo  .display(ubxNav, force) ;
          break ;
        case 2:
          break ;
        }

        force = false ;
      }
      break ;
    case 2:
      {
        // fatfs
      }
    }
    sched = (sched + 1) % 3 ;
  }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
