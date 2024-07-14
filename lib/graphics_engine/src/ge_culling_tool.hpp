#ifndef HEADER_GE_CULLING_TOOL_HPP
#define HEADER_GE_CULLING_TOOL_HPP

#include "aabbox3d.h"
#include "quaternion.h"
#include "matrix4.h"
#include "IVideoDriver.h"

#include <unordered_map>

namespace irr
{
    namespace scene { class ISceneNode; }
}

template<>
struct std::hash<irr::core::aabbox3df>
{
    std::size_t operator()(const irr::core::aabbox3df& box) const noexcept
    {
        std::size_t ret = std::hash<float>{}(box.MinEdge.X);
        ret = (ret * 0x100000001b3ull) ^ std::hash<float>{}(box.MinEdge.Y);
        ret = (ret * 0x100000001b3ull) ^ std::hash<float>{}(box.MinEdge.Z);
        ret = (ret * 0x100000001b3ull) ^ std::hash<float>{}(box.MaxEdge.X);
        ret = (ret * 0x100000001b3ull) ^ std::hash<float>{}(box.MaxEdge.Y);
        ret = (ret * 0x100000001b3ull) ^ std::hash<float>{}(box.MaxEdge.Z);
        return ret;
    }
};

namespace GE
{
class GESPMBuffer;
class GEVulkanCameraSceneNode;

class GECullingTool
{
private:
    irr::video::IVideoDriver *m_driver;

    irr::core::quaternion m_frustum[6];

    irr::core::aabbox3df m_cam_bbox;

    std::unordered_map<irr::core::aabbox3df, bool> m_results;
public:
    // ------------------------------------------------------------------------
    void init(irr::video::IVideoDriver *driver, GEVulkanCameraSceneNode* cam);
    // ------------------------------------------------------------------------
    bool isCulled(irr::core::aabbox3df& bb);
    // ------------------------------------------------------------------------
    bool isCulled(GESPMBuffer* buffer, irr::scene::ISceneNode* node);
};   // GECullingTool

}

#endif
