//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

#ifndef HEADER_COMPRESS_NETWORK_BODY_HPP
#define HEADER_COMPRESS_NETWORK_BODY_HPP

#include "network/network_string.hpp"
#include "utils/mini_glm.hpp"

namespace CompressNetworkBody
{
    using namespace MiniGLM;
    // ------------------------------------------------------------------------
    inline void compress(const btTransform& t, const Vec3& lv, const Vec3& av,
                         BareNetworkString* bns)
    {
        bns->add(t.getOrigin());
        bns->addUInt32(compressQuaternion(t.getRotation()));
        bns->addUInt16(toFloat16(lv.x()))
            .addUInt16(toFloat16(lv.y())).addUInt16(toFloat16(lv.z()));
        bns->addUInt16(toFloat16(av.x()))
            .addUInt16(toFloat16(av.y())).addUInt16(toFloat16(av.z()));
    }   // compress
    // ------------------------------------------------------------------------
    inline void decompress(const BareNetworkString* bns, btTransform* t,
                           Vec3* lv, Vec3* av)
    {
        t->setOrigin(bns->getVec3());
        t->setRotation(decompressbtQuaternion(bns->getUInt32()));
        short vec[3];
        vec[0] = bns->getUInt16();
        vec[1] = bns->getUInt16();
        vec[2] = bns->getUInt16();
        *lv = Vec3(toFloat32(vec[0]), toFloat32(vec[1]), toFloat32(vec[2]));
        vec[0] = bns->getUInt16();
        vec[1] = bns->getUInt16();
        vec[2] = bns->getUInt16();
        *av = Vec3(toFloat32(vec[0]), toFloat32(vec[1]), toFloat32(vec[2]));
    }   // compress
};

#endif // HEADER_COMPRESS_NETWORK_BODY_HPP
