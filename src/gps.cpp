////////////////////////////////////////////////////////////////////////////////
// gps.cpp
////////////////////////////////////////////////////////////////////////////////

#include "gps.h"
#include "ubx.h"

////////////////////////////////////////////////////////////////////////////////

Lcd& lcd(Lcd::lcd()) ;
Usart& usart(Usart::usart0()) ;

////////////////////////////////////////////////////////////////////////////////

bool ack(uint8_t clsId, uint8_t msgId, const std::vector<uint8_t> data)
{
  if (data.size() < sizeof(UbxAck))
    return false ;

  const UbxAck *ubx = (const UbxAck*) data.data() ;
  return (clsId == ubx->clsId) && (msgId == ubx->msgId) ;
}

bool navPosllh(const std::vector<uint8_t> &data, uint32_t &iTOW, int32_t &lon, int32_t &lat, int32_t &alt)
{
  if (data.size() < sizeof(UbxNavPosllh))
    return false ;

  const UbxNavPosllh *ubx = (const UbxNavPosllh*) data.data() ;
  iTOW = ubx->iTOW ;
  lon = ubx->lon ;
  lat = ubx->lat ;
  alt = ubx->hMSL ;
  return true ;
}

bool navSvinfo(const std::vector<uint8_t> &data, uint32_t &iTOW, uint8_t &nChan)
{
  if (data.size() < sizeof(UbxNavSvinfo))
    return false ;

  const UbxNavSvinfo *ubx = (const UbxNavSvinfo*) data.data() ;
  iTOW = ubx->iTOW ;
  nChan = ubx->numCh ;
  return true ;
}

bool navStatus(const std::vector<uint8_t> &data, uint32_t &iTOW, uint8_t &gpsFix)
{
  if (data.size() < sizeof(UbxNavStatus))
    return false ;

  const UbxNavStatus *ubx = (const UbxNavStatus*) data.data() ;
  iTOW = ubx->iTOW ;
  gpsFix = ubx->gpsFix ;
  return true ;
}

bool navTimeUtc(const std::vector<uint8_t> &data, uint32_t &iTOW, uint16_t &year, uint8_t &month, uint8_t &day, uint8_t &hour, uint8_t &min, uint8_t &sec, uint8_t &valid)
{
  if (data.size() < sizeof(UbxNavTimeUtc))
    return false ;

  const UbxNavTimeUtc *ubx = (const UbxNavTimeUtc*) data.data() ;
  iTOW = ubx->iTOW ;
  year = ubx->year ;
  month = ubx->month ;
  day = ubx->day ;
  hour = ubx->hour ;
  min = ubx->min ;
  sec = ubx->sec ;
  valid = ubx->valid ;
  return true ;
}

std::vector<uint8_t> cfgPrtUart(uint8_t portId, uint32_t mode, uint32_t baudRate, uint16_t inProtoMask, uint16_t outProtoMask)
{
  std::vector<uint8_t> data(sizeof(UbxCfgPrtUart)) ;
  UbxCfgPrtUart *ubx = (UbxCfgPrtUart*) data.data() ;
  ubx->portId = portId ;
  ubx->reserved0 = 0 ;
  ubx->txReady = 0 ;
  ubx->mode = mode ;
  ubx->baudRate = baudRate ;
  ubx->inProtoMask = inProtoMask ;
  ubx->outProtoMask = outProtoMask ;
  ubx->reserved4 = 0 ;
  ubx->reserved5 = 0 ;
  return data ;
}

std::vector<uint8_t> cfgPrtUsb(uint16_t inProtoMask, uint16_t outProtoMask)
{
  std::vector<uint8_t> data(sizeof(UbxCfgPrtUart)) ;
  UbxCfgPrtUart *ubx = (UbxCfgPrtUart*) data.data() ;
  ubx->portId = 3 ;
  ubx->reserved0 = 0 ;
  ubx->txReady = 0 ;
  ubx->mode = 0 ;
  ubx->baudRate = 0 ;
  ubx->inProtoMask = inProtoMask ;
  ubx->outProtoMask = outProtoMask ;
  ubx->reserved4 = 0 ;
  ubx->reserved5 = 0 ;
  return data ;
}

std::vector<uint8_t> cfgPrtSpi(uint32_t mode, uint16_t inProtoMask, uint16_t outProtoMask)
{
  std::vector<uint8_t> data(sizeof(UbxCfgPrtUart)) ;
  UbxCfgPrtUart *ubx = (UbxCfgPrtUart*) data.data() ;
  ubx->portId = 4 ;
  ubx->reserved0 = 0 ;
  ubx->txReady = 0 ;
  ubx->mode = mode ;
  ubx->baudRate = 0 ;
  ubx->inProtoMask = inProtoMask ;
  ubx->outProtoMask = outProtoMask ;
  ubx->reserved4 = 0 ;
  ubx->reserved5 = 0 ;
  return data ;
}

std::vector<uint8_t> cfgPrtDdc(uint32_t mode, uint16_t inProtoMask, uint16_t outProtoMask)
{
  std::vector<uint8_t> data(sizeof(UbxCfgPrtUart)) ;
  UbxCfgPrtUart *ubx = (UbxCfgPrtUart*) data.data() ;
  ubx->portId = 0 ;
  ubx->reserved0 = 0 ;
  ubx->txReady = 0 ;
  ubx->mode = mode ;
  ubx->baudRate = 0 ;
  ubx->inProtoMask = inProtoMask ;
  ubx->outProtoMask = outProtoMask ;
  ubx->reserved4 = 0 ;
  ubx->reserved5 = 0 ;
  return data ;
}

////////////////////////////////////////////////////////////////////////////////

GpsTx::GpsTx(uint8_t clsId, uint8_t msgId, const std::vector<uint8_t>& data) :
  _clsId{clsId}, _msgId{msgId}, _chkA(0), _chkB(0)
{
  _data.push_back(0xb5) ;
  _data.push_back(0x62) ;
  add(_clsId) ;
  add(_msgId) ;
  uint16_t len = (uint16_t) data.size() ;
  add(len >> 0) ;
  add(len >> 8) ;
  for (uint8_t b : data)
    add(b) ;
  _data.push_back(_chkA) ;
  _data.push_back(_chkB) ;
}

GpsTx::~GpsTx()
{
}

void GpsTx::add(uint8_t b)
{
  csum(b) ;
  _data.push_back(b) ;
}

void GpsTx::send() const
{
  for (uint8_t b : _data)
  {
    while (!usart.put(b)) ;
  }
}

void GpsTx::csum(uint8_t b)
{
  _chkA += b         ;
  _chkB += _chkA ;
}

////////////////////////////////////////////////////////////////////////////////

GpsRx::GpsRx()
{
  reset() ;
}

GpsRx::~GpsRx()
{
}

void GpsRx::reset()
{
  _clsId    = 0 ;
  _msgId    = 0;
  _len      = 0 ;
  _data.clear() ;
  _data.reserve(1000) ;
  _idx      = 0 ;
  _size     = 4 ;
  _chkA     = 0 ;
  _chkB     = 0 ;
  _chkAcalc = 0 ;
  _chkBcalc = 0 ;
}

bool GpsRx::poll()
{
  static enum { waitB5, wait62, collect } state(waitB5) ;

  uint8_t b ;
  while (usart.get(b))
  {  
    switch (state)
    {
    case waitB5:
      if (b == 0xb5)
        state = wait62 ;
      else if (b == '$')
        lcd.put('$') ;
      break ;
    case wait62:
      if (b == 0x62)
        state = collect ;
      else
        state = waitB5 ;
      break ;
    case collect:
      if (addUbx(b))
      {
        state = waitB5 ;
        return true ;
      }
    }
  }
  return false ;
}

bool GpsRx::addUbx(uint8_t b)
{
  if      (_idx == 0) { csum(b) ; _clsId = b ; }
  else if (_idx == 1) { csum(b) ; _msgId = b ; }
  else if (_idx == 2) { csum(b) ; _len   = b ; }
  else if (_idx == 3) { csum(b) ; _len  += b*10 ; _size = _len + 6 ; }
  else if (_idx <  _size - 2) { csum(b) ; _data.push_back(b) ; }
  else if (_idx == _size - 2) { _chkA = b ; }
  else if (_idx == _size - 1) { _chkB = b ; }

  _idx += 1 ;
  return _idx == _size ;
}

void GpsRx::csum(uint8_t b)
{
  _chkAcalc += b         ;
  _chkBcalc += _chkAcalc ;
}

bool GpsRx::valid() const
{
  return (_chkA == _chkAcalc) && (_chkB == _chkBcalc) ;
}

bool GpsRx::is(uint8_t clsId, uint8_t msgId) const
{
  return (_clsId == clsId) && (_msgId == msgId) ;
}

bool GpsRx::is(uint8_t clsId, uint8_t msgId, uint16_t len) const
{
  return (_clsId == clsId) && (_msgId == msgId) && (len == _data.size()) ;
}

////////////////////////////////////////////////////////////////////////////////

UbxData::UbxData(uint32_t line, const char *label, uint32_t timeout) :
  _t(timeout), _la(lcd)
{
  if (label)
  {
    _la.area( 0,  40, 16*line, 16) ; _la.clear() ; _la.put(label) ;
    _la.area(40, 120, 16*line, 16) ; _la.clear() ;
  }
  else
  {
    _la.area(00, 160, 16*line, 16) ; _la.clear() ;
  }
}

UbxData::UbxData(uint32_t x, uint32_t timeout) :
  _t(timeout), _la(lcd)
{
  _la.area(x, 20, 0, 16) ; _la.clear() ;
}

UbxData::UbxData(uint32_t timeout) :
  _t(timeout), _la(lcd)
{
  _la.area(155, 5, 0, 5) ; _la.clear() ;
}

UbxTime::UbxTime(uint32_t line, const char *label, uint32_t timeout) :
  UbxData(line, label, timeout), _iTOW{0xffffffff}
{
}

void UbxTime::set(uint32_t iTOW, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec, uint8_t valid)
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
  _la.fill(_la.x(), _la.xMax()-_la.x(), _la.y()-_la.baseLineOffset(), _la.font()->yAdvance) ;
}

void UbxTime::expire()
{
  if (_t())
  {
    _la.clear() ;
    _iTOW = 0xffffffff; 
  }
}

UbxLatLon::UbxLatLon(uint32_t line, const char *label, uint32_t timeout) :
  UbxData(line, label, timeout), _deg{0x7fffffff}
{
}

void UbxLatLon::set(int32_t deg)
{
  _t.restart() ;
  deg /= 1000 ;
  if (deg == _deg)
    return ;
  _deg = deg ;
  int32_t  g = deg / 10000 ;
  uint32_t t = deg - g*10000 ;
  _la.txtPos(0) ;
  _la.put(g) ;
  _la.put('.') ;
  _la.put(t, 4, '0') ;
  _la.fill(_la.x(), _la.xMax()-_la.x(), _la.y()-_la.baseLineOffset(), _la.font()->yAdvance) ;
}

void UbxLatLon::expire()
{
  if (_t())
  {
    _la.clear() ;
    _deg = 0x7fffffff; 
  }
}
  
UbxAlt::UbxAlt(uint32_t line, const char *label, uint32_t timeout) :
  UbxData(line, label, timeout), _alt{0x7fffffff}
{
}

void UbxAlt::set(int32_t alt)
{
  _t.restart() ;
  alt /= 100 ;
  if (alt == _alt)
    return ;
  _alt = alt ;
  int32_t  g = alt / 10 ;
  uint32_t t = alt - g*10 ;
  _la.txtPos(0) ;
  _la.put(g) ;
  _la.put('.') ;
  _la.put(t) ;
  _la.fill(_la.x(), _la.xMax()-_la.x(), _la.y()-_la.baseLineOffset(), _la.font()->yAdvance) ;
}

void UbxAlt::expire()
{
  if (_t())
  {
    _la.clear() ;
    _alt = 0x7fffffff; 
  }
}
  
UbxInd::UbxInd(uint32_t x, uint32_t timeout) :
  UbxData(x, timeout), _ind{0xff}
{
}

void UbxInd::set(int8_t ind)
{
  _t.restart() ;
  if (ind == _ind)
    return ;
  _ind = ind ;
  _la.txtPos(0) ;
  _la.put(ind) ;
  _la.fill(_la.x(), _la.xMax()-_la.x(), _la.y()-_la.baseLineOffset(), _la.font()->yAdvance) ;
}

void UbxInd::expire()
{
  if (_t())
  {
    _la.clear() ;
    _ind = 0xff ; 
  }
}

UbxTow::UbxTow(uint32_t timeout) :
  UbxData(timeout), _tow{0xffffffff}, _toggle{false}
{
}

void UbxTow::set(uint32_t tow)
{
  _t.restart() ;
  tow /= 1000 ;
  if (tow == _tow)
    return ;
  _tow = tow ;
  _la.clear(_toggle ? 0x00ff00 : 0x000000) ;
  _toggle = !_toggle ;
}

void UbxTow::expire()
{
  if (_t())
    _la.clear(0xff0000) ;
}
                  
////////////////////////////////////////////////////////////////////////////////

void initNeo6M()
{
  std::vector<GpsTx> cfg
    {
      // cfg-prt // disable ports
//    { 0x06, 0x00, cfgPrtUart(1, 0b0000100011000000, 115200, 0b111, 0b001) },
//    { 0x06, 0x00, cfgPrtUart(1, 0b0000100011000000,  19200, 0b111, 0b001) },
      { 0x06, 0x00, cfgPrtUart(1, 0b0000100011000000,   9600, 0b111, 0b001) },
      { 0x06, 0x00, cfgPrtUart(2, 0b0000100011000000,   9600, 0b000, 0b000) },
      { 0x06, 0x00, cfgPrtDdc(0b00000000, 0b000, 0b000) },
      { 0x06, 0x00, cfgPrtUsb(0b000, 0b000) },
      { 0x06, 0x00, cfgPrtSpi(0b00000000, 0b000, 0b000) },
      // cfg-msg
      { 0x06, 0x01, {0x01, 0x02, 1}}, // NAV-POSLLH
      { 0x06, 0x01, {0x01, 0x03, 1}}, // NAV-STATUS
      { 0x06, 0x01, {0x01, 0x30, 4}}, // NAV-SVINFO
      { 0x06, 0x01, {0x01, 0x21, 1}}, // NAV-TIMEUTC
    } ;

  GpsRx gpsRx ;
  for (const GpsTx &tx : cfg)
  {
    TickTimer t(1000, 0UL, true) ;
    bool found{false} ;
    while (!found)
    {
      if (t())
        tx.send() ;
      if (gpsRx.poll())
      {
        found = gpsRx.valid() && gpsRx.is(5, 1) && ack(tx.clsId(), tx.msgId(), gpsRx.data()) ;
        gpsRx.reset() ;
      }
    }
  }
}

int main()
{
  lcd.setup() ;
  usart.setup(9600) ;

  LcdArea laTitle (lcd,   0, 110,  0, 16, &::RV::Longan::Roboto_Bold7pt7b     , 0xffffffUL, 0xa00000UL) ;

  laTitle.clear() ;
  laTitle.put("  GPS RECEIVER  ") ;

  GpsRx gpsRx ;

  initNeo6M() ;
  uint32_t iTOW ;

  UbxInd    ubxGpsFix(115, 10000) ;
  UbxInd    ubxNChan (135, 10000) ;
  UbxTime   ubxTime(1, nullptr,  5000) ;
  UbxLatLon ubxLat (2, "Lat"  , 10000) ;
  UbxLatLon ubxLon (3, "Lon"  , 10000) ;
  UbxAlt    ubxAlt (4, "Alt"  , 10000) ;
  UbxTow    ubxTow (10000) ;
  
  while (true)
  {
    if (gpsRx.poll())
    {
      if (gpsRx.valid())
      {
        if (gpsRx.is(0x01, 0x30)) // NAV-SVINFO
        {
          uint8_t nChan ;
          if (navSvinfo(gpsRx.data(), iTOW, nChan))
          {
            ubxTow  .set(iTOW) ;
            ubxNChan.set(nChan) ;
          }
        }
        else if (gpsRx.is(0x01, 0x03)) // NAV-STATUS
        {
          uint8_t gpsFix ;
          if (navStatus(gpsRx.data(), iTOW, gpsFix))
          {
            ubxTow   .set(iTOW) ;
            ubxGpsFix.set(gpsFix) ;
          }
        }
        else if (gpsRx.is(0x01, 0x02)) // NAV-POSLLH
        {
          int32_t  lat, lon, alt ;
          if (navPosllh(gpsRx.data(), iTOW, lat, lon, alt))
          {
            ubxTow .set(iTOW) ;
            ubxLat .set(lat) ;
            ubxLon .set(lon) ;
            ubxAlt .set(alt) ;
          }
        }
        else if (gpsRx.is(0x01, 0x21)) // NAV-TIMEUTC
        {
          uint32_t iTOW ;
          uint16_t year ;
          uint8_t month, day, hour, min, sec, valid ;
          if (navTimeUtc(gpsRx.data(), iTOW, year, month, day, hour, min, sec, valid))
          {
            ubxTow .set(iTOW) ;
            ubxTime.set(iTOW, year, month, day, hour, min, sec, valid) ;
          }
        }
      }
      gpsRx.reset() ;

      ubxGpsFix.expire() ;
      ubxNChan .expire() ;
      ubxTime  .expire() ;
      ubxLat   .expire() ;
      ubxLon   .expire() ;
      ubxAlt   .expire() ;
      ubxTow   .expire() ;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
