//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_TRACK_H
#define HEADER_TRACK_H

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif
#include <plib/sg.h>
#include <string>
#include <vector>


class Track {
 private:
    float       gravity;
    std::string ident;
    std::string screenshot;
    std::string topview;
    std::string music_filename;
    std::string herringStyle;
    std::string description;

 public:

    std::string name;
    sgVec4      sky_color;
    bool        use_fog;
    sgVec4      fog_color;
    float       fog_density;
    float       fog_start;
    float       fog_end;
    sgVec3      sun_position;   /** Position of the sun */
    sgVec4      ambientcol;
    sgVec4      specularcol;
    sgVec4      diffusecol;

public:
    /** sgVec3 is a float[3] array, so unfortunately we can't put it in a
    * std::vector because it lacks a copy-constructor, this hack should help...
    */
    class sgVec3Wrapper
    {
    private:
    sgVec3 vec;

    public:
    sgVec3Wrapper(const sgVec3& o) {
      sgCopyVec3(vec, o);
    }

    operator const float* () const {
      return vec;
    }

    operator float* () {
      return vec;
    }
    };
    //FIXME: Maybe the next 4 vectors should be inside an struct and be used
    //from a vector of structs?
    //FIXME: should the driveline be set as a sgVec2?
    std::vector<sgVec3Wrapper> driveline;
    std::vector<SGfloat> distance_from_start;
    std::vector<SGfloat> path_width;
    std::vector<SGfloat> angle;

    sgVec2 driveline_min;
    sgVec2 driveline_max;


    float total_distance;

    float track2DWidth,  // Width and heigth of the 2d display of the track
        track2DHeight;
    float scaleX,        // Scaling to fit track into the size determined by
        scaleY;        // track2dWidth/Heightheigth
    bool doStretch;      // 2d track display might be stretched to fit better

public:
                     Track            (const char* filename,float w=100,
				       float h=100, bool stretch=1);
                    ~Track            ();
  void               draw2Dview       (float x, float y            ) const ;
  void               drawScaled2D     (float x, float y, float w,
				       float h                     ) const ;
  int                absSpatialToTrack(sgVec2 dst, sgVec3 xyz      ) const ;
  void               trackToSpatial   (sgVec3 xyz, int last_hint   ) const ;
  int                spatialToTrack   (sgVec2 last_pos, sgVec3 xyz,
				       int hint                    ) const ;

  float              getGravity       () const {return gravity;       }
  float              getTrackLength   () const {return total_distance;}
  const char*        getIdent         () const {return ident.c_str(); }
  const char*        getName          () const {return name.c_str();  }
  const char*        getMusic         () const {return music_filename.c_str();}
  const sgVec3& getSunPos             () const {return sun_position;  }
  const sgVec4& getAmbientCol         () const {return ambientcol;    }
  const sgVec4& getDiffuseCol         () const {return diffusecol;    }
  const sgVec4& getSpecularCol        () const {return specularcol;   }
  const bool&   useFog                () const {return use_fog;       }
  const sgVec4& getFogColor           () const {return fog_color;     }
  const float&  getFogDensity         () const {return fog_density;   }
  const float&  getFogStart           () const {return fog_start;     }
  const float&  getFogEnd             () const {return fog_end;       }
  const sgVec4& getSkyColor           () const {return sky_color;     }
  const std::string& getDescription   () const {return description;   }
  const std::string& getTopviewFile   () const {return topview;       }
  const std::string& getScreenshotFile() const {return screenshot;    }
  const std::vector<sgVec3Wrapper>& getDriveline () const {return driveline;}
  const std::vector<SGfloat>& getWidth() const {return path_width;    }
  const std::string& getHerringStyle  () const {return herringStyle;  }
  void               glVtx            (sgVec2 v, float xOff, float yOff) const {
    //                                       yOff-=(driveline_max[1]-driveline_min[1])*scaleY;
                                       glVertex2f(
                                       xOff+(v[0]-driveline_min[0])*scaleX,
				       yOff+(v[1]-driveline_min[1])*scaleY);}

private:
  void loadTrack                      (const char* filename);
  void loadDriveline                  ();
  void readDrivelineFromFile          (std::vector<sgVec3Wrapper>& line,
				       const std::string& file_ext      );
};   // class Track

#endif
