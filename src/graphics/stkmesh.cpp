#include "stkmesh.hpp"
#include "graphics/irr_driver.hpp"
#include "tracks/track.hpp"
#include <ISceneManager.h>
#include <IMaterialRenderer.h>
#include "config/user_config.hpp"
#include "graphics/callbacks.hpp"
#include "utils/helpers.hpp"
#include "graphics/camera.hpp"
#include "modes/world.hpp"

GeometricMaterial MaterialTypeToGeometricMaterial(video::E_MATERIAL_TYPE MaterialType, video::E_VERTEX_TYPE tp)
{
    if (MaterialType == irr_driver->getShader(ES_NORMAL_MAP))
        return FPSM_NORMAL_MAP;
    else if (MaterialType == irr_driver->getShader(ES_OBJECTPASS_REF) || MaterialType == video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF)
        return FPSM_ALPHA_REF_TEXTURE;
    else if (MaterialType == irr_driver->getShader(ES_GRASS) || MaterialType == irr_driver->getShader(ES_GRASS_REF))
        return FPSM_GRASS;
    else if (tp == video::EVT_2TCOORDS)
        return FPSM_DEFAULT_2TCOORD;
    assert(tp == video::EVT_STANDARD);
    return FPSM_DEFAULT_STANDARD;
}

ShadedMaterial MaterialTypeToShadedMaterial(video::E_MATERIAL_TYPE type, video::ITexture **textures, video::E_VERTEX_TYPE tp)
{
    if (type == irr_driver->getShader(ES_SPHERE_MAP))
        return SM_SPHEREMAP;
    else if (type == irr_driver->getShader(ES_SPLATTING))
        return SM_SPLATTING;
    else if (type == irr_driver->getShader(ES_OBJECTPASS_REF) || type == video::EMT_TRANSPARENT_ALPHA_CHANNEL_REF)
        return SM_ALPHA_REF_TEXTURE;
    else if (type == irr_driver->getShader(ES_GRASS) || type == irr_driver->getShader(ES_GRASS_REF))
        return SM_GRASS;
    else if (type == irr_driver->getShader(ES_OBJECT_UNLIT))
        return SM_UNLIT;
    else if (tp == video::EVT_2TCOORDS)
        return SM_DETAILS;
    else if (tp == video::EVT_TANGENTS)
        return SM_DEFAULT_TANGENT;
    return SM_DEFAULT_STANDARD;
}

TransparentMaterial MaterialTypeToTransparentMaterial(video::E_MATERIAL_TYPE type, f32 MaterialTypeParam)
{
    if (type == irr_driver->getShader(ES_BUBBLES))
        return TM_BUBBLE;
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

GLMesh allocateMeshBuffer(scene::IMeshBuffer* mb)
{
    GLMesh result = {};
    if (!mb)
        return result;

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
    for (unsigned i = 0; i < 6; i++)
        result.textures[i] = mb->getMaterial().getTexture(i);
    result.TextureMatrix = 0;
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
    glBufferData(GL_ARRAY_BUFFER, vertexCount * mesh.Stride, vbuf, GL_STATIC_DRAW);
    assert(vertexCount);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.index_buffer);
    const void* indices = mb->getIndices();

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.IndexCount * getUnsignedSize(mesh.IndexType), indices, GL_STATIC_DRAW);

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

core::matrix4 computeTIMV(const core::matrix4 &ModelMatrix)
{
    core::matrix4 TransposeInverseModelView = irr_driver->getViewMatrix();
    TransposeInverseModelView *= ModelMatrix;
    TransposeInverseModelView.makeInverse();
    TransposeInverseModelView = TransposeInverseModelView.getTransposed();
    return TransposeInverseModelView;
}

core::vector3df getWind()
{
    const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;
    GrassShaderProvider *gsp = (GrassShaderProvider *)irr_driver->getCallback(ES_GRASS);
    float m_speed = gsp->getSpeed();

    return m_speed * vector3df(1., 0., 0.) * cos(time);
}

void drawGrassPass1(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, const core::matrix4 &TransposeInverseModelView, core::vector3df windDir)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;
    assert(mesh.VAOType == video::EVT_STANDARD);

    compressTexture(mesh.textures[0], true);
    setTexture(0, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

    MeshShader::GrassPass1Shader::setUniforms(ModelViewProjectionMatrix, TransposeInverseModelView, windDir, 0);
    glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh.vaoOffset, mesh.vaoBaseVertex);
}

void drawGrassPass2(const GLMesh &mesh, const core::matrix4 & ModelViewProjectionMatrix, core::vector3df windDir)
{
    irr_driver->IncreaseObjectCount();
    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;
    assert(mesh.VAOType == video::EVT_STANDARD);

    if (!mesh.textures[0])
        const_cast<GLMesh &>(mesh).textures[0] = getUnicolorTexture(video::SColor(255, 255, 255, 255));
    compressTexture(mesh.textures[0], true);
    setTexture(MeshShader::GrassPass2Shader::TU_Albedo, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);
    if (irr_driver->getLightViz())
    {
      GLint swizzleMask[] = {GL_ONE, GL_ONE, GL_ONE, GL_ALPHA};
      glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    }
    else
    {
      GLint swizzleMask[] = {GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA};
      glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
    }

    MeshShader::GrassPass2Shader::setUniforms(ModelViewProjectionMatrix, windDir);
    glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh.vaoOffset, mesh.vaoBaseVertex);
}

void drawBubble(const GLMesh &mesh, const core::matrix4 &ModelViewProjectionMatrix)
{
    irr_driver->IncreaseObjectCount();
    const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;
    float transparency = 1.;

    GLenum ptype = mesh.PrimitiveType;
    GLenum itype = mesh.IndexType;
    size_t count = mesh.IndexCount;

    compressTexture(mesh.textures[0], true);
    setTexture(0, getTextureGLuint(mesh.textures[0]), GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

    MeshShader::BubbleShader::setUniforms(ModelViewProjectionMatrix, 0, time, transparency);
    glDrawElementsBaseVertex(ptype, count, itype, (GLvoid *)mesh.vaoOffset, mesh.vaoBaseVertex);
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
    if (type == irr_driver->getShader(ES_BUBBLES))
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

void initvaostate(GLMesh &mesh, GeometricMaterial GeoMat, ShadedMaterial ShadedMat)
{
    mesh.vao = createVAO(mesh.vertex_buffer, mesh.index_buffer, getVTXTYPEFromStride(mesh.Stride));
}

void initvaostate(GLMesh &mesh, TransparentMaterial TranspMat)
{
    mesh.vao = createVAO(mesh.vertex_buffer, mesh.index_buffer, getVTXTYPEFromStride(mesh.Stride));
}

std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4> > ListDefaultStandardG::Arguments;
std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4> > ListDefault2TCoordG::Arguments;
std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, core::matrix4> > ListAlphaRefG::Arguments;
std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4> > ListNormalG::Arguments;

std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, video::SColorf> > ListDefaultStandardSM::Arguments;
std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, video::SColorf> > ListDefaultTangentSM::Arguments;
std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, video::SColorf> > ListAlphaRefSM::Arguments;
std::vector<std::tuple<GLMesh *, core::matrix4> > ListSplattingSM::Arguments;
std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, video::SColorf> > ListSphereMapSM::Arguments;
std::vector<std::tuple<GLMesh *, core::matrix4> > ListUnlitSM::Arguments;
std::vector<std::tuple<GLMesh *, core::matrix4, video::SColorf> > ListDetailSM::Arguments;
std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4> > ListBlendTransparent::Arguments;
std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4> > ListAdditiveTransparent::Arguments;
std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, float, float, float, float, float, core::vector3df, core::vector3df> > ListBlendTransparentFog::Arguments;
std::vector<std::tuple<GLMesh *, core::matrix4, core::matrix4, float, float, float, float, float, core::vector3df, core::vector3df> > ListAdditiveTransparentFog::Arguments;
