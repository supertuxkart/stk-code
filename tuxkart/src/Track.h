

#define MAX_DRIVELINE  1000
#define TRACKVIEW_SIZE 100.0f

class Track
{
    sgVec2 min ;
    sgVec2 max ;
    sgVec2 center ;
    float  scale ;

    float  total_distance ;

  public:
    Track ( char *drv_fname ) ;

    void glVtx ( sgVec2 v, float xoff, float yoff )
    {
      glVertex2f ( xoff + ( v[0] - center[0] ) * scale,
                   yoff + ( v[1] - center[1] ) * scale ) ;
    }

    int  absSpatialToTrack ( sgVec2 dst, sgVec3 xyz ) ;
    int  spatialToTrack ( sgVec2 last_pos, sgVec3 xyz, int hint ) ;
    void trackToSpatial ( sgVec3 xyz, int last_hint ) ;

    float getTrackLength () { return total_distance ; }
    void draw2Dview ( float x, float y ) ;
} ;


extern Track *curr_track ;

