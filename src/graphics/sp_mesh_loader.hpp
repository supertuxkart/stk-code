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

#ifndef HEADER_SP_MESH_LOADER_HPP
#define HEADER_SP_MESH_LOADER_HPP

#include <IMeshLoader.h>
#include <ISceneManager.h>
#include <ISkinnedMesh.h>
#include <IReadFile.h>
#include <array>
#include <cstdint>
#include <vector>

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
    inline uint32_t vectorTo3Int10Bit(const irr::core::vector3df& vec)
    {
        int part;
        uint32_t sum;
        float v;
        sum = 0;
        v = fminf(1.0, fmaxf(-1.0, vec.X));
        if (v > 0.0)
        {
            part = (int)((v * 511.0) + 0.5);
        }
        else
        {
            part = (int)((v * 512.0) - 0.5);
        }
        sum |= ((uint32_t)part & 1023) << 0;
        v = fminf(1.0, fmaxf(-1.0, vec.Y));
        if (v > 0.0)
        {
            part = (int)((v * 511.0) + 0.5);
        }
        else
        {
            part = (int)((v * 512.0) - 0.5);
        }
        sum |= ((uint32_t)part & 1023) << 10;
        v = fminf(1.0, fmaxf(-1.0, vec.Z));
        if (v > 0.0)
        {
            part = (int)((v * 511.0) + 0.5);
        }
        else
        {
            part = (int)((v * 512.0) - 0.5);
        }
        sum |= ((uint32_t)part & 1023) << 20;
        v = 0.0f;
        part = (int)((v * 2.0) - 0.5);
        sum |= ((uint32_t)part & 3) << 30;
        return sum;
    }   // vectorTo3Int10Bit
    // ------------------------------------------------------------------------
    inline core::vector3df extract3Int10Bit(uint32_t sum)
    {
        core::vector3df ret;
        int part = sum & 1023;
        if (part & 512)
        {
            ret.X = (float)(1024 - part) * (-1.0f / 512.0f);
        }
        else
        {
            ret.X = (float)part * (1.0f / 511.0f);
        }
        part = (sum >> 10) & 1023;
        if (part & 512)
        {
            ret.Y = (float)(1024 - part) * (-1.0f / 512.0f);
        }
        else
        {
            ret.Y = (float)part * (1.0f / 511.0f);
        }
        part = (sum >> 20) & 1023;
        if (part & 512)
        {
            ret.Z = (float)(1024 - part) * (-1.0f / 512.0f);
        }
        else
        {
            ret.Z = (float)part * (1.0f / 511.0f);
        }
        return ret.normalize();
    }   // extract3Int10Bit

}

class SPMeshLoader : public scene::IMeshLoader
{
private:
    // ------------------------------------------------------------------------
    struct LocRotScale
    {
        core::vector3df m_loc;

        core::quaternion m_rot;

        core::vector3df m_scale;
        // --------------------------------------------------------------------
        inline core::matrix4 toMatrix() const
        {
            core::matrix4 lm, sm, rm;
            lm.setTranslation(m_loc);
            sm.setScale(m_scale);
            m_rot.getMatrix_transposed(rm);
            return lm * rm * sm;
        }
        // --------------------------------------------------------------------
        void read(irr::io::IReadFile* spm);

    };
    struct Armature
    {
        unsigned m_joint_used;

        std::vector<std::string> m_joint_names;

        std::vector<core::matrix4> m_joint_matrices;

        std::vector<core::matrix4> m_interpolated_matrices;

        std::vector<std::pair<core::matrix4, bool> > m_world_matrices;

        std::vector<int> m_parent_infos;

        std::vector<std::pair<int, std::vector<LocRotScale> > >
            m_frame_pose_matrices;

        // --------------------------------------------------------------------
        void read(irr::io::IReadFile* spm);
        // --------------------------------------------------------------------
        void getPose(float frame, core::matrix4* dest);
        // --------------------------------------------------------------------
        void getInterpolatedMatrices(float frame);
        // --------------------------------------------------------------------
        core::matrix4 getWorldMatrix(const std::vector<core::matrix4>& matrix,
                                     unsigned id);
    };
    // ------------------------------------------------------------------------
    unsigned m_bind_frame, m_joint_count;//, m_frame_count;
    // ------------------------------------------------------------------------
    std::vector<Armature> m_all_armatures;
    // ------------------------------------------------------------------------
    std::vector<core::matrix4> m_to_bind_pose_matrices;
    // ------------------------------------------------------------------------
    enum SPVertexType: unsigned int
    {
        SPVT_NORMAL,
        SPVT_SKINNED
    };
    // ------------------------------------------------------------------------
    void decompress(irr::io::IReadFile* spm, unsigned vertices_count,
                    unsigned indices_count, bool read_normal, bool read_vcolor,
                    bool read_tangent, bool uv_one, bool uv_two,
                    SPVertexType vt, const video::SMaterial& m);
    // ------------------------------------------------------------------------
    void createAnimationData(irr::io::IReadFile* spm);
    // ------------------------------------------------------------------------
    void convertIrrlicht();

    scene::ISkinnedMesh* m_mesh;

    scene::ISceneManager* m_scene_manager;

    std::vector<std::vector<
        std::pair<std::array<short, 4>, std::array<float, 4> > > > m_joints;

public:
    // ------------------------------------------------------------------------
    SPMeshLoader(scene::ISceneManager* smgr) : m_scene_manager(smgr) {}
    // ------------------------------------------------------------------------
    virtual bool isALoadableFileExtension(const io::path& filename) const;
    // ------------------------------------------------------------------------
    virtual scene::IAnimatedMesh* createMesh(io::IReadFile* file);

};

#endif

