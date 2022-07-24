//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2017 SuperTuxKart-Team
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

#include "utils/constants.hpp"
#include "utils/log.hpp"
#include "mini_glm.hpp"

namespace MiniGLM
{
    // ------------------------------------------------------------------------
    void unitTesting()
    {
        Log::info("MiniGLM::unitTesting", "Half float compression");
        float tester = M_PI;
        short result = toFloat16(tester);
        float decompressed = toFloat32(result);
        Log::info("MiniGLM::unitTesting", "Result before: %f, after: %f",
            tester, decompressed);
        tester = -0.1234567f;
        result = toFloat16(tester);
        decompressed = toFloat32(result);
        Log::info("MiniGLM::unitTesting", "Result before: %f, after: %f",
            tester, decompressed);

        Log::info("MiniGLM::unitTesting", "Vector3D compression");
        core::vector3df vec(11.0f, 44.0f, 55.0f);
        vec.normalize();
        uint32_t packed = compressVector3(vec);
        core::vector3df out = decompressVector3(packed);
        Log::info("MiniGLM::unitTesting", "Result before: x:%f y:%f z:%f,"
            " after: x:%f y:%f z:%f", vec.X, vec.Y, vec.Z, out.X, out.Y,
            out.Z);
        vec = core::vector3df(-20.0f, -18.0f, 32.0f);
        vec.normalize();
        packed = compressVector3(vec);
        out = decompressVector3(packed);
        Log::info("MiniGLM::unitTesting", "Result before: x:%f y:%f z:%f,"
            " after: x:%f y:%f z:%f", vec.X, vec.Y, vec.Z, out.X, out.Y,
            out.Z);

        Log::info("MiniGLM::unitTesting", "Quaternion compression");
        core::quaternion quat(11.0f, 44.0f, 55.0f, 77.0f);
        quat.normalize();
        packed = compressIrrQuaternion(quat);
        core::quaternion out_quat = decompressQuaternion(packed);
        Log::info("MiniGLM::unitTesting", "Result before: x:%f y:%f z:%f w:%f,"
            " after: x:%f y:%f z:%f w:%f", quat.X, quat.Y, quat.Z, quat.W,
            out_quat.X, out_quat.Y, out_quat.Z, out_quat.W);
        quat = core::quaternion(-23.0f, -44.0f, -7.0f, 0.0f);
        quat.normalize();
        packed = compressIrrQuaternion(quat);
        out_quat = decompressQuaternion(packed);
        Log::info("MiniGLM::unitTesting", "Result before: x:%f y:%f z:%f w:%f,"
            " after: x:%f y:%f z:%f w:%f", quat.X, quat.Y, quat.Z, quat.W,
            out_quat.X, out_quat.Y, out_quat.Z, out_quat.W);
        quat = core::quaternion(0.0f, 0.0f, 0.0f, 1.0f);
        quat.normalize();
        packed = compressIrrQuaternion(quat);
        out_quat = decompressQuaternion(packed);
        Log::info("MiniGLM::unitTesting", "Result before: x:%f y:%f z:%f w:%f,"
            " after: x:%f y:%f z:%f w:%f", quat.X, quat.Y, quat.Z, quat.W,
            out_quat.X, out_quat.Y, out_quat.Z, out_quat.W);
        quat = core::quaternion(0.0f, 0.0f, 0.0f, -1.0f);
        quat.normalize();
        packed = compressIrrQuaternion(quat);
        out_quat = decompressQuaternion(packed);
        Log::info("MiniGLM::unitTesting", "Result before: x:%f y:%f z:%f w:%f,"
            " after: x:%f y:%f z:%f w:%f", quat.X, quat.Y, quat.Z, quat.W,
            out_quat.X, out_quat.Y, out_quat.Z, out_quat.W);
        quat = core::quaternion(-43.0f, 20.0f, 16.0f, -88.0f);
        quat.normalize();
        packed = compressIrrQuaternion(quat);
        out_quat = decompressQuaternion(packed);
        Log::info("MiniGLM::unitTesting", "Result before: x:%f y:%f z:%f w:%f,"
            " after: x:%f y:%f z:%f w:%f", quat.X, quat.Y, quat.Z, quat.W,
            out_quat.X, out_quat.Y, out_quat.Z, out_quat.W);
    }
}
