
#define DEEPEST_HELL -1000000.0f

float getHeightAndNormal ( sgVec3 my_position, sgVec3 normal ) ;

inline float getHeight ( sgVec3 my_position )
{
  return getHeightAndNormal ( my_position, NULL ) ;
}


