////////////////////////////////////////////////////////////////////////////////
// ubx.cpp
////////////////////////////////////////////////////////////////////////////////

#include "disp.h"
#include "ubx.h"

#include "GD32VF103/time.h"
#include "GD32VF103/usart.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::GD32VF103::Usart ;

////////////////////////////////////////////////////////////////////////////////

Usart& usart(Usart::usart0()) ;

////////////////////////////////////////////////////////////////////////////////

UbxId::UbxId(uint8_t clsId, uint8_t msgId) :
  _clsId{clsId}, _msgId{msgId}
{
}

UbxId::UbxId() :
  _clsId{0}, _msgId{0}
{
}

bool UbxId::operator==(const UbxId &that) const
{
  return
    (_clsId == that._clsId) &&
    (_msgId == that._msgId) ;
}

uint8_t UbxId::clsId() const { return _clsId ; }
uint8_t UbxId::msgId() const { return _msgId ; }
uint8_t& UbxId::clsId() { return _clsId ; }
uint8_t& UbxId::msgId() { return _msgId ; }

UbxId UbxId::NavPosllh (0x01, 0x02) ;
UbxId UbxId::NavStatus (0x01, 0x03) ;
UbxId UbxId::NavTimeUtc(0x01, 0x21) ;
UbxId UbxId::NavSvinfo (0x01, 0x30) ;

UbxId UbxId::AckAck    (0x05, 0x01) ;

UbxId UbxId::CfgPrt    (0x06, 0x00) ;
UbxId UbxId::CfgMsg    (0x06, 0x01) ;

////////////////////////////////////////////////////////////////////////////////

bool ack(const UbxId &ubxId, const std::vector<uint8_t> data)
{
  if (data.size() < sizeof(UbxAck))
    return false ;

  const UbxAck *ubx = (const UbxAck*) data.data() ;
  return (ubxId.clsId() == ubx->clsId) && (ubxId.msgId() == ubx->msgId) ;
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

bool navSvinfo(const std::vector<uint8_t> &data, uint32_t &iTOW, uint8_t &nChan, std::vector<SvInfo> &svInfos)
{
  if (data.size() < sizeof(UbxNavSvinfo))
    return false ;

  const UbxNavSvinfo *ubx = (const UbxNavSvinfo*) data.data() ;
  iTOW = ubx->iTOW ;
  nChan = ubx->numCh ;

  uint8_t n = (nChan > 16) ? 16 : nChan ;
  svInfos.resize(nChan) ;
  for (uint8_t i = 0 ; i < n ; ++i)
  {
    uint8_t chn, svid, flags, quality, cno ;
    if (navSvinfoRep(data, i, chn, svid, flags, quality, cno))
      svInfos[i] = SvInfo{chn, svid, flags, quality, cno} ;
  }  
  
  return true ;
}

bool navSvinfoRep(const std::vector<uint8_t> &data, uint8_t index, uint8_t &chn, uint8_t &svid, uint8_t &flags, uint8_t &quality, uint8_t &cno)
{
  if (data.size() < (sizeof(UbxNavSvinfo) + (index+1) * sizeof(UbxNavSvinfoRep)))
    return false ;

  const UbxNavSvinfoRep *ubx = (const UbxNavSvinfoRep*) (data.data() + sizeof(UbxNavSvinfo) + index * sizeof(UbxNavSvinfoRep)) ;
  chn = ubx->chn ;
  svid = ubx->svid ;
  flags = ubx->flags ;
  quality = ubx->quality ;
  cno = ubx->cno ;
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

UbxTx::UbxTx(const UbxId &ubxId, const std::vector<uint8_t>& data) :
  _ubxId{ubxId}, _chkA(0), _chkB(0)
{
  _data.push_back(0xb5) ;
  _data.push_back(0x62) ;
  add(ubxId.clsId()) ;
  add(ubxId.msgId()) ;
  uint16_t len = (uint16_t) data.size() ;
  add(len >> 0) ;
  add(len >> 8) ;
  for (uint8_t b : data)
    add(b) ;
  _data.push_back(_chkA) ;
  _data.push_back(_chkB) ;
}

UbxTx::~UbxTx()
{
}

void UbxTx::add(uint8_t b)
{
  csum(b) ;
  _data.push_back(b) ;
}

void UbxTx::send() const
{
  for (uint8_t b : _data)
  {
    while (!usart.put(b)) ;
  }
}

void UbxTx::csum(uint8_t b)
{
  _chkA += b         ;
  _chkB += _chkA ;
}

////////////////////////////////////////////////////////////////////////////////

UbxRx::UbxRx()
{
  reset() ;
}

UbxRx::~UbxRx()
{
}

void UbxRx::reset()
{
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

extern bool ubxAscFound ;
bool UbxRx::poll()
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
        ubxAscFound = true ;
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

bool UbxRx::addUbx(uint8_t b)
{
  if      (_idx == 0) { csum(b) ; _ubxId.clsId() = b ; }
  else if (_idx == 1) { csum(b) ; _ubxId.msgId() = b ; }
  else if (_idx == 2) { csum(b) ; _len   = b ; }
  else if (_idx == 3) { csum(b) ; _len  += b*10 ; _size = _len + 6 ; }
  else if (_idx <  _size - 2) { csum(b) ; _data.push_back(b) ; }
  else if (_idx == _size - 2) { _chkA = b ; }
  else if (_idx == _size - 1) { _chkB = b ; }

  _idx += 1 ;
  return _idx == _size ;
}

void UbxRx::csum(uint8_t b)
{
  _chkAcalc += b         ;
  _chkBcalc += _chkAcalc ;
}

bool UbxRx::valid() const
{
  return (_chkA == _chkAcalc) && (_chkB == _chkBcalc) ;
}

bool UbxRx::is(const UbxId &ubxId) const
{
  return (_ubxId == ubxId) ;
}

bool UbxRx::is(const UbxId &ubxId, uint16_t len) const
{
  return (_ubxId == ubxId) && (len == _data.size()) ;
}

////////////////////////////////////////////////////////////////////////////////

bool ubxPoll(const UbxTx &tx)
{
  TickTimer t(500) ;
  UbxRx rx ;
  tx.send() ;
  while (true)
  {
    if (t())
      return false ;

    if (rx.poll())
    {
      if (rx.valid() && rx.is(UbxId::AckAck) && ack(tx.ubxId(), rx.data()))
        return true ;

      rx.reset() ;
    }
  }
}

bool ubxTryBaud(uint32_t tryBaud)
{
  UbxTx tx(UbxId::CfgPrt, std::vector<uint8_t>()) ;
  
  TickTimer::delayMs(200) ;
  usart.baud(tryBaud) ;
  TickTimer::delayMs(100) ;
  
  usart.clear() ;

  return ubxPoll(tx) ;
}

uint32_t ubxGetBaud(LcdArea &la)
{
  std::vector<uint32_t> tryBauds{115200, 9600, 19200, 38400, 57600, 4800} ;
  for (uint32_t tryBaud : tryBauds)
  {
    la.clear() ; la.put("Probing ") ; la.put(tryBaud) ; la.put(" baud") ;
    if (ubxTryBaud(tryBaud))
      return tryBaud ;
  }
  return 0 ;                   
}

void ubxSetup(LcdArea &la)
{
  UbxTx txCfgPrtUart1{ UbxId::CfgPrt, cfgPrtUart(1, 0b0000100011000000, 115200, 0b111, 0b001) } ;
  
  std::vector<UbxTx> txCfg
    {
      // cfg-prt // disable ports
      { UbxId::CfgPrt, cfgPrtUart(2, 0b0000100011000000,   9600, 0b000, 0b000) },
      { UbxId::CfgPrt, cfgPrtDdc(0b00000000, 0b000, 0b000) },
      { UbxId::CfgPrt, cfgPrtUsb(0b000, 0b000) },
      { UbxId::CfgPrt, cfgPrtSpi(0b00000000, 0b000, 0b000) },
      // cfg-msg
      { UbxId::CfgMsg, {UbxId::NavPosllh.clsId() , UbxId::NavPosllh.msgId() , 1}},
      { UbxId::CfgMsg, {UbxId::NavStatus.clsId() , UbxId::NavStatus.msgId() , 1}}, // NAV-STATUS
      { UbxId::CfgMsg, {UbxId::NavSvinfo.clsId() , UbxId::NavSvinfo.msgId() , 4}}, // NAV-SVINFO
      { UbxId::CfgMsg, {UbxId::NavTimeUtc.clsId(), UbxId::NavTimeUtc.msgId(), 1}}, // NAV-TIMEUTC
    } ;

  usart.setup(9600) ;

 tryBaud:
  uint32_t baud{0} ;
  while (!baud)
    baud = ubxGetBaud(la) ;
  if (baud != 115200)
  {
    la.clear() ; la.put("Setting to 115200") ;
    txCfgPrtUart1.send() ;

    if (!ubxTryBaud(115200))
      goto tryBaud ;
  }
  else
  {
    txCfgPrtUart1.send() ;
  }
  
  for (const UbxTx &tx : txCfg)
  {
    la.clear() ; la.put("Configuring") ;
    while (!ubxPoll(tx)) ;
  }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
