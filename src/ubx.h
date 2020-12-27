////////////////////////////////////////////////////////////////////////////////
// ubx.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

extern "C"
{
  #include "gd32vf103.h"
}

#include <vector>

class LcdArea ;

////////////////////////////////////////////////////////////////////////////////

#pragma pack(push,1)

////////////////////////////////////////////////////////////////////////////////

class UbxId
{
public:
  static UbxId NavPosllh ;
  static UbxId NavStatus ;
  static UbxId NavTimeUtc ;
  static UbxId NavSvinfo ;
  
  static UbxId AckAck ;

  static UbxId CfgPrt ;
  static UbxId CfgMsg ;

  UbxId() ;
  UbxId(uint8_t clsId, uint8_t msgId) ;
  bool operator==(const UbxId&) const ;
  uint8_t clsId() const ;
  uint8_t msgId() const ;
  uint8_t& clsId() ;
  uint8_t& msgId() ;
  
private:
  uint8_t _clsId ;
  uint8_t _msgId ;
} ;

////////////////////////////////////////////////////////////////////////////////

struct UbxAck
{
  uint8_t clsId ;
  uint8_t msgId ;
} ;

////////////////////////////////////////////////////////////////////////////////

struct UbxCfgPrtUart
{
  uint8_t  portId ;
  uint8_t  reserved0 ;
  uint16_t txReady ;
  uint32_t mode ;
  uint32_t baudRate ;
  uint16_t inProtoMask ;
  uint16_t outProtoMask ;
  uint16_t reserved4 ;
  uint16_t reserved5 ;
} ;

////////////////////////////////////////////////////////////////////////////////

struct UbxNavPosllh
{
  mutable
  uint32_t iTOW ; // ms
  int32_t  lon ;  // deg
  int32_t  lat ;  // deg
  int32_t  alt ;  // mm
  int32_t  hMSL ; // mm
  uint32_t hAcc ; // mm
  uint32_t vAcc ; // mm
} ;

struct UbxNavStatus
{
  mutable
  uint32_t iTOW ;   // ms
  uint8_t  gpsFix ; // 0 no fix, 1 dead, 2 2d, 3 3d, 4 gps + dead, 5 time
  uint8_t  flags ;
  uint8_t  fixStat ;
  uint8_t  flags2 ;
  uint32_t ttff ;
  uint32_t msss ;
} ;

struct UbxNavSvinfo
{
  mutable
  uint32_t iTOW ; // ms
  uint8_t  numCh ;
  uint8_t  globalFlags ;
  uint16_t reserved2 ;
} ;

struct UbxNavSvinfoRep
{
  uint8_t chn ;
  uint8_t svid ;
  uint8_t flags ;
  uint8_t quality ;
  uint8_t cno ;
  int8_t  elev ;
  int16_t azim ;
  int32_t prRes ;
} ;

struct UbxNavTimeUtc
{
  mutable
  uint32_t iTOW ;  // ms
  uint32_t tAcc ;  // ns
  int32_t  nano ;  // ns
  uint16_t year ;  // y
  uint8_t  month ; // m
  uint8_t  day ;   // d
  uint8_t  hour ;  // d
  uint8_t  min ;   // m
  uint8_t  sec ;   // s
  uint8_t  valid ;
} ;

class UbxNav
{
public:
  UbxNav() ;
  
  bool posllhValid() const ;
  bool statusValid() const ;
  bool svinfoValid() const ;
  bool timeUtcValid() const ;
  bool valid() const ;
  uint32_t lastTow() const ;
  const UbxNavPosllh& posllh() const ;
  const UbxNavStatus& status() const ;
  const UbxNavSvinfo& svinfo() const ;
  const std::vector<UbxNavSvinfoRep>& svinfoRep() const ;
  const UbxNavTimeUtc& timeUtc() const ;
  std::string timeUtcStr(bool compressed = false) const ;
  std::string latStr() const ;
  std::string lonStr() const ;
  std::string altStr() const ;
  uint8_t sats() const ;
  
  bool posllh(const std::vector<uint8_t> &data) ;
  bool svinfo(const std::vector<uint8_t> &data) ;
  bool status(const std::vector<uint8_t> &data) ;
  bool timeUtc(const std::vector<uint8_t> &data) ;

private:
  void tow(uint32_t tow) ;
  bool valid(uint32_t &tow, uint32_t maxMs = 5000) const ;

  uint32_t     _lastTow ;
  uint32_t     _tickMsAtLastTow ;
  
  UbxNavPosllh _posllh ;
  UbxNavStatus _status ;
  UbxNavSvinfo _svinfo ;
  std::vector<UbxNavSvinfoRep> _svinfoRep ;
  UbxNavTimeUtc _timeUtc ;
} ;

////////////////////////////////////////////////////////////////////////////////

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////

class UbxTx
{
public:
  UbxTx(const UbxId &ubxId, const std::vector<uint8_t>& data) ;
  ~UbxTx() ;

  void send() const ;

  const UbxId& ubxId() const { return _ubxId ; }
  uint8_t clsId() const { return _ubxId.clsId() ; }
  uint8_t msgId() const { return _ubxId.msgId() ; }

private:
  void add(uint8_t b) ;
  void csum(uint8_t b) ;

  UbxId _ubxId ;
  std::vector<uint8_t> _data ;

  uint8_t _chkA, _chkB ;
} ;

////////////////////////////////////////////////////////////////////////////////

class UbxRx
{
public:
  UbxRx() ;
  ~UbxRx() ;

  void reset() ;
  bool poll() ;
  bool valid() const ; // true if csum ok
  bool is(const UbxId &ubxId) const ;
  bool is(const UbxId &ubxId, uint16_t len) const ;

  uint8_t clsId() const { return _ubxId.clsId() ; }
  uint8_t msgId() const { return _ubxId.msgId() ; }
  uint16_t len() const { return _len ; }
  const std::vector<uint8_t> data() const { return _data ; }

private:
  bool addUbx(uint8_t) ; // true if msg complete
  void csum(uint8_t b) ;

  UbxId _ubxId ;
  uint16_t _len ;
  std::vector<uint8_t> _data ;
  uint8_t _idx ;
  uint16_t _size ;
  uint8_t _chkA, _chkB, _chkAcalc, _chkBcalc ;
} ;

////////////////////////////////////////////////////////////////////////////////

void ubxSetup(LcdArea &la) ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
