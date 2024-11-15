#include "ge_vulkan_camera_scene_node.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_driver.hpp"
#include "ge_vulkan_scene_manager.hpp"

namespace GE
{
// ----------------------------------------------------------------------------
GEVulkanCameraSceneNode::GEVulkanCameraSceneNode(irr::scene::ISceneNode* parent,
                                                 irr::scene::ISceneManager* mgr,
                                                 irr::s32 id,
                                           const irr::core::vector3df& position,
                                             const irr::core::vector3df& lookat)
                       : CCameraSceneNode(parent, mgr, id, position, lookat)
{
    static_cast<GEVulkanSceneManager*>(SceneManager)->addDrawCall(this);
}   // GEVulkanCameraSceneNode

// ----------------------------------------------------------------------------
GEVulkanCameraSceneNode::~GEVulkanCameraSceneNode()
{
    static_cast<GEVulkanSceneManager*>(SceneManager)->removeDrawCall(this);
}   // ~GEVulkanCameraSceneNode

// ----------------------------------------------------------------------------
void GEVulkanCameraSceneNode::render()
{
    irr::scene::CCameraSceneNode::render();

    m_ubo_data.m_view_matrix = ViewArea.getTransform(irr::video::ETS_VIEW);
    m_ubo_data.m_projection_matrix = ViewArea.getTransform(irr::video::ETS_PROJECTION);
    // https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
    // Vulkan clip space has inverted Y and half Z
    irr::core::matrix4 clip;
    clip[5] = -1.0f;
    clip[10] = 0.5f;
    clip[14] = 0.5f;
    m_ubo_data.m_projection_matrix = clip * m_ubo_data.m_projection_matrix;
    GEVulkanDriver* vk = getVKDriver();
    if (!vk->getRTTTexture())
    {
        m_ubo_data.m_projection_matrix = vk->getPreRotationMatrix() *
            m_ubo_data.m_projection_matrix;
    }

    irr::core::matrix4 mat;
    ViewArea.getTransform(irr::video::ETS_VIEW).getInverse(mat);
    m_ubo_data.m_inverse_view_matrix = mat;

    m_ubo_data.m_projection_matrix.getInverse(mat);
    m_ubo_data.m_inverse_projection_matrix = mat;

    mat = m_ubo_data.m_projection_matrix * m_ubo_data.m_view_matrix;

    m_ubo_data.m_projection_view_matrix = mat;

    m_ubo_data.m_projection_view_matrix.getInverse(
        m_ubo_data.m_inverse_projection_view_matrix);
    
    clip[5] = 1.0f;

    irr::core::vector3df viewdir = getTarget() - getAbsolutePosition();
    viewdir = viewdir.normalize();
    irr::core::vector3df lightdir = -vk->getGlobalLightUBO(irr::core::matrix4())->m_sun_direction;
    float cosgamma	= viewdir.dotProduct(lightdir);
	float singamma	= sqrtf(1.0f - cosgamma * cosgamma);

    float factor = std::max(singamma - sinf(getFOV() / 2), 0.0f) / (1.0f - sinf(getFOV() / 2));
    float far = 20.0f + 100.0f * factor;

	m_ubo_data.m_light_view_matrix = clip * getLightVM(5.0f, 150.0f);
    
    m_ubo_data.m_viewport.UpperLeftCorner.X = m_viewport.UpperLeftCorner.X;
    m_ubo_data.m_viewport.UpperLeftCorner.Y = m_viewport.UpperLeftCorner.Y;
    m_ubo_data.m_viewport.LowerRightCorner.X = m_viewport.getWidth();
    m_ubo_data.m_viewport.LowerRightCorner.Y = m_viewport.getHeight();

    m_ubo_data_shadow = m_ubo_data;
    m_ubo_data_shadow.m_projection_view_matrix = m_ubo_data.m_light_view_matrix;
}   // render

// ----------------------------------------------------------------------------
irr::core::matrix4 GEVulkanCameraSceneNode::getPVM() const
{
    // Use the original unedited matrix for culling
    return ViewArea.getTransform(irr::video::ETS_PROJECTION) *
        ViewArea.getTransform(irr::video::ETS_VIEW);
}   // getPVM

// ----------------------------------------------------------------------------
irr::core::matrix4 GEVulkanCameraSceneNode::getLightVM(float vnear, float vfar) const
{
    GEVulkanDriver* vk = getVKDriver();

    // calculate light space perspective frustum

    irr::core::vector3df eyepos = getAbsolutePosition();
    irr::core::vector3df viewdir = getTarget() - eyepos;
    viewdir = viewdir.normalize();
    irr::core::vector3df lightdir = -vk->getGlobalLightUBO(irr::core::matrix4())->m_sun_direction;
	irr::core::matrix4 lslightview = irr::core::matrix4();
	irr::core::matrix4 lispsmproj = irr::core::matrix4();
	irr::core::vector3df lsleft = lightdir.crossProduct(viewdir).normalize();
    irr::core::vector3df lsup = lsleft.crossProduct(lightdir).normalize();

	lslightview(0, 0) = lsleft.X; lslightview(0, 1) = lsup.X; lslightview(0, 2) = lightdir.X;
	lslightview(1, 0) = lsleft.Y; lslightview(1, 1) = lsup.Y; lslightview(1, 2) = lightdir.Y;
	lslightview(2, 0) = lsleft.Z; lslightview(2, 1) = lsup.Z; lslightview(2, 2) = lightdir.Z;

	lslightview(3, 0) = -lsleft.dotProduct(eyepos);
	lslightview(3, 1) = -lsup.dotProduct(eyepos);
	lslightview(3, 2) = -lightdir.dotProduct(eyepos);
    
    std::vector<irr::core::vector3df> points;
    irr::scene::ISceneNode *scene = getSceneManager()->getRootSceneNode();

    float scale = (vfar - getNearValue()) / (getFarValue() - getNearValue());
    points.push_back(scale * getViewFrustum()->getFarLeftDown() + (1.0 - scale) * getViewFrustum()->getNearLeftDown());
    points.push_back(scale * getViewFrustum()->getFarLeftUp() + (1.0 - scale) * getViewFrustum()->getNearLeftUp());
    points.push_back(scale * getViewFrustum()->getFarRightDown() + (1.0 - scale) * getViewFrustum()->getNearRightDown());
    points.push_back(scale * getViewFrustum()->getFarRightUp() + (1.0 - scale) * getViewFrustum()->getNearRightUp());
    points.push_back(getViewFrustum()->getNearLeftDown());
    points.push_back(getViewFrustum()->getNearLeftUp());
    points.push_back(getViewFrustum()->getNearRightDown());
    points.push_back(getViewFrustum()->getNearRightUp());
    points.push_back(getViewFrustum()->getNearLeftDown() + 100 * lightdir);
    points.push_back(getViewFrustum()->getNearRightDown() + 100 * lightdir);

    if (scene)
    {
        irr::core::aabbox3df scenebox = irr::core::aabbox3df();
        scenebox.addInternalPoint(irr::core::vector3df(-1000, -1000, -1000));
        scenebox.addInternalPoint(irr::core::vector3df(1000, 1000, 1000));

        std::vector<irr::core::vector3df> tmppoints;
        tmppoints.reserve(8);

        for (size_t i = 0; i < points.size(); ++i)
        {
            irr::core::vector3df invraydir = lightdir;
            invraydir.invert();

            irr::core::vector3df tnear	= invraydir * (scenebox.MinEdge - points[i]);
            irr::core::vector3df tfar	= invraydir * (scenebox.MaxEdge - points[i]);
            if (tnear.X > tfar.X) std::swap(tnear.X, tfar.X);
            if (tnear.Y > tfar.Y) std::swap(tnear.Y, tfar.Y);
            if (tnear.Z > tfar.Z) std::swap(tnear.Z, tfar.Z);

            float u = std::max(tnear.X, std::max(tnear.Y, tnear.Z));
            float v = std::min(tnear.X, std::min(tnear.Y, tnear.Z));

            if (v >= 0 && v >= u) {
                if (u >= 1e-3) tmppoints.push_back(points[i] + u * lightdir);
                if (v >= 1e-3) tmppoints.push_back(points[i] + v * lightdir);
            }
        }

        points.insert(points.end(), tmppoints.begin(), tmppoints.end());
    }

    irr::core::aabbox3df lsbody = irr::core::aabbox3df();

    float cosgamma	= viewdir.dotProduct(lightdir);
	float singamma	= sqrtf(1.0f - cosgamma * cosgamma);
    float dznear    = std::max(vnear - getNearValue(), 0.0f);
    float dzfar     = std::max(getFarValue() - vfar, 0.0f);

    for (int i = 0; i < points.size(); i++)
    {
        irr::core::vector3df point = points[i];
        float vec[4];
        ViewArea.getTransform(irr::video::ETS_VIEW).transformVect(vec, point);
        point.X = vec[0] / vec[3];
        point.Y = vec[1] / vec[3];
        point.Z = vec[2] / vec[3];
        if (i == 0) lsbody.reset(point);
        else lsbody.addInternalPoint(point);
    }

    float znear = std::max(getNearValue(), lsbody.MinEdge.Y);
    float zfar = std::min(getFarValue(), lsbody.MaxEdge.Y);

    for (int i = 0; i < points.size(); i++)
    {
        irr::core::vector3df point = points[i];
        float vec[4];
        lslightview.transformVect(vec, point);
        point.X = vec[0] / vec[3];
        point.Y = vec[1] / vec[3];
        point.Z = vec[2] / vec[3];
        if (i == 0) lsbody.reset(point);
        else lsbody.addInternalPoint(point);
    }

	float n	 = lsbody.MinEdge.Y;
    float f  = lsbody.MaxEdge.Y;
	float d	 = lsbody.getExtent().Y;

    float z0 = znear;
    float z1 = z0 + d * singamma;

    if (singamma > 0.02f && 3.0f * (dznear / (zfar - znear)) < 2.0f)
    {
        float vz0 = std::max(0.0f, std::max(std::max(znear, getNearValue() + dznear), z0));
        float vz1 = std::max(0.0f, std::min(std::min(zfar, getFarValue() - dzfar), z1));

        float n = (z0 + sqrtf(vz0 * vz1)) / singamma;
        n = std::max(n, dznear / (2.0f - 3.0f * (dznear / (zfar - znear))));

	    float f	= n + d;

        lispsmproj(1, 1) = (f + n) / d;
        lispsmproj(3, 1) = -2.0 * f * n / d;
        lispsmproj(1, 3) = 1;
        lispsmproj(3, 3) = 0;

        irr::core::vector3df point = eyepos;
        float vec[4];
        lslightview.transformVect(vec, point);
        point.X = vec[0] / vec[3];
        point.Y = vec[1] / vec[3];
        point.Z = vec[2] / vec[3];

        irr::core::matrix4 correct;
        correct(3, 0) = -point.X;
        correct(3, 1) = -lsbody.MinEdge.Y + n;

        lispsmproj = lispsmproj * correct;
    }

    irr::core::matrix4 lslightviewproj = lslightview;

    lslightviewproj = lispsmproj * lslightview;
    
    for (int i = 0; i < points.size(); i++)
    {
        irr::core::vector3df point = points[i];
        float vec[4];
        lslightviewproj.transformVect(vec, point);
        point.X = vec[0] / vec[3];
        point.Y = vec[1] / vec[3];
        point.Z = vec[2] / vec[3];
        if (i == 0) lsbody.reset(point);
        else lsbody.addInternalPoint(point);
    } 

    irr::core::matrix4 fittounitcube;
    fittounitcube.buildProjectionMatrixOrthoLH(lsbody.MinEdge.X,
                                               lsbody.MaxEdge.X,
                                               lsbody.MaxEdge.Y,
                                               lsbody.MinEdge.Y,
                                               lsbody.MinEdge.Z,
                                               lsbody.MaxEdge.Z);
    

	lslightviewproj = fittounitcube * lslightviewproj; 
    
    return lslightviewproj;
}

}
