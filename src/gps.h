////////////////////////////////////////////////////////////////////////////////
// gps.h
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
  #include "gd32vf103.h"
}

#include <vector>

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
// EOF
////////////////////////////////////////////////////////////////////////////////
