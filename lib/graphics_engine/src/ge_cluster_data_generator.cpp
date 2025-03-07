#include "ge_cluster_data_generator.hpp"

#include "ge_main.hpp"
#include "ge_vulkan_camera_scene_node.hpp"
#include "ge_vulkan_command_loader.hpp"

#include "ICameraSceneNode.h"
#include "ILightSceneNode.h"

#include <exception>
#include <stdexcept>

namespace GE
{

GEClusterDataGenerator::GEClusterDataGenerator()
{
    m_initialized = false;
    m_generated = false;
}

GEClusterDataGenerator::~GEClusterDataGenerator()
{
    clear();
}

void GEClusterDataGenerator::init(GEVulkanCameraSceneNode *cam)
{
    clear();

    m_initialized = true;
    
    float scale = getGEConfig()->m_render_scale;
    m_screen = irr::core::dimension2du(
        cam->getViewPort().getWidth() * scale,
        cam->getViewPort().getHeight() * scale);

    m_frustum_fld = cam->getViewFrustum()->getFarLeftDown();
    m_frustum_fru = cam->getViewFrustum()->getFarRightUp();

    m_view_matrix = cam->getViewMatrix();
    m_view_matrix.transformVect(m_frustum_fld);
    m_view_matrix.transformVect(m_frustum_fru);

    m_origin = cam->getAbsolutePosition();
    m_near = cam->getNearValue();

    for (unsigned i = 0u; i < m_screen.Width; i += getTileSize())
    {
        irr::core::vector3df ray = m_frustum_fru.getInterpolated(m_frustum_fld, float(i) / m_screen.Width);
        m_x_planes.push_back(irr::core::vector3df(-ray.Z, 0., ray.X).normalize());
    }
    m_x_planes.push_back(irr::core::vector3df(-m_frustum_fru.Z, 0., m_frustum_fru.X).normalize());

    for (unsigned i = 0u; i < m_screen.Height; i += getTileSize())
    {
        irr::core::vector3df ray = m_frustum_fru.getInterpolated(m_frustum_fld, float(i) / m_screen.Height);
        m_y_planes.push_back(irr::core::vector3df(0., -ray.Z, ray.Y).normalize());
    }
    m_y_planes.push_back(irr::core::vector3df(0., -m_frustum_fru.Z, m_frustum_fru.Y).normalize());
}

void GEClusterDataGenerator::addObject(irr::core::vector3df world_pos, float radius, unsigned id)
{
    if (!m_initialized)
    {
        throw std::logic_error("Adding new object before initializing the generator with a camera");
    }
    if (m_generated)
    {
        throw std::logic_error("Adding new object after generating the data");
    }
    m_object_pos.emplace_back(world_pos);
    m_object_rad.push_back(radius);
    m_object_ids.push_back(id);
}

void GEClusterDataGenerator::generate()
{
    if (!m_initialized)
    {
        throw std::logic_error("Generating cluster data before initializing with a camera");
    }
    if (m_generated)
    {
        return;
    }
    m_generated = true;

    m_far = 0.f;
    for (unsigned i = 0; i < m_object_ids.size(); i++)
    {
        m_far = std::max(m_far, m_origin.getDistanceFrom(m_object_pos[i]) + m_object_rad[i]);
    }
    
    float near  = m_near;
    float far   = m_far;
    float knear = depthKernel(near);
    float kfar  = depthKernel(far);

    std::vector<unsigned> inv_id;
    for (unsigned i = 0; i < m_object_ids.size(); i++)
    {
        if (m_object_ids[i] >= inv_id.size())
        {
            inv_id.resize(m_object_ids[i] + 1u, -1u);
        }
        inv_id[m_object_ids[i]] = i;
    }

    m_set_size = ((inv_id.size() - 1u) >> 5u) + 1u;

    m_cluster_data_xz.resize((m_x_planes.size() - 1) * getDepthDivisions() * m_set_size);
    m_cluster_data_yz.resize((m_x_planes.size() - 1) * getDepthDivisions() * m_set_size);

    unsigned setsize = m_set_size;

    auto fill_slice = [near, far, knear, kfar, setsize]
        (uint32_t *data, int slice, float n, float f, irr::u32 id)
    {
        if (f < near || n > far)
        {
            return;
        }
        n = std::max(n, near);
        f = std::min(f, far);
        int lrange = std::max((depthKernel(n) - knear) / (kfar - knear) * getDepthDivisions(), 0.f);
        int rrange = std::min((depthKernel(f) - knear) / (kfar - knear) * getDepthDivisions(), float(getDepthDivisions() - 1u));
        int offset = id >> 5;
        uint32_t value = 1u << (id & 31);
        for (int i = lrange; i <= rrange; i++)
        {
            data[(slice * getDepthDivisions() + i) * setsize + offset] |= value; 
        }
    };

    auto fill_slice_sphere = [fill_slice]
        (uint32_t *data, int slice, float planedis, float dis, float rad, irr::u32 id)
    {
        float dis2 = sqrtf(dis * dis - planedis * planedis);
        float rad2 = sqrtf(rad * rad - planedis * planedis);
        float n = dis2 - rad2;
        float f = dis2 + rad2;
        fill_slice(data, slice, dis2 - rad2, dis2 + rad2, id);
    };
    
    for (unsigned i = 0u; i < inv_id.size(); i += getChunkSize())
    {
        std::vector<irr::core::vector3df> points;
        std::vector<float> rads;
        std::vector<unsigned> ids;

        for (unsigned j = i; j < i + getChunkSize() && j < inv_id.size(); j++)
        {
            if (inv_id[j] == -1u)
            {
                continue;
            }
            points.push_back(m_object_pos[inv_id[j]]);
            rads.push_back(m_object_rad[inv_id[j]]);
            ids.push_back(j);
        }

        m_tasks.push_back(std::async(std::launch::deferred, 
            [this, points, rads, ids, fill_slice, fill_slice_sphere]() mutable
        {
            for (unsigned i = 0; i < ids.size(); i++)
            {
                irr::core::vector3df point = points[i];
                float rad = rads[i];
                unsigned id = ids[i];
                
                m_view_matrix.transformVect(point);

                float dis = point.getLength();
                int xcenter = 0;
                int ycenter = 0;

                if (m_x_planes[0].dotProduct(point) < 0.f && m_x_planes[m_x_planes.size() - 1].dotProduct(point) > 0.f)
                {
                    float coordx = point.X * (m_frustum_fld.Z / point.Z);
                    coordx = (coordx - m_frustum_fld.X) / (m_frustum_fru.X - m_frustum_fld.X) * float(m_screen.Width);
                    xcenter = int(coordx) / getTileSize();
                    fill_slice(m_cluster_data_xz.data(), xcenter, dis - rad, dis + rad, id);
                }
                else
                {
                    if (abs(m_x_planes[0].dotProduct(point)) < rad) xcenter = -1;
                    else if (abs(m_x_planes[m_x_planes.size() - 1].dotProduct(point)) < rad) xcenter = m_x_planes.size() - 1;
                    else return;
                }
                if (m_y_planes[0].dotProduct(point) < 0.f && m_y_planes[m_y_planes.size() - 1].dotProduct(point) > 0.f)
                {
                    float coordy = point.Y * (m_frustum_fld.Z / point.Z);
                    coordy = (coordy - m_frustum_fld.Y) / (m_frustum_fru.Y - m_frustum_fld.Y) * float(m_screen.Height);
                    ycenter = int(coordy) / getTileSize();
                    fill_slice(m_cluster_data_yz.data(), ycenter, dis - rad, dis + rad, id);
                }
                else
                {
                    if (abs(m_y_planes[0].dotProduct(point)) < rad) ycenter = -1;
                    else if (abs(m_y_planes[m_y_planes.size() - 1].dotProduct(point)) < rad) ycenter = m_y_planes.size() - 1;
                    else return;
                }
                for (int i = xcenter - 1; i >= 0; i--)
                {
                    float planedis = abs(m_x_planes[i + 1].dotProduct(point));
                    if (planedis > rad) break;
                    fill_slice_sphere(m_cluster_data_xz.data(), i, planedis, dis, rad, id);
                }
                for (int i = xcenter + 1; i < m_x_planes.size() - 1; i++)
                {
                    float planedis = abs(m_x_planes[i].dotProduct(point));
                    if (planedis > rad) break;
                    fill_slice_sphere(m_cluster_data_xz.data(), i, planedis, dis, rad, id);
                }
                for (int i = ycenter - 1; i >= 0; i--)
                {
                    float planedis = abs(m_y_planes[i + 1].dotProduct(point));
                    if (planedis > rad) break;
                    fill_slice_sphere(m_cluster_data_yz.data(), i, planedis, dis, rad, id);
                }
                for (int i = ycenter + 1; i < m_y_planes.size() - 1; i++)
                {
                    float planedis = abs(m_y_planes[i].dotProduct(point));
                    if (planedis > rad) break;
                    fill_slice_sphere(m_cluster_data_yz.data(), i, planedis, dis, rad, id);
                }
            }
        }));
    }

    /*size_t cluster_xz_size = cluster_data_xz.size() * sizeof(uint32_t);

    if (written_size + cluster_xz_size > m_sbo_data->getSize())
    {
        min_size = (written_size + cluster_xz_size) * 2;
        goto start;
    }

    memcpy(mapped_addr, cluster_data_xz.data(), cluster_xz_size);
    written_size += cluster_xz_size;
    mapped_addr += cluster_xz_size;

    if (cluster_xz_size > m_clusters_xz_padded_size)
    {
        m_update_data_descriptor_sets = true;
        size_t cur_padding = getPadding(written_size, sbo_alignment);
        if (cur_padding > 0)
        {
            if (written_size + cur_padding > m_sbo_data->getSize())
            {
                min_size = (written_size + cur_padding) * 2;
                goto start;
            }
            written_size += cur_padding;
            mapped_addr += cur_padding;
            cluster_xz_size += cur_padding;
        }
        m_clusters_xz_padded_size = cluster_xz_size;
    }
    else
    {
        size_t extra = m_clusters_xz_padded_size - cluster_xz_size;
        if (written_size + extra > m_sbo_data->getSize())
        {
            min_size = (written_size + extra) * 2;
            goto start;
        }
        cluster_xz_size = m_clusters_xz_padded_size;
        written_size += extra;
        mapped_addr += extra;
    }

    size_t cluster_yz_size = cluster_data_yz.size() * sizeof(uint32_t);

    if (written_size + cluster_yz_size > m_sbo_data->getSize())
    {
        min_size = (written_size + cluster_yz_size) * 2;
        goto start;
    }

    memcpy(mapped_addr, cluster_data_yz.data(), cluster_yz_size);
    written_size += cluster_yz_size;
    mapped_addr += cluster_yz_size;

    if (cluster_yz_size > m_clusters_yz_padded_size)
    {
        m_update_data_descriptor_sets = true;
        size_t cur_padding = getPadding(written_size, sbo_alignment);
        if (cur_padding > 0)
        {
            if (written_size + cur_padding > m_sbo_data->getSize())
            {
                min_size = (written_size + cur_padding) * 2;
                goto start;
            }
            written_size += cur_padding;
            mapped_addr += cur_padding;
            cluster_yz_size += cur_padding;
        }
        m_clusters_yz_padded_size = cluster_yz_size;
    }
    else
    {
        size_t extra = m_clusters_yz_padded_size - cluster_yz_size;
        if (written_size + extra > m_sbo_data->getSize())
        {
            min_size = (written_size + extra) * 2;
            goto start;
        }
        cluster_yz_size = m_clusters_yz_padded_size;
        written_size += extra;
        mapped_addr += extra;
    }*/

}

const std::vector<uint32_t>& GEClusterDataGenerator::getClusterDataXZ()
{
    generate();
    
    m_tasks.clear();

    return m_cluster_data_xz;
}

const std::vector<uint32_t>& GEClusterDataGenerator::getClusterDataYZ()
{
    generate();
    
    m_tasks.clear();

    return m_cluster_data_yz;
}

void GEClusterDataGenerator::clear()
{
    m_tasks.clear();

    m_x_planes.clear();
    m_y_planes.clear();

    m_cluster_data_xz.clear();
    m_cluster_data_yz.clear();

    m_object_pos.clear();
    m_object_rad.clear();
    m_object_ids.clear();

    m_initialized = false;
    m_generated = false;
}

}
