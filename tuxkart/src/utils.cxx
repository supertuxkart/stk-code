
#include "tuxkart.h"

void pr_from_normal ( sgVec3 hpr, sgVec3 nrm )
{
  float sy = sin ( -hpr [ 0 ] * SG_DEGREES_TO_RADIANS ) ;
  float cy = cos ( -hpr [ 0 ] * SG_DEGREES_TO_RADIANS ) ;

  hpr[2] =  SG_RADIANS_TO_DEGREES * atan2 ( nrm[0] * cy - nrm[1] * sy, nrm[2] ) ;
  hpr[1] = -SG_RADIANS_TO_DEGREES * atan2 ( nrm[1] * cy + nrm[0] * sy, nrm[2] ) ;
}


void hpr_from_normal ( sgVec3 hpr, sgVec3 nrm )
{
  pr_from_normal ( hpr, nrm ) ;
  hpr[0] = -SG_RADIANS_TO_DEGREES * atan2 ( nrm[0], nrm[1] ) ;
}


bool canAccess ( char *fname )
{
#ifdef _MSC_VER
  return _access ( fname, 04 ) == 0 ;
#else
  return access ( fname, F_OK ) == 0 ;
#endif
}


bool chDir ( char *dir )
{
#ifdef _MSC_VER
  return _chdir ( dir ) == -1 ;
#else
  return chdir ( dir ) == -1 ;
#endif
}


void secondSleep ( int s )
{
#ifdef _MSC_VER
  Sleep ( 1000 * s ) ;
#else
  sleep ( s ) ;
#endif
}


