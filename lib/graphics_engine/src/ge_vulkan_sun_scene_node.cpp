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

    bool stable_mode = false;

    // Calculate shadow view matrix
    irr::core::vector3df eyepos = cam->getAbsolutePosition();
    irr::core::vector3df viewdir = cam->getTarget() - eyepos;
    irr::core::vector3df lightdir = -getPosition();
    viewdir = viewdir.normalize();
    lightdir = lightdir.normalize();

	irr::core::vector3df lsleft = lightdir.crossProduct(stable_mode ? irr::core::vector3df(1.0, 0.0, 0.0) : viewdir).normalize();
    irr::core::vector3df lsup = lsleft.crossProduct(lightdir).normalize();

    m_shadow_view_matrix = irr::core::matrix4();
	m_shadow_view_matrix(0, 0) = lsleft.X, m_shadow_view_matrix(0, 1) = lsup.X, m_shadow_view_matrix(0, 2) = lightdir.X;
	m_shadow_view_matrix(1, 0) = lsleft.Y, m_shadow_view_matrix(1, 1) = lsup.Y, m_shadow_view_matrix(1, 2) = lightdir.Y;
	m_shadow_view_matrix(2, 0) = lsleft.Z, m_shadow_view_matrix(2, 1) = lsup.Z, m_shadow_view_matrix(2, 2) = lightdir.Z;

    if (!stable_mode)
    {
        m_shadow_view_matrix(3, 0) = -lsleft.dotProduct(eyepos);
        m_shadow_view_matrix(3, 1) = -lsup.dotProduct(eyepos);
        m_shadow_view_matrix(3, 2) = -lightdir.dotProduct(eyepos);
    }

    // Calculate light space perspective (warp) matrix * shadow perspective matrix
    float cosgamma	= viewdir.dotProduct(lightdir);
    float singamma	= sqrtf(1.0f - cosgamma * cosgamma);

    for (int i = 0; i < GVSCC_COUNT; i++)
    {
        std::vector<irr::core::vector3df> points;
        irr::core::aabbox3df lsbody;
        irr::core::matrix4 lslightproj;

        float snear     = getSplitNear(GEVulkanShadowCameraCascade(i));
        float sfar      = getSplitFar(GEVulkanShadowCameraCascade(i));
        float vnear     = i ? getSplitFar(GEVulkanShadowCameraCascade(i - 1)) : 3.0;
        float vfar      = sfar;
        float dznear    = std::max(vnear - cam->getNearValue(), 0.0f);
        float dzfar     = std::max(cam->getFarValue() - vfar, 0.0f);

        // Frustum split by vnear and vfar
        float f1 = (snear - cam->getNearValue()) / (cam->getFarValue() - cam->getNearValue());
        float f2 = (sfar - cam->getNearValue()) / (cam->getFarValue() - cam->getNearValue());
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

        float znear = std::max(cam->getNearValue(), lsbody.MinEdge.Z);
        float zfar = std::min(cam->getFarValue(), lsbody.MaxEdge.Z);

        // lsbody as PSR (Potential Shadow Receiver) in light view space
        for (int j = 0; j < points.size(); j++)
        {
            m_shadow_view_matrix.transformVect(points[j]);
            irr::core::vector3df point = points[j];
            if (j == 0) lsbody.reset(point);
            else lsbody.addInternalPoint(point);
        }

        double bounding_sphere[4] = {};

        for (int j = 0; j < points.size(); j++)
        {
            bounding_sphere[0] += points[j].X / points.size();
            bounding_sphere[1] += points[j].Y / points.size();
            bounding_sphere[2] += points[j].Z / points.size();
        }
        irr::core::vector3df scenter(bounding_sphere[0], bounding_sphere[1], bounding_sphere[2]);

        for (int j = 0; j < points.size(); j++)
        {
            bounding_sphere[3] = std::max(bounding_sphere[3], (double) points[j].getDistanceFromSQ(scenter));
        }
        bounding_sphere[3] = std::sqrt(bounding_sphere[3]);

        lslightproj(2, 2) = 1.0 / lsbody.getExtent().Z;
        lslightproj(3, 2) = -lsbody.MinEdge.Z / lsbody.getExtent().Z;

        for (int j = 0; j < points.size(); j++)
        {
            float vec[4];
            irr::core::vector3df point = points[j];
            lslightproj.transformVect(vec, point);
            point.X = vec[0] / vec[3];
            point.Y = vec[1] / vec[3];
            point.Z = vec[2] / vec[3];
            if (j == 0) lsbody.reset(point);
            else lsbody.addInternalPoint(point);
        }

        float n	 = lsbody.MinEdge.Y;
        float f  = lsbody.MaxEdge.Y;
        float d	 = lsbody.getExtent().Y;

        float z0 = znear;
        float z1 = z0 + d * singamma;

        if (!stable_mode && singamma > 0.02f && 3.0f * (dznear / (zfar - znear)) < 2.0f)
        {
            float vz0 = std::max(0.0f, std::max(std::max(znear, cam->getNearValue() + dznear), z0));
            float vz1 = std::max(0.0f, std::min(std::min(zfar, cam->getFarValue() - dzfar), z1));

            float n = (z0 + sqrtf(vz0 * vz1)) / singamma;
            n = std::max(n, dznear / (2.0f - 3.0f * (dznear / (zfar - znear))));

            float f	= n + d;

            irr::core::matrix4 lispsmproj;
            lispsmproj(0, 0) = n;
            lispsmproj(1, 1) = (f + n) / d;
            lispsmproj(2, 2) = n;
            lispsmproj(3, 1) = -2.0 * f * n / d;
            lispsmproj(1, 3) = 1;
            lispsmproj(3, 3) = 0;

            irr::core::vector3df point = eyepos;
            m_shadow_view_matrix.transformVect(point);
            
            float vec[4];
            lslightproj.transformVect(vec, point);
            point.X = vec[0] / vec[3];
            point.Y = vec[1] / vec[3];
            point.Z = vec[2] / vec[3];

            irr::core::matrix4 correct;
            correct(3, 0) = -point.X;
            correct(3, 1) = -lsbody.MinEdge.Y + n;

            lslightproj = lispsmproj * correct * lslightproj;

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
        }

        irr::core::matrix4 fittounitcube;

        if (stable_mode)
        {
            fittounitcube(0, 0) = 1.0f / bounding_sphere[3];
            fittounitcube(1, 1) = 1.0f / bounding_sphere[3];
            fittounitcube(3, 0) = -bounding_sphere[0] / bounding_sphere[3];
            fittounitcube(3, 1) = -bounding_sphere[1] / bounding_sphere[3];

            fittounitcube(3, 0) -= fmod((double)fittounitcube(3, 0), 1. / (getShadowMapSize() / 4));
            fittounitcube(3, 1) -= fmod((double)fittounitcube(3, 1), 1. / (getShadowMapSize() / 4));
        }
        else
        {
            fittounitcube(0, 0) = 2.0f / lsbody.getExtent().X;
            fittounitcube(1, 1) = 2.0f / lsbody.getExtent().Y;
            fittounitcube(3, 0) = -(lsbody.MinEdge.X + lsbody.MaxEdge.X) / lsbody.getExtent().X;
            fittounitcube(3, 1) = -(lsbody.MinEdge.Y + lsbody.MaxEdge.Y) / lsbody.getExtent().Y;
        }
        
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

    m_shadow_ubo_data.m_light_view_matrix = m_shadow_view_matrix;

    for (int i = 0; i < GVSCC_COUNT; i++)
    {
        // View & Proj
        m_shadow_camera_ubo_data[i].m_view_matrix = m_shadow_view_matrix[i];
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
