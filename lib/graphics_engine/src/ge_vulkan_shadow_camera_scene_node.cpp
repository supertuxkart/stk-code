#include "ge_vulkan_shadow_camera_scene_node.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_scene_manager.hpp"

namespace GE
{
// ------------------------------------------------------------------------
GEVulkanShadowCameraSceneNode::GEVulkanShadowCameraSceneNode(
    irr::scene::ICameraSceneNode* parent,
    irr::scene::ISceneManager* mgr, irr::s32 id,
    const irr::core::vector3df& sun) : irr::scene::ISceneNode(parent, mgr, id, sun)
{
    static_cast<GEVulkanSceneManager*>(SceneManager)->addShadowDrawCall(this);
    m_camera = parent;
}
// ------------------------------------------------------------------------
GEVulkanShadowCameraSceneNode::~GEVulkanShadowCameraSceneNode()
{
    static_cast<GEVulkanSceneManager*>(SceneManager)->removeShadowDrawCall(this);
}
// ------------------------------------------------------------------------
void GEVulkanShadowCameraSceneNode::render()
{
    GEVulkanDriver* vk = getVKDriver();

    // Calculate shadow view matrix

    irr::core::vector3df eyepos = m_camera->getAbsolutePosition();
    irr::core::vector3df viewdir = m_camera->getTarget() - eyepos;
    irr::core::vector3df lightdir = -getPosition();
    viewdir = viewdir.normalize();
    lightdir = lightdir.normalize();

	irr::core::vector3df lsleft = lightdir.crossProduct(viewdir).normalize();
    irr::core::vector3df lsup = lsleft.crossProduct(lightdir).normalize();

    m_view_matrix = irr::core::matrix4();
	m_view_matrix(0, 0) = lsleft.X, m_view_matrix(0, 1) = lsup.X, m_view_matrix(0, 2) = lightdir.X;
	m_view_matrix(1, 0) = lsleft.Y, m_view_matrix(1, 1) = lsup.Y, m_view_matrix(1, 2) = lightdir.Y;
	m_view_matrix(2, 0) = lsleft.Z, m_view_matrix(2, 1) = lsup.Z, m_view_matrix(2, 2) = lightdir.Z;

	m_view_matrix(3, 0) = -lsleft.dotProduct(eyepos);
	m_view_matrix(3, 1) = -lsup.dotProduct(eyepos);
	m_view_matrix(3, 2) = -lightdir.dotProduct(eyepos);

    // Calculate light space perspective (warp) matrix * shadow perspective matrix

    float warp_strength[GVSCC_COUNT];
    float cosgamma	= viewdir.dotProduct(lightdir);
    float singamma	= sqrtf(1.0f - cosgamma * cosgamma);

    for (int i = 0; i < GVSCC_COUNT; i++)
    {
        std::vector<irr::core::vector3df> points;
        irr::core::aabbox3df lsbody;
        irr::core::matrix4 lslightproj;

        float vnear     = getSplitNear(GEVulkanShadowCameraCascade(i));
        float vfar      = getSplitFar(GEVulkanShadowCameraCascade(i));
        float dznear    = std::max(vnear - m_camera->getNearValue(), 0.0f);
        float dzfar     = std::max(m_camera->getFarValue() - vfar, 0.0f);

        // Frustum split by vnear and vfar
        float f1 = (vnear - m_camera->getNearValue()) / (m_camera->getFarValue() - m_camera->getNearValue());
        float f2 = (vfar - m_camera->getNearValue()) / (m_camera->getFarValue() - m_camera->getNearValue());
        const irr::scene::SViewFrustum *frust = m_camera->getViewFrustum();
        points.push_back(frust->getFarLeftDown() * f1 + frust->getNearLeftDown() * (1.0 - f1));
        points.push_back(frust->getFarLeftDown() * f2 + frust->getNearLeftDown() * (1.0 - f2));
        points.push_back(frust->getFarRightDown() * f1 + frust->getNearRightDown() * (1.0 - f1));
        points.push_back(frust->getFarRightDown() * f2 + frust->getNearRightDown() * (1.0 - f2));
        points.push_back(frust->getFarLeftUp() * f1 + frust->getNearLeftUp() * (1.0 - f1));
        points.push_back(frust->getFarLeftUp() * f2 + frust->getNearLeftUp() * (1.0 - f2));
        points.push_back(frust->getFarRightUp() * f1 + frust->getNearRightUp() * (1.0 - f1));
        points.push_back(frust->getFarRightUp() * f2 + frust->getNearRightUp() * (1.0 - f2));

        // Reset Bounding Box
        for (int j = 0; j < points.size(); j++)
        {
            if (i == 0 && j == 0) m_bounding_box.reset(points[j]);
            else m_bounding_box.addInternalPoint(points[j]);
        }

        // lsbody as PSR (Potential Shadow Receiver) in camera view space
        for (int j = 0; j < points.size(); j++)
        {
            irr::core::vector3df point = points[j];
            m_camera->getViewMatrix().transformVect(point);
            if (j == 0) lsbody.reset(point);
            else lsbody.addInternalPoint(point);
        }

        float znear = std::max(m_camera->getNearValue(), lsbody.MinEdge.Y);
        float zfar = std::min(m_camera->getFarValue(), lsbody.MaxEdge.Y);

        // lsbody as PSR (Potential Shadow Receiver) in light view space
        for (int j = 0; j < points.size(); j++)
        {
            m_view_matrix.transformVect(points[j]);
            if (j == 0) lsbody.reset(points[j]);
            else lsbody.addInternalPoint(points[j]);
            
        }

        float n	 = lsbody.MinEdge.Y;
        float f  = lsbody.MaxEdge.Y;
        float d	 = lsbody.getExtent().Y;

        float z0 = znear;
        float z1 = z0 + d * singamma;
        warp_strength[i] = 1.0f;

        if (singamma > 0.02f && 3.0f * (dznear / (zfar - znear)) < 2.0f)
        {
            float vz0 = std::max(0.0f, std::max(std::max(znear, m_camera->getNearValue() + dznear), z0));
            float vz1 = std::max(0.0f, std::min(std::min(zfar, m_camera->getFarValue() - dzfar), z1));

            float n = (z0 + sqrtf(vz0 * vz1)) / singamma;
            n = std::max(n, dznear / (2.0f - 3.0f * (dznear / (zfar - znear))));

            float f	= n + d;
            warp_strength[i] = n / f;

            lslightproj(1, 1) = (f + n) / d;
            lslightproj(3, 1) = -2.0 * f * n / d;
            lslightproj(1, 3) = 1;
            lslightproj(3, 3) = 0;

            irr::core::vector3df point = eyepos;
            m_view_matrix.transformVect(point);

            irr::core::matrix4 correct;
            correct(3, 0) = -point.X;
            correct(3, 1) = -lsbody.MinEdge.Y + n;

            lslightproj = lslightproj * correct;
        }
        
        // lsbody as PSR (Potential Shadow Receiver) in light clip space
        for (int j = 0; j < points.size(); j++)
        {
            irr::core::vector3df point = points[j];
            float vec[4];
            lslightproj.transformVect(vec, point);
            point.X = vec[0] / vec[3];
            point.Y = vec[1] / vec[3];
            point.Z = vec[2] / vec[3];
            if (j == 0) lsbody.reset(point);
            else lsbody.addInternalPoint(point);
        }

        irr::core::matrix4 fittounitcube;
        fittounitcube.buildProjectionMatrixOrthoLH(lsbody.MinEdge.X,
                                                   lsbody.MaxEdge.X,
                                                   lsbody.MaxEdge.Y,
                                                   lsbody.MinEdge.Y,
                                                   lsbody.MinEdge.Z,
                                                   lsbody.MaxEdge.Z);
        
        lslightproj = fittounitcube * lslightproj;

        m_projection_matrices[i] = lslightproj;
    }

    // Build shadow camera UBOs

    // https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
    // Vulkan clip space has inverted Y and half Z
    irr::core::matrix4 clip;
    clip[10] = 0.5f;
    clip[14] = 0.5f;

    for (int i = 0; i < GVSCC_COUNT; i++)
    {
        // View & Proj
        m_camera_ubo_data[i].m_view_matrix = m_view_matrix;
        m_camera_ubo_data[i].m_projection_matrix = clip * m_projection_matrices[i];
        // Inverse View & Inverse Proj
        m_camera_ubo_data[i].m_view_matrix.getInverse(m_camera_ubo_data[i].m_inverse_view_matrix);
        m_camera_ubo_data[i].m_projection_matrix.getInverse(m_camera_ubo_data[i].m_inverse_projection_matrix);
        // ProjView
        m_camera_ubo_data[i].m_projection_view_matrix = 
            m_camera_ubo_data[i].m_projection_matrix * m_camera_ubo_data[i].m_view_matrix;
        // Inverse ProjView
        m_camera_ubo_data[i].m_projection_view_matrix.getInverse(
            m_camera_ubo_data[i].m_inverse_projection_view_matrix);

        // Build shadow UBO
        m_shadow_ubo_data.m_light_projection_view_matrix[i] = m_camera_ubo_data[i].m_projection_view_matrix;
        m_shadow_ubo_data.m_warp_strength[i] = warp_strength[i];
    }
}

}
