#include <stdio.h>
#include <stdlib.h>

int main ()
{
  char *fname = "bs.drv" ;
  FILE *fd = fopen ( fname, "ra" ) ;
 
  if ( fd == NULL )
  {
    fprintf ( stderr, "Can't open '%s' for reading.\n", fname ) ;
    exit ( 1 ) ;
  }
 
  for ( int i = 0 ; !feof(fd) ; i++ )
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
 
    x -= 82.993 ;
    y = (-y) + 37.17 ;

    printf ( "%g,%g\n", x, y ) ;
  }
 
  fclose ( fd ) ;                                                               
}

