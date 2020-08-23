////////////////////////////////////////////////////////////////////////////////
// ubx.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

extern "C"
{
  #include "gd32vf103.h"
}

#include <vector>

////////////////////////////////////////////////////////////////////////////////

#pragma pack(push,1)

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

////////////////////////////////////////////////////////////////////////////////

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////

class UbxTx
{
public:
  UbxTx(uint8_t clsId, uint8_t msgId, const std::vector<uint8_t>& data) ;
  ~UbxTx() ;

  void send() const ;

  uint8_t clsId() const { return _clsId ; }
  uint8_t msgId() const { return _msgId ; }

private:
  void add(uint8_t b) ;
  void csum(uint8_t b) ;

  uint8_t _clsId ;
  uint8_t _msgId ;
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
  bool is(uint8_t clsId, uint8_t msgId) const ;
  bool is(uint8_t clsId, uint8_t msgId, uint16_t len) const ;

  uint8_t clsId() const { return _clsId ; }
  uint8_t msgId() const { return _msgId ; }
  uint16_t len() const { return _len ; }
  const std::vector<uint8_t> data() const { return _data ; }

private:
  bool addUbx(uint8_t) ; // true if msg complete
  void csum(uint8_t b) ;

  uint8_t _clsId ;
  uint8_t _msgId ;
  uint16_t _len ;
  std::vector<uint8_t> _data ;
  uint8_t _idx ;
  uint16_t _size ;
  uint8_t _chkA, _chkB, _chkAcalc, _chkBcalc ;
} ;

////////////////////////////////////////////////////////////////////////////////

bool navPosllh(const std::vector<uint8_t> &data, uint32_t &iTOW, int32_t &lon, int32_t &lat, int32_t &alt) ;
bool navSvinfo(const std::vector<uint8_t> &data, uint32_t &iTOW, uint8_t &nChan) ;
bool navStatus(const std::vector<uint8_t> &data, uint32_t &iTOW, uint8_t &gpsFix) ;
bool navTimeUtc(const std::vector<uint8_t> &data, uint32_t &iTOW, uint16_t &year, uint8_t &month, uint8_t &day, uint8_t &hour, uint8_t &min, uint8_t &sec, uint8_t &valid) ;

void ubxSetup(LcdArea &la) ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
