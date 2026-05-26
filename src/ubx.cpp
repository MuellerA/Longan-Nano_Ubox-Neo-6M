////////////////////////////////////////////////////////////////////////////////
// ubx.cpp
////////////////////////////////////////////////////////////////////////////////

#include "disp.h"
#include "ubx.h"

#include "GD32VF103/time.h"
#include "GD32VF103/usart.h"
#include "Longan/toStr.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::GD32VF103::Usart ;

////////////////////////////////////////////////////////////////////////////////

#ifdef USART_GPS
Usart& usart(Usart::USART_GPS()) ;
#else
#error "undefined USART (-D USART_GPS=usartN)"
#endif

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
UbxId UbxId::NavSat    (0x01, 0x35) ;

UbxId UbxId::AckAck    (0x05, 0x01) ;
UbxId UbxId::AckNak    (0x05, 0x00) ;

UbxId UbxId::CfgPrt    (0x06, 0x00) ;
UbxId UbxId::CfgMsg    (0x06, 0x01) ;

////////////////////////////////////////////////////////////////////////////////

UbxTx UbxCfgPrtUart::ubxTx()
{
  return UbxTx(UbxId::CfgPrt, std::vector<uint8_t>((uint8_t*)this, (uint8_t*)(this+1))) ;
}

////////////////////////////////////////////////////////////////////////////////

bool ack(const UbxId &ubxId, const std::vector<uint8_t> data)
{
  if (data.size() < sizeof(UbxAck))
    return false ;

  const UbxAck *ubx = (const UbxAck*) data.data() ;
  return (ubxId.clsId() == ubx->clsId) && (ubxId.msgId() == ubx->msgId) ;
}

////////////////////////////////////////////////////////////////////////////////

UbxNav::UbxNav()
{
  _lastTow = ~0UL ;
  _posllh.iTOW = ~0UL ;
  _status.iTOW = ~0UL ;
  _svinfo.iTOW = ~0UL ;
  _sat.iTOW    = ~0UL ;
  _timeUtc.iTOW = ~0UL ;
}

bool UbxNav::posllhValid()  const { return valid(_posllh .iTOW, 10000) ; }
bool UbxNav::statusValid()  const { return valid(_status .iTOW, 10000) ; }
bool UbxNav::svinfoValid()  const { return valid(_svinfo .iTOW, 10000) ; }
bool UbxNav::satValid()     const { return valid(_sat    .iTOW, 10000) ; }
bool UbxNav::timeUtcValid() const { return valid(_timeUtc.iTOW,  5000) ; }

bool UbxNav::valid() const
{
  uint32_t tow = _lastTow ;
  return valid(tow,  2000) ;
}

uint32_t UbxNav::lastTow() const { return _lastTow ; }

const UbxNavPosllh& UbxNav::posllh() const { return _posllh ; }
const UbxNavStatus& UbxNav::status() const { return _status ; }
const UbxNavSvinfo& UbxNav::svinfo() const { return _svinfo ; }
const std::vector<UbxNavSvinfoRep>& UbxNav::svinfoRep() const { return _svinfoRep ; }
const UbxNavSat& UbxNav::sat() const { return _sat ; }
const std::vector<UbxNavSatRep>& UbxNav::satRep() const { return _satRep ; }
const UbxNavTimeUtc& UbxNav::timeUtc() const { return _timeUtc ; }

std::string UbxNav::timeUtcStr(bool compressed) const
{
  if (compressed)
  {
    //  0123456789012345678
    // "xxxxxxxx xxxxxx"
    char text[15] ;
    RV::toStr(_timeUtc.year , text+ 0, 4, '0') ;
    RV::toStr(_timeUtc.month, text+ 4, 2, '0') ;
    RV::toStr(_timeUtc.day  , text+ 6, 2, '0') ; text[8] = ' ' ;
    RV::toStr(_timeUtc.hour , text+ 9, 2, '0') ;
    RV::toStr(_timeUtc.min  , text+11, 2, '0') ;
    RV::toStr(_timeUtc.sec  , text+13, 2, '0') ;
    return std::string(text, 15) ;
  }
  else
  {
    //  0123456789012345678
    // "xxxx-xx-xx xx:xx:xx"
    char text[19] ;
    RV::toStr(_timeUtc.year , text+ 0, 4, '0') ; text[ 4] = '-' ;
    RV::toStr(_timeUtc.month, text+ 5, 2, '0') ; text[ 7] = '-' ;
    RV::toStr(_timeUtc.day  , text+ 8, 2, '0') ; text[10] = ' ' ;
    RV::toStr(_timeUtc.hour , text+11, 2, '0') ; text[13] = ':' ;
    RV::toStr(_timeUtc.min  , text+14, 2, '0') ; text[16] = ':' ;
    RV::toStr(_timeUtc.sec  , text+17, 2, '0') ;
    return std::string(text, 19) ;
  }
}

std::string UbxNav::latStr() const
{
  int32_t lat = _posllh.lat / 100 ;
  int32_t g = lat / 100000 ;
  int32_t t = lat - g * 100000 ;
  if (t < 0) t = -t ;
  char text[32] ;
  char *start = RV::toStr(g, text, 16) ;
  text[16] = '.' ;
  RV::toStr(t, text+17, 5, '0') ;
  return std::string(start, 16 - (start-text) + 6) ;
}

std::string UbxNav::lonStr() const
{
  int32_t lon = _posllh.lon / 100 ;
  int32_t g = lon / 100000 ;
  int32_t t = lon - g * 100000 ;
  if (t < 0) t = -t ;
  char text[32] ;
  char *start = RV::toStr(g, text, 16) ;
  text[16] = '.' ;
  RV::toStr(t, text+17, 5, '0') ;
  return std::string(start, 16 - (start-text) + 6) ;
}

std::string UbxNav::altStr() const
{
  int32_t alt = _posllh.alt / 100 ;
  int32_t g = alt / 10 ;
  int32_t t = alt - g * 10 ;
  if (t < 0) t = -t ;
  char text[32] ;
  char *start = RV::toStr(g, text, 16) ;
  text[16] = '.' ;
  RV::toStr(t, text+17, 1, '0') ;
  return std::string(start, 16 - (start-text) + 2) ;
}

uint8_t UbxNav::sats() const
{
  uint32_t sat{0} ;
  if (svinfoValid())
  {
    for (const UbxNavSvinfoRep &svInfoRep : _svinfoRep)
      if (svInfoRep.flags & 1)
        sat += 1 ;
  }
  else if (satValid())
  {
    for (const UbxNavSatRep &satRep : _satRep)
      if (satRep.flags & 0x08)
        sat += 1 ;
  }
  return sat ;
}

bool UbxNav::posllh(const std::vector<uint8_t> &data)
{
  if (data.size() < sizeof(UbxNavPosllh))
    return false ;

  _posllh = *(const UbxNavPosllh*) data.data() ;
  tow(_posllh.iTOW) ;
  return true ;
}

bool UbxNav::svinfo(const std::vector<uint8_t> &data)
{
  if (data.size() < sizeof(UbxNavSvinfo))
    return false ;
  const UbxNavSvinfo *svinfo = (const UbxNavSvinfo*) data.data() ;
  if (data.size() < (sizeof(UbxNavSvinfo) + svinfo->numCh * sizeof(UbxNavSvinfoRep)))
    return false ;
  
  _svinfo = *(const UbxNavSvinfo*) data.data() ;
  tow(_svinfo.iTOW) ;
  _svinfoRep.clear() ;
  _svinfoRep.reserve(_svinfo.numCh) ;
  for (uint8_t iCh = 0 ; iCh < _svinfo.numCh ; ++iCh)
    _svinfoRep.emplace_back(*(const UbxNavSvinfoRep*)(data.data() + sizeof(UbxNavSvinfo) + iCh * sizeof(UbxNavSvinfoRep))) ;
  return true ;
}

bool UbxNav::sat(const std::vector<uint8_t> &data)
{
  if (data.size() < sizeof(UbxNavSat))
    return false ;
  const UbxNavSat *sat = (const UbxNavSat*) data.data() ;
  if (data.size() < (sizeof(UbxNavSat) + sat->numSvs * sizeof(UbxNavSatRep)))
    return false ;
  
  _sat = *(const UbxNavSat*) data.data() ;
  tow(_sat.iTOW) ;
  _satRep.clear() ;
  _satRep.reserve(_sat.numSvs) ;
  for (uint8_t iCh = 0 ; iCh < _sat.numSvs ; ++iCh)
    _satRep.emplace_back(*(const UbxNavSatRep*)(data.data() + sizeof(UbxNavSat) + iCh * sizeof(UbxNavSatRep))) ;
  return true ;
}

bool UbxNav::status(const std::vector<uint8_t> &data)
{
  if (data.size() < sizeof(UbxNavStatus))
    return false ;

  _status = *(const UbxNavStatus*) data.data() ;
  tow(_status.iTOW) ;
  return true ;
}

bool UbxNav::timeUtc(const std::vector<uint8_t> &data)
{
  if (data.size() < sizeof(UbxNavTimeUtc))
    return false ;

  _timeUtc = *(const UbxNavTimeUtc*) data.data() ;
  tow(_timeUtc.iTOW) ;
  return true ;
}

void UbxNav::tow(uint32_t tow)
{
  _lastTow = tow ;
  _tickMsAtLastTow = TickTimer::tickToMs(TickTimer::now()) ;
}

bool UbxNav::valid(uint32_t &tow, uint32_t maxMs) const
{
  if (tow == ~0U)
    return false ;
  uint32_t nowMs = TickTimer::tickToMs(TickTimer::now()) ;

  if (((_lastTow - tow) + (nowMs - _tickMsAtLastTow)) < maxMs)
    return true ;

  tow = ~0U ;
  return false ;
}

////////////////////////////////////////////////////////////////////////////////

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

bool ubxSet(const UbxTx &tx)
{
  TickTimer t(500) ;
  UbxRx rx ;
  tx.send() ;
  while (true)
  {
    if (t())
    {
      TickTimer::delayMs(100) ;
      return false ;
    }

    if (rx.poll())
    {
      if (rx.valid())
      {
        if ((rx.is(UbxId::AckAck) || rx.is(UbxId::AckNak)) && ack(tx.ubxId(), rx.data()))
          return rx.is(UbxId::AckAck) ;
      }
      rx.reset() ;
    }
  }
}

bool ubxPoll(const UbxTx &tx, std::vector<uint8_t> &rxData)
{
  TickTimer t(500) ;
  UbxRx rx ;
  tx.send() ;
  rxData.clear() ;
  while (true)
  {
    if (t())
      return false ;

    if (rx.poll())
    {
      if (rx.valid())
      {
        if (rx.is(tx.ubxId()))
          rxData = rx.data() ;

        if ((rx.is(UbxId::AckAck) || rx.is(UbxId::AckNak)) && ack(tx.ubxId(), rx.data()))
          return rx.is(UbxId::AckAck) ;
      }
      rx.reset() ;
    }
  }
}

bool ubxTryBaud(uint32_t tryBaud, UbxCfgPrtUart &port)
{
  UbxTx tx(UbxId::CfgPrt, std::vector<uint8_t>()) ;
  
  TickTimer::delayMs(200) ;
  usart.baud(tryBaud) ;
  TickTimer::delayMs(100) ;
  
  usart.clear() ;

  std::vector<uint8_t> data ;
  if (!ubxPoll(tx, data) ||
      data.size() != sizeof(port))
    return false ;

  port = *(UbxCfgPrtUart*)data.data() ;
  return true ;
}

uint32_t ubxGetBaud(LcdArea &la, UbxCfgPrtUart &port)
{
  std::vector<uint32_t> tryBauds{115200, 9600, 19200, 38400, 57600, 4800} ;
  for (uint32_t tryBaud : tryBauds)
  {
    la.clear() ; la.put("Probing ") ; la.put(tryBaud) ; la.put(" baud") ;
    if (ubxTryBaud(tryBaud, port))
      return tryBaud ;
  }
  return 0 ;                   
}

void ubxSetup(LcdArea &la)
{
  UbxCfgPrtUart port ;

  usart.setup(9600) ;

tryBaud:
  uint32_t baud{0} ;
  while (!baud)
    baud = ubxGetBaud(la, port) ;

  port.mode = 0b00'00'100'0'11'000000 ;
  port.baudRate = 115200 ;
  port.inProtoMask = 0b111 ;
  port.outProtoMask = 0b001 ;
  if (baud != 115200)
  {
    la.clear() ; la.put("Setting to 115200") ;
    ubxSet(port.ubxTx()) ;

    if (!ubxTryBaud(115200, port))
      goto tryBaud ;
  }
  else
  {
    ubxSet(port.ubxTx()) ;
  }

  std::vector<uint8_t> rxData ;

  if (port.portId != 0)
  {
    UbxCfgPrtUart portDdc ;
    if (ubxPoll(UbxTx(UbxId::CfgPrt, { 0x00 }), rxData) && (rxData.size() == sizeof(portDdc)))
    {
      portDdc = *(UbxCfgPrtUart*)rxData.data() ;
      portDdc.mode = 0b0000'0000 ;
      portDdc.inProtoMask = 0b000 ;
      portDdc.outProtoMask = 0b000 ;
      ubxSet(portDdc.ubxTx()) ;
    }
  }

  if (port.portId != 1)
  {
    UbxCfgPrtUart portUart1 ;
    if (ubxPoll(UbxTx(UbxId::CfgPrt, { 0x01 }), rxData) && (rxData.size() == sizeof(portUart1)))
    {
      portUart1 = *(UbxCfgPrtUart*)rxData.data() ;
      portUart1.mode = 0b0000'0000 ;
      portUart1.inProtoMask = 0b000 ;
      portUart1.outProtoMask = 0b000 ;
      ubxSet(portUart1.ubxTx()) ;
    }
  }

  if (port.portId != 2)
  {
    UbxCfgPrtUart portUart2 ;
    if (ubxPoll(UbxTx(UbxId::CfgPrt, { 0x02 }), rxData) && (rxData.size() == sizeof(portUart2)))
    {
      portUart2 = *(UbxCfgPrtUart*)rxData.data() ;
      portUart2.mode = 0b0000'0000 ;
      portUart2.inProtoMask = 0b000 ;
      portUart2.outProtoMask = 0b000 ;
      ubxSet(portUart2.ubxTx()) ;
    }
  }

  if (port.portId != 3)
  {
    UbxCfgPrtUart portUsb ;
    if (ubxPoll(UbxTx(UbxId::CfgPrt, { 0x03 }), rxData) && (rxData.size() == sizeof(portUsb)))
    {
      portUsb = *(UbxCfgPrtUart*)rxData.data() ;
      portUsb.inProtoMask = 0b000 ;
      portUsb.outProtoMask = 0b000 ;
      ubxSet(portUsb.ubxTx()) ;
    }
  }

  if (port.portId != 4)
  {
    UbxCfgPrtUart portSpi ;
    if (ubxPoll(UbxTx(UbxId::CfgPrt, { 0x04 }), rxData) && (rxData.size() == sizeof(portSpi)))
    {
      portSpi = *(UbxCfgPrtUart*)rxData.data() ;
      portSpi.mode = 0b0000'0000 ;
      portSpi.inProtoMask = 0b000 ;
      portSpi.outProtoMask = 0b000 ;
      ubxSet(portSpi.ubxTx()) ;
    }
  }

  std::vector<UbxTx> txCfg
    {
      // cfg-msg
      { UbxId::CfgMsg, {UbxId::NavPosllh.clsId() , UbxId::NavPosllh.msgId() , 1}},
      { UbxId::CfgMsg, {UbxId::NavStatus.clsId() , UbxId::NavStatus.msgId() , 1}}, // NAV-STATUS
      { UbxId::CfgMsg, {UbxId::NavTimeUtc.clsId(), UbxId::NavTimeUtc.msgId(), 1}}, // NAV-TIMEUTC
    } ;

  la.clear() ; la.put("Configuring") ;
  for (const UbxTx &tx : txCfg)
  {
    for (size_t i = 0 ; (i < 5) && !ubxSet(tx) ; ++i) ;
  }

  {
    UbxTx cfgMsgNavSat{ UbxId::CfgMsg, {UbxId::NavSat.clsId(), UbxId::NavSat.msgId(), 1}} ; // NAV-SAT
    UbxTx cfgMsgNavSvinfo{ UbxId::CfgMsg, {UbxId::NavSvinfo.clsId(), UbxId::NavSvinfo.msgId(), 1}} ; // NAV-SVINFO
    bool found{false} ;
    for (size_t i = 0 ; (i < 5) && !found ; ++i)
      found = ubxSet(cfgMsgNavSat) ;
    for (size_t i = 0 ; (i < 5) && !found ; ++i)
      found = ubxSet(cfgMsgNavSvinfo) ;
  }
  la.clear() ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
