//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2011 the SuperTuxKart team
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

#include <IShaderConstantSetCallBack.h>
#include <SMaterial.h>

namespace irr
{
    namespace video { class IVideoDriver;   class ITexture; }
}
using namespace irr;

class PostProcessing : public video::IShaderConstantSetCallBack
{
private:
    video::ITexture            *m_render_target;
    video::SMaterial            m_material;
    bool                        m_supported;
    
    /** Boost amount, used to tune the motion blur. Must be in the range 0.0 to 1.0 */
    float                       m_boost_amount;
public:
    PostProcessing();
    virtual ~PostProcessing();
    
    /** Initialization/termination management */
    void            init(video::IVideoDriver* video_driver);
    void            shut();
    
    /** Those should be called around the part where we render the scene to be post-processed */
    void            beginCapture();
    void            endCapture();
    
    /** Render the post-processed scene */
    void            render();
    
    /** Is the hardware able to use post-processing? */
    inline bool     isSupported() const                 {return m_supported;}
    
    /** Set the boost amount according to the speed of the camera */
    void            setCameraSpeed(float cam_speed);
    
    /** Implement IShaderConstantsSetCallback. Shader constants setter for post-processing */
    virtual void    OnSetConstants(video::IMaterialRendererServices *services, s32 user_data);
};

#endif // HEADER_POST_PROCESSING_HPP
