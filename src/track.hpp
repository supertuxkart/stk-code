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
#  ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#  endif
#  include <GL/gl.h>
#endif
#include <plib/sg.h>
#include <plib/ssg.h>
#include <string>
#include <vector>
#include "btBulletDynamicsCommon.h"
#include "material.hpp"
#include "triangle_mesh.hpp"
#include "music_information.hpp"

class Track
{
private:
    float                    m_gravity;
    std::string              m_ident;
    std::string              m_screenshot;
    std::string              m_top_view;
    std::vector<MusicInformation*> m_music;
    std::vector<float>       m_start_x, m_start_y, m_start_z, m_start_heading;
    std::string              m_herring_style;
    std::string              m_description;
    std::string              m_designer;
    std::string              m_filename;
    ssgBranch*               m_model;
    TriangleMesh*            m_track_mesh;
    TriangleMesh*            m_non_collision_mesh;
    // The next two variables are for AI improvements: the AI sometimes does
    // not estimate curve speed and/or angle correctly, resulting in too much
    // braking. These factors are used to adjust this.
    float                    m_AI_angle_adjustment;
    float                    m_AI_curve_speed_adjustment;
public:
    enum RoadSide{ RS_DONT_KNOW = -1, RS_LEFT = 0, RS_RIGHT = 1 };

    //An enum is not used for the QUAD_TRI_* constants because of limitations
    //of the conversion between enums and ints.
    static const int QUAD_TRI_NONE;
    static const int QUAD_TRI_FIRST;
    static const int QUAD_TRI_SECOND;

    static const int UNKNOWN_SECTOR;

    struct SegmentTriangle
    {
        int segment;
        int triangle;

        SegmentTriangle
        (
            int _segment,
            int _triangle
        ) : segment(_segment), triangle(_triangle) {};
    };

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
    static const float NOHIT;

    float m_track_2d_width,  // Width and heigth of the 2d display of the track
          m_track_2d_height;
    float m_scale_x,        // Scaling to fit track into the size determined by
          m_scale_y;        // track2dWidth/Heightheigth
    bool m_do_stretch;      // 2d track display might be stretched to fit better

                       Track             (std::string filename,float w=100,
                                          float h=100, bool stretch=1);
                      ~Track             ();
    void               cleanup           ();
    void               addDebugToScene   (int type                    ) const;
    void               draw2Dview        (float x_offset,
                                          float y_offset              ) const;
    void               drawScaled2D      (float x, float y, float w,
                                          float h                     ) const;

    void               findRoadSector    (const sgVec3 XYZ, int *sector) const;
    int                findOutOfRoadSector(const sgVec3 XYZ,
                                           const RoadSide SIDE,
                                           const int CURR_SECTOR
                                           ) const;
    int                spatialToTrack    (sgVec3 dst,
                                          const sgVec2 POS,
                                          const int SECTOR            ) const;
    void               trackToSpatial    (sgVec3 xyz, const int SECTOR) const;
    void               loadTrackModel    ();
    bool               isShortcut        (const int OLDSEC, const int NEWSEC) const;
    void               addMusic          (MusicInformation* mi)
                                                  {m_music.push_back(mi);       }
    ssgBranch*         getModel          () const {return m_model;              }
    float              getGravity        () const {return m_gravity;            }
    float              getTrackLength    () const {return m_total_distance;     }
    const std::string& getIdent          () const {return m_ident;              }
    const char*        getName           () const {return m_name.c_str();       }
    void               startMusic        () const;
    const std::string& getFilename       () const {return m_filename;           }
    const sgVec3& getSunPos              () const {return m_sun_position;       }
    const sgVec4& getAmbientCol          () const {return m_ambient_col;        }
    const sgVec4& getDiffuseCol          () const {return m_diffuse_col;        }
    const sgVec4& getSpecularCol         () const {return m_specular_col;       }
    const bool&   useFog                 () const {return m_use_fog;            }
    const sgVec4& getFogColor            () const {return m_fog_color;          }
    const float&  getFogDensity          () const {return m_fog_density;        }
    const float&  getFogStart            () const {return m_fog_start;          }
    const float&  getFogEnd              () const {return m_fog_end;            }
    const float&  getAIAngleAdjustment   () const {return m_AI_angle_adjustment;}
    const float&  getAICurveSpeedAdjustment() const {return m_AI_curve_speed_adjustment;}
    const sgVec4& getSkyColor            () const {return m_sky_color;          }
    const std::string& getDescription    () const {return m_description;        }
    const std::string& getDesigner       () const {return m_designer;           }
    const std::string& getTopviewFile    () const {return m_top_view;           }
    const std::string& getScreenshotFile () const {return m_screenshot;         }
    const std::vector<SGfloat>& getWidth () const {return m_path_width;         }
    const std::string& getHerringStyle   () const {return m_herring_style;      }
    void               getStartCoords    (unsigned int pos, sgCoord* coords) const;
    void  getTerrainInfo(const btVector3 &pos, float *hot, btVector3* normal, 
                         const Material **material) const;
    void createPhysicsModel              ();
    void               glVtx             (sgVec2 v, float x_offset, float y_offset) const
    {
        glVertex2f(
            x_offset+(v[0]-m_driveline_min[0])*m_scale_x,
            y_offset+(v[1]-m_driveline_min[1])*m_scale_y);
    }

private:
    void  loadTrack                      (std::string filename);
    void  herring_command                (sgVec3 *xyz, char htype, int bNeedHeight);
    void  loadDriveline                  ();
    void  readDrivelineFromFile          (std::vector<sgVec3Wrapper>& line,
                                         const std::string& file_ext      );
    void  convertTrackToBullet           (ssgEntity *track, sgMat4 m);

    float pointSideToLine(const sgVec2 L1, const sgVec2 L2,
                          const sgVec2 P ) const;
    int   pointInQuad(const sgVec2 A, const sgVec2 B,
                      const sgVec2 C, const sgVec2 D, const sgVec2 POINT ) const;
    void  getMusicInformation(std::vector<std::string>&             filenames, 
                              std::vector<MusicInformation*>& m_music   );
}
;   // class Track

#endif
