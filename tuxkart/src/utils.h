
/*
  Some 3D geometry functions.
*/

void  pr_from_normal ( sgVec3 hpr, sgVec3 nrm ) ;
void hpr_from_normal ( sgVec3 hpr, sgVec3 nrm ) ;

/*
  Special stdio wrappers for stuff that
  the *nice* Mr Gates decided to make
  horribly non-standard.
*/

bool canAccess ( char *fname ) ;
bool chDir     ( char *dir   ) ;
void secondSleep ( int s ) ;


