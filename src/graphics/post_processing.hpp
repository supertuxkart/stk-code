//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011-2013 the SuperTuxKart team 
//  Copyright (C) 2013      Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
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

#ifndef HEADER_POST_PROCESSING_HPP
#define HEADER_POST_PROCESSING_HPP

#include "IShaderConstantSetCallBack.h"
#include "S3DVertex.h"
#include "SMaterial.h"

#include <vector>

namespace irr
{
    namespace video { class IVideoDriver;   class ITexture; }
}
using namespace irr;

/** \brief   Handles post processing, eg motion blur
 *  \ingroup graphics
 */
class PostProcessing : public video::IShaderConstantSetCallBack
{
private:
    video::ITexture    *m_render_target;
    /** Material to be used when blurring is used. */
    video::SMaterial    m_blur_material;

    bool                m_supported;
    
    /** Boost time, how long the boost should be displayed. This also
     *  affects the strength of the effect: longer boost time will
     *  have a stronger effect. */
    std::vector<float>  m_boost_time;

    /** The center of blurring, in texture coordinates [0,1]).*/
    std::vector<core::vector2df> m_center;

    /** The center to which the blurring is aimed at, in [0,1]. */
    std::vector<core::vector2df> m_direction;

    /** True if any of the cameras is using post processing. */
    bool                m_used_pp_this_frame;

    /** Currently active camera during post-processing, needed in the
     *  OnSetConstants callback. */
    unsigned int        m_current_camera;


    struct Quad { video::S3DVertex v0, v1, v2, v3; };

    /** The vertices for the rectangle used for each camera. This includes
     *  the vertex position, normal, and texture coordinate. */
    std::vector<Quad> m_vertices;
    
public:
                 PostProcessing(video::IVideoDriver* video_driver);
    virtual     ~PostProcessing();
        
    void         reset();
    /** Those should be called around the part where we render the scene to be post-processed */
    void         beginCapture();
    void         endCapture();
    void         update(float dt);

    /** Render the post-processed scene */
    void         render();
    
    /** Is the hardware able to use post-processing? */
    inline bool  isSupported() const                 {return m_supported;}
    
    /** Use motion blur for a short time */
    void         giveBoost(unsigned int cam_index);
    
    /** Implement IShaderConstantsSetCallback. Shader constants setter for post-processing */
    virtual void OnSetConstants(video::IMaterialRendererServices *services, s32 user_data);
};

#endif // HEADER_POST_PROCESSING_HPP
