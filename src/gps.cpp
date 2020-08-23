////////////////////////////////////////////////////////////////////////////////
// gps.cpp
////////////////////////////////////////////////////////////////////////////////

#include "ubx.h"
#include "disp.h"

////////////////////////////////////////////////////////////////////////////////

int main()
{
  dispSetup() ;
  ubxSetup() ;

  UbxRx ubxRx ;

  DispInd    dispGpsFix(115, 10000) ;
  DispInd    dispNChan (135, 10000) ;
  DispTime   dispTime( 5000) ;
  DispPos    dispPos (10000) ;
  DispTow    dispTow (10000) ;

  while (true)
  {
    if (ubxRx.poll())
    {
      if (ubxRx.valid())
      {
        uint32_t iTOW ;
        
        if (ubxRx.is(0x01, 0x30)) // NAV-SVINFO
        {
          uint8_t nChan ;
          if (navSvinfo(ubxRx.data(), iTOW, nChan))
          {
            dispTow  .set(iTOW) ;
            dispNChan.set(nChan) ;
          }
        }
        else if (ubxRx.is(0x01, 0x03)) // NAV-STATUS
        {
          uint8_t gpsFix ;
          if (navStatus(ubxRx.data(), iTOW, gpsFix))
          {
            dispTow   .set(iTOW) ;
            dispGpsFix.set(gpsFix) ;
          }
        }
        else if (ubxRx.is(0x01, 0x02)) // NAV-POSLLH
        {
          int32_t  lat, lon, alt ;
          if (navPosllh(ubxRx.data(), iTOW, lat, lon, alt))
          {
            dispTow.set(iTOW) ;
            dispPos.set(lat, lon, alt) ;
          }
        }
        else if (ubxRx.is(0x01, 0x21)) // NAV-TIMEUTC
        {
          uint32_t iTOW ;
          uint16_t year ;
          uint8_t month, day, hour, min, sec, valid ;
          if (navTimeUtc(ubxRx.data(), iTOW, year, month, day, hour, min, sec, valid))
          {
            dispTow .set(iTOW) ;
            dispTime.set(iTOW, year, month, day, hour, min, sec, valid) ;
          }
        }
      }
      ubxRx.reset() ;

      dispGpsFix.expire() ;
      dispNChan .expire() ;
      dispTime  .expire() ;
      dispPos   .expire() ;
      dispTow   .expire() ;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
