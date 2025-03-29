#include "ge_environment_map.hpp"

#include "vector2d.h"

#include <assert.h>

namespace GE
{
// ----------------------------------------------------------------------------
GECubemapSampler::GECubemapSampler(std::array<GEMipmap*, 6> cubemap, unsigned max_size)
{
    auto fill_buf = [](GEImageLevel dst, int x1, int y1, GEImageLevel src, int x2, int y2)
    {
        int ind1 = dst.m_dim.Width * y1 + x1;
        int ind2 = src.m_dim.Width * y2 + x2;
        irr::video::SColorf *conv = (irr::video::SColorf*) dst.m_data;
        uint32_t color = ((uint32_t*) src.m_data)[ind2];
        conv[ind1] = irr::video::SColorf(color);
        conv[ind1].b = powf(conv[ind1].b, 2.2f);
        conv[ind1].g = powf(conv[ind1].g, 2.2f);
        conv[ind1].r = powf(conv[ind1].r, 2.2f);
        conv[ind1].a = powf(conv[ind1].a, 2.2f);
    };
    auto fill_corner = [](GEImageLevel dst, int xt, int yt, int x1, int y1, int x2, int y2, int x3, int y3)
    {
        int ind1 = dst.m_dim.Width * y1 + x1;
        int ind2 = dst.m_dim.Width * y2 + x2;
        int ind3 = dst.m_dim.Width * y3 + x3;
        int indt = dst.m_dim.Width * yt + xt;
        irr::video::SColorf *conv = (irr::video::SColorf*) dst.m_data;
        conv[indt] = conv[ind1].getInterpolated(conv[ind2].getInterpolated(conv[ind3], 0.5f), 1.f / 3.f);
    };
    
    std::array<std::vector<GEImageLevel>, 6> src;

    for (int i = 0; i < 6; i++)
    {
        unsigned offset = 0;
        while (cubemap[i]->getAllLevels()[offset].m_dim.Width > max_size
            && cubemap[i]->getAllLevels()[offset].m_dim.Height > max_size)
        {
            offset++;
        }
        for (int j = 0; j < cubemap[i]->getAllLevels().size() - offset; j++)
        {
            src[i].push_back(cubemap[i]->getAllLevels()[j + offset]);
        }
    }
    m_max_size = max_size;

    unsigned buf_size = 0;

    for (int i = 0; i < 6; i++)
    {
        m_map[i].resize(src[i].size());
        for (int j = 0; j < src[i].size(); j++)
        {
            m_map[i][j].m_dim.Width = src[i][j].m_dim.Width + 2;
            m_map[i][j].m_dim.Height = src[i][j].m_dim.Height + 2;
            m_map[i][j].m_size = m_map[i][j].m_dim.Width * m_map[i][j].m_dim.Height;
            buf_size += m_map[i][j].m_size;
        }
    }

    m_data = new irr::video::SColorf[buf_size];
    unsigned data_offset = 0;
    
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < src[i].size(); j++)
        {
            m_map[i][j].m_data = m_data + data_offset;
            data_offset += m_map[i][j].m_size;

            for (int u = 0; u < src[i][j].m_dim.Width; u++)
            {
                for (int v = 0; v < src[i][j].m_dim.Height; v++)
                {
                    fill_buf(m_map[i][j], u + 1, v + 1, src[i][j], u, v);
                }
            }
        }
    }
    const int horizon[4] = { 0, 5, 1, 4 };
    for (int h = 0; h < 4; h++)
    {
        int i = horizon[h];
        int r = horizon[(h + 1) % 4];
        for (int j = 0; j < m_map[i].size(); j++)
        {
            for (int k = 0; k < src[i][j].m_dim.Height; k++)
            {
                fill_buf(m_map[r][j], 0, k + 1, src[i][j], src[i][j].m_dim.Width - 1, k);
                fill_buf(m_map[i][j], m_map[i][j].m_dim.Width - 1, k + 1, src[r][j], 0, k);
            }
            for (int k = 0; k < src[i][j].m_dim.Width; k++)
            {
                int x1[4] = { (int) src[2][j].m_dim.Width - 1, (int) src[2][j].m_dim.Width - k - 1, 0, k };
                int y1[4] = { (int) src[2][j].m_dim.Height - k - 1, 0, k, (int) src[2][j].m_dim.Height - 1 };
                int x2[4] = { (int) m_map[2][j].m_dim.Width - 1, (int) m_map[2][j].m_dim.Width - k - 2, 0, k + 1 };
                int y2[4] = { (int) m_map[2][j].m_dim.Height - k - 2, 0,  k + 1, (int) m_map[2][j].m_dim.Height - 1 };
                
                int l = (h + 2) % 4;
                fill_buf(m_map[i][j], k + 1, src[i][j].m_dim.Height - 1, src[3][j], x1[l], y1[h]);
                fill_buf(m_map[i][j], k + 1, 0,                          src[2][j], x1[h], y1[h]);
                fill_buf(m_map[3][j], x2[l], y2[h], src[i][j], k, src[i][j].m_dim.Height - 1);
                fill_buf(m_map[2][j], x2[h], y2[h], src[i][j], k, 0);
            }
        }
    }
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < m_map[i].size(); j++)
        {
            int w = m_map[i][j].m_dim.Width;
            int h = m_map[i][j].m_dim.Height;
            fill_corner(m_map[i][j], 0, 0, 0, 1, 1, 0, 1, 1);
            fill_corner(m_map[i][j], 0, h - 1, 0, h - 2, 1, h - 1, 1, h - 2);
            fill_corner(m_map[i][j], w - 1, 0, w - 1, 1, w - 2, 0, w - 2, 1);
            fill_corner(m_map[i][j], w - 1, h - 1, w - 1, h - 2, w - 2, h - 1, w - 2, h - 2);
        }
    }
}

// ----------------------------------------------------------------------------
irr::video::SColorf GECubemapSampler::sample(irr::core::vector3df dir, float lod) const
{
    using irr::core::vector2df;
    using irr::core::vector2di;
    using irr::core::vector3df;
    using irr::video::SColorf;
    using irr::core::clamp;

    auto sample_lod = [](const std::vector<GEImageLevel>& mipmap,
        vector2df uv, float lod) -> SColorf
    {
        auto sample = [](const GEImageLevel& level, vector2df uv) -> SColorf
        {
            uv = vector2df(
            clamp<float>(uv.X * (level.m_dim.Width - 2.0f) + 1.0f, 1.0f, -1.0f + level.m_dim.Width),
            clamp<float>(uv.Y * (level.m_dim.Height - 2.0f) + 1.0f, 1.0f, -1.0f + level.m_dim.Height));

            vector2di point = vector2di(uv.X - 0.5f, uv.Y - 0.5f);
            vector2df lerp = vector2df(uv.X - (0.5f + point.X), uv.Y - (0.5f + point.Y));

            SColorf texel0 = 
                (((SColorf*)level.m_data)[point.Y * level.m_dim.Width + point.X]);
            SColorf texel1 = 
                (((SColorf*)level.m_data)[(point.Y + 1) * level.m_dim.Width + point.X]);
                texel0 = texel1.getInterpolated(texel0, lerp.Y);
            SColorf texel2 = 
                (((SColorf*)level.m_data)[point.Y * level.m_dim.Width + (point.X + 1)]);
            SColorf texel3 = 
                (((SColorf*)level.m_data)[(point.Y + 1) * level.m_dim.Width + (point.X + 1)]);
            texel2 = texel3.getInterpolated(texel2, lerp.Y);
            texel0 = texel2.getInterpolated(texel0, lerp.X);
            return texel0;
        };
        lod = clamp(lod, 0.0f, float(mipmap.size() - 1));
        float level;
        float lerp = modff(lod, &level);
        SColorf lod0 = sample(mipmap[level], uv);
        if (lerp == 0.0) return lod0;
        SColorf lod1 = sample(mipmap[level + 1], uv);
        return lod0.getInterpolated(lod1, lerp);
    };
    float ax = abs(dir.X), ay = abs(dir.Y), az = abs(dir.Z);

    if (ax > ay && ax > az)
    {
        return dir.X > 0.0 ? sample_lod(m_map[0], vector2df(-dir.Z / ax * 0.5 + 0.5, -dir.Y / ax * 0.5 + 0.5), lod)
                           : sample_lod(m_map[1], vector2df(dir.Z / ax * 0.5 + 0.5, -dir.Y / ax * 0.5 + 0.5), lod);
    }
    else if (ay > ax && ay > az)
    {
        return dir.Y > 0.0 ? sample_lod(m_map[2], vector2df(dir.X / ay * 0.5 + 0.5, dir.Z / ay * 0.5 + 0.5), lod)
                           : sample_lod(m_map[3], vector2df(-dir.X / ay * 0.5 + 0.5, dir.Z / ay * 0.5 + 0.5), lod);
    }
    else
    {
        return dir.Z > 0.0 ? sample_lod(m_map[4], vector2df(dir.X / az * 0.5 + 0.5, -dir.Y / az * 0.5 + 0.5), lod)
                           : sample_lod(m_map[5], vector2df(-dir.X / az * 0.5 + 0.5, -dir.Y / az * 0.5 + 0.5), lod);
    }
}

// ----------------------------------------------------------------------------
GEEnvironmentMap::GEEnvironmentMap(GEMipmap *mipmap, GECubemapSampler &sampler,
                                   int face, bool diffuse,
                                   irr::core::quaternion occlusion_sh2)
{
    using irr::core::PI;
    using irr::core::vector2df;
    using irr::core::vector3df;
    using irr::core::quaternion;
    using irr::video::SColorf;
    using irr::core::clamp;

    auto hammersly2d = [](int i, int n)->vector2df
    {
        uint32_t bits = i;
        bits = (bits << 16u) | (bits >> 16u);
        bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
        bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
        bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
        bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
        return vector2df(float(i) / float(n), 
                         float(bits) * 2.3283064365386963e-10);
    };

    int sample_res = diffuse ? getIrradianceResolution() : getRadianceResolution();
    int sample_count = diffuse ? getIrradianceNumSamples() : getRadianceNumSamples();

    m_channels = mipmap->getChannels();
    assert(m_channels == 4);

    int offset = 0;
    while (mipmap->getAllLevels()[offset].m_dim.Width > sample_res
        && mipmap->getAllLevels()[offset].m_dim.Height > sample_res)
    {
        offset++;
    }

    m_levels.resize(diffuse ? 1 : std::max(size_t(1), mipmap->getAllLevels().size() - offset));
    unsigned buf_size = 0;

    for (int i = 0; i < m_levels.size(); i++)
    {
        m_levels[i].m_dim.Width = mipmap->getAllLevels()[i + offset].m_dim.Width;
        m_levels[i].m_dim.Height = mipmap->getAllLevels()[i + offset].m_dim.Height;
        m_levels[i].m_size = m_levels[i].m_dim.Width * m_levels[i].m_dim.Height * 4;
        buf_size += m_levels[i].m_size;
    }

    m_data = new uint8_t[buf_size];
    
    for (int i = 0; i < m_levels.size(); i++)
    {
        m_levels[i].m_data = m_data + m_mipmap_sizes;
        m_mipmap_sizes += m_levels[i].m_size;
    }

    float lodbias = log2f(1.0f * sampler.getMaxSize() / m_levels[0].m_dim.Width);

    for (int level = 0; level < m_levels.size(); level++)
    {
        std::vector<vector3df> local_space_dirs;
        std::vector<float> lods;
        local_space_dirs.resize(sample_count);
        lods.resize(sample_count);
        for (int i = 0; i < sample_count; i++)
        {
            // Lambertian
            vector2df xi = hammersly2d(i, sample_count);

            if (diffuse)
            {
                float cos_theta = sqrtf(1.0 - xi.Y);
                float sin_theta = sqrtf(xi.Y);
                float phi = 2.0 * PI * xi.X;
                float pdf = cos_theta / PI;
                local_space_dirs[i] = vector3df(
                    sin_theta * cosf(phi),
                    sin_theta * sinf(phi),
                    cos_theta).normalize();
                lods[i] = 0.5f * log2(6.0 * m_levels[level].m_dim.Width
                                          * m_levels[level].m_dim.Height
                                          / (sample_count * pdf)) + lodbias;
            }
            else
            {
                float roughness = 1.0f * level / (m_levels.size() - 1);
                float alpha = roughness * roughness;
                float cos_theta = clamp(sqrtf((1.0 - xi.Y) / (1.0 + (alpha * alpha - 1.0) * xi.Y)), 0.0f, 1.0f);
                float sin_theta = sqrtf(1.0 - cos_theta * cos_theta);
                float phi = 2.0 * PI * xi.X;
                float ggx_a = cos_theta * alpha;
                float pdf = alpha / (1.0 - cos_theta * cos_theta + ggx_a * ggx_a);
                pdf = pdf * pdf / PI / 4.0;  // GGX + Jacobian
                local_space_dirs[i] = vector3df(
                    sin_theta * cosf(phi),
                    sin_theta * sinf(phi),
                    cos_theta).normalize();
                lods[i] = 0.5f * log2(6.0 * m_levels[level].m_dim.Width
                                          * m_levels[level].m_dim.Height
                                          / (sample_count * pdf)) + lodbias;
            }
        }
        for (int i = 0; i < m_levels[level].m_dim.Width * m_levels[level].m_dim.Height; i++)
        {
            vector2df uv = vector2df(
                (0.5f + i % m_levels[level].m_dim.Width) / m_levels[level].m_dim.Width  * 2.0f - 1.0f,
                (0.5f + i / m_levels[level].m_dim.Width) / m_levels[level].m_dim.Height * 2.0f - 1.0f);
            vector3df scan = vector3df();
            switch(face)
            {
                case 0: scan = vector3df( 1.f,  uv.Y, -uv.X); break;
                case 1: scan = vector3df(-1.f,  uv.Y,  uv.X); break;
                case 2: scan = vector3df( uv.X, -1.f,  uv.Y); break;
                case 3: scan = vector3df( uv.X,  1.f, -uv.Y); break;
                case 4: scan = vector3df( uv.X, uv.Y,  1.f ); break;
                case 5: scan = vector3df(-uv.X, uv.Y, -1.f ); break;
            }
            vector3df dir = scan.normalize(); // Normal
            dir.Y = -dir.Y;

            if (!diffuse && level == 0)
            {
                SColorf color = sampler.sample(dir, lodbias);
                float occlusion = clamp(occlusion_sh2.dotProduct(quaternion(dir.X, dir.Y, dir.Z, 1.0f)), 0.0f, 1.0f);
                ((uint8_t*)m_levels[level].m_data)[i * 4 + 0] = ((uint8_t)(powf(color.b * occlusion, 1.f / 2.2f) * 255.));
                ((uint8_t*)m_levels[level].m_data)[i * 4 + 1] = ((uint8_t)(powf(color.g * occlusion, 1.f / 2.2f) * 255.));
                ((uint8_t*)m_levels[level].m_data)[i * 4 + 2] = ((uint8_t)(powf(color.r * occlusion, 1.f / 2.2f) * 255.));
                ((uint8_t*)m_levels[level].m_data)[i * 4 + 3] = ((uint8_t)(powf(color.a * occlusion, 1.f / 2.2f) * 255.));
                continue;
            }

            vector3df bitangent = vector3df(0.0, 1.0, 0.0);
            float ndotup = dir.dotProduct(vector3df(0.0, 1.0, 0.0));
            if (1.0 - abs(ndotup) < 0.00001)
            {
                bitangent = ndotup > 0.0 ? vector3df(0.0, 0.0, 1.0)
                                        : vector3df(0.0, 0.0, -1.0);
            }
            vector3df tangent = bitangent.crossProduct(dir).normalize();
            bitangent         = dir.crossProduct(tangent);

            SColorf color = SColorf();
            float weight = 0.;
        
            for (int i = 0; i < sample_count; i++)
            {
                // TBN * local_space_dir
                vector3df ldir = local_space_dirs[i];
                float lod = lods[i];

                vector3df world_dir = vector3df(
                    tangent.X * ldir.X + bitangent.X * ldir.Y + dir.X * ldir.Z,
                    tangent.Y * ldir.X + bitangent.Y * ldir.Y + dir.Y * ldir.Z,
                    tangent.Z * ldir.X + bitangent.Z * ldir.Y + dir.Z * ldir.Z
                );

                // Reflect
                if (!diffuse)
                {
                    vector3df light = -dir;
                    light -= 2.0f * world_dir * light.dotProduct(world_dir);
                    world_dir = light.normalize();
                }
                float NdotL = diffuse ? 1.0 : dir.dotProduct(world_dir);

                SColorf samp = sampler.sample(world_dir, lod);
                float occlusion = 
                    clamp(occlusion_sh2.dotProduct(quaternion(world_dir.X, world_dir.Y, world_dir.Z, 1.0f)), 0.0f, 1.0f);
                color.a += samp.a * occlusion;
                color.r += samp.r * occlusion;
                color.g += samp.g * occlusion;
                color.b += samp.b * occlusion;
                weight += NdotL;
            }
            ((uint8_t*)m_levels[level].m_data)[i * 4 + 0] = ((uint8_t)(powf(color.b / weight, 1.f / 2.2f) * 255.));
            ((uint8_t*)m_levels[level].m_data)[i * 4 + 1] = ((uint8_t)(powf(color.g / weight, 1.f / 2.2f) * 255.));
            ((uint8_t*)m_levels[level].m_data)[i * 4 + 2] = ((uint8_t)(powf(color.r / weight, 1.f / 2.2f) * 255.));
            ((uint8_t*)m_levels[level].m_data)[i * 4 + 3] = ((uint8_t)(powf(color.a / weight, 1.f / 2.2f) * 255.));
        }
    }
}   // GEEnvironmentMap

}
