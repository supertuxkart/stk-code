#ifndef HEADER_GE_MATERIAL_MANAGER_HPP
#define HEADER_GE_MATERIAL_MANAGER_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <EMaterialTypes.h>

namespace GE
{

struct GEMaterial
{
    std::string m_vertex_shader;
    std::string m_skinning_vertex_shader;
    std::string m_fragment_shader;
    std::string m_depth_only_fragment_shader;
    std::function<void(uint32_t*, void**)> m_push_constants;
    // Fallback material used when PBR is disabled
    std::string m_nonpbr_fallback;
    bool m_alphablend;
    bool m_additive;
    bool m_backface_culling;
    bool m_depth_test;
    bool m_depth_write;
    std::vector<bool> m_srgb_settings;
    // ------------------------------------------------------------------------
    GEMaterial()
    {
        m_alphablend = false;
        m_additive = false;
        m_backface_culling = true;
        m_depth_test = true;
        m_depth_write = true;
        m_srgb_settings =
            { true, true, false, false, false, false, false, false };
    }
    // ------------------------------------------------------------------------
    bool texturelessDepth() const
    {
        return m_depth_only_fragment_shader.empty() ||
            m_depth_only_fragment_shader == "depth_only.frag";
    }
    // ------------------------------------------------------------------------
    bool isTransparent() const { return m_alphablend || m_additive; }
};   // GEMaterial

namespace GEMaterialManager
{
// ----------------------------------------------------------------------------
extern std::vector<std::pair<std::string, std::shared_ptr<const GEMaterial> > >
                                                                   g_materials;
// ----------------------------------------------------------------------------
void init();
// ----------------------------------------------------------------------------
void update();
// ----------------------------------------------------------------------------
irr::video::E_MATERIAL_TYPE getIrrMaterialType(const std::string& shader_name);
// ----------------------------------------------------------------------------
const std::string& getShader(irr::video::E_MATERIAL_TYPE mt);
// ----------------------------------------------------------------------------
std::shared_ptr<const GEMaterial> getMaterial(const std::string& shader_name);

};   // GEMaterialManager

}

#endif
