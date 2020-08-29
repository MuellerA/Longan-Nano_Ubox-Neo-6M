///////////////////////////////////////////////////////////////////////////////
// gps.cpp
////////////////////////////////////////////////////////////////////////////////

#include "disp.h"
#include "ubx.h"
#include "GD32VF103/gpio.h"

using ::RV::GD32VF103::Gpio ;
using ::RV::GD32VF103::GpioIrq ;

GpioIrq &button(GpioIrq::gpioA8()) ;

bool ubxAscFound{false} ;
bool buttonPressed{false} ;

////////////////////////////////////////////////////////////////////////////////

void buttonIrqHdl(bool rising)
{
  if (rising)
    buttonPressed = true ;
}

int main()
{
  dispSetup() ;

  {
    LcdArea laUbx(Lcd::lcd(), 0, 160, 16, 16) ;
    ubxSetup(laUbx) ;
  }
  
  LcdArea laData(Lcd::lcd(), 0, 150, 16, 64) ;
  
  button.setup(GpioIrq::Mode::IN_FL, buttonIrqHdl) ;
  
  UbxRx ubxRx ;

  DispInd    dispGpsFix(115, 10000) ;
  DispSvInfo dispSvInfo(135, 10000) ;
  DispTime   dispTime( 5000) ;
  DispPos    dispPos (10000) ;
  DispTow    dispTow (10000) ;
  DispAscFound dispAscFound(1100) ;
  
  uint32_t page{0} ;
  bool force{true} ;
  
  while (true)
  {
    if (buttonPressed)
    {
      static TickTimer t(500, (uint32_t)0) ; // ignore multi presses within 500ms

      buttonPressed = false ;
      
      if (t())
      {
        t.restart() ;
        laData.clear() ;
        page = page ^ 0x01 ;
        force = true ;
      }
    }
    
    if (ubxAscFound)
    {
      ubxAscFound = false ;
      dispAscFound.set() ;
    }
    
    if (ubxRx.poll())
    {
      if (ubxRx.valid())
      {
        uint32_t iTOW ;
        
        if (ubxRx.is(UbxId::NavSvinfo))
        {
          std::vector<SvInfo> svInfos(16) ;
          uint8_t nChan ;
          if (navSvinfo(ubxRx.data(), iTOW, nChan, svInfos))
          {
            dispTow   .set(iTOW) ;
            dispSvInfo.set(std::move(svInfos)) ;
          }
        }
        else if (ubxRx.is(UbxId::NavStatus))
        {
          uint8_t gpsFix ;
          if (navStatus(ubxRx.data(), iTOW, gpsFix))
          {
            dispTow   .set(iTOW) ;
            dispGpsFix.set(gpsFix) ;
          }
        }
        else if (ubxRx.is(UbxId::NavPosllh))
        {
          int32_t  lat, lon, alt ;
          if (navPosllh(ubxRx.data(), iTOW, lat, lon, alt))
          {
            dispTow.set(iTOW) ;
            dispPos.set(lat, lon, alt) ;
          }
        }
        else if (ubxRx.is(UbxId::NavTimeUtc))
        {
          uint32_t iTOW ;
          uint16_t year ;
          uint8_t month, day, hour, min, sec, valid ;
          if (navTimeUtc(ubxRx.data(), iTOW, year, month, day, hour, min, sec, valid))
          {
            dispTow .set(iTOW) ;
            dispTime.set(year, month, day, hour, min, sec, valid) ;
          }
        }
      }
      ubxRx.reset() ;
    }
    
    dispGpsFix  .display(page, force) ;
    dispSvInfo  .display(page, force) ;
    dispTime    .display(page, force) ;
    dispPos     .display(page, force) ;
    dispTow     .display(page, force) ;
    dispAscFound.display(page, force) ;

    force = false ;
  }
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
