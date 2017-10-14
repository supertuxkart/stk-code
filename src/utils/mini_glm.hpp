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

#ifndef HEADER_MINI_GLM_HPP
#define HEADER_MINI_GLM_HPP

#include "LinearMath/btQuaternion.h"

#include <array>
#include <cassert>
#include <cstdint>
#include <quaternion.h>
#include <vector3d.h>

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
    inline uint32_t normalizedSignedFloatsTo1010102(std::array<float, 4> src)
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
        v = fminf(1.0f, fmaxf(-1.0f, src[3]));
        if (v > 0.0f)
        {
            part = (int)((v * 1.0f) + 0.5f);
        }
        else
        {
            part = (int)((v * 2.0f) - 0.5f);
        }
        part = (int)((v * 2.0f) - 0.5f);
        packed |= ((uint32_t)part & 3) << 30;
        return packed;
    }   // fourFloatsTo1010102
    // ------------------------------------------------------------------------
    inline std::array<float, 4> extractNormalizedSignedFloats(uint32_t packed,
        bool calculate_w = false)
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
        if (calculate_w)
        {
            float w = sqrtf(fmaxf(0.0f, 1.0f -
                (ret[0] * ret[0]) - (ret[1] * ret[1]) - (ret[2] * ret[2])));
            ret[3] = ret[3] > 0.0f ? w : -w;
        }
        return ret;
    }   // extractNormalizedSignedFloats
    // ------------------------------------------------------------------------
    // Please normalize vector and quaternion before compressing
    // ------------------------------------------------------------------------
    inline uint32_t compressVector3(const irr::core::vector3df& vec)
    {
        return normalizedSignedFloatsTo1010102({{vec.X, vec.Y, vec.Z, 0.0f}});
    }   // compressVector3
    // ------------------------------------------------------------------------
    inline core::vector3df decompressVector3(uint32_t packed)
    {
        const std::array<float, 4> out = extractNormalizedSignedFloats(packed);
        core::vector3df ret(out[0], out[1], out[2]);
        return ret.normalize();
    }   // decompressVector3
    // ------------------------------------------------------------------------
    inline uint32_t compressQuaternion(const core::quaternion& q)
    {
        return normalizedSignedFloatsTo1010102({{q.X, q.Y, q.Z,
            q.W >= 0.0f ? 1.0f : -1.0f}});
    }   // compressQuaternion
    // ------------------------------------------------------------------------
    inline core::quaternion decompressQuaternion(uint32_t packed)
    {
        const std::array<float, 4> out = extractNormalizedSignedFloats(packed,
            true/*calculate_w*/);
        core::quaternion ret(out[0], out[1], out[2], out[3]);
        return ret.normalize();
    }   // decompressQuaternion
    // ------------------------------------------------------------------------
    inline uint32_t compressbtQuaternion(const btQuaternion& q)
    {
        return normalizedSignedFloatsTo1010102({{q.x(), q.y(), q.z(),
            q.w() >= 0.0f ? 1.0f : -1.0f}});
    }   // compressbtQuaternion
    // ------------------------------------------------------------------------
    inline btQuaternion decompressbtQuaternion(uint32_t packed)
    {
        const std::array<float, 4> out = extractNormalizedSignedFloats(packed,
            true/*calculate_w*/);
        btQuaternion ret(out[0], out[1], out[2], out[3]);
        return ret.normalize();
    }   // decompressbtQuaternion
    // ------------------------------------------------------------------------
    void unitTesting();
}

#endif
