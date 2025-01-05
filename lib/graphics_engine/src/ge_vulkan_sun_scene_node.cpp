#include "ge_vulkan_sun_scene_node.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_scene_manager.hpp"

namespace GE
{
// ----------------------------------------------------------------------------
GEVulkanSunSceneNode::GEVulkanSunSceneNode(irr::scene::ISceneNode* parent,
                irr::scene::ISceneManager* mgr, irr::s32 id,
          const irr::core::vector3df& position, irr::video::SColorf color, 
                irr::f32 radius)
                    : CLightSceneNode(parent, mgr, id, position, color, radius)
{
    CLightSceneNode::setLightType(irr::video::ELT_DIRECTIONAL);
}   // GEVulkanCameraSceneNode

// ------------------------------------------------------------------------
void GEVulkanSunSceneNode::render()
{
    GEVulkanDriver* vk = getVKDriver();
    irr::scene::ICameraSceneNode* cam = getSceneManager()->getActiveCamera();

    vk->addDynamicLight(getLightData());

    if (!cam || !getCastShadow())
    {
        return;
    }

    // Calculate shadow view matrix
    irr::core::vector3df eyepos = cam->getAbsolutePosition();
    irr::core::vector3df viewdir = cam->getTarget() - eyepos;
    irr::core::vector3df lightdir = -getPosition();
    viewdir = viewdir.normalize();
    lightdir = lightdir.normalize();

	irr::core::vector3df lsleft = lightdir.crossProduct(viewdir).normalize();
    irr::core::vector3df lsup = lsleft.crossProduct(lightdir).normalize();

    m_shadow_view_matrix = irr::core::matrix4();
	m_shadow_view_matrix(0, 0) = lsleft.X, m_shadow_view_matrix(0, 1) = lsup.X, m_shadow_view_matrix(0, 2) = lightdir.X;
	m_shadow_view_matrix(1, 0) = lsleft.Y, m_shadow_view_matrix(1, 1) = lsup.Y, m_shadow_view_matrix(1, 2) = lightdir.Y;
	m_shadow_view_matrix(2, 0) = lsleft.Z, m_shadow_view_matrix(2, 1) = lsup.Z, m_shadow_view_matrix(2, 2) = lightdir.Z;

	m_shadow_view_matrix(3, 0) = -lsleft.dotProduct(eyepos);
	m_shadow_view_matrix(3, 1) = -lsup.dotProduct(eyepos);
	m_shadow_view_matrix(3, 2) = -lightdir.dotProduct(eyepos);

    // Calculate light space perspective (warp) matrix * shadow perspective matrix
    float cosgamma	= viewdir.dotProduct(lightdir);
    float singamma	= sqrtf(1.0f - cosgamma * cosgamma);

    for (int i = 0; i < GVSCC_COUNT; i++)
    {
        std::vector<irr::core::vector3df> points;
        irr::core::aabbox3df lsbody;
        irr::core::matrix4 lslightproj;

        float vnear     = getSplitNear(GEVulkanShadowCameraCascade(i));
        float vfar      = getSplitFar(GEVulkanShadowCameraCascade(i));
        float dznear    = std::max(vnear - cam->getNearValue(), 0.0f);
        float dzfar     = std::max(cam->getFarValue() - vfar, 0.0f);

        // Frustum split by vnear and vfar
        float f1 = (vnear - cam->getNearValue()) / (cam->getFarValue() - cam->getNearValue());
        float f2 = (vfar - cam->getNearValue()) / (cam->getFarValue() - cam->getNearValue());
        const irr::scene::SViewFrustum *frust = cam->getViewFrustum();
        points.push_back(frust->getFarLeftDown() * f1 + frust->getNearLeftDown() * (1.0 - f1));
        points.push_back(frust->getFarLeftDown() * f2 + frust->getNearLeftDown() * (1.0 - f2));
        points.push_back(frust->getFarRightDown() * f1 + frust->getNearRightDown() * (1.0 - f1));
        points.push_back(frust->getFarRightDown() * f2 + frust->getNearRightDown() * (1.0 - f2));
        points.push_back(frust->getFarLeftUp() * f1 + frust->getNearLeftUp() * (1.0 - f1));
        points.push_back(frust->getFarLeftUp() * f2 + frust->getNearLeftUp() * (1.0 - f2));
        points.push_back(frust->getFarRightUp() * f1 + frust->getNearRightUp() * (1.0 - f1));
        points.push_back(frust->getFarRightUp() * f2 + frust->getNearRightUp() * (1.0 - f2));

        // lsbody as PSR (Potential Shadow Receiver) in camera view space
        for (int j = 0; j < points.size(); j++)
        {
            irr::core::vector3df point = points[j];
            cam->getViewMatrix().transformVect(point);
            if (j == 0) lsbody.reset(point);
            else lsbody.addInternalPoint(point);
        }

        float znear = std::max(cam->getNearValue(), lsbody.MinEdge.Y);
        float zfar = std::min(cam->getFarValue(), lsbody.MaxEdge.Y);

        // lsbody as PSR (Potential Shadow Receiver) in light view space
        for (int j = 0; j < points.size(); j++)
        {
            m_shadow_view_matrix.transformVect(points[j]);
            if (j == 0) lsbody.reset(points[j]);
            else lsbody.addInternalPoint(points[j]);
            
        }

        float n	 = lsbody.MinEdge.Y;
        float f  = lsbody.MaxEdge.Y;
        float d	 = lsbody.getExtent().Y;

        float z0 = znear;
        float z1 = z0 + d * singamma;

        if (singamma > 0.02f && 3.0f * (dznear / (zfar - znear)) < 2.0f)
        {
            float vz0 = std::max(0.0f, std::max(std::max(znear, cam->getNearValue() + dznear), z0));
            float vz1 = std::max(0.0f, std::min(std::min(zfar, cam->getFarValue() - dzfar), z1));

            float n = (z0 + sqrtf(vz0 * vz1)) / singamma;
            n = std::max(n, dznear / (2.0f - 3.0f * (dznear / (zfar - znear))));

            float f	= n + d;

            lslightproj(1, 1) = (f + n) / d;
            lslightproj(3, 1) = -2.0 * f * n / d;
            lslightproj(1, 3) = 1;
            lslightproj(3, 3) = 0;

            irr::core::vector3df point = eyepos;
            m_shadow_view_matrix.transformVect(point);

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

        m_shadow_projection_matrices[i] = lslightproj;
    }

    // Build shadow camera UBOs

    // https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
    // Vulkan clip space has inverted Y and half Z
    irr::core::matrix4 clip;
    clip[10] = 0.5f;
    clip[14] = 0.5f;

    irr::core::matrix4 inv_view;
    m_shadow_view_matrix.getInverse(inv_view);

    for (int i = 0; i < GVSCC_COUNT; i++)
    {
        // View & Proj
        m_shadow_camera_ubo_data[i].m_view_matrix = m_shadow_view_matrix;
        m_shadow_camera_ubo_data[i].m_projection_matrix = clip * m_shadow_projection_matrices[i];
        // Inverse View & Inverse Proj
        m_shadow_camera_ubo_data[i].m_inverse_view_matrix = inv_view;
        m_shadow_camera_ubo_data[i].m_projection_matrix.getInverse(m_shadow_camera_ubo_data[i].m_inverse_projection_matrix);
        // ProjView
        m_shadow_camera_ubo_data[i].m_projection_view_matrix = 
            m_shadow_camera_ubo_data[i].m_projection_matrix * m_shadow_view_matrix;
        // Inverse ProjView
        m_shadow_camera_ubo_data[i].m_inverse_projection_view_matrix =
            inv_view * m_shadow_camera_ubo_data[i].m_inverse_projection_matrix;

        // Build shadow UBO
        m_shadow_ubo_data.m_light_projection_view_matrix[i] = m_shadow_camera_ubo_data[i].m_projection_view_matrix;
    }
}

}
