////////////////////////////////////////////////////////////////////////////////
// file.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

extern "C"
{
  #include "gd32vf103.h"
}

#include "GD32VF103/time.h"
#include "Longan/fatfs.h"

class UbxNav ;

class File
{
public:
  enum class State
    {
      closed,
      pending, // waiting for utc time
      open,    // waiting for fix
      writing  // got fix
    } ;
  
  File(const UbxNav &nav) ;
  bool open() ;
  bool close() ;
  void loop() ;
  State state() const ;
  
private:
  State write0() ;
  State open0() ;
  State close0() ;
  static const uint32_t _QueueSize = 512 ;
  
  const UbxNav &_nav ;
  State _state ;
  std::string _queue ; // write every 512 bytes
  RV::Longan::FatFs::File _file ;
  RV::GD32VF103::TickTimer _timer ;
} ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
