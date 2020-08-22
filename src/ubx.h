////////////////////////////////////////////////////////////////////////////////
// ubx.h
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

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
