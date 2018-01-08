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

#ifndef HEADER_SP_PER_OBJECT_UNIFORM_HPP
#define HEADER_SP_PER_OBJECT_UNIFORM_HPP

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace SP
{

class SPUniformAssigner;
class SPPerObjectUniform
{
private:
    std::unordered_map<std::string,
        std::function<void(SPUniformAssigner*)> > m_all_uniforms;
public:
    // ------------------------------------------------------------------------
    void addAssignerFunction(const std::string& name,
                             std::function<void(SPUniformAssigner*)> func)
    {
        m_all_uniforms[name] = func;
    }
    // ------------------------------------------------------------------------
    void removeAssignerFunction(const std::string& name)
    {
        auto it = m_all_uniforms.find(name);
        if (it != m_all_uniforms.end())
        {
            m_all_uniforms.erase(it);
        }
    }
    // ------------------------------------------------------------------------
    bool hasUniform(const std::string& name) const
    {
        auto ret = m_all_uniforms.find(name);
        if (ret == m_all_uniforms.end())
        {
            return false;
        }
        return true;
    }
    // ------------------------------------------------------------------------
    bool assignUniform(const std::string& name, SPUniformAssigner* ua) const;
    // ------------------------------------------------------------------------
    bool isEmpty() const                     { return m_all_uniforms.empty(); }

};

}

#endif
