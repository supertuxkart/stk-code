//  SuperTuxKart - a fun racing game with go-kart
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

#ifndef HEADER_RTTS_HPP
#define HEADER_RTTS_HPP

namespace irr {
    namespace video {
        class ITexture;
    };
};

using irr::video::ITexture;

enum TypeRTT
{
    RTT_TMP1 = 0,
    RTT_TMP2,
    RTT_TMP3,
    RTT_TMP4,
    RTT_NORMAL_AND_DEPTH,
    RTT_COLOR,
    RTT_SPECULARMAP,

    RTT_HALF1,
    RTT_HALF2,

    RTT_QUARTER1,
    RTT_QUARTER2,
    RTT_QUARTER3,
    RTT_QUARTER4,

    RTT_EIGHTH1,
    RTT_EIGHTH2,

    RTT_SIXTEENTH1,
    RTT_SIXTEENTH2,

    RTT_SSAO,

    RTT_SHADOW0,
    RTT_SHADOW1,
    RTT_SHADOW2,
    RTT_COLLAPSE,
    RTT_COLLAPSEH,
    RTT_COLLAPSEV,
    RTT_COLLAPSEH2,
    RTT_COLLAPSEV2,
    RTT_WARPH,
    RTT_WARPV,

    RTT_HALF_SOFT,

    RTT_DISPLACE,

    RTT_COUNT
};

class RTT
{
public:
    RTT();
    ~RTT();

    ITexture *getRTT(TypeRTT which);
    unsigned getShadowFBO() const { return shadowFBO; }
    unsigned getShadowDepthTex() const { return shadowDepthTex; }

private:
    ITexture *rtts[RTT_COUNT];
    unsigned shadowFBO, shadowColorTex, shadowDepthTex;
};

#endif

