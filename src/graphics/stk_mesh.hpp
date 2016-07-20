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


#ifndef HEADER_STK_MESH_H
#define HEADER_STK_MESH_H

#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "utils/singleton.hpp"
#include "utils/tuple.hpp"

#include <IMeshSceneNode.h>
#include <IMesh.h>
#include "../lib/irrlicht/source/Irrlicht/CMeshSceneNode.h"

#include <vector>

class RenderInfo;

enum TransparentMaterial
{
    TM_DEFAULT,
    TM_ADDITIVE,
    TM_DISPLACEMENT,
    TM_COUNT
};   // TransparentMaterial

struct GLMesh
{
    GLuint vao;
    GLuint vertex_buffer;
    GLuint index_buffer;
    video::ITexture *textures[8];
    GLenum PrimitiveType;
    GLenum IndexType;
    size_t IndexCount;
    size_t Stride;
    core::matrix4 TextureMatrix;
    size_t vaoBaseVertex;
    size_t vaoOffset;
    video::E_VERTEX_TYPE VAOType;
    uint64_t TextureHandles[6];
    scene::IMeshBuffer *mb;
    RenderInfo* m_render_info;
    Material* m_material;
#ifdef DEBUG
    std::string debug_name;
#endif
};   // GLMesh

// ----------------------------------------------------------------------------
GLMesh               allocateMeshBuffer(scene::IMeshBuffer* mb,
                                        const std::string& debug_name,
                                        RenderInfo* render_info);
void                 fillLocalBuffer(GLMesh &, scene::IMeshBuffer* mb);
video::E_VERTEX_TYPE getVTXTYPEFromStride(size_t stride);
GLuint               createVAO(GLuint vbo, GLuint idx, video::E_VERTEX_TYPE type);
core::matrix4        computeMVP(const core::matrix4 &ModelViewProjectionMatrix);
bool                 isObject(video::E_MATERIAL_TYPE type);
core::vector3df      getWindDir();


// ----------------------------------------------------------------------------
class STKMeshCommon
{
protected:
    std::string m_debug_name;

public:
    PtrVector<GLMesh, REF> MeshSolidMaterial[Material::SHADERTYPE_COUNT];
    PtrVector<GLMesh, REF> TransparentMesh[TM_COUNT];
    virtual void updateNoGL() = 0;
    virtual void updateGL() = 0;
    virtual bool glow() const = 0;
    virtual bool isImmediateDraw() const { return false; }
};   // STKMeshCommon

// ----------------------------------------------------------------------------
template<typename T, typename... Args>
class MeshList : public Singleton<T>
{
public:
    std::vector<STK::Tuple<Args...> > SolidPass, Shadows[4], RSM;
    void clear()
    {
        SolidPass.clear();
        RSM.clear();
        for (unsigned i = 0; i < 4; i++)
            Shadows[i].clear();
    }
};   // MeshList

// ----------------------------------------------------------------------------
template<typename T>
class InstancedMeshList : public Singleton<T>
{
public:
    std::vector<GLMesh *> SolidPass, Shadows[4], RSM;
    void clear()
    {
        SolidPass.clear();
        RSM.clear();
        for (unsigned i = 0; i < 4; i++)
            Shadows[i].clear();
    }
};   // InstancedMeshList

// ----------------------------------------------------------------------------
class ListMatDefault : public MeshList<ListMatDefault, GLMesh *, core::matrix4,
                                      core::matrix4, core::matrix4>
{};

// ----------------------------------------------------------------------------
class ListInstancedMatDefault : public InstancedMeshList<ListInstancedMatDefault>
{};

// ----------------------------------------------------------------------------
class ListMatAlphaRef : public MeshList<ListMatAlphaRef, GLMesh *, core::matrix4,
                                        core::matrix4, core::matrix4>
{};

// ----------------------------------------------------------------------------
class ListInstancedMatAlphaRef : public InstancedMeshList<ListInstancedMatAlphaRef>
{};

// ----------------------------------------------------------------------------
class ListMatNormalMap : public MeshList<ListMatNormalMap, GLMesh *, core::matrix4,
                                         core::matrix4, core::matrix4>
{};

// ----------------------------------------------------------------------------
class ListInstancedMatNormalMap : public InstancedMeshList<ListInstancedMatNormalMap>
{};

// ----------------------------------------------------------------------------
class ListMatGrass : public MeshList<ListMatGrass, GLMesh *, core::matrix4, 
                                     core::matrix4, core::vector3df>
{};

// ----------------------------------------------------------------------------
class ListInstancedMatGrass : public InstancedMeshList<ListInstancedMatGrass>
{};

// ----------------------------------------------------------------------------
class ListMatSphereMap : public MeshList<ListMatSphereMap, GLMesh *,
                                         core::matrix4, core::matrix4,
                                         core::matrix4>
{};

// ----------------------------------------------------------------------------
class ListInstancedMatSphereMap : public InstancedMeshList<ListInstancedMatSphereMap>
{};

// ----------------------------------------------------------------------------
class ListMatSplatting : public MeshList<ListMatSplatting, GLMesh *,
                                         core::matrix4, core::matrix4>
{};

// ----------------------------------------------------------------------------
class ListMatUnlit : public MeshList<ListMatUnlit, GLMesh *, core::matrix4,
                                     core::matrix4, core::matrix4>
{};

// ----------------------------------------------------------------------------
class ListInstancedMatUnlit : public InstancedMeshList<ListInstancedMatUnlit>
{};

// ----------------------------------------------------------------------------
class ListMatDetails : public MeshList<ListMatDetails, GLMesh *, core::matrix4,
                                       core::matrix4, core::matrix4>
{};

// ----------------------------------------------------------------------------
class ListInstancedMatDetails : public InstancedMeshList<ListInstancedMatDetails>
{};

// ----------------------------------------------------------------------------
// Transparent
template <typename T, typename ...Args>
class MiscList : public Singleton<T>, public std::vector<STK::Tuple<Args...> >
{};

// ----------------------------------------------------------------------------
class ListBlendTransparent : public MiscList<ListBlendTransparent, GLMesh *,
                                             core::matrix4, core::matrix4>
{};

// ----------------------------------------------------------------------------
class ListAdditiveTransparent : public MiscList<ListAdditiveTransparent,
                                                GLMesh *, core::matrix4,
                                                core::matrix4>
{};

// ----------------------------------------------------------------------------
class ListBlendTransparentFog : public MiscList<ListBlendTransparentFog,
                                                GLMesh *, core::matrix4,
                                                core::matrix4, float, float,
                                                float, float, float,
                                                video::SColorf>
{};

// ----------------------------------------------------------------------------
class ListAdditiveTransparentFog : public MiscList<ListAdditiveTransparentFog,
                                                   GLMesh *, core::matrix4,
                                                   core::matrix4, float, float,
                                                   float, float, float,
                                                   video::SColorf>
{};

// ----------------------------------------------------------------------------
class ListDisplacement : public MiscList<ListDisplacement, GLMesh *,
                                         core::matrix4>
{};

// ----------------------------------------------------------------------------
class ListInstancedGlow : public Singleton<ListInstancedGlow>
                        , public std::vector<GLMesh *>
{};

// ----------------------------------------------------------------------------
Material::ShaderType getMeshMaterialFromType(video::E_MATERIAL_TYPE MaterialType,
                                             video::E_VERTEX_TYPE tp,
                                             Material* material,
                                             Material* layer2Material);
// ----------------------------------------------------------------------------
TransparentMaterial getTransparentMaterialFromType(video::E_MATERIAL_TYPE,
                                                   f32 MaterialTypeParam,
                                                   Material* material);

// ----------------------------------------------------------------------------
void initTextures(GLMesh &mesh, Material::ShaderType);
// ----------------------------------------------------------------------------
void initTexturesTransparent(GLMesh &mesh);

#endif // STKMESH_H
