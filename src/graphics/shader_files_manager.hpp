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

#ifndef HEADER_SHADER_FILES_MANAGER_HPP
#define HEADER_SHADER_FILES_MANAGER_HPP

#include "graphics/gl_headers.hpp"
#include "utils/log.hpp"
#include "utils/no_copy.hpp"
#include "utils/singleton.hpp"

#include <algorithm>
#include <cassert>
#include <string>
#include <memory>
#include <unordered_map>

class ShaderFilesManager : public Singleton<ShaderFilesManager>, NoCopy
{
private:
    typedef std::shared_ptr<GLuint> SharedShader;
    /**
     * Map from a filename in full path to a shader indentifier.
     * Used for caching shaders.
     */
    std::unordered_map<std::string, SharedShader> m_shader_files_loaded;

    // ------------------------------------------------------------------------
    const std::string& getHeader();
    // ------------------------------------------------------------------------
    void readFile(const std::string& file, std::ostringstream& code,
                  bool not_header = true);
    // ------------------------------------------------------------------------
    SharedShader addShaderFile(const std::string& full_path, unsigned type);

public:
    // ------------------------------------------------------------------------
    ShaderFilesManager() {}
    // ------------------------------------------------------------------------
    ~ShaderFilesManager()
    {
        removeAllShaderFiles();
    }
    // ------------------------------------------------------------------------
    void removeAllShaderFiles()
    {
        removeUnusedShaderFiles();
        if (!m_shader_files_loaded.empty())
        {
#ifdef DEBUG
            Log::error("ShaderFilesManager", "Some shader file > 1 ref_count");
#endif
            m_shader_files_loaded.clear();
        }
    }
    // ------------------------------------------------------------------------
    void removeUnusedShaderFiles()
    {
        for (auto it = m_shader_files_loaded.begin();
             it != m_shader_files_loaded.end();)
        {
            if (it->second.use_count() == 1 || !it->second)
            {
                it = m_shader_files_loaded.erase(it);
            }
            else
            {
                it++;
            }
        }
    }
    // ------------------------------------------------------------------------
    SharedShader loadShader(const std::string& full_path, unsigned type);
    // ------------------------------------------------------------------------
    SharedShader getShaderFile(const std::string& file, unsigned type);

};   // ShaderFilesManager

#endif

#endif   // !SERVER_ONLY
