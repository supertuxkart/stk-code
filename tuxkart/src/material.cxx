
#include "tuxkart.h"

#define UCLAMP   1
#define VCLAMP   2

static ulList *materials = NULL ;


bool Material::parseBool ( char **p )
{
  /* Skip leading spaces */

  while ( **p <= ' ' && **p != '\0' ) (*p)++ ;

  bool res = ( ( **p == 'Y' ) || ( **p == 'y' ) ) ;

  while ( **p > ' ' && **p != '\0' ) (*p)++ ;

  return res ;
}


int Material::parseInt ( char **p )
{
  /* Skip leading spaces */

  while ( **p <= ' ' && **p != '\0' ) (*p)++ ;

  return strtol ( *p, p, 0 ) ;
}



float Material::parseFloat ( char **p )
{
  /* Skip leading spaces */

  while ( **p <= ' ' && **p != '\0' ) (*p)++ ;

  return strtod ( *p, p ) ;
}



Material::Material ()
{
  texname = new char [ 1 ] ;
  texname [ 0 ] = '\0' ;

  init    () ;
  install () ;
}


Material::Material ( char *fname, char *description )
{
  texname = new char [ strlen ( fname ) + 1 ] ;
  strcpy ( texname, fname ) ;

  init () ;

  clamp_tex    = parseBool  ( & description ) ? UCLAMP : 0 ;
  clamp_tex   += parseBool  ( & description ) ? VCLAMP : 0 ;

  transparency = parseBool  ( & description ) ;
  alpha_ref    = parseFloat ( & description ) ;
  lighting     = parseBool  ( & description ) ;
  friction     = parseFloat ( & description ) ;
  ignore       = parseBool  ( & description ) ;
  zipper       = parseBool  ( & description ) ;
  resetter     = parseBool  ( & description ) ;
  collideable  = parseBool  ( & description ) ;

  install () ;
}


void Material::init ()
{
  materials -> addEntity ( this ) ;
  index = materials -> searchForEntity ( this ) ;

  clamp_tex    = 0     ;
  transparency = false ;
  alpha_ref    = 0.1   ;
  lighting     = true  ;
  friction     = 1.0   ;
  ignore       = false ;
  zipper       = false ;
  resetter     = false ;
  collideable  = true  ;
}


void Material::install ()
{
  ssgSimpleState *s = new ssgSimpleState ;

  state = s ;

  s -> ref () ;
  s -> setExternalPropertyIndex ( index ) ;

  if ( texname != NULL && texname [ 0 ] != '\0' )
  {
    char fn [ 1024 ] ;
    sprintf ( fn, "images/%s", texname ) ;

    s -> setTexture ( fn, !(clamp_tex & UCLAMP),
                          !(clamp_tex & VCLAMP) ) ;
    s -> enable  ( GL_TEXTURE_2D ) ;
  }
  else
    s -> disable ( GL_TEXTURE_2D ) ;

  if ( lighting )
    s -> enable  ( GL_LIGHTING ) ;
  else
    s -> disable ( GL_LIGHTING ) ;

  s -> setShadeModel ( GL_SMOOTH ) ;
  s -> enable        ( GL_COLOR_MATERIAL ) ;
  s -> enable        ( GL_CULL_FACE      ) ;
  s -> setColourMaterial ( GL_AMBIENT_AND_DIFFUSE ) ;
  s -> setMaterial   ( GL_EMISSION, 0, 0, 0, 1 ) ;
  s -> setMaterial   ( GL_SPECULAR, 0, 0, 0, 1 ) ;
  s -> setShininess  ( 0 ) ;

  if ( transparency )
  {
    s -> setTranslucent () ;
    s -> enable         ( GL_ALPHA_TEST ) ;
    s -> setAlphaClamp  ( alpha_ref ) ;
    s -> enable         ( GL_BLEND ) ;
  }
  else
  {
    s -> setOpaque () ;
    s -> disable   ( GL_BLEND ) ;
  }
}


char *parseFileName ( char **str )
{
  char *p = *str ;

  /* Skip leading spaces */

  while ( *p <= ' ' && *p != '\0' ) p++ ;

  /* Skip blank lines and comments */

  if ( *p == '#' || *p == '\0' )
    return NULL ;

  if ( *p != '"' )
  {
    fprintf ( stderr, "ERROR: Material file entries must start with '\"'\n" ) ;
    fprintf ( stderr, "ERROR: Offending line is '%s'\n", *str ) ;
    return NULL ;
  }

  /* Filename? */

  char *f = ++p ;

  while ( *p != '"' && *p != '\0' ) p++ ;

  if ( *p != '"' )
  {
    fprintf ( stderr,
      "ERROR: Unterminated string constant '%s' in materials file.\n", *str ) ;
    return NULL ;
  }

  *p = '\0' ;
  *str = ++p ;

  return f ;
}


int parseMaterial ( FILE *fd )
{
  char str [ 1024 ] ;

  while ( ! feof ( fd ) )
  {
    char *s = str ;

    if ( fgets ( s, 1024, fd ) == NULL )
      return false ;

    s [ strlen(s) - 1 ] = '\0' ;

    char *f = parseFileName ( & s ) ;

    if ( f != NULL )
    {
      new Material ( f, s ) ;
      return true ;
    }
  }
 
  return false ;
}


Material *getMaterial ( ssgLeaf *l )
{
  int m = l -> getExternalPropertyIndex () ;

  return (Material *) materials -> getEntity ( m ) ;
}


Material *getMaterial ( char *fname )
{
  if ( fname == NULL || fname[0] == '\0' )
    return (Material *) materials -> getEntity ( 0 ) ;

  char *fn ;

  /* Remove all leading path information. */

  for ( fn = & fname [ strlen ( fname ) - 1 ] ; fn != fname &&
                                               *fn != '/' ; fn-- )
    /* Search back for a '/' */ ;

  if ( *fn == '/' )
    fn++ ;

  char basename [ 1024 ] ;

  strcpy ( basename, fn ) ;

  /* Remove last trailing extension. */

  for ( fn = & basename [ strlen ( basename ) - 1 ] ; fn != basename &&
                                                     *fn != '.' ; fn-- )
    /* Search back for a '.' */ ;

  if ( *fn == '.' )
    *fn = '\0' ;

  for ( int i = 0 ; i < materials -> getNumEntities () ; i++ )
  {
    char *fname2 = ((Material *)(materials -> getEntity(i)))-> getTexFname () ;

    if ( fname2 != NULL && fname2[0] != '\0' )
    {
      char *fn2 ;

      /* Remove all leading path information. */

      for ( fn2 = & fname2 [ strlen ( fname2 ) -1 ] ; fn2 != fname2 &&
                                                     *fn2 != '/' ; fn2-- )
        /* Search back for a '/' */ ;

      if ( *fn2 == '/' )
        fn2++ ;

      char basename2 [ 1024 ] ;

      strcpy ( basename2, fn2 ) ;

      /* Remove last trailing extension. */

      for ( fn2 = & basename2 [ strlen ( basename2 ) - 1 ] ; fn2 != basename2 &&
                                                         *fn2 != '.' ; fn2-- )
        /* Search back for a '.' */ ;

      if ( *fn2 == '.' )
        *fn2 = '\0' ;

      if ( strcmp ( basename, basename2 ) == 0 )
        return (Material *) materials -> getEntity ( i ) ;
    }
  }

  strcpy ( fname, basename  ) ;
  strcat ( fname, ".rgb"    ) ;
  return NULL ;
}


ssgState *getAppState ( char *fname )
{
  Material *m = getMaterial ( fname ) ;

  return ( m == NULL ) ? NULL : m -> getState () ;
}


void initMaterials ()
{
  fprintf ( stderr, "Loading Materials.\n" ) ;

  /* Create list - and default material zero */

  materials = new ulList ( 100 ) ;
  materials -> addEntity ( new Material () ) ;

  char fname [ 1000 ] ;

  sprintf ( fname, "data/materials.dat" ) ;

  FILE *fd = fopen ( fname, "ra" ) ;

  if ( fd == NULL )
  {
    fprintf ( stderr, "FATAL: No Such File as '%s'\n", fname ) ;
    exit ( 1 ) ;
  }

  while ( parseMaterial ( fd ) ) 
    /* Read file */ ;

  fclose ( fd ) ;

  ssgSetAppStateCallback ( getAppState ) ;
}

