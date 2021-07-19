#include "karts/official_karts.hpp"

#include "karts/kart_properties_manager.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "utils/file_utils.hpp"
#include "utils/log.hpp"
#include "utils/vec3.hpp"

#include <cassert>
#include <fstream>
#include <sstream>
#include <vector>

namespace OfficialKarts
{
// ============================================================================
struct OfficialKart
{
std::string name;
std::string type;
float width;
float height;
float length;
Vec3 gravity_shift;
OfficialKart(const std::string& n, const std::string& t, float w, float h,
             float l, const Vec3& g) : name(n), type(t), width(w), height(h),
             length(l), gravity_shift(g) {}
};   // OfficialKart
std::vector<OfficialKart> g_official_karts;

// ----------------------------------------------------------------------------
void dumpOfficialKarts()
{
    std::stringstream ss;
    ss << "<?xml version=\"1.0\"?>\n";
    ss << "<karts>\n";
    for (unsigned i = 0; i < kart_properties_manager->getNumberOfKarts(); i++)
    {
        const KartProperties* kp = kart_properties_manager->getKartById(i);
        if (kp->isAddon())
            continue;
        auto km = kp->getKartModelCopy();
        ss << "    <kart name=\"" << kp->getIdent() << "\" type=\"" <<
            kp->getKartType() << "\" width=\"" << km->getWidth() <<
            "\" height=\"" << km->getHeight() << "\" length=\"" <<
            km->getLength() << "\" gravity-shift=\"" <<
            kp->getGravityCenterShift().x() << " " <<
            kp->getGravityCenterShift().y() << " " <<
            kp->getGravityCenterShift().z() << "\"/>\n";
    }
    ss << "</karts>\n";
    std::string s = ss.str();
    std::ofstream xml("official_karts.xml", std::ofstream::out);
    xml << ss.rdbuf();
    xml.close();
}   // getAllData

// ----------------------------------------------------------------------------
void load()
{
    const std::string file_name = file_manager->getAsset("official_karts.xml");
    if (file_name.empty())
        Log::fatal("OfficialKarts", "Missing official_karts.xml");
    const XMLNode *root = file_manager->createXMLTree(file_name);
    assert(root);
    for (unsigned int i = 0; i < root->getNumNodes(); i++)
    {
        const XMLNode *node = root->getNode(i);
        std::string name;
        std::string type;
        float width = 0.0f;
        float height = 0.0f;
        float length = 0.0f;
        Vec3 gravity_shift;
        node->get("name", &name);
        node->get("type", &type);
        node->get("width", &width);
        node->get("height", &height);
        node->get("length", &length);
        node->get("gravity-shift", &gravity_shift);
        g_official_karts.emplace_back(name, type, width, height, length,
            gravity_shift);
    }
    delete root;
}   // load

// ----------------------------------------------------------------------------
std::set<std::string> getOfficialKarts()
{
    std::set<std::string> result;
    for (OfficialKart& ok : g_official_karts)
        result.insert(ok.name);
    return result;
}   // getOfficialKarts

// ----------------------------------------------------------------------------
const KartProperties* getKartByIdent(const std::string& ident,
                                     float* width, float* height,
                                     float* length, Vec3* gravity_shift)
{
    for (OfficialKart& ok : g_official_karts)
    {
        if (ok.name == ident)
        {
            for (unsigned i = 0;
                i < kart_properties_manager->getNumberOfKarts(); i++)
            {
                const KartProperties* kp =
                    kart_properties_manager->getKartById(i);
                if (kp->isAddon())
                    continue;
                if (kp->getKartType() == ok.type)
                {
                    *width = ok.width;
                    *height = ok.height;
                    *length = ok.length;
                    *gravity_shift = ok.gravity_shift;
                    return kp;
                }
            }
        }
    }
    return NULL;
}   // getKartByIdent

}   // OfficialKarts
