#include "ge_material_manager.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"

#include "vector3d.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <unordered_map>

#include "../source/Irrlicht/os.h"

namespace GE
{
// ============================================================================
namespace GEMaterialManager
{
std::vector<
      std::pair<std::string, std::shared_ptr<const GEMaterial> > > g_materials;

irr::core::vector3df g_wind_direction;

std::unordered_map<std::string, std::shared_ptr<const GEMaterial> > g_mat_map;

std::unordered_map<std::string, irr::video::E_MATERIAL_TYPE> g_mat_id_map;

std::unordered_map<uint32_t, std::string> g_id_mat_map;

std::unordered_map<std::string, std::function<void(uint32_t*, void**)> >
    g_default_push_constants =
    {
        {
            "grass",
            [](uint32_t* size, void** data)
            {
                *size = sizeof(irr::core::vector3df);
                *data = &g_wind_direction;
            }
        },
        {
            "displace",
            [](uint32_t* size, void** data)
            {
                *size = sizeof(getDisplaceDirection());
                *data = getDisplaceDirection().data();
            }
        }
    };
// ============================================================================
}   // GEMaterialManager
// ----------------------------------------------------------------------------
std::string readString(io::IXMLReaderUTF8* xml)
{
    if (xml->read() && xml->getNodeType() == io::EXN_TEXT)
        return xml->getNodeData();
    return "";
}   // readString

// ----------------------------------------------------------------------------
bool readBool(io::IXMLReaderUTF8* xml)
{
    return readString(xml) == "true";
}   // readBool

// ============================================================================
void GEMaterialManager::init()
{
    std::array<std::string, irr::video::EMT_MATERIAL_COUNT> def_mappings = {};
    def_mappings[irr::video::EMT_SOLID] = "solid";
    def_mappings[irr::video::EMT_NORMAL_MAP_SOLID] = "normalmap";
    def_mappings[irr::video::EMT_SOLID_2_LAYER] = "decal";
    def_mappings[irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF] = "alphatest";
    def_mappings[irr::video::EMT_STK_GRASS] = "grass";
    def_mappings[irr::video::EMT_TRANSPARENT_ALPHA_CHANNEL] = "alphablend";
    def_mappings[irr::video::EMT_TRANSPARENT_ADD_COLOR] = "additive";
    uint32_t mapping_cursor = 0;

    g_materials.clear();
    g_mat_map.clear();
    g_mat_id_map.clear();
    g_id_mat_map.clear();
    io::IXMLReaderUTF8* xml =
        getDriver()->getFileSystem()->createXMLReaderUTF8(
        (GE::getShaderFolder() + "shader_settings.xml").c_str());
    if (!xml)
        throw std::runtime_error("Could not load shader_settings.xml");

    while (xml->read())
    {
        if (xml->getNodeType() == io::EXN_ELEMENT &&
            !strcmp(xml->getNodeName(), "setting"))
        {
            GEMaterial settings;
            std::string name = xml->getAttributeValue("name");
            while (xml->read())
            {
                if (xml->getNodeType() == io::EXN_ELEMENT)
                {
                    if (!strcmp(xml->getNodeName(), "properties"))
                    {
                        while (xml->read())
                        {
                            if (xml->getNodeType() == io::EXN_ELEMENT)
                            {
                                const char* node_name = xml->getNodeName();
                                if (!strcmp(node_name, "depth-write"))
                                    settings.m_depth_write = readBool(xml);
                                else if (!strcmp(node_name, "depth-test"))
                                    settings.m_depth_test = readBool(xml);
                                else if (!strcmp(node_name, "backface-culling"))
                                    settings.m_backface_culling = readBool(xml);
                                else if (!strcmp(node_name, "nonpbr-fallback"))
                                    settings.m_nonpbr_fallback = readString(xml);
                                else if (!strcmp(node_name, "alphablend"))
                                    settings.m_alphablend = readBool(xml);
                                else if (!strcmp(node_name, "additive"))
                                    settings.m_additive = readBool(xml);
                                else if (!strcmp(node_name, "srgb-settings"))
                                {
                                    std::string srgb_str = readString(xml);
                                    for (unsigned i = 0; i < std::min(srgb_str.size(),
                                        settings.m_srgb_settings.size()); i++)
                                    {
                                        settings.m_srgb_settings[i] = (srgb_str[i] == 'Y');
                                    }
                                }
                            }
                            else if (xml->getNodeType() == io::EXN_ELEMENT_END &&
                                !strcmp(xml->getNodeName(), "properties"))
                                break;
                        }
                    }
                    else if (!strcmp(xml->getNodeName(), "shaders"))
                    {
                        while (xml->read())
                        {
                            if (xml->getNodeType() == io::EXN_ELEMENT)
                            {
                                const char* node_name = xml->getNodeName();
                                if (!strcmp(node_name, "vertex"))
                                    settings.m_vertex_shader = readString(xml);
                                else if (!strcmp(node_name, "fragment"))
                                    settings.m_fragment_shader = readString(xml);
                                else if (!strcmp(node_name, "depth"))
                                    settings.m_depth_only_fragment_shader = readString(xml);
                                else if (!strcmp(node_name, "skinning-vertex"))
                                    settings.m_skinning_vertex_shader = readString(xml);
                            }
                            else if (xml->getNodeType() == io::EXN_ELEMENT_END &&
                                !strcmp(xml->getNodeName(), "shaders"))
                                break;
                        }
                    }
                }
                else if (xml->getNodeType() == io::EXN_ELEMENT_END &&
                    !strcmp(xml->getNodeName(), "setting"))
                    break;
            }
            if (g_default_push_constants.find(name) !=
                g_default_push_constants.end())
            {
                settings.m_push_constants = g_default_push_constants.at(name);
            }

            bool found = false;
            for (uint32_t i = 0; i < def_mappings.size(); i++)
            {
                if (def_mappings[i] == name)
                {
                    g_mat_id_map[name] = (irr::video::E_MATERIAL_TYPE)i;
                    g_id_mat_map[i] = name;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                while (mapping_cursor < def_mappings.size() &&
                    !def_mappings[mapping_cursor].empty())
                {
                    mapping_cursor = mapping_cursor + 1;
                }
                if (mapping_cursor < def_mappings.size())
                {
                    g_mat_id_map[name] = (irr::video::E_MATERIAL_TYPE)mapping_cursor;
                    g_id_mat_map[mapping_cursor] = name;
                    def_mappings[mapping_cursor] = name;
                    mapping_cursor = mapping_cursor + 1;
                }
                else
                {
                    char msg[50] = {};
                    snprintf(msg, 50, "Too many materials which exceeded %u.",
                        irr::video::EMT_MATERIAL_COUNT);
                    os::Printer::log("GEMaterialManager", msg);
                    xml->drop();
                    return;
                }
            }
            auto m = std::make_shared<const GEMaterial>(settings);
            g_materials.emplace_back(name, m);
            g_mat_map[name] = m;
        }
    }

    xml->drop();
}   // init

// ----------------------------------------------------------------------------
void GEMaterialManager::update()
{
    g_wind_direction = irr::core::vector3df(1.0f, 0.0f, 0.0f) *
        (getMonoTimeMs() / 1000.0f) * 1.5f;
}   // update

// ----------------------------------------------------------------------------
irr::video::E_MATERIAL_TYPE
          GEMaterialManager::getIrrMaterialType(const std::string& shader_name)
{
    if (g_mat_id_map.find(shader_name) != g_mat_id_map.end())
        return g_mat_id_map.at(shader_name);
    return (irr::video::E_MATERIAL_TYPE)0;
}   // getIrrMaterialType

// ----------------------------------------------------------------------------
const std::string& GEMaterialManager::getShader(irr::video::E_MATERIAL_TYPE mt)
{
    uint32_t id = mt;
    if (g_id_mat_map.find(id) != g_id_mat_map.end())
        return g_id_mat_map.at(id);
    return g_id_mat_map.at(0);
}   // getShader

// ----------------------------------------------------------------------------
std::shared_ptr<const GEMaterial>
                 GEMaterialManager::getMaterial(const std::string& shader_name)
{
    if (g_mat_map.find(shader_name) != g_mat_map.end())
        return g_mat_map.at(shader_name);
    return nullptr;
}   // getMaterial

}
