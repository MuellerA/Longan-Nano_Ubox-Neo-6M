////////////////////////////////////////////////////////////////////////////////
// file.cpp
////////////////////////////////////////////////////////////////////////////////

#include "file.h"
#include "ubx.h"

File::File(const UbxNav &nav) :
  _nav{nav}, _state{File::State::closed}, _timer(5000, true)
{
  _queue.reserve(_QueueSize + 128) ;
}

bool File::open()
{
  if (_state == State::closed)
    _state = State::pending ;
  return true ;
}

void Dbg(char c) ;

bool File::close()
{
  Dbg('D') ;
  switch (_state)
  {
  case State::pending:
  case State::closed:
    _state = State::closed ;
    break ;
  case State::open:
  case State::writing:
    size_t size = _queue.size() ;
    _file.write(_queue.data(), size) ;
    _state = close0() ;
    break ;
  }
  return true ;
}

void File::loop()
{
  switch (_state)
  {
  case State::closed:
    break ;
    
  case State::pending:
    _state = open0() ;
    break ;
    
  case State::open:
    _state = write0() ;
    if (_state == State::writing)
      _timer.restart() ;
    break ;

  case State::writing:
    if (_timer())
      _state = write0() ;
    break ;
  }
}

File::State File::state() const
{
  return _state ;
}
  
File::State File::open0()
{
  if (!_nav.timeUtcValid() || !(_nav.timeUtc().valid & 0b100))
    return State::pending ;
  std::string name = _nav.timeUtcStr(true) + ".csv" ;
  if (_file.open(name.c_str(), FA_CREATE_ALWAYS | FA_WRITE) != RV::Longan::FF::FR_OK)
  {
    return State::closed ;
  }
  
  _queue = "utc_d,utc_t,lat,lon,alt\n" ;
  return State::open ;
}

File::State File::close0()
{
  _file.close() ;
  _queue.clear() ;
  return State::closed ;
}

File::State File::write0()
{
  uint8_t fix = 0 ;
  if (_nav.statusValid())
  {
    fix = _nav.status().gpsFix ;
    if ((fix != 2) && (fix != 3))
      fix = 0 ;
  }
  
  if (!_nav.posllhValid() || !fix)
    return State::open ;
  std::string str = _nav.timeUtcStr() ;
  str[10] = ',' ;
  _queue += str ;
  _queue += ',' ;
  _queue += _nav.latStr() ;
  _queue += ',' ;
  _queue += _nav.lonStr() ;
  _queue += ',' ;
  if (fix == 3)
    _queue += _nav.altStr() ;
  _queue += '\n' ;
  
  if (_queue.size() >= _QueueSize)
  {
    size_t size = _QueueSize ;
    if (_file.write(_queue.data(), size) != RV::Longan::FF::FR_OK)
    {
      return close0() ;
    }
    _queue.erase(0, _QueueSize) ;
  }
  return State::writing ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

