#include "graphics/glwrap.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/stkmesh.hpp"
#include "tracks/track.hpp"
#include <ISceneManager.h>
#include <IMaterialRenderer.h>
#include "config/user_config.hpp"
#include "graphics/callbacks.hpp"
#include "utils/helpers.hpp"
#include "graphics/camera.hpp"
#include "modes/world.hpp"


Material::ShaderType MaterialTypeToMeshMaterial(video::E_MATERIAL_TYPE MaterialType, video::E_VERTEX_TYPE tp,
    Material* material, Material* layer2Material)
{
    if (layer2Material != NULL && layer2Material->getShaderType() == Material::SHADERTYPE_SPLATTING)
        return Material::SHADERTYPE_SPLATTING;

    switch (material->getShaderType())
    {
    default:
        return material->getShaderType();   
    case Material::SHADERTYPE_SOLID:
        if (MaterialType == irr_driver->getShader(ES_NORMAL_MAP))
            return Material::SHADERTYPE_NORMAL_MAP;
        else if (tp == video::EVT_2TCOORDS)
            return Material::SHADERTYPE_DETAIL_MAP;
        return Material::SHADERTYPE_SOLID;
    }
}

TransparentMaterial MaterialTypeToTransparentMaterial(video::E_MATERIAL_TYPE type, f32 MaterialTypeParam, Material* material)
{
    if (type == irr_driver->getShader(ES_DISPLACE))
        return TM_DISPLACEMENT;
    video::E_BLEND_FACTOR srcFact, DstFact;
    video::E_MODULATE_FUNC mod;
    u32 alpha;
    unpack_textureBlendFunc(srcFact, DstFact, mod, alpha, MaterialTypeParam);
    if (DstFact == video::EBF_ONE || type == video::EMT_TRANSPARENT_ADD_COLOR)
        return TM_ADDITIVE;
    return TM_DEFAULT;
}

video::E_VERTEX_TYPE getVTXTYPEFromStride(size_t stride)
{
    if (stride == sizeof(video::S3DVertex))
        return video::EVT_STANDARD;
    else if (stride == sizeof(video::S3DVertex2TCoords))
        return video::EVT_2TCOORDS;
    assert(stride == sizeof(video::S3DVertexTangents));
    return video::EVT_TANGENTS;
}

GLuint createVAO(GLuint vbo, GLuint idx, video::E_VERTEX_TYPE type)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    assert(vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    switch (type)
    {
    case video::EVT_STANDARD:
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), 0);
        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), (GLvoid*)12);
        // Color
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, getVertexPitchFromType(type), (GLvoid*)24);
        // Texcoord
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), (GLvoid*)28);
        break;
    case video::EVT_2TCOORDS:
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), 0);
        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), (GLvoid*)12);
        // Color
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, getVertexPitchFromType(type), (GLvoid*)24);
        // Texcoord
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), (GLvoid*)28);
        // SecondTexcoord
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), (GLvoid*)36);
        break;
    case video::EVT_TANGENTS:
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), 0);
        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), (GLvoid*)12);
        // Color
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, getVertexPitchFromType(type), (GLvoid*)24);
        // Texcoord
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), (GLvoid*)28);
        // Tangent
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), (GLvoid*)36);
        // Bitangent
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), (GLvoid*)48);
        break;
    default:
        assert(0 && "Wrong vertex type");
    }
    assert(idx);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx);
    return vao;
}

GLMesh allocateMeshBuffer(scene::IMeshBuffer* mb, const std::string& debug_name)
{
    GLMesh result = {};
    if (!mb)
        return result;
    result.mb = mb;

#ifdef DEBUG
    result.debug_name = debug_name;
#endif

    result.IndexCount = mb->getIndexCount();
    switch (mb->getIndexType())
    {
    case irr::video::EIT_16BIT:
    {
        result.IndexType = GL_UNSIGNED_SHORT;
        break;
    }
    case irr::video::EIT_32BIT:
    {
        result.IndexType = GL_UNSIGNED_INT;
        break;
    }
    default:
    {
        assert(0 && "Wrong index size");
    }
    }
    result.VAOType = mb->getVertexType();
    result.Stride = getVertexPitchFromType(result.VAOType);


    result.IndexCount = mb->getIndexCount();
    switch (mb->getPrimitiveType())
    {
    case scene::EPT_POINTS:
        result.PrimitiveType = GL_POINTS;
        break;
    case scene::EPT_TRIANGLE_STRIP:
        result.PrimitiveType = GL_TRIANGLE_STRIP;
        break;
    case scene::EPT_TRIANGLE_FAN:
        result.PrimitiveType = GL_TRIANGLE_FAN;
        break;
    case scene::EPT_LINES:
        result.PrimitiveType = GL_LINES;
        break;
    case scene::EPT_TRIANGLES:
        result.PrimitiveType = GL_TRIANGLES;
        break;
    case scene::EPT_POINT_SPRITES:
    case scene::EPT_LINE_LOOP:
    case scene::EPT_POLYGON:
    case scene::EPT_LINE_STRIP:
    case scene::EPT_QUAD_STRIP:
    case scene::EPT_QUADS:
        assert(0 && "Unsupported primitive type");
    }
    for (unsigned i = 0; i < 8; i++)
        result.textures[i] = mb->getMaterial().getTexture(i);
    result.TextureMatrix = 0;
    result.VAOType = mb->getVertexType();
    return result;
}

static
size_t getUnsignedSize(unsigned tp)
{
    switch (tp)
    {
    case GL_UNSIGNED_SHORT:
        return sizeof(u16);
    case GL_UNSIGNED_INT:
        return sizeof(u32);
    default:
        assert(0 && "Unsupported index type");
        return 0;
    }
}

void fillLocalBuffer(GLMesh &mesh, scene::IMeshBuffer* mb)
{
    glBindVertexArray(0);
    glGenBuffers(1, &(mesh.vertex_buffer));
    glGenBuffers(1, &(mesh.index_buffer));

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_buffer);
    const void* vertices = mb->getVertices();
    const u32 vertexCount = mb->getVertexCount();

    const c8* vbuf = static_cast<const c8*>(vertices);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * mesh.Stride, vbuf, GL_STREAM_DRAW);
    assert(vertexCount);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.index_buffer);
    const void* indices = mb->getIndices();
    mesh.IndexCount = mb->getIndexCount();

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.IndexCount * getUnsignedSize(mesh.IndexType), indices, GL_STREAM_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


core::matrix4 computeMVP(const core::matrix4 &ModelMatrix)
{
    core::matrix4 ModelViewProjectionMatrix = irr_driver->getProjMatrix();
    ModelViewProjectionMatrix *= irr_driver->getViewMatrix();
    ModelViewProjectionMatrix *= ModelMatrix;
    return ModelViewProjectionMatrix;
}

core::vector3df getWindDir()
{
    const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;
    GrassShaderProvider *gsp = (GrassShaderProvider *)irr_driver->getCallback(ES_GRASS);
    float m_speed = gsp->getSpeed();

    return m_speed * vector3df(1., 0., 0.) * cos(time);
}

bool isObject(video::E_MATERIAL_TYPE type)
{
    if (type == irr_driver->getShader(ES_OBJECTPASS))
        return true;
    if (type == irr_driver->getShader(ES_OBJECTPASS_REF))
        return true;
    if (type == irr_driver->getShader(ES_OBJECTPASS_RIMLIT))
        return true;
    if (type == irr_driver->getShader(ES_NORMAL_MAP))
        return true;
    if (type == irr_driver->getShader(ES_SPHERE_MAP))
        return true;
    if (type == irr_driver->getShader(ES_SPLATTING))
        return true;
    if (type == irr_driver->getShader(ES_GRASS))
        return true;
    if (type == irr_driver->getShader(ES_GRASS_REF))
        return true;
    if (type == irr_driver->getShader(ES_DISPLACE))
        return true;
    if (type == irr_driver->getShader(ES_OBJECT_UNLIT))
        return true;
    if (type == video::EMT_TRANSPARENT_ALPHA_CHANNEL)
        return true;
    if (type == video::EMT_ONETEXTURE_BLEND)
        return true;
    if (type == video::EMT_TRANSPARENT_ADD_COLOR)
        return true;
    if (type == video::EMT_SOLID)
        return true;
    if (type == video::EMT_LIGHTMAP_LIGHTING)
        return true;
    if (type == video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF)
        return true;
    return false;
}

static void
SetTexture(GLMesh &mesh, unsigned i, bool isSrgb, const std::string &matname)
{
    if (!mesh.textures[i])
    {
        Log::fatal("STKMesh", "Missing texture %d for material %s", i, matname.c_str());
        return;
    }
    compressTexture(mesh.textures[i], isSrgb);
    if (UserConfigParams::m_azdo)
    {
        if (!mesh.TextureHandles[i])
            mesh.TextureHandles[i] = glGetTextureSamplerHandleARB(getTextureGLuint(mesh.textures[i]), MeshShader::ObjectPass1Shader::getInstance()->SamplersId[0]);
        if (!glIsTextureHandleResidentARB(mesh.TextureHandles[i]))
            glMakeTextureHandleResidentARB(mesh.TextureHandles[i]);
    }
}

static std::string
getShaderTypeName(Material::ShaderType Mat)
{
    switch (Mat)
    {
    default:
    case Material::SHADERTYPE_SOLID:
        return "Solid";
    case Material::SHADERTYPE_ALPHA_TEST:
        return "Alpha Test";
    case Material::SHADERTYPE_VEGETATION:
        return "Grass";
    case Material::SHADERTYPE_SPHERE_MAP:
        return "Sphere Map";
    case Material::SHADERTYPE_SOLID_UNLIT:
        return "Unlit";
    case Material::SHADERTYPE_DETAIL_MAP:
        return "Detail";
    case Material::SHADERTYPE_NORMAL_MAP:
        return "Normal";
    case Material::SHADERTYPE_SPLATTING:
        return "Splatting";
    }
}

void InitTextures(GLMesh &mesh, Material::ShaderType Mat)
{
    switch (Mat)
    {
    default:
    case Material::SHADERTYPE_SOLID:
    case Material::SHADERTYPE_ALPHA_TEST:
    case Material::SHADERTYPE_VEGETATION:
    case Material::SHADERTYPE_SPHERE_MAP:
    case Material::SHADERTYPE_SOLID_UNLIT:
        SetTexture(mesh, 0, true, getShaderTypeName(Mat));
        SetTexture(mesh, 1, false, getShaderTypeName(Mat));
        break;
    case Material::SHADERTYPE_DETAIL_MAP:
    case Material::SHADERTYPE_NORMAL_MAP:
        SetTexture(mesh, 0, true, getShaderTypeName(Mat));
        SetTexture(mesh, 1, false, getShaderTypeName(Mat));
        SetTexture(mesh, 2, false, getShaderTypeName(Mat));
        break;
    case Material::SHADERTYPE_SPLATTING:
        SetTexture(mesh, 0, true, getShaderTypeName(Mat));
        SetTexture(mesh, 1, false, getShaderTypeName(Mat));
        SetTexture(mesh, 2, true, getShaderTypeName(Mat));
        SetTexture(mesh, 3, true, getShaderTypeName(Mat));
        SetTexture(mesh, 4, true, getShaderTypeName(Mat));
        SetTexture(mesh, 5, true, getShaderTypeName(Mat));
        SetTexture(mesh, 6, false, getShaderTypeName(Mat));
        break;
    }
}

void InitTexturesTransparent(GLMesh &mesh)
{
    if (!mesh.textures[0])
    {
        Log::fatal("STKMesh", "Missing texture for material transparent");
        return;
    }
    compressTexture(mesh.textures[0], true);
    if (UserConfigParams::m_azdo)
    {
        if (!mesh.TextureHandles[0])
            mesh.TextureHandles[0] = glGetTextureSamplerHandleARB(getTextureGLuint(mesh.textures[0]), MeshShader::ObjectPass1Shader::getInstance()->SamplersId[0]);
        if (!glIsTextureHandleResidentARB(mesh.TextureHandles[0]))
            glMakeTextureHandleResidentARB(mesh.TextureHandles[0]);
    }
}