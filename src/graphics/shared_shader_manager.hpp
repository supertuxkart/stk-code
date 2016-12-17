//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#ifndef SERVER_ONLY

#ifndef HEADER_SHARED_SHADER_MANAGER_HPP
#define HEADER_SHARED_SHADER_MANAGER_HPP

#include "graphics/shared_shader.hpp"
#include "utils/no_copy.hpp"
#include "utils/singleton.hpp"

#include <algorithm>
#include <typeindex>
#include <unordered_map>
#include <vector>

class SharedShaderManager : public Singleton<SharedShaderManager>, NoCopy
{
private:
    std::unordered_map<std::type_index, int> m_shaders_map;

    std::vector<SharedShader*> m_shared_shaders;

    unsigned int m_shared_shader_loaded;

public:
    // ------------------------------------------------------------------------
    SharedShaderManager() { m_shared_shader_loaded = 0; }
    // ------------------------------------------------------------------------
    ~SharedShaderManager()
    {
        for (SharedShader* ss : m_shared_shaders)
            delete ss;
    }   // ~SharedShaderManager
    // ------------------------------------------------------------------------
    template <typename T>
    void addSharedShader(T* ss)
    {
        ss->loadSharedShader();
        m_shaders_map[std::type_index(typeid(T))] = m_shared_shader_loaded++;
        m_shared_shaders.push_back(ss);
    }   // addSharedShader
    // ------------------------------------------------------------------------
    template <typename T> T* getSharedShader()
    {
        std::unordered_map<std::type_index, int>::const_iterator i =
            m_shaders_map.find(std::type_index(typeid(T)));
        if (i != m_shaders_map.end())
        {
            T* ss = dynamic_cast<T*>(m_shared_shaders[i->second]);
            assert(ss != NULL);
            return ss;
        }
        else
        {
            T* new_ss = new T();
            addSharedShader(new_ss);
            return new_ss;
        }
    }   // getSharedShader
};   // SharedShaderManager

#endif

#endif   // !SERVER_ONLY

