#include "ge_main.hpp"
#include "ge_spm.hpp"
#include "ge_spm_buffer.hpp"
#include "ge_vulkan_driver.hpp"
#include "mini_glm.hpp"

#include "IMesh.h"
#include "IMeshBuffer.h"
#include "S3DVertex.h"

#include <algorithm>
#include <chrono>

namespace GE
{
irr::video::IVideoDriver* g_driver = NULL;
GEConfig g_config =
{
    false,
    false,
    false,
    true,
    false,
    false,
    {},
    1.0f
};
std::string g_shader_folder = "";
std::chrono::steady_clock::time_point g_mono_start =
    std::chrono::steady_clock::now();

void setVideoDriver(irr::video::IVideoDriver* driver)
{
    if (driver != g_driver)
    {
        // Reset everytime driver is recreated
        g_config.m_ondemand_load_texture_paths.clear();
        g_driver = driver;
    }
}

irr::video::IVideoDriver* getDriver()
{
    return g_driver;
}

GE::GEVulkanDriver* getVKDriver()
{
    return dynamic_cast<GE::GEVulkanDriver*>(g_driver);
}

GEConfig* getGEConfig()
{
    return &g_config;
}

void setShaderFolder(const std::string& path)
{
    g_shader_folder = path + "ge_shaders/";
}

const std::string& getShaderFolder()
{
    return g_shader_folder;
}

void deinit()
{
}

uint64_t getMonoTimeMs()
{
    auto duration = std::chrono::steady_clock::now() - g_mono_start;
    auto value =
        std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    return value.count();
}

void mathPlaneNormf(float *p)
{
    float f = 1.0f / sqrtf(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
    p[0] *= f;
    p[1] *= f;
    p[2] *= f;
    p[3] *= f;
}

void mathPlaneFrustumf(float* out, const irr::core::matrix4& pvm)
{
    // return 6 planes, 24 floats
    const float* m = pvm.pointer();

    // near
    out[0] = m[3] + m[2];
    out[1] = m[7] + m[6];
    out[2] = m[11] + m[10];
    out[3] = m[15] + m[14];
    mathPlaneNormf(&out[0]);

    // right
    out[4] = m[3] - m[0];
    out[4 + 1] = m[7] - m[4];
    out[4 + 2] = m[11] - m[8];
    out[4 + 3] = m[15] - m[12];
    mathPlaneNormf(&out[4]);

    // left
    out[2 * 4] = m[3] + m[0];
    out[2 * 4 + 1] = m[7] + m[4];
    out[2 * 4 + 2] = m[11] + m[8];
    out[2 * 4 + 3] = m[15] + m[12];
    mathPlaneNormf(&out[2 * 4]);

    // bottom
    out[3 * 4] = m[3] + m[1];
    out[3 * 4 + 1] = m[7] + m[5];
    out[3 * 4 + 2] = m[11] + m[9];
    out[3 * 4 + 3] = m[15] + m[13];
    mathPlaneNormf(&out[3 * 4]);

    // top
    out[4 * 4] = m[3] - m[1];
    out[4 * 4 + 1] = m[7] - m[5];
    out[4 * 4 + 2] = m[11] - m[9];
    out[4 * 4 + 3] = m[15] - m[13];
    mathPlaneNormf(&out[4 * 4]);

    // far
    out[5 * 4] = m[3] - m[2];
    out[5 * 4 + 1] = m[7] - m[6];
    out[5 * 4 + 2] = m[11] - m[10];
    out[5 * 4 + 3] = m[15] - m[14];
    mathPlaneNormf(&out[5 * 4]);
}

irr::scene::IAnimatedMesh* convertIrrlichtMeshToSPM(irr::scene::IMesh* mesh)
{
    GESPM* spm = new GESPM();
    for (unsigned i = 0; i < mesh->getMeshBufferCount(); i++)
    {
        std::vector<video::S3DVertexSkinnedMesh> vertices;
        scene::IMeshBuffer* mb = mesh->getMeshBuffer(i);
        if (!mb)
            continue;

        GESPMBuffer* spm_mb = new GESPMBuffer();
        assert(mb->getVertexType() == video::EVT_STANDARD);
        video::S3DVertex* v_ptr = (video::S3DVertex*)mb->getVertices();
        for (unsigned j = 0; j < mb->getVertexCount(); j++)
        {
            video::S3DVertexSkinnedMesh sp;
            sp.m_position = v_ptr[j].Pos;
            sp.m_normal = MiniGLM::compressVector3(v_ptr[j].Normal);
            video::SColorf orig(v_ptr[j].Color);
            video::SColorf diffuse(mb->getMaterial().DiffuseColor);
            orig.r = orig.r * diffuse.r;
            orig.g = orig.g * diffuse.g;
            orig.b = orig.b * diffuse.b;
            orig.a = orig.a * diffuse.a;
            sp.m_color = orig.toSColor();
            sp.m_all_uvs[0] = MiniGLM::toFloat16(v_ptr[j].TCoords.X);
            sp.m_all_uvs[1] = MiniGLM::toFloat16(v_ptr[j].TCoords.Y);
            spm_mb->getVerticesVector().push_back(sp);
        }
        uint16_t* idx_ptr = mb->getIndices();
        std::vector<uint16_t> indices(idx_ptr, idx_ptr + mb->getIndexCount());
        std::swap(spm_mb->getIndicesVector(), indices);
        spm_mb->getMaterial() = mb->getMaterial();
        spm_mb->recalculateBoundingBox();
        spm->addMeshBuffer(spm_mb);
    }
    spm->finalize();
    return spm;
}

}
