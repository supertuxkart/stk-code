#include "stkinstancedscenenode.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/callbacks.hpp"

STKInstancedSceneNode::STKInstancedSceneNode(irr::scene::IMesh* mesh, ISceneNode* parent, irr::scene::ISceneManager* mgr, irr::s32 id,
    const irr::core::vector3df& position,
    const irr::core::vector3df& rotation,
    const irr::core::vector3df& scale) :
    CMeshSceneNode(mesh, parent, mgr, id, position, rotation, scale)
{
    m_ref_count = 0;
    irr_driver->grabAllTextures(mesh);

    if (irr_driver->isGLSL())
    {
        createGLMeshes();
        setAutomaticCulling(0);
    }
}

void STKInstancedSceneNode::cleanGL()
{
    for (u32 i = 0; i < GLmeshes.size(); ++i)
    {
        GLMesh mesh = GLmeshes[i];
        if (!mesh.vertex_buffer)
            continue;
        if (mesh.vao)
            glDeleteVertexArrays(1, &(mesh.vao));
        if (mesh.vao_shadow_pass)
            glDeleteVertexArrays(1, &(mesh.vao_shadow_pass));
        glDeleteBuffers(1, &(mesh.vertex_buffer));
        glDeleteBuffers(1, &(mesh.index_buffer));
    }
    glDeleteBuffers(1, &instances_vbo);
}

STKInstancedSceneNode::~STKInstancedSceneNode()
{
    irr_driver->dropAllTextures(getMesh());
    irr_driver->removeMeshFromCache(getMesh());

    if (irr_driver->isGLSL())
        cleanGL();
}

void STKInstancedSceneNode::createGLMeshes()
{
    for (u32 i = 0; i<Mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        GLmeshes.push_back(allocateMeshBuffer(mb));
        fillLocalBuffer(GLmeshes.back(), mb);
    }
    isMaterialInitialized = false;
}

void STKInstancedSceneNode::initinstancedvaostate(GLMesh &mesh)
{
    mesh.vao = createVAO(mesh.vertex_buffer, mesh.index_buffer, getVTXTYPEFromStride(mesh.Stride));
    glGenBuffers(1, &instances_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, instances_vbo);
    glBufferData(GL_ARRAY_BUFFER, instance_pos.size() * sizeof(float), instance_pos.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
    glVertexAttribDivisor(7, 1);
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
    glVertexAttribDivisor(8, 1);
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (GLvoid*)(6 * sizeof(float)));
    glVertexAttribDivisor(9, 1);

    mesh.vao_shadow_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, getVTXTYPEFromStride(mesh.Stride));
    glBindBuffer(GL_ARRAY_BUFFER, instances_vbo);
    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), 0);
    glVertexAttribDivisor(7, 4);
    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
    glVertexAttribDivisor(8, 4);
    glEnableVertexAttribArray(9);
    glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (GLvoid*)(6 * sizeof(float)));
    glVertexAttribDivisor(9, 4);

    glBindVertexArray(0);
}

void STKInstancedSceneNode::setFirstTimeMaterial()
{
    if (isMaterialInitialized)
        return;
    for (u32 i = 0; i<Mesh->getMeshBufferCount(); ++i)
    {
        scene::IMeshBuffer* mb = Mesh->getMeshBuffer(i);
        if (!mb)
            continue;
        video::E_MATERIAL_TYPE type = mb->getMaterial().MaterialType;

        GLMesh &mesh = GLmeshes[i];
        MeshMaterial MatType = MaterialTypeToMeshMaterial(type, mb->getVertexType());
        initinstancedvaostate(mesh);
        MeshSolidMaterial[MatType].push_back(&mesh);
    }
    isMaterialInitialized = true;
}

void STKInstancedSceneNode::addInstance(const core::vector3df &origin, const core::vector3df &orientation, const core::vector3df &scale)
{
    instance_pos.push_back(origin.X);
    instance_pos.push_back(origin.Y);
    instance_pos.push_back(origin.Z);
    instance_pos.push_back(orientation.X);
    instance_pos.push_back(orientation.Y);
    instance_pos.push_back(orientation.Z);
    instance_pos.push_back(scale.X);
    instance_pos.push_back(scale.Y);
    instance_pos.push_back(scale.Z);
}

core::matrix4 STKInstancedSceneNode::getInstanceTransform(int id)
{
    core::matrix4 mat;

    int offset = id * 9;
    mat.setTranslation(core::vector3df(
        instance_pos[offset],
        instance_pos[offset + 1],
        instance_pos[offset + 2]));
    mat.setRotationDegrees(core::vector3df(
        instance_pos[offset + 3],
        instance_pos[offset + 4],
        instance_pos[offset + 5]));
    mat.setScale(core::vector3df(
        instance_pos[offset + 6],
        instance_pos[offset + 7],
        instance_pos[offset + 8]));

    return mat;
}

void STKInstancedSceneNode::render()
{
    if (!irr_driver->isGLSL())
    {
        CMeshSceneNode::render();
        return;
    }

    setFirstTimeMaterial();

    for (auto mesh : MeshSolidMaterial[MAT_DEFAULT])
        ListInstancedMatDefault::getInstance()->push_back(STK::make_tuple(mesh, instance_pos.size() / 9));

    for (auto mesh : MeshSolidMaterial[MAT_ALPHA_REF])
        ListInstancedMatAlphaRef::getInstance()->push_back(STK::make_tuple(mesh, instance_pos.size() / 9));

    windDir = getWind();
    SunLightProvider * const cb = (SunLightProvider *)irr_driver->getCallback(ES_SUNLIGHT);
    for (auto mesh : MeshSolidMaterial[MAT_GRASS])
        ListInstancedMatGrass::getInstance()->push_back(STK::make_tuple(mesh, instance_pos.size() / 9, windDir, cb->getPosition()));
}
