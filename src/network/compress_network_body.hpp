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

#include "LinearMath/btMotionState.h"
#include "btBulletDynamicsCommon.h"

namespace CompressNetworkBody
{
    using namespace MiniGLM;
    // ------------------------------------------------------------------------
    inline void compress(btTransform t, const Vec3& lv, const Vec3& av,
                         BareNetworkString* bns, btRigidBody* body,
                         btMotionState* ms)
    {
        bns->add(t.getOrigin());
        uint32_t compressed_q = compressQuaternion(t.getRotation());
        bns->addUInt32(compressed_q);
        std::array<short, 3> lvs =
            {{ toFloat16(lv.x()), toFloat16(lv.y()), toFloat16(lv.z()) }};
        bns->addUInt16(lvs[0]).addUInt16(lvs[1]).addUInt16(lvs[2]);
        std::array<short, 3> avs =
            {{ toFloat16(av.x()), toFloat16(av.y()), toFloat16(av.z()) }};
        bns->addUInt16(avs[0]).addUInt16(avs[1]).addUInt16(avs[2]);

        btQuaternion uncompressed_q = decompressbtQuaternion(compressed_q);
        t.setRotation(uncompressed_q);
        Vec3 uncompressed_lv(toFloat32(lvs[0]), toFloat32(lvs[1]),
            toFloat32(lvs[2]));
        Vec3 uncompressed_av(toFloat32(avs[0]), toFloat32(avs[1]),
            toFloat32(avs[2]));
        body->setWorldTransform(t);
        ms->setWorldTransform(t);
        body->setInterpolationWorldTransform(t);
        body->setLinearVelocity(uncompressed_lv);
        body->setAngularVelocity(uncompressed_av);
        body->setInterpolationLinearVelocity(uncompressed_lv);
        body->setInterpolationAngularVelocity(uncompressed_av);
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
    }   // decompress
};

#endif // HEADER_COMPRESS_NETWORK_BODY_HPP
