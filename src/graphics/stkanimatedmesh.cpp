#include "graphics/stkanimatedmesh.hpp"
#include <ISceneManager.h>
#include <IMaterialRenderer.h>
#include <ISkinnedMesh.h>
#include "graphics/irr_driver.hpp"
#include "config/user_config.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"

using namespace irr;

STKAnimatedMesh::STKAnimatedMesh(irr::scene::IAnimatedMesh* mesh, irr::scene::ISceneNode* parent,
irr::scene::ISceneManager* mgr, s32 id,
const core::vector3df& position,
const core::vector3df& rotation,
const core::vector3df& scale) :
    CAnimatedMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale)
{
	firstTime = true;
}

void STKAnimatedMesh::setMesh(scene::IAnimatedMesh* mesh)
{
	firstTime = true;
	GLmeshes.clear();
    for (unsigned i = 0; i < FPSM_COUNT; i++)
        GeometricMesh[i].clear();
    for (unsigned i = 0; i < SM_COUNT; i++)
        ShadedMesh[i].clear();
	CAnimatedMeshSceneNode::setMesh(mesh);
}

void STKAnimatedMesh::drawSolidPass1(const GLMesh &mesh, GeometricMaterial type)
{
    switch (type)
    {
    case FPSM_ALPHA_REF_TEXTURE:
        drawObjectRefPass1(mesh, ModelViewProjectionMatrix, TransposeInverseModelView, mesh.TextureMatrix);
        break;
    case FPSM_DEFAULT:
        drawObjectPass1(mesh, ModelViewProjectionMatrix, TransposeInverseModelView);
        break;
    default:
        assert(0 && "Wrong geometric material");
        break;
    }
}

void STKAnimatedMesh::drawSolidPass2(const GLMesh &mesh, ShadedMaterial type)
{
    switch (type)
    {
    case SM_ALPHA_REF_TEXTURE:
        drawObjectRefPass2(mesh, ModelViewProjectionMatrix, mesh.TextureMatrix);
        break;
    case SM_RIMLIT:
        drawObjectRimLimit(mesh, ModelViewProjectionMatrix, TransposeInverseModelView, mesh.TextureMatrix);
        break;
    case SM_UNLIT:
        drawObjectUnlit(mesh, ModelViewProjectionMatrix);
        break;
    case SM_DETAILS:
        drawDetailledObjectPass2(mesh, ModelViewProjectionMatrix);
        break;
    case SM_DEFAULT:
        drawObjectPass2(mesh, ModelViewProjectionMatrix, mesh.TextureMatrix);
        break;
    default:
        assert(0 && "Wrong shaded material");
        break;
    }
}

void STKAnimatedMesh::render()
{
	video::IVideoDriver* driver = SceneManager->getVideoDriver();

	bool isTransparentPass =
		SceneManager->getSceneNodeRenderPass() == scene::ESNRP_TRANSPARENT;

	++PassCount;

	scene::IMesh* m = getMeshForCurrentFrame();

	if (m)
	{
		Box = m->getBoundingBox();
	}
	else
	{
		Log::error("animated mesh", "Animated Mesh returned no mesh to render.");
		return;
	}

	driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

    if (firstTime)
    {
        for (u32 i = 0; i < m->getMeshBufferCount(); ++i)
        {
            scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
            GLmeshes.push_back(allocateMeshBuffer(mb));
        }

        for (u32 i = 0; i < m->getMeshBufferCount(); ++i)
        {
            scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
            if (!mb)
                continue;
            video::E_MATERIAL_TYPE type = mb->getMaterial().MaterialType;
            video::IMaterialRenderer* rnd = driver->getMaterialRenderer(type);
            if (!isObject(type))
            {
#ifdef DEBUG
                Log::warn("material", "Unhandled (animated) material type : %d", type);
#endif
                continue;
            }
            GLMesh &mesh = GLmeshes[i];
            if (rnd->isTransparent())
            {
                TransparentMaterial TranspMat = MaterialTypeToTransparentMaterial(type);
                initvaostate(mesh, TranspMat);
                TransparentMesh[TranspMat].push_back(&mesh);
            }
            else
            {
                GeometricMaterial GeometricType = MaterialTypeToGeometricMaterial(type);
                ShadedMaterial ShadedType = MaterialTypeToShadedMaterial(type, mesh.textures);
                initvaostate(mesh, GeometricType, ShadedType);
                GeometricMesh[GeometricType].push_back(&mesh);
                ShadedMesh[ShadedType].push_back(&mesh);
            }
        }
    }
	firstTime = false;

    for (u32 i = 0; i<m->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = m->getMeshBuffer(i);
        const video::SMaterial& material = ReadOnlyMaterials ? mb->getMaterial() : Materials[i];
        if (isObject(material.MaterialType))
        {
           if (irr_driver->getPhase() == SOLID_NORMAL_AND_DEPTH_PASS)
           {
               glBindVertexArray(0);
               glBindBuffer(GL_ARRAY_BUFFER, GLmeshes[i].vertex_buffer);
               glBufferSubData(GL_ARRAY_BUFFER, 0, mb->getVertexCount() * GLmeshes[i].Stride, mb->getVertices());
           }
        }
        if (mb)
            GLmeshes[i].TextureMatrix = getMaterial(i).getTextureMatrix(0);

        video::IMaterialRenderer* rnd = driver->getMaterialRenderer(Materials[i].MaterialType);
        bool transparent = (rnd && rnd->isTransparent());

       // only render transparent buffer if this is the transparent render pass
       // and solid only in solid pass
       if (transparent != isTransparentPass)
          continue;

       if (RenderFromIdentity)
         driver->setTransform(video::ETS_WORLD, core::IdentityMatrix);
       else if (Mesh->getMeshType() == scene::EAMT_SKINNED)
         driver->setTransform(video::ETS_WORLD, AbsoluteTransformation * ((scene::SSkinMeshBuffer*)mb)->Transformation);

    }

    if (irr_driver->getPhase() == SOLID_NORMAL_AND_DEPTH_PASS)
    {
        computeMVP(ModelViewProjectionMatrix);
        computeTIMV(TransposeInverseModelView);

        glUseProgram(MeshShader::ObjectPass1Shader::Program);
        for (unsigned i = 0; i < GeometricMesh[FPSM_DEFAULT].size(); i++)
            drawSolidPass1(*GeometricMesh[FPSM_DEFAULT][i], FPSM_DEFAULT);

        glUseProgram(MeshShader::ObjectRefPass1Shader::Program);
        for (unsigned i = 0; i < GeometricMesh[FPSM_ALPHA_REF_TEXTURE].size(); i++)
            drawSolidPass1(*GeometricMesh[FPSM_ALPHA_REF_TEXTURE][i], FPSM_ALPHA_REF_TEXTURE);

        return;
    }

    if (irr_driver->getPhase() == SOLID_LIT_PASS)
    {
      glUseProgram(MeshShader::ObjectPass2Shader::Program);
      for (unsigned i = 0; i < ShadedMesh[SM_DEFAULT].size(); i++)
          drawSolidPass2(*ShadedMesh[SM_DEFAULT][i], SM_DEFAULT);

      glUseProgram(MeshShader::ObjectRefPass2Shader::Program);
      for (unsigned i = 0; i < ShadedMesh[SM_ALPHA_REF_TEXTURE].size(); i++)
          drawSolidPass2(*ShadedMesh[SM_ALPHA_REF_TEXTURE][i], SM_ALPHA_REF_TEXTURE);

      glUseProgram(MeshShader::ObjectRimLimitShader::Program);
      for (unsigned i = 0; i < ShadedMesh[SM_RIMLIT].size(); i++)
          drawSolidPass2(*ShadedMesh[SM_RIMLIT][i], SM_RIMLIT);

      glUseProgram(MeshShader::ObjectUnlitShader::Program);
      for (unsigned i = 0; i < ShadedMesh[SM_UNLIT].size(); i++)
          drawSolidPass2(*ShadedMesh[SM_UNLIT][i], SM_UNLIT);

      glUseProgram(MeshShader::DetailledObjectPass2Shader::Program);
      for (unsigned i = 0; i < ShadedMesh[SM_DETAILS].size(); i++)
          drawSolidPass2(*ShadedMesh[SM_DETAILS][i], SM_DETAILS);

      return;
    }

    if (irr_driver->getPhase() == SHADOW_PASS)
    {
        glUseProgram(MeshShader::ShadowShader::Program);
        for (unsigned i = 0; i < GeometricMesh[FPSM_DEFAULT].size(); i++)
            drawShadow(*GeometricMesh[FPSM_DEFAULT][i]);

        glUseProgram(MeshShader::RefShadowShader::Program);
        for (unsigned i = 0; i < GeometricMesh[FPSM_ALPHA_REF_TEXTURE].size(); i++)
            drawShadowRef(*GeometricMesh[FPSM_ALPHA_REF_TEXTURE][i]);
        return;
    }

    if (irr_driver->getPhase() == TRANSPARENT_PASS)
    {
        computeMVP(ModelViewProjectionMatrix);

        glUseProgram(MeshShader::BubbleShader::Program);
        for (unsigned i = 0; i < TransparentMesh[TM_BUBBLE].size(); i++)
            drawBubble(*TransparentMesh[TM_BUBBLE][i], ModelViewProjectionMatrix);

        glUseProgram(MeshShader::TransparentShader::Program);
        for (unsigned i = 0; i < TransparentMesh[TM_DEFAULT].size(); i++)
            drawTransparentObject(*TransparentMesh[TM_DEFAULT][i], ModelViewProjectionMatrix, (*TransparentMesh[TM_DEFAULT][i]).TextureMatrix);
        return;
    }
}
