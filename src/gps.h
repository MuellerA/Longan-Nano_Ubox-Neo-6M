////////////////////////////////////////////////////////////////////////////////
// gps.h
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
  #include "gd32vf103.h"
}

#include "GD32VF103/time.h"
#include "GD32VF103/usart.h"
#include "Longan/lcd.h"
#include "Longan/fonts.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::GD32VF103::Usart ;
using ::RV::Longan::Lcd ;
using ::RV::Longan::LcdArea ;

////////////////////////////////////////////////////////////////////////////////

class GpsTx
{
public:
  GpsTx(uint8_t clsId, uint8_t msgId, const std::vector<uint8_t>& data) ;
  ~GpsTx() ;

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

class GpsRx
{
public:
  GpsRx() ;
  ~GpsRx() ;

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

class UbxData
{
protected:
  UbxData(uint32_t line, const char *label, uint32_t timeout) ;
  UbxData(uint32_t x, uint32_t timeout) ;
  UbxData(uint32_t timeout) ;

  TickTimer _t ;
  LcdArea _la ;
} ;

class UbxTime : public UbxData
{
public:
  UbxTime(uint32_t line, const char *label, uint32_t timeout) ;
  void set(uint32_t iTOW, uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec, uint8_t valid) ;
  void expire() ;
private:
  uint32_t _iTOW ;
} ;

class UbxLatLon : public UbxData
{
public:
  UbxLatLon(uint32_t line, const char *label, uint32_t timeout) ;
  void set(int32_t deg) ;
  void expire() ;
private:
  int32_t _deg ;
} ;

class UbxAlt : public UbxData
{
public:
  UbxAlt(uint32_t line, const char *label, uint32_t timeout) ;
  void set(int32_t alt) ;
  void expire() ;
private:
  int32_t _alt ;
} ;

class UbxInd : public UbxData
{
public:
  UbxInd(uint32_t x, uint32_t timeout) ;
  void set(int8_t ind) ;
  void expire() ;
private:
  uint8_t _ind ;
} ;

class UbxTow : public UbxData
{
public:
  UbxTow(uint32_t timeout) ;
  void set(uint32_t tow) ;
  void expire() ;
private:
  uint32_t _tow ;
  bool _toggle ;
} ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
