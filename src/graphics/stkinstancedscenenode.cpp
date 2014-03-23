#include "stkinstancedscenenode.hpp"
#include "graphics/irr_driver.hpp"

STKInstancedSceneNode::STKInstancedSceneNode(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr, irr::s32 id,
    const irr::core::vector3df& position,
    const irr::core::vector3df& rotation,
    const irr::core::vector3df& scale) :
    CMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale)
{
    createGLMeshes();
    setAutomaticCulling(0);
}

void STKInstancedSceneNode::createGLMeshes()
{
    for (u32 i = 0; i<Mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        GLmeshes.push_back(allocateMeshBuffer(mb));
    }
    isMaterialInitialized = false;
}

void STKInstancedSceneNode::initinstancedvaostate(GLMesh &mesh, GeometricMaterial GeoMat, ShadedMaterial ShadedMat)
{
    glGenVertexArrays(1, &mesh.vao_first_pass);
    glBindVertexArray(mesh.vao_first_pass);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_buffer);
    glEnableVertexAttribArray(MeshShader::InstancedObjectPass1Shader::attrib_position);
    glEnableVertexAttribArray(MeshShader::InstancedObjectPass1Shader::attrib_normal);
    glVertexAttribPointer(MeshShader::InstancedObjectPass1Shader::attrib_position, 3, GL_FLOAT, GL_FALSE, mesh.Stride, 0);
    glVertexAttribPointer(MeshShader::InstancedObjectPass1Shader::attrib_normal, 3, GL_FLOAT, GL_FALSE, mesh.Stride, (GLvoid*)12);

    glGenBuffers(1, &instances_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, instances_vbo);
    glBufferData(GL_ARRAY_BUFFER, instance_pos.size() * sizeof(float), instance_pos.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(MeshShader::InstancedObjectPass1Shader::attrib_origin);
    glVertexAttribPointer(MeshShader::InstancedObjectPass1Shader::attrib_origin, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    glVertexAttribDivisor(MeshShader::InstancedObjectPass1Shader::attrib_origin, 1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.index_buffer);

    glGenVertexArrays(1, &mesh.vao_second_pass);
    glBindVertexArray(mesh.vao_second_pass);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertex_buffer);
    glEnableVertexAttribArray(MeshShader::InstancedObjectPass2Shader::attrib_position);
    glEnableVertexAttribArray(MeshShader::InstancedObjectPass2Shader::attrib_texcoord);
    glVertexAttribPointer(MeshShader::InstancedObjectPass2Shader::attrib_position, 3, GL_FLOAT, GL_FALSE, mesh.Stride, 0);
    glVertexAttribPointer(MeshShader::InstancedObjectPass2Shader::attrib_texcoord, 3, GL_FLOAT, GL_FALSE, mesh.Stride, (GLvoid*)28);

    glBindBuffer(GL_ARRAY_BUFFER, instances_vbo);
    glEnableVertexAttribArray(MeshShader::InstancedObjectPass2Shader::attrib_origin);
    glVertexAttribPointer(MeshShader::InstancedObjectPass2Shader::attrib_origin, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    glVertexAttribDivisor(MeshShader::InstancedObjectPass2Shader::attrib_origin, 1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.index_buffer);
}

void STKInstancedSceneNode::setFirstTimeMaterial()
{
    if (isMaterialInitialized)
        return;
    irr::video::IVideoDriver* driver = irr_driver->getVideoDriver();
    for (u32 i = 0; i<Mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        if (!mb)
            continue;
        video::E_MATERIAL_TYPE type = mb->getMaterial().MaterialType;

        GLMesh &mesh = GLmeshes[i];
        GeometricMaterial GeometricType = MaterialTypeToGeometricMaterial(type);
        ShadedMaterial ShadedType = MaterialTypeToShadedMaterial(type, mesh.textures);
        initinstancedvaostate(mesh, GeometricType, ShadedType);
    }
    isMaterialInitialized = true;
    printf("instance count : %d\n", instance_pos.size() / 3);
}

void STKInstancedSceneNode::addWorldMatrix(const core::vector3df &v)
{
    instance_pos.push_back(v.X);
    instance_pos.push_back(v.Y);
    instance_pos.push_back(v.Z);
}

void STKInstancedSceneNode::render()
{
    irr::video::IVideoDriver* driver = irr_driver->getVideoDriver();
    setFirstTimeMaterial();
//    AbsoluteTransformation.setTranslation(vector3df(0., 0., 10.));
//    driver->setTransform(video::ETS_WORLD, AbsoluteTransformation);

    if (irr_driver->getPhase() == SOLID_NORMAL_AND_DEPTH_PASS)
    {
        ModelViewProjectionMatrix = irr_driver->getVideoDriver()->getTransform(video::ETS_PROJECTION);
        ModelViewProjectionMatrix *= irr_driver->getVideoDriver()->getTransform(video::ETS_VIEW);

        glUseProgram(MeshShader::InstancedObjectPass1Shader::Program);
        for (unsigned i = 0; i < GLmeshes.size(); i++)
        {
            GLMesh &mesh = GLmeshes[i];

            irr_driver->IncreaseObjectCount();
            GLenum ptype = mesh.PrimitiveType;
            GLenum itype = mesh.IndexType;
            size_t count = mesh.IndexCount;

            MeshShader::InstancedObjectPass1Shader::setUniforms(ModelViewProjectionMatrix, irr_driver->getVideoDriver()->getTransform(video::ETS_VIEW));

            glBindVertexArray(mesh.vao_first_pass);
            glDrawElementsInstanced(ptype, count, itype, 0, instance_pos.size() / 3);
        }
        return;
    }

    if (irr_driver->getPhase() == SOLID_LIT_PASS)
    {
        glUseProgram(MeshShader::InstancedObjectPass2Shader::Program);
        for (unsigned i = 0; i < GLmeshes.size(); i++)
        {
            GLMesh &mesh = GLmeshes[i];

            irr_driver->IncreaseObjectCount();
            GLenum ptype = mesh.PrimitiveType;
            GLenum itype = mesh.IndexType;
            size_t count = mesh.IndexCount;

            setTexture(MeshShader::InstancedObjectPass2Shader::TU_Albedo, mesh.textures[0], GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, true);

            MeshShader::InstancedObjectPass2Shader::setUniforms(ModelViewProjectionMatrix, core::matrix4::EM4CONST_IDENTITY);

            assert(mesh.vao_second_pass);
            glBindVertexArray(mesh.vao_second_pass);
            glDrawElementsInstanced(ptype, count, itype, 0, instance_pos.size() / 3);
        }
        return;
    }
}