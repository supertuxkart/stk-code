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

#ifndef HEADER_SP_UNIFORM_ASSIGNER_HPP
#define HEADER_SP_UNIFORM_ASSIGNER_HPP

#include "graphics/gl_headers.hpp"
#include "utils/no_copy.hpp"

#ifdef DEBUG
#include "utils/log.hpp"
#endif

#include <typeinfo>
#include <typeindex>
#include <array>
#include <string>
#include <vector>

#include <matrix4.h>
#include <SColor.h>
#include <vector2d.h>
#include <vector3d.h>

namespace SP
{

class SPUniformAssigner : public NoCopy
{
private:
    const GLuint m_location;

    const std::type_index m_type;

    mutable bool m_assigned;

public:
    // ------------------------------------------------------------------------
    SPUniformAssigner(const std::type_index& ti, GLuint location)
        : m_location(location), m_type(ti), m_assigned(false) {}
    // ------------------------------------------------------------------------
    bool runtimeChecking(const std::type_info& ti) const
    {
#ifdef DEBUG
        if (m_type != ti)
        {
            Log::error("SPUniformAssigner", "%s doesn't match %s which is the"
                " type of this SPUniformAssigner", ti.name(), m_type.name());
            return false;
        }
#endif
        return true;
    }
    // ------------------------------------------------------------------------
    void getValue(const GLuint& p, irr::core::matrix4& mat) const
    {
        if (runtimeChecking(typeid(mat)))
        {
#ifndef SERVER_ONLY
            glGetUniformfv(p, m_location, mat.pointer());
#endif
        }
    }
    // ------------------------------------------------------------------------
    void getValue(const GLuint& p, std::array<float, 4>& v) const
    {
        if (runtimeChecking(typeid(v)))
        {
#ifndef SERVER_ONLY
            glGetUniformfv(p, m_location, v.data());
#endif
        }
    }
    // ------------------------------------------------------------------------
    void getValue(const GLuint& p, irr::core::vector3df& v) const
    {
        if (runtimeChecking(typeid(v)))
        {
#ifndef SERVER_ONLY
            glGetUniformfv(p, m_location, &v.X);
#endif
        }
    }
    // ------------------------------------------------------------------------
    void getValue(const GLuint& p, irr::core::vector2df& v) const
    {
        if (runtimeChecking(typeid(v)))
        {
#ifndef SERVER_ONLY
            glGetUniformfv(p, m_location, &v.X);
#endif
        }
    }
    // ------------------------------------------------------------------------
    void getValue(const GLuint& p, float& v) const
    {
        if (runtimeChecking(typeid(v)))
        {
#ifndef SERVER_ONLY
            glGetUniformfv(p, m_location, &v);
#endif
        }
    }
    // ------------------------------------------------------------------------
    void getValue(const GLuint& p, int& v) const
    {
        if (runtimeChecking(typeid(v)))
        {
#ifndef SERVER_ONLY
            glGetUniformiv(p, m_location, &v);
#endif
        }
    }
    // ------------------------------------------------------------------------
    void setValue(const irr::core::matrix4& mat) const
    {
        if (runtimeChecking(typeid(mat)))
        {
#ifndef SERVER_ONLY
            glUniformMatrix4fv(m_location, 1, GL_FALSE, mat.pointer());
            m_assigned = true;
#endif
        }
    }
    // ------------------------------------------------------------------------
    void setValue(const std::array<float, 4>& v) const
    {
        if (runtimeChecking(typeid(v)))
        {
#ifndef SERVER_ONLY
            glUniform4f(m_location, v[0], v[1], v[2], v[3]);
            m_assigned = true;
#endif
        }
    }
    // ------------------------------------------------------------------------
    void setValue(const irr::core::vector3df& v) const
    {
        if (runtimeChecking(typeid(v)))
        {
#ifndef SERVER_ONLY
            glUniform3f(m_location, v.X, v.Y, v.Z);
            m_assigned = true;
#endif
        }
    }
    // ------------------------------------------------------------------------
    void setValue(const irr::core::vector2df& v) const
    {
        if (runtimeChecking(typeid(v)))
        {
#ifndef SERVER_ONLY
            glUniform2f(m_location, v.X, v.Y);
            m_assigned = true;
#endif
        }
    }
    // ------------------------------------------------------------------------
    void setValue(float v) const
    {
        if (runtimeChecking(typeid(v)))
        {
#ifndef SERVER_ONLY
            glUniform1f(m_location, v);
            m_assigned = true;
#endif
        }
    }
    // ------------------------------------------------------------------------
    void setValue(int v) const
    {
        if (runtimeChecking(typeid(v)))
        {
#ifndef SERVER_ONLY
            glUniform1i(m_location, v);
            m_assigned = true;
#endif
        }
    }
    // ------------------------------------------------------------------------
    void reset() const
    {
        if (m_assigned)
        {
            m_assigned = false;
#ifndef SERVER_ONLY
            if (m_type == typeid(int))
            {
                glUniform1i(m_location, 0);
            }
            else if (m_type == typeid(float))
            {
                glUniform1f(m_location, 0.0f);
            }
            else if (m_type == typeid(irr::core::matrix4))
            {
                static const char zeroes[64] = {};
                glUniformMatrix4fv(m_location, 1, GL_FALSE, (float*)zeroes);
            }
            else if (m_type == typeid(std::array<float, 4>))
            {
                glUniform4f(m_location, 0.0f, 0.0f, 0.0f,0.0f);
            }
            else if (m_type == typeid(irr::core::vector3df))
            {
                glUniform3f(m_location, 0.0f, 0.0f, 0.0f);
            }
            else if (m_type == typeid(irr::core::vector2df))
            {
                glUniform2f(m_location, 0.0f, 0.0f);
            }
#endif
        }
    }
};

}

#endif
