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


class Track
{
private:
    float       m_gravity;
    std::string m_ident;
    std::string m_screenshot;
    std::string m_top_view;
    std::string m_music_filename;
    std::string m_herring_style;
    std::string m_description;
    std::string m_filename;

public:

    std::string m_name;
    sgVec4      m_sky_color;
    bool        m_use_fog;
    sgVec4      m_fog_color;
    float       m_fog_density;
    float       m_fog_start;
    float       m_fog_end;
    sgVec3      m_sun_position;   /** Position of the sun */
    sgVec4      m_ambient_col;
    sgVec4      m_specular_col;
    sgVec4      m_diffuse_col;

public:
    /** sgVec3 is a float[3] array, so unfortunately we can't put it in a
    * std::vector because it lacks a copy-constructor, this hack should help...
    */
    class sgVec3Wrapper
    {
    private:
        sgVec3 vec;

    public:
        sgVec3Wrapper(const sgVec3& o)
        {
            sgCopyVec3(vec, o);
        }

        operator const float* () const
        {
            return vec;
        }

        operator float* ()
        {
            return vec;
        }
    };
    //FIXME: Maybe the next 4 vectors should be inside an struct and be used
    //from a vector of structs?
    //FIXME: should the driveline be set as a sgVec2?
    std::vector<sgVec3Wrapper> m_driveline;
    std::vector<SGfloat> m_distance_from_start;
    std::vector<SGfloat> m_path_width;
    std::vector<SGfloat> m_angle;

	//Left and Right drivelines for overhead map rendering.
	//(Should probably be private as they are only use internally right now)
    std::vector<sgVec3Wrapper> m_left_driveline;
    std::vector<sgVec3Wrapper> m_right_driveline;  

    sgVec2 m_driveline_min;
    sgVec2 m_driveline_max;


    float m_total_distance;

    float m_track_2d_width,  // Width and heigth of the 2d display of the track
          m_track_2d_height;
    float m_scale_x,        // Scaling to fit track into the size determined by
          m_scale_y;        // track2dWidth/Heightheigth
    bool m_do_stretch;      // 2d track display might be stretched to fit better

public:
    Track            (std::string filename,float w=100,
                      float h=100, bool stretch=1);
    ~Track            ();
    void               draw2Dview       (float x, float y            ) const ;
    void               drawScaled2D     (float x, float y, float w,
                                         float h                     ) const ;
    int                absSpatialToTrack(sgVec2 dst, sgVec3 xyz      ) const ;
    void               trackToSpatial   (sgVec3 xyz, int last_hint   ) const ;
    int                spatialToTrack   (sgVec2 last_pos, sgVec3 xyz,
                                         int hint                    ) const ;

    float              getGravity       () const {return m_gravity;       }
    float              getTrackLength   () const {return m_total_distance;}
    const char*        getIdent         () const {return m_ident.c_str(); }
    const char*        getName          () const {return m_name.c_str();  }
    const std::string& getMusic         () const {return m_music_filename;}
    const std::string& getFilename      () const {return m_filename; }
    const sgVec3& getSunPos             () const {return m_sun_position;  }
    const sgVec4& getAmbientCol         () const {return m_ambient_col;    }
    const sgVec4& getDiffuseCol         () const {return m_diffuse_col;    }
    const sgVec4& getSpecularCol        () const {return m_specular_col;   }
    const bool&   useFog                () const {return m_use_fog;       }
    const sgVec4& getFogColor           () const {return m_fog_color;     }
    const float&  getFogDensity         () const {return m_fog_density;   }
    const float&  getFogStart           () const {return m_fog_start;     }
    const float&  getFogEnd             () const {return m_fog_end;       }
    const sgVec4& getSkyColor           () const {return m_sky_color;     }
    const std::string& getDescription   () const {return m_description;   }
    const std::string& getTopviewFile   () const {return m_top_view;       }
    const std::string& getScreenshotFile() const {return m_screenshot;    }
    const std::vector<sgVec3Wrapper>& getDriveline () const {return m_driveline;}
    const std::vector<SGfloat>& getWidth() const {return m_path_width;    }
    const std::string& getHerringStyle  () const {return m_herring_style;  }
    void               glVtx            (sgVec2 v, float xOff, float yOff) const
    {
        //                                       yOff-=(driveline_max[1]-m_driveline_min[1])*scaleY;
        glVertex2f(
            xOff+(v[0]-m_driveline_min[0])*m_scale_x,
            yOff+(v[1]-m_driveline_min[1])*m_scale_y);
    }

private:
    void loadTrack                      (std::string filename);
    void loadDriveline                  ();
    void readDrivelineFromFile          (std::vector<sgVec3Wrapper>& line,
                                         const std::string& file_ext      );
}
;   // class Track

#endif
