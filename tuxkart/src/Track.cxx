
#include "tuxkart.h"

static int npoints ;
static sgVec3 driveline[MAX_DRIVELINE] ;

Track::Track ( char *fname )
{
  int i ;
  float d = 0.0f ;

  FILE *fd = fopen ( fname, "ra" ) ;

  if ( fd == NULL )
  {
    fprintf ( stderr, "Can't open '%s' for reading.\n", fname ) ;
    exit ( 1 ) ;
  } 

  for ( i = 0 ; i < MAX_DRIVELINE && !feof(fd) ; i++ )
  {
    char s [ 1024 ] ;

    if ( fgets ( s, 1023, fd ) == NULL )
      break ;

    if ( *s == '#' || *s < ' ' )
      continue ;

    float x, y ;

    if ( sscanf ( s, "%f,%f", &x, &y ) != 2 )
    {
      fprintf ( stderr, "Syntax error in '%s'\n", fname ) ;
      exit ( 1 ) ;
    } 

    driveline[i][0] = x ;
    driveline[i][1] = y ;
    driveline[i][2] = 0.0f ;
    npoints = i + 1 ;
  }

  fclose ( fd ) ;

  sgSetVec2 ( min,  SG_MAX/2.0f,  SG_MAX/2.0f ) ;
  sgSetVec2 ( max, -SG_MAX/2.0f, -SG_MAX/2.0f ) ;

  for ( i = 0 ; i < npoints ; i++ )
  {
    if ( driveline[i][0] < min[0] ) min[0] = driveline[i][0] ;
    if ( driveline[i][1] < min[1] ) min[1] = driveline[i][1] ;
    if ( driveline[i][0] > max[0] ) max[0] = driveline[i][0] ;
    if ( driveline[i][1] > max[1] ) max[1] = driveline[i][1] ;

    driveline [i][2] = d ;

    if ( i == npoints - 1 )
      d += sgDistanceVec2 ( driveline[i], driveline[0] ) ;
    else
      d += sgDistanceVec2 ( driveline[i], driveline[i+1] ) ;
  }

  total_distance = d ;
}




extern int check_hint ;

int Track::spatialToTrack ( sgVec3 res, sgVec3 xyz, int hint )
{
  int nearest = 0 ;
  float d ;
  float nearest_d = 99999 ;

  /*
    If we don't have a good hint, search all the
    points on the track to find our nearest centerline point
  */

/*
if ( check_hint )
fprintf(stderr,"ih=%d ", hint ) ;
  if ( hint < 0 || hint >= npoints )
*/
  {
    for ( int i = 0 ; i < npoints ; i++ )
    {
      d = sgDistanceVec2 ( driveline[i], xyz ) ;

      if ( d < nearest_d )
      {
        nearest_d = d ;
        nearest = i ;
      }
    }

    hint = nearest ;
/*
if ( check_hint )
fprintf(stderr,"hint=%d\n", hint ) ;
*/
  }

  /*
    Check the two points on the centerline either side
    of the hint.
  */

/*
  int hp = ( hint <=      0     ) ? (npoints-1) : (hint-1) ;
  int hn = ( hint >= (npoints-1)) ?      0      : (hint+1) ;

  float dp = sgDistanceVec2 ( driveline[ hp ], xyz ) ;
  float d0 = sgDistanceVec2 ( driveline[hint], xyz ) ;
  float dn = sgDistanceVec2 ( driveline[ hn ], xyz ) ;

if ( check_hint )
fprintf(stderr,"d=(%f->%f->%f), %d/%d/%d ", dp,d0,dn, hp,hint,hn ) ;

  if ( d0 < dp && d0 < dn )
  {
    nearest   = hint ;
    nearest_d =  d0  ;
  }
  else
  if ( dp < dn )
  {
    nearest   = hp ;
    nearest_d = dp ;
  }
  else
  {
    nearest   = hn ;
    nearest_d = dn ;
  }

if ( check_hint )
fprintf(stderr,"new hint=%d\n", nearest ) ;
*/
  /*
    OK - so we have the closest point
  */

  int    prev,  next ;
  float dprev, dnext ;

  prev = ( nearest   ==   0     ) ? (npoints - 1) : (nearest - 1) ;
  next = ( nearest+1 >= npoints ) ?      0        : (nearest + 1) ;

  dprev = sgDistanceVec2 ( driveline[prev], xyz ) ;
  dnext = sgDistanceVec2 ( driveline[next], xyz ) ;

  int   p1, p2 ;
  float d1, d2 ;

  if ( dnext < dprev )
  {
    p1 = nearest   ; p2 =  next ;
    d1 = nearest_d ; d2 = dnext ;
  }
  else
  {
    p1 =  prev ; p2 = nearest   ;
    d1 = dprev ; d2 = nearest_d ;
  }

  sgVec3 line_eqn ;
  sgVec3 tmp ;

  sgMake2DLine ( line_eqn, driveline[p1], driveline[p2] ) ;

  res [ 0 ] = sgDistToLineVec2 ( line_eqn, xyz ) ;

  sgAddScaledVec2 ( tmp, xyz, line_eqn, -res [0] ) ;

  res [ 1 ] = sgDistanceVec2 ( tmp, driveline[p1] ) + driveline[p1][2] ;

  return nearest ;
}

int Track::absSpatialToTrack ( sgVec3 res, sgVec3 xyz )
{
  return spatialToTrack ( res, xyz, 100000 ) ;
}


void Track::trackToSpatial ( sgVec3 xyz, int hint )
{
  sgCopyVec3 ( xyz, driveline [ hint ] ) ;
}


void Track::draw2Dview ( float x, float y )
{
  sgVec2 sc ;

  sgAddScaledVec2 ( center, min, max, 0.5f ) ;
  sgSubVec2 ( sc, max, center ) ;

  scale = ( sc[0] > sc[1] ) ? ( TRACKVIEW_SIZE / sc[0] ) :
                              ( TRACKVIEW_SIZE / sc[1] ) ;
 
  glBegin ( GL_LINE_LOOP ) ;
    for ( int i = 0 ; i < npoints ; i++ )
      glVtx ( driveline[i], x, y ) ;
  glEnd () ;
}

