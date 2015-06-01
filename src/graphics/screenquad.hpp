//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
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

#ifndef HEADER_SCREENQUAD_H
#define HEADER_SCREENQUAD_H

#include <SMaterial.h>
#include <IVideoDriver.h>

using namespace irr;
using namespace video;

class ScreenQuad {

public:
    ScreenQuad(IVideoDriver *xy)
    {
        vd = xy;

        mat.Lighting = false;
        mat.ZBuffer = video::ECFN_ALWAYS;
        mat.ZWriteEnable = false;

        for(u32 c = 0; c < MATERIAL_MAX_TEXTURES; c++)
        {
           mat.TextureLayer[c].TextureWrapU = video::ETC_CLAMP_TO_EDGE;
           mat.TextureLayer[c].TextureWrapV = video::ETC_CLAMP_TO_EDGE;
        }
    }

    SMaterial& getMaterial() { return mat; }

    //Set the texture to render with the quad
    void setTexture(ITexture* tex, u32 layer = 0)
    {
        mat.TextureLayer[layer].Texture = tex;
    }

    ITexture* getTexture(u32 layer = 0) const { return mat.TextureLayer[layer].Texture; }

    void setMaterialType(E_MATERIAL_TYPE mt) { mat.MaterialType = mt; }

    void render(bool setRTToFrameBuff = true) const
    {
        if(setRTToFrameBuff)
           vd->setRenderTarget(video::ERT_FRAME_BUFFER);

        sq_dorender();
    }

    void render(ITexture* rt) const
    {
        vd->setRenderTarget(rt);

        sq_dorender();
    }

protected:
    static const S3DVertex vertices[4];
    static const u16 indices[4];
    SMaterial mat;

    IVideoDriver* vd;

    void sq_dorender() const
    {
        vd->setMaterial(mat);
        vd->setTransform(ETS_WORLD, core::IdentityMatrix);
        vd->setTransform(ETS_VIEW, core::IdentityMatrix);
        vd->setTransform(ETS_PROJECTION, core::IdentityMatrix);
        vd->drawVertexPrimitiveList(vertices, 4, indices, 2, EVT_STANDARD,
                        scene::EPT_TRIANGLE_STRIP);
    }
};

#endif
