/* ==========================================================================
 * Copyright (c) 2022 SuperTuxKart-Team
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ==========================================================================
 */

#ifndef HEADER_MINI_GLM_HPP
#define HEADER_MINI_GLM_HPP

#include "LinearMath/btQuaternion.h"
#include "LinearMath/btTransform.h"
#include "LinearMath/btVector3.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <quaternion.h>
#include <vector3d.h>

#include "irrMath.h"

using namespace irr;

// GLM without template
namespace MiniGLM
{
    // ------------------------------------------------------------------------
    inline float overflow()
    {
        volatile float f = 1e10;
        for (int i = 0; i < 10; i++)
            f *= f; // this will overflow before the for loop terminates
        return f;
    }   // overflow
    // ------------------------------------------------------------------------
    inline float toFloat32(short value)
    {
        int s = (value >> 15) & 0x00000001;
        int e = (value >> 10) & 0x0000001f;
        int m =  value        & 0x000003ff;
        if (e == 0)
        {
            if (m == 0)
            {
                //
                // Plus or minus zero
                //
                uint32_t tmp_data = (unsigned int)(s << 31);
                float ret;
                memcpy(&ret, &tmp_data, 4);
                return ret;
            }
            else
            {
                //
                // Denormalized number -- renormalize it
                //
                while(!(m & 0x00000400))
                {
                    m <<= 1;
                    e -=  1;
                }

                e += 1;
                m &= ~0x00000400;
            }
        }
        else if (e == 31)
        {
            if (m == 0)
            {
                //
                // Positive or negative infinity
                //
                uint32_t tmp_data = (unsigned int)((s << 31) | 0x7f800000);
                float ret;
                memcpy(&ret, &tmp_data, 4);
                return ret;
            }
            else
            {
                //
                // Nan -- preserve sign and significand bits
                //
                uint32_t tmp_data = (unsigned int)((s << 31) | 0x7f800000 |
                    (m << 13));
                float ret;
                memcpy(&ret, &tmp_data, 4);
                return ret;
            }
        }

        //
        // Normalized number
        //
        e = e + (127 - 15);
        m = m << 13;
        //
        // Assemble s, e and m.
        //
        uint32_t tmp_data = (unsigned int)((s << 31) | (e << 23) | m);
        float ret;
        memcpy(&ret, &tmp_data, 4);
        return ret;
    }   // toFloat32
    // ------------------------------------------------------------------------
    inline short toFloat16(float const & f)
    {
        int i;
        memcpy(&i, &f, 4);
        //
        // Our floating point number, f, is represented by the bit
        // pattern in integer i.  Disassemble that bit pattern into
        // the sign, s, the exponent, e, and the significand, m.
        // Shift s into the position where it will go in in the
        // resulting half number.
        // Adjust e, accounting for the different exponent bias
        // of float and half (127 versus 15).
        //
        int s =  (i >> 16) & 0x00008000;
        int e = ((i >> 23) & 0x000000ff) - (127 - 15);
        int m =   i        & 0x007fffff;

        //
        // Now reassemble s, e and m into a half:
        //
        if (e <= 0)
        {
            if (e < -10)
            {
                //
                // E is less than -10.  The absolute value of f is
                // less than half_MIN (f may be a small normalized
                // float, a denormalized float or a zero).
                //
                // We convert f to a half zero.
                //
                return short(s);
            }

            //
            // E is between -10 and 0.  F is a normalized float,
            // whose magnitude is less than __half_NRM_MIN.
            //
            // We convert f to a denormalized half.
            //
            m = (m | 0x00800000) >> (1 - e);

            //
            // Round to nearest, round "0.5" up.
            //
            // Rounding may cause the significand to overflow and make
            // our number normalized.  Because of the way a half's bits
            // are laid out, we don't have to treat this case separately;
            // the code below will handle it correctly.
            //
            if (m & 0x00001000)
                m += 0x00002000;

            //
            // Assemble the half from s, e (zero) and m.
            //
            return short(s | (m >> 13));
        }
        else if (e == 0xff - (127 - 15))
        {
            if (m == 0)
            {
                //
                // F is an infinity; convert f to a half
                // infinity with the same sign as f.
                //
                return short(s | 0x7c00);
            }
            else
            {
                //
                // F is a NAN; we produce a half NAN that preserves
                // the sign bit and the 10 leftmost bits of the
                // significand of f, with one exception: If the 10
                // leftmost bits are all zero, the NAN would turn
                // into an infinity, so we have to set at least one
                // bit in the significand.
                //
                m >>= 13;
                return short(s | 0x7c00 | m | (m == 0));
            }
        }
        else
        {
            //
            // E is greater than zero.  F is a normalized float.
            // We try to convert f to a normalized half.
            //
            //
            // Round to nearest, round "0.5" up
            //
            if (m &  0x00001000)
            {
                m += 0x00002000;
                if (m & 0x00800000)
                {
                    m =  0;     // overflow in significand,
                    e += 1;     // adjust exponent
                }
            }

            //
            // Handle exponent overflow
            //
            if (e > 30)
            {
                overflow();        // Cause a hardware floating point overflow;

                return short(s | 0x7c00);
                // if this returns, the half becomes an
            }   // infinity with the same sign as f.

            //
            // Assemble the half from s, e and m.
            //
            return short(s | (e << 10) | (m >> 13));
        }
    }   // toFloat16
    // ------------------------------------------------------------------------
    inline uint32_t normalizedSignedFloatsTo1010102
        (const std::array<float, 3>& src, int extra_2_bit = -1)
    {
        int part = 0;
        uint32_t packed = 0;
        float v = fminf(1.0f, fmaxf(-1.0f, src[0]));
        if (v > 0.0f)
        {
            part = (int)((v * 511.0f) + 0.5f);
        }
        else
        {
            part = (int)((v * 512.0f) - 0.5f);
        }
        packed |= ((uint32_t)part & 1023) << 0;
        v = fminf(1.0f, fmaxf(-1.0f, src[1]));
        if (v > 0.0f)
        {
            part = (int)((v * 511.0f) + 0.5f);
        }
        else
        {
            part = (int)((v * 512.0f) - 0.5f);
        }
        packed |= ((uint32_t)part & 1023) << 10;
        v = fminf(1.0f, fmaxf(-1.0f, src[2]));
        if (v > 0.0f)
        {
            part = (int)((v * 511.0f) + 0.5f);
        }
        else
        {
            part = (int)((v * 512.0f) - 0.5f);
        }
        packed |= ((uint32_t)part & 1023) << 20;
        if (extra_2_bit >= 0)
        {
            part = extra_2_bit;
        }
        else
        {
            part = (int)(-0.5f);
        }
        packed |= ((uint32_t)part & 3) << 30;
        return packed;
    }   // normalizedSignedFloatsTo1010102
    // ------------------------------------------------------------------------
    inline std::array<short, 4> vertexType2101010RevTo4HF(uint32_t packed)
    {
        std::array<float, 4> ret;
        int part = packed & 1023;
        if (part & 512)
        {
            ret[0] = (float)(1024 - part) * (-1.0f / 512.0f);
        }
        else
        {
            ret[0] = (float)part * (1.0f / 511.0f);
        }
        part = (packed >> 10) & 1023;
        if (part & 512)
        {
            ret[1] = (float)(1024 - part) * (-1.0f / 512.0f);
        }
        else
        {
            ret[1] = (float)part * (1.0f / 511.0f);
        }
        part = (packed >> 20) & 1023;
        if (part & 512)
        {
            ret[2] = (float)(1024 - part) * (-1.0f / 512.0f);
        }
        else
        {
            ret[2] = (float)part * (1.0f / 511.0f);
        }
        part = (packed >> 30) & 3;
        if (part & 2)
        {
            ret[3] = (float)(4 - part) * (-1.0f / 2.0f);
        }
        else
        {
            ret[3] = (float)part;
        }
        std::array<short, 4> result;
        for (int i = 0; i < 4; i++)
        {
            result[i] = toFloat16(ret[i]);
        }
        return result;
    }   // vertexType2101010RevTo4HF
    // ------------------------------------------------------------------------
    inline std::array<float, 4> extractNormalizedSignedFloats(uint32_t packed,
        bool calculate_w = false)
    {
        std::array<float, 4> ret = {};
        int part = packed & 1023;
        if (part & 512)
        {
            ret[0] = (float)(1024 - part) * (-1.0f / 512.0f);
        }
        else
        {
            ret[0] = (float)part * (1.0f / 511.0f);
        }
        part = (packed >> 10) & 1023;
        if (part & 512)
        {
            ret[1] = (float)(1024 - part) * (-1.0f / 512.0f);
        }
        else
        {
            ret[1] = (float)part * (1.0f / 511.0f);
        }
        part = (packed >> 20) & 1023;
        if (part & 512)
        {
            ret[2] = (float)(1024 - part) * (-1.0f / 512.0f);
        }
        else
        {
            ret[2] = (float)part * (1.0f / 511.0f);
        }
        if (calculate_w)
        {
            float inv_sqrt_2 = 1.0f / sqrtf(2.0f);
            ret[0] *= inv_sqrt_2;
            ret[1] *= inv_sqrt_2;
            ret[2] *= inv_sqrt_2;
            float largest_val = sqrtf(fmaxf(0.0f, 1.0f -
                (ret[0] * ret[0]) - (ret[1] * ret[1]) - (ret[2] * ret[2])));
            part = (packed >> 30) & 3;
            switch(part)
            {
            case 0:
            {
                auto tmp = ret;
                ret[0] = largest_val;
                ret[1] = tmp[0];
                ret[2] = tmp[1];
                ret[3] = tmp[2];
                break;
            }
            case 1:
            {
                auto tmp = ret;
                ret[0] = tmp[0];
                ret[1] = largest_val;
                ret[2] = tmp[1];
                ret[3] = tmp[2];
                break;
            }
            case 2:
            {
                auto tmp = ret;
                ret[0] = tmp[0];
                ret[1] = tmp[1];
                ret[2] = largest_val;
                ret[3] = tmp[2];
                break;
            }
            case 3:
                ret[3] = largest_val;
                break;
            default:
                assert(false);
                break;
            }
        }
        return ret;
    }   // extractNormalizedSignedFloats
    // ------------------------------------------------------------------------
    // Please normalize vector before compressing
    // ------------------------------------------------------------------------
    inline uint32_t compressVector3(const irr::core::vector3df& vec)
    {
        return normalizedSignedFloatsTo1010102({{vec.X, vec.Y, vec.Z}});
    }   // compressVector3
    // ------------------------------------------------------------------------
    inline core::vector3df decompressVector3(uint32_t packed)
    {
        const std::array<float, 4> out = extractNormalizedSignedFloats(packed);
        core::vector3df ret(out[0], out[1], out[2]);
        return ret.normalize();
    }   // decompressVector3
    // ------------------------------------------------------------------------
    inline uint32_t compressQuaternion(const btQuaternion& q)
    {
        const float length = q.length();
        assert(length != 0.0f);
        std::array<float, 4> tmp_2 =
            {{
                q.x() / length,
                q.y() / length,
                q.z() / length,
                q.w() / length
            }};
        std::array<float, 3> tmp_3 = {};
        auto ret = std::max_element(tmp_2.begin(), tmp_2.end(),
            [](float a, float b) { return std::abs(a) < std::abs(b); });
        int extra_2_bit = int(std::distance(tmp_2.begin(), ret));
        float sqrt_2 = sqrtf(2.0f);
        switch (extra_2_bit)
        {
        case 0:
        {
            float neg = tmp_2[0] < 0.0f ? -1.0f : 1.0f;
            tmp_3[0] = tmp_2[1] * neg * sqrt_2;
            tmp_3[1] = tmp_2[2] * neg * sqrt_2;
            tmp_3[2] = tmp_2[3] * neg * sqrt_2;
            break;
        }
        case 1:
        {
            float neg = tmp_2[1] < 0.0f ? -1.0f : 1.0f;
            tmp_3[0] = tmp_2[0] * neg * sqrt_2;
            tmp_3[1] = tmp_2[2] * neg * sqrt_2;
            tmp_3[2] = tmp_2[3] * neg * sqrt_2;
            break;
        }
        case 2:
        {
            float neg = tmp_2[2] < 0.0f ? -1.0f : 1.0f;
            tmp_3[0] = tmp_2[0] * neg * sqrt_2;
            tmp_3[1] = tmp_2[1] * neg * sqrt_2;
            tmp_3[2] = tmp_2[3] * neg * sqrt_2;
            break;
        }
        case 3:
        {
            float neg = tmp_2[3] < 0.0f ? -1.0f : 1.0f;
            tmp_3[0] = tmp_2[0] * neg * sqrt_2;
            tmp_3[1] = tmp_2[1] * neg * sqrt_2;
            tmp_3[2] = tmp_2[2] * neg * sqrt_2;
            break;
        }
        default:
            assert(false);
            break;
        }
        return normalizedSignedFloatsTo1010102(tmp_3, extra_2_bit);
    }   // compressQuaternion
    // ------------------------------------------------------------------------
    inline uint32_t compressIrrQuaternion(const core::quaternion& q)
    {
        return compressQuaternion(btQuaternion(q.X, q.Y, q.Z, q.W));
    }
    // ------------------------------------------------------------------------
    inline core::quaternion decompressQuaternion(uint32_t packed)
    {
        const std::array<float, 4> out = extractNormalizedSignedFloats(packed,
            true/*calculate_w*/);
        core::quaternion ret(out[0], out[1], out[2], out[3]);
        return ret.normalize();
    }   // decompressQuaternion
    // ------------------------------------------------------------------------
    inline btQuaternion decompressbtQuaternion(uint32_t packed)
    {
        const std::array<float, 4> out = extractNormalizedSignedFloats(packed,
            true/*calculate_w*/);
        btQuaternion ret(out[0], out[1], out[2], out[3]);
        return ret.normalize();
    }   // decompressbtQuaternion
    // ------------------------------------------------------------------------
    inline std::array<float, 4> getQuaternionInternal(const core::matrix4& m)
    {
        btVector3 row[3];
        memcpy(&row[0][0], &m[0], 12);
        memcpy(&row[1][0], &m[4], 12);
        memcpy(&row[2][0], &m[8], 12);
        std::array<float, 4> q;
        float root = row[0].x() + row[1].y() + row[2].z();
        const float trace = root;
        if (trace > 0.0f)
        {
            root = sqrtf(trace + 1.0f);
            q[3] = 0.5f * root;
            root = 0.5f / root;
            q[0] = root * (row[1].z() - row[2].y());
            q[1] = root * (row[2].x() - row[0].z());
            q[2] = root * (row[0].y() - row[1].x());
        }
        else
        {
            static int next[3] = {1, 2, 0};
            int i = 0;
            int j = 0;
            int k = 0;
            if (row[1].y() > row[0].x())
            {
                i = 1;
            }
            if (row[2].z() > row[i][i])
            {
                i = 2;
            }
            j = next[i];
            k = next[j];

            root = sqrtf(row[i][i] - row[j][j] - row[k][k] + 1.0f);
            q[i] = 0.5f * root;
            root = 0.5f / root;
            q[j] = root * (row[i][j] + row[j][i]);
            q[k] = root * (row[i][k] + row[k][i]);
            q[3] = root * (row[j][k] - row[k][j]);
        }
        return q;
    }
    // ------------------------------------------------------------------------
    inline core::quaternion getQuaternion(const core::matrix4& m)
    {
        std::array<float, 4> q = getQuaternionInternal(m);
        return core::quaternion(q[0], q[1], q[2], q[3]).normalize();
    }
    // ------------------------------------------------------------------------
    inline btQuaternion getBulletQuaternion(const core::matrix4& m)
    {
        std::array<float, 4> q = getQuaternionInternal(m);
        return btQuaternion(q[0], q[1], q[2], q[3]).normalize();
    }
    // ------------------------------------------------------------------------

    inline uint32_t quickTangent(uint32_t packed_normal)
    {
        core::vector3df normal = decompressVector3(packed_normal);
        core::vector3df tangent;
        core::vector3df c1 =
            normal.crossProduct(core::vector3df(0.0f, 0.0f, 1.0f));
        core::vector3df c2 =
            normal.crossProduct(core::vector3df(0.0f, 1.0f, 0.0f));
        if (c1.getLengthSQ() > c2.getLengthSQ())
        {
            tangent = c1;
        }
        else
        {
            tangent = c2;
        }
        tangent.normalize();
        // Assume bitangent sign is positive 1.0f
        return compressVector3(tangent) | 1 << 30;
    }   // quickTangent
    // ------------------------------------------------------------------------
    /** Round and save compressed values (optionally) btTransform.
     *  It will round with 2 digits with min / max +/- 2^23 / 100 for origin in
     *  btTransform and call compressQuaternion above to compress the rotation
     *  part, if compressed_data is provided, 3 24 bits and 1 32 bits of
     *  compressed data will be written in an int[4] array.
     */
    inline void compressbtTransform(btTransform& cur_t,
                                    int* compressed_data = NULL)
    {
        int x = (int)(cur_t.getOrigin().x() * 100.0f);
        int y = (int)(cur_t.getOrigin().y() * 100.0f);
        int z = (int)(cur_t.getOrigin().z() * 100.0f);
        x = core::clamp(x, -0x800000, 0x7fffff);
        y = core::clamp(y, -0x800000, 0x7fffff);
        z = core::clamp(z, -0x800000, 0x7fffff);
        uint32_t compressed_q = compressQuaternion(cur_t.getRotation());
        cur_t.setOrigin(btVector3(
            (float)x / 100.0f,
            (float)y / 100.0f,
            (float)z / 100.0f));
        cur_t.setRotation(decompressbtQuaternion(compressed_q));
        if (compressed_data)
        {
            compressed_data[0] = x;
            compressed_data[1] = y;
            compressed_data[2] = z;
            compressed_data[3] = (int)compressed_q;
        }
    }   // compressbtTransform
    // ------------------------------------------------------------------------
    inline btTransform decompressbtTransform(int* compressed_data)
    {
        btTransform trans;
        trans.setOrigin(btVector3(
            (float)compressed_data[0] / 100.0f,
            (float)compressed_data[1] / 100.0f,
            (float)compressed_data[2] / 100.0f));
        trans.setRotation(decompressbtQuaternion(
            (uint32_t)compressed_data[3]));
        return trans;
    }   // decompressbtTransform
    // ------------------------------------------------------------------------
    void unitTesting();
}

#endif
