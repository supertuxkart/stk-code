//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 SuperTuxKart-Team
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef SERVER_ONLY

#include "graphics/stk_mesh.hpp"

#include "graphics/callbacks.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/shaders.hpp"
#include "graphics/stk_tex_manager.hpp"

#include <ISceneManager.h>
#include <IMaterialRenderer.h>


// ============================================================================
Material::ShaderType getMeshMaterialFromType(video::E_MATERIAL_TYPE material_type,
                                              video::E_VERTEX_TYPE tp,
                                              Material* material,
                                              Material* layer2_material)
{
    if (layer2_material != NULL && 
        layer2_material->getShaderType() == Material::SHADERTYPE_SPLATTING)
        return Material::SHADERTYPE_SPLATTING;

    if (tp == video::EVT_SKINNED_MESH)
    {
        switch (material->getShaderType())
        {
        case Material::SHADERTYPE_SOLID:
            if (material_type == Shaders::getShader(ES_NORMAL_MAP))
                return Material::SHADERTYPE_NORMAL_MAP_SKINNED_MESH;
            else
                return Material::SHADERTYPE_SOLID_SKINNED_MESH;
        case Material::SHADERTYPE_ALPHA_TEST:
            return Material::SHADERTYPE_ALPHA_TEST_SKINNED_MESH;
        case Material::SHADERTYPE_SOLID_UNLIT:
            return Material::SHADERTYPE_SOLID_UNLIT_SKINNED_MESH;
        default:
            return Material::SHADERTYPE_SOLID_SKINNED_MESH;
        }
    }

    switch (material->getShaderType())
    {
    default:
        return material->getShaderType();   
    case Material::SHADERTYPE_SOLID:
        if (material_type == Shaders::getShader(ES_NORMAL_MAP))
            return Material::SHADERTYPE_NORMAL_MAP;
        else if (tp == video::EVT_2TCOORDS)
            return Material::SHADERTYPE_DETAIL_MAP;
        return Material::SHADERTYPE_SOLID;
    }
}   // getMeshMaterialFromType

// ----------------------------------------------------------------------------
TransparentMaterial getTransparentMaterialFromType(video::E_MATERIAL_TYPE type,
                                                   video::E_VERTEX_TYPE tp,
                                                   f32 MaterialTypeParam,
                                                   Material* material)
{
    if (tp == video::EVT_SKINNED_MESH)
        return TM_TRANSLUCENT_SKN;

    if (type == Shaders::getShader(ES_DISPLACE))
    {
        if (CVS->isDefferedEnabled())
            return TM_DISPLACEMENT;
        else if (tp == video::EVT_2TCOORDS)
            return TM_TRANSLUCENT_2TC;
        else
            return TM_TRANSLUCENT_STD;
    }
    if (material->getShaderType() == Material::SHADERTYPE_ADDITIVE)
        return TM_ADDITIVE;
    return TM_DEFAULT;
}

// ----------------------------------------------------------------------------
video::E_VERTEX_TYPE getVTXTYPEFromStride(size_t stride)
{
    if (stride == sizeof(video::S3DVertex))
        return video::EVT_STANDARD;
    else if (stride == sizeof(video::S3DVertex2TCoords))
        return video::EVT_2TCOORDS;
    assert(stride == sizeof(video::S3DVertexTangents));
    return video::EVT_TANGENTS;
}   // getVTXTYPEFromStride

// ----------------------------------------------------------------------------
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              getVertexPitchFromType(type), 0);
        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                              getVertexPitchFromType(type), (GLvoid*)12);
        // Color
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                              getVertexPitchFromType(type), (GLvoid*)24);
        // Texcoord
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE,
                              getVertexPitchFromType(type), (GLvoid*)28);
        break;
    case video::EVT_2TCOORDS:
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              getVertexPitchFromType(type), 0);
        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                              getVertexPitchFromType(type), (GLvoid*)12);
        // Color
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 
                              getVertexPitchFromType(type), (GLvoid*)24);
        // Texcoord
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE,
                              getVertexPitchFromType(type), (GLvoid*)28);
        // SecondTexcoord
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 
                              getVertexPitchFromType(type), (GLvoid*)36);
        break;
    case video::EVT_TANGENTS:
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                              getVertexPitchFromType(type), 0);
        // Normal
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                              getVertexPitchFromType(type), (GLvoid*)12);
        // Color
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                              getVertexPitchFromType(type), (GLvoid*)24);
        // Texcoord
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE,
                              getVertexPitchFromType(type), (GLvoid*)28);
        // Tangent
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE,
                              getVertexPitchFromType(type), (GLvoid*)36);
        // Bitangent
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE,
                              getVertexPitchFromType(type), (GLvoid*)48);
        break;
    case video::EVT_SKINNED_MESH:
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), (GLvoid*)12);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, getVertexPitchFromType(type), (GLvoid*)24);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), (GLvoid*)28);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, getVertexPitchFromType(type), (GLvoid*)44);
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_SHORT, getVertexPitchFromType(type), (GLvoid*)60);
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_HALF_FLOAT, GL_FALSE, getVertexPitchFromType(type), (GLvoid*)68);
        break;
    default:
        assert(0 && "Wrong vertex type");
    }
    assert(idx);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx);
    return vao;
}   // createVAO

// ----------------------------------------------------------------------------
GLMesh allocateMeshBuffer(scene::IMeshBuffer* mb, const std::string& debug_name,
                          RenderInfo* render_info)
{
    GLMesh result = {};
    result.m_material = NULL;
    result.m_render_info = NULL;
    if (!mb)
        return result;
    result.mb = mb;
    if (render_info != NULL)
    {
        result.m_render_info = render_info;
        result.m_material = material_manager->getMaterialFor(mb
            ->getMaterial().getTexture(0), mb);
    }
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
    result.texture_trans = core::vector2df(0.0f, 0.0f);

    return result;
}   // allocateMeshBuffer

// ----------------------------------------------------------------------------
static size_t getUnsignedSize(unsigned tp)
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
}   // getUnsignedSize

// ----------------------------------------------------------------------------
void fillLocalBuffer(GLMesh &mesh, scene::IMeshBuffer* mb)
{
    glBindVertexArray(0);
    glGenBuffers(1, &(mesh.vertex_buffer));
    glGenBuffers(1, &(mesh.index_buffer));

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_buffer);

    const void* vertices = mb->getVertices();
    const u32 vertexCount = mb->getVertexCount();
    // This can happen for certain debug structures, e.g. ShowCurve
    if (vertexCount == 0)
        return;

    const c8* vbuf = static_cast<const c8*>(vertices);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * mesh.Stride, vbuf,
                 GL_STREAM_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.index_buffer);
    const void* indices = mb->getIndices();
    mesh.IndexCount = mb->getIndexCount();

    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mesh.IndexCount * getUnsignedSize(mesh.IndexType),
                 indices, GL_STREAM_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}   // fillLocalBuffer

// ----------------------------------------------------------------------------
core::matrix4 computeMVP(const core::matrix4 &model_matrix)
{
    core::matrix4 ModelViewProjectionMatrix = irr_driver->getProjMatrix();
    ModelViewProjectionMatrix *= irr_driver->getViewMatrix();
    ModelViewProjectionMatrix *= model_matrix;
    return ModelViewProjectionMatrix;
}   // computeMVP

// ----------------------------------------------------------------------------
core::vector3df getWindDir()
{
    const float time = irr_driver->getDevice()->getTimer()->getTime() / 1000.0f;
    GrassShaderProvider *gsp = 
        (GrassShaderProvider *)Shaders::getCallback(ES_GRASS);
    return (gsp->getSpeed() * time * vector3df(1., 0., 0.));
}   // getWindDir

// ----------------------------------------------------------------------------
bool isObject(video::E_MATERIAL_TYPE type)
{
    if (type == Shaders::getShader(ES_OBJECTPASS))
        return true;
    if (type == Shaders::getShader(ES_OBJECTPASS_REF))
        return true;
    if (type == Shaders::getShader(ES_OBJECTPASS_RIMLIT))
        return true;
    if (type == Shaders::getShader(ES_NORMAL_MAP))
        return true;
    if (type == Shaders::getShader(ES_SPHERE_MAP))
        return true;
    if (type == Shaders::getShader(ES_SPLATTING))
        return true;
    if (type == Shaders::getShader(ES_GRASS))
        return true;
    if (type == Shaders::getShader(ES_GRASS_REF))
        return true;
    if (type == Shaders::getShader(ES_DISPLACE))
        return true;
    if (type == Shaders::getShader(ES_OBJECT_UNLIT))
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
}   // isObject

// ----------------------------------------------------------------------------
static void setTexture(GLMesh &mesh, unsigned i, bool is_srgb,
                       const std::string &mat_name)
{
    if (!mesh.textures[i])
    {
        Log::error("STKMesh", "Missing texture %d for material %s", i,
                   mat_name.c_str());
        // use unicolor texture to replace missing texture
        mesh.textures[i] = 
                      STKTexManager::getInstance()->getUnicolorTexture(video::SColor(255, 127, 127, 127));
    }
#if !defined(USE_GLES2)
    if (CVS->isAZDOEnabled())
    {
        if (!mesh.TextureHandles[i])
        {
            mesh.TextureHandles[i] = mesh.textures[i]->getHandle();
        }
    }
#endif
}   // setTexture

// ----------------------------------------------------------------------------
static std::string getShaderTypeName(Material::ShaderType mat)
{
    switch (mat)
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
}   // getShaderTypeName

// ----------------------------------------------------------------------------
void initTextures(GLMesh &mesh, Material::ShaderType mat)
{
    switch (mat)
    {
    default:
    case Material::SHADERTYPE_SOLID:
    case Material::SHADERTYPE_ALPHA_TEST:
    case Material::SHADERTYPE_SPHERE_MAP:
    case Material::SHADERTYPE_SOLID_UNLIT:
    case Material::SHADERTYPE_VEGETATION:
    case Material::SHADERTYPE_SOLID_SKINNED_MESH:
    case Material::SHADERTYPE_ALPHA_TEST_SKINNED_MESH:
    case Material::SHADERTYPE_SOLID_UNLIT_SKINNED_MESH:
        setTexture(mesh, 0, true, getShaderTypeName(mat));
        setTexture(mesh, 1, false, getShaderTypeName(mat));
        setTexture(mesh, 2, false, getShaderTypeName(mat));
        break;
    case Material::SHADERTYPE_DETAIL_MAP:
    case Material::SHADERTYPE_NORMAL_MAP:
    case Material::SHADERTYPE_NORMAL_MAP_SKINNED_MESH:
        setTexture(mesh, 0, true, getShaderTypeName(mat));
        setTexture(mesh, 1, false, getShaderTypeName(mat));
        setTexture(mesh, 2, false, getShaderTypeName(mat));
        setTexture(mesh, 3, false, getShaderTypeName(mat));
        break;
    case Material::SHADERTYPE_SPLATTING:
        setTexture(mesh, 0, true, getShaderTypeName(mat));
        setTexture(mesh, 1, false, getShaderTypeName(mat));
        setTexture(mesh, 2, true, getShaderTypeName(mat));
        setTexture(mesh, 3, true, getShaderTypeName(mat));
        setTexture(mesh, 4, true, getShaderTypeName(mat));
        setTexture(mesh, 5, true, getShaderTypeName(mat));
        setTexture(mesh, 6, false, getShaderTypeName(mat));
        setTexture(mesh, 7, false, getShaderTypeName(mat));
        break;
    }
}   // initTextures

// ----------------------------------------------------------------------------
void initTexturesTransparent(GLMesh &mesh)
{
    if (!mesh.textures[0])
    {
        mesh.textures[0] = STKTexManager::getInstance()->getUnicolorTexture(video::SColor(255, 255, 255, 255));
    }
#if !defined(USE_GLES2)
    if (CVS->isAZDOEnabled())
    {
        if (!mesh.TextureHandles[0])
        {
            mesh.TextureHandles[0] = mesh.textures[0]->getHandle();
        }
    }
#endif
}   // initTexturesTransparent

#endif   // !SERVER_ONLY

