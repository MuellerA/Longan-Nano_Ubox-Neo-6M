////////////////////////////////////////////////////////////////////////////////
// ubx.cpp
////////////////////////////////////////////////////////////////////////////////

#include "ubx.h"

#include "GD32VF103/time.h"
#include "GD32VF103/usart.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::GD32VF103::Usart ;

////////////////////////////////////////////////////////////////////////////////

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

UbxTx::UbxTx(uint8_t clsId, uint8_t msgId, const std::vector<uint8_t>& data) :
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

extern void dispAscFound() ;
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
        dispAscFound() ;
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

void UbxRx::csum(uint8_t b)
{
  _chkAcalc += b         ;
  _chkBcalc += _chkAcalc ;
}

bool UbxRx::valid() const
{
  return (_chkA == _chkAcalc) && (_chkB == _chkBcalc) ;
}

bool UbxRx::is(uint8_t clsId, uint8_t msgId) const
{
  return (_clsId == clsId) && (_msgId == msgId) ;
}

bool UbxRx::is(uint8_t clsId, uint8_t msgId, uint16_t len) const
{
  return (_clsId == clsId) && (_msgId == msgId) && (len == _data.size()) ;
}

////////////////////////////////////////////////////////////////////////////////

void ubxSetup()
{
  std::vector<UbxTx> cfg
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

  usart.setup(9600) ;
  
  UbxRx ubxRx ;
  for (const UbxTx &tx : cfg)
  {
    TickTimer t(1000, 0UL, true) ;
    bool found{false} ;
    while (!found)
    {
      if (t())
        tx.send() ;
      if (ubxRx.poll())
      {
        found = ubxRx.valid() && ubxRx.is(5, 1) && ack(tx.clsId(), tx.msgId(), ubxRx.data()) ;
        ubxRx.reset() ;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
