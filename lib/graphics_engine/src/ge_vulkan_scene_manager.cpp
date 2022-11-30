#include "ge_vulkan_scene_manager.hpp"

#include "../source/Irrlicht/os.h"

#include "ge_main.hpp"
#include "ge_vulkan_animated_mesh_scene_node.hpp"
#include "ge_vulkan_camera_scene_node.hpp"
#include "ge_vulkan_command_loader.hpp"
#include "ge_vulkan_draw_call.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_fbo_texture.hpp"
#include "ge_vulkan_mesh_cache.hpp"
#include "ge_vulkan_mesh_scene_node.hpp"
#include "ge_vulkan_skybox_renderer.hpp"
#include "ge_vulkan_texture_descriptor.hpp"

#include "IBillboardSceneNode.h"
#include <sstream>

namespace GE
{
// ----------------------------------------------------------------------------
GEVulkanSceneManager::GEVulkanSceneManager(irr::video::IVideoDriver* driver,
                                           irr::io::IFileSystem* fs,
                                           irr::gui::ICursorControl* cursor_control,
                                           irr::gui::IGUIEnvironment* gui_environment)
                    : CSceneManager(driver, fs, cursor_control,
                                    new GEVulkanMeshCache(), gui_environment)
{
    // CSceneManager grabbed it
    getMeshCache()->drop();
}   // GEVulkanSceneManager

// ----------------------------------------------------------------------------
GEVulkanSceneManager::~GEVulkanSceneManager()
{
}   // ~GEVulkanSceneManager

// ----------------------------------------------------------------------------
void GEVulkanSceneManager::clear()
{
    irr::scene::CSceneManager::clear();
    static_cast<GEVulkanDriver*>(getVideoDriver())
        ->getMeshTextureDescriptor()->clear();
    GEVulkanSkyBoxRenderer::destroy();
}   // clear

// ----------------------------------------------------------------------------
irr::scene::ICameraSceneNode* GEVulkanSceneManager::addCameraSceneNode(
                                                irr::scene::ISceneNode* parent,
                                          const irr::core::vector3df& position,
                                            const irr::core::vector3df& lookat,
                                                 irr::s32 id, bool make_active)
{
    if (!parent)
        parent = this;

    irr::scene::ICameraSceneNode* node = new GEVulkanCameraSceneNode(parent,
        this, id, position, lookat);

    if (make_active)
        setActiveCamera(node);
    node->drop();

    return node;
}   // addCameraSceneNode

// ----------------------------------------------------------------------------
irr::scene::IAnimatedMeshSceneNode* GEVulkanSceneManager::addAnimatedMeshSceneNode(
    irr::scene::IAnimatedMesh* mesh, irr::scene::ISceneNode* parent,
    irr::s32 id,
    const irr::core::vector3df& position,
    const irr::core::vector3df& rotation,
    const irr::core::vector3df& scale,
    bool alsoAddIfMeshPointerZero)
{
    if (!alsoAddIfMeshPointerZero && (!mesh ||
        mesh->getMeshType() != irr::scene::EAMT_SPM))
        return NULL;

    if (!parent)
        parent = this;

    irr::scene::IAnimatedMeshSceneNode* node =
        new GEVulkanAnimatedMeshSceneNode(mesh, parent, this, id, position,
        rotation, scale);
    node->drop();
    node->setMesh(mesh);
    return node;
}   // addAnimatedMeshSceneNode

// ----------------------------------------------------------------------------
irr::scene::IMeshSceneNode* GEVulkanSceneManager::addMeshSceneNode(
    irr::scene::IMesh* mesh,
    irr::scene::ISceneNode* parent, irr::s32 id,
    const irr::core::vector3df& position,
    const irr::core::vector3df& rotation,
    const irr::core::vector3df& scale,
    bool alsoAddIfMeshPointerZero)
{
    if (!alsoAddIfMeshPointerZero && !mesh)
        return NULL;

    bool convert_irrlicht_mesh = false;
    if (mesh)
    {
        for (unsigned i = 0; i < mesh->getMeshBufferCount(); i++)
        {
            irr::scene::IMeshBuffer* b = mesh->getMeshBuffer(i);
            if (b->getVertexType() != irr::video::EVT_SKINNED_MESH)
            {
                if (!getGEConfig()->m_convert_irrlicht_mesh)
                {
                    return irr::scene::CSceneManager::addMeshSceneNode(
                        mesh, parent, id, position, rotation, scale,
                        alsoAddIfMeshPointerZero);
                }
                else
                {
                    convert_irrlicht_mesh = true;
                    break;
                }
            }
        }
    }

    if (!parent)
        parent = this;

    if (convert_irrlicht_mesh)
    {
        irr::scene::IAnimatedMesh* spm = convertIrrlichtMeshToSPM(mesh);
        std::stringstream oss;
        oss << (uint64_t)spm;
        getMeshCache()->addMesh(oss.str().c_str(), spm);
        mesh = spm;
    }

    GEVulkanMeshSceneNode* vulkan_node =
        new GEVulkanMeshSceneNode(mesh, parent, this, id, position, rotation,
        scale);
    irr::scene::IMeshSceneNode* node = vulkan_node;
    node->drop();

    if (convert_irrlicht_mesh)
    {
        vulkan_node->setRemoveFromMeshCache(true);
        mesh->drop();
    }
    return node;
}   // addMeshSceneNode

// ----------------------------------------------------------------------------
void GEVulkanSceneManager::drawAllInternal()
{
    static_cast<GEVulkanMeshCache*>(getMeshCache())->updateCache();
    GEVulkanCameraSceneNode* cam = NULL;
    if (getActiveCamera())
    {
        cam = static_cast<
            GEVulkanCameraSceneNode*>(getActiveCamera());
    }
    OnAnimate(os::Timer::getTime());
    if (cam)
    {
        cam->render();
        auto it = m_draw_calls.find(cam);
        if (it == m_draw_calls.end())
            return;

        it->second->prepare(cam);
        OnRegisterSceneNode();
        it->second->generate(static_cast<GEVulkanDriver*>(getVideoDriver()));
    }
}   // drawAllInternal

// ----------------------------------------------------------------------------
void GEVulkanSceneManager::drawAll(irr::u32 flags)
{
    drawAllInternal();
    GEVulkanDriver* vk = static_cast<GEVulkanDriver*>(getVideoDriver());
    GEVulkanFBOTexture* rtt = vk->getSeparateRTTTexture();
    if (!rtt)
        return;

    std::array<VkClearValue, 2> clear_values = {};
    video::SColorf cf(vk->getRTTClearColor());
    clear_values[0].color =
    {
        cf.getRed(), cf.getGreen(), cf.getBlue(), cf.getAlpha()
    };
    clear_values[1].depthStencil = {1.0f, 0};

    VkCommandBuffer cmd = GEVulkanCommandLoader::beginSingleTimeCommands();

    GEVulkanCameraSceneNode* cam = static_cast<
        GEVulkanCameraSceneNode*>(getActiveCamera());
    std::unique_ptr<GEVulkanDrawCall>& dc = m_draw_calls.at(cam);
    dc->uploadDynamicData(vk, cam, cmd);

    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = rtt->getRTTRenderPass();
    render_pass_info.framebuffer = rtt->getRTTFramebuffer();
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent =
        { rtt->getSize().Width, rtt->getSize().Height };
    render_pass_info.clearValueCount = (uint32_t)(clear_values.size());
    render_pass_info.pClearValues = &clear_values[0];
    vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    cam->setViewPort(
        core::recti(0, 0, rtt->getSize().Width, rtt->getSize().Height));
    dc->render(vk, cam, cmd);
    vk->addRTTPolyCount(dc->getPolyCount());
    dc->reset();

    vkCmdEndRenderPass(cmd);

    GEVulkanCommandLoader::endSingleTimeCommands(cmd);
    vk->handleDeletedTextures();
}   // drawAll

// ----------------------------------------------------------------------------
irr::u32 GEVulkanSceneManager::registerNodeForRendering(
    irr::scene::ISceneNode* node,
    irr::scene::E_SCENE_NODE_RENDER_PASS pass)
{
    if (!getActiveCamera())
        return 0;

    GEVulkanCameraSceneNode* cam = static_cast<
        GEVulkanCameraSceneNode*>(getActiveCamera());

    if (node->getType() == irr::scene::ESNT_SKY_BOX)
    {
        GEVulkanSkyBoxRenderer::addSkyBox(cam, node);
        return 1;
    }

    if (node->getType() == irr::scene::ESNT_BILLBOARD ||
        node->getType() == irr::scene::ESNT_PARTICLE_SYSTEM)
    {
        m_draw_calls.at(cam)->addBillboardNode(node, node->getType());
        return 1;
    }

    if ((node->getType() == irr::scene::ESNT_ANIMATED_MESH &&
        pass != irr::scene::ESNRP_SOLID) ||
        (node->getType() == irr::scene::ESNT_MESH &&
        pass != irr::scene::ESNRP_SOLID))
        return 0;

    m_draw_calls.at(cam)->addNode(node);
    return 1;
}   // registerNodeForRendering

// ----------------------------------------------------------------------------
void GEVulkanSceneManager::addDrawCall(GEVulkanCameraSceneNode* cam)
{
    GEVulkanDriver* gevk = static_cast<GEVulkanDriver*>(getVideoDriver());
    m_draw_calls[cam] = gevk->getDrawCallFromCache();
}   // addDrawCall

// ----------------------------------------------------------------------------
void GEVulkanSceneManager::removeDrawCall(GEVulkanCameraSceneNode* cam)
{
    if (m_draw_calls.find(cam) == m_draw_calls.end())
        return;
    GEVulkanDriver* gevk = static_cast<GEVulkanDriver*>(getVideoDriver());
    auto& dc = m_draw_calls.at(cam);
    gevk->addDrawCallToCache(dc);
    m_draw_calls.erase(cam);
}   // removeDrawCall

}
