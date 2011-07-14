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

#ifndef HEADER_HARDWARE_SKINNING_HPP
#define HEADER_HARDWARE_SKINNING_HPP

#include <IShaderConstantSetCallBack.h>

using namespace irr;

namespace irr
{
namespace scene { class IAnimatedMeshSceneNode; }
}

class HardwareSkinning
{
public:
    static void init();
    static void prepareNode(scene::IAnimatedMeshSceneNode* node);
};

class HWSkinningCallback : public video::IShaderConstantSetCallBack
{
private:
    //const video::SMaterial *m_used_material;
    scene::IAnimatedMeshSceneNode* m_node;
    
public:
    HWSkinningCallback(scene::IAnimatedMeshSceneNode* node);
    virtual ~HWSkinningCallback();
    
/*    virtual void OnSetMaterial(const video::SMaterial& material)
    {
        m_used_material=&material;
    }
*/
    virtual void OnSetConstants(video::IMaterialRendererServices *services, s32 userData);
};

#endif // HEADER_HARDWARE_SKINNING_HPP
