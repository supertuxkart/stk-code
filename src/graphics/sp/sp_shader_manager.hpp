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

#ifndef HEADER_SP_SHADER_MANAGER_HPP
#define HEADER_SP_SHADER_MANAGER_HPP

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "utils/no_copy.hpp"

class XMLNode;
namespace SP
{
class SPShader;
class SPUniformAssigner;

enum SamplerType: unsigned int;
enum RenderPass: unsigned int;

class SPShaderManager : public NoCopy
{
public:
    struct PassInfo
    {
        std::function<void()> m_use_function;

        std::function<void()> m_unuse_function;

        std::string m_vertex_shader;

        std::string m_fragment_shader;

        std::string m_skinned_mesh_shader;

        std::vector<std::tuple<std::string, std::string, bool, SamplerType> >
            m_prefilled_textures;
    };

private:
    typedef std::vector<std::pair< std::string, std::function<void
        (SPUniformAssigner*)> > > UniformAssigners;
    struct ShaderInfo
    {
        std::string m_shader_name, m_fallback_name;

        int m_drawing_priority = 0;

        bool m_transparent_shader = false;

        bool m_use_alpha_channel = false;

        bool m_use_tangents = false;

        std::array<bool, 6> m_srgb =
            {{
                true, true, false, false, false, false
            }};
    };

    static SPShaderManager* m_spsm;

    std::unordered_map<std::string, std::shared_ptr<SPShader> > m_shaders;

    std::vector<std::shared_ptr<SPShader> > m_official_shaders;

    std::unordered_map<std::string, SamplerType> m_official_sampler_types;

    std::unordered_map<std::string, std::function<void(SPUniformAssigner*)> >
        m_official_uniform_assigner_functions;

    std::unordered_map<std::string, std::function<void()> >
        m_official_use_functions;

    std::unordered_map<std::string, std::function<void()> >
        m_official_unuse_functions;

    std::string m_shader_directory;

    // ------------------------------------------------------------------------
    std::string getShaderFullPath(const std::string& name);
    // ------------------------------------------------------------------------
    void loadPassInfo(const XMLNode* pass, PassInfo& pi);
    // ------------------------------------------------------------------------
    void loadEachShader(const std::string& file_name);
    // ------------------------------------------------------------------------
    std::shared_ptr<SPShader> buildSPShader(const ShaderInfo& si,
                                            const std::array<PassInfo, 2>& pi,
                                            const UniformAssigners& ua,
                                            bool skinned);

public:
    // ------------------------------------------------------------------------
    static SPShaderManager* get()
    {
        if (m_spsm == NULL)
        {
            m_spsm = new SPShaderManager();
        }
        return m_spsm;
    }
    // ------------------------------------------------------------------------
    static void destroy()
    {
        delete m_spsm;
        m_spsm = NULL;
    }
    // ------------------------------------------------------------------------
    static void addPrefilledTexturesToShader(SPShader* s,
        const std::vector<std::tuple<std::string, std::string, bool,
        SamplerType> >& t, RenderPass rp);
    // ------------------------------------------------------------------------
    SPShaderManager();
    // ------------------------------------------------------------------------
    ~SPShaderManager();
    // ------------------------------------------------------------------------
    std::shared_ptr<SPShader> getSPShader(const std::string& name)
    {
        auto ret = m_shaders.find(name);
        if (ret != m_shaders.end())
        {
            return ret->second;
        }
        return NULL;
    }
    // ------------------------------------------------------------------------
    void loadSPShaders(const std::string& directory_name);
    // ------------------------------------------------------------------------
    void addSPShader(const std::string& name,
                     std::shared_ptr<SPShader> shader)
    {
        m_shaders[name] = shader;
    }
    // ------------------------------------------------------------------------
    void unloadAll();
    // ------------------------------------------------------------------------
    void initAll();
    // ------------------------------------------------------------------------
    void removeUnusedShaders();
    // ------------------------------------------------------------------------
    void setOfficialShaders()
    {
        for (auto& p : m_shaders)
        {
            m_official_shaders.push_back(p.second);
        }
    }

};

}

#endif
