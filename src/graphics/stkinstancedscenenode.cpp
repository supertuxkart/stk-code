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
        GLMesh &mesh = GLmeshes.back();
        if (irr_driver->hasARB_base_instance())
        {
            std::pair<unsigned, unsigned> p = VAOManager::getInstance()->getBase(mb);
            mesh.vaoBaseVertex = p.first;
            mesh.vaoOffset = p.second;
            mesh.VAOType = mb->getVertexType();
        }
        else
            fillLocalBuffer(mesh, mb);
    }
    isMaterialInitialized = false;
}

void STKInstancedSceneNode::initinstancedvaostate(GLMesh &mesh)
{
    if (irr_driver->hasARB_base_instance())
        mesh.vaoBaseInstance = VAOManager::getInstance()->appendInstance(InstanceTypeDefault, instanceData);
    else
    {
        mesh.vao = createVAO(mesh.vertex_buffer, mesh.index_buffer, getVTXTYPEFromStride(mesh.Stride));
        glGenBuffers(1, &instances_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, instances_vbo);
        glBufferData(GL_ARRAY_BUFFER, instanceData.size() * sizeof(InstanceData), instanceData.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 0);
        glVertexAttribDivisor(7, 1);
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(3 * sizeof(float)));
        glVertexAttribDivisor(8, 1);
        glEnableVertexAttribArray(9);
        glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(6 * sizeof(float)));
        glVertexAttribDivisor(9, 1);

        mesh.vao_shadow_pass = createVAO(mesh.vertex_buffer, mesh.index_buffer, getVTXTYPEFromStride(mesh.Stride));
        glBindBuffer(GL_ARRAY_BUFFER, instances_vbo);
        glEnableVertexAttribArray(7);
        glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), 0);
        glVertexAttribDivisor(7, 4);
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(3 * sizeof(float)));
        glVertexAttribDivisor(8, 4);
        glEnableVertexAttribArray(9);
        glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (GLvoid*)(6 * sizeof(float)));
        glVertexAttribDivisor(9, 4);

        glBindVertexArray(0);
    }
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
    InstanceData instance = {
        {
            origin.X,
            origin.Y,
            origin.Z
        },
        {
            orientation.X,
            orientation.Y,
            orientation.Z
        },
        {
            scale.X,
            scale.Y,
            scale.Z
        },
        0
    };
    instanceData.push_back(instance);
}

core::matrix4 STKInstancedSceneNode::getInstanceTransform(int id)
{
    core::matrix4 mat;

    const InstanceData &instance = instanceData[id];
    mat.setTranslation(core::vector3df(
        instance.Origin.X,
        instance.Origin.Y,
        instance.Origin.Z));
    mat.setRotationDegrees(core::vector3df(
        instance.Orientation.X,
        instance.Orientation.Y,
        instance.Orientation.Z));
    mat.setScale(core::vector3df(
        instance.Scale.X,
        instance.Scale.Y,
        instance.Scale.Z));

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

    
    for(unsigned i = 0; i < MeshSolidMaterial[MAT_DEFAULT].size(); i++)
    {
        GLMesh *mesh = MeshSolidMaterial[MAT_DEFAULT][i];
        ListInstancedMatDefault::getInstance()->push_back(STK::make_tuple(mesh, instanceData.size()));
    }

    for(unsigned i = 0; i < MeshSolidMaterial[MAT_ALPHA_REF].size(); i++)
    {
        GLMesh *mesh = MeshSolidMaterial[MAT_ALPHA_REF][i];
        ListInstancedMatAlphaRef::getInstance()->push_back(STK::make_tuple(mesh, instanceData.size()));
    }

    windDir = getWind();
    SunLightProvider * const cb = (SunLightProvider *)irr_driver->getCallback(ES_SUNLIGHT);
    for(unsigned i = 0; i < MeshSolidMaterial[MAT_GRASS].size(); i++)
    {
        GLMesh *mesh = MeshSolidMaterial[MAT_GRASS][i];
        ListInstancedMatGrass::getInstance()->push_back(STK::make_tuple(mesh, instanceData.size(), windDir, cb->getPosition()));
    }

    for(unsigned i = 0; i < MeshSolidMaterial[MAT_NORMAL_MAP].size(); i++)
    {
        GLMesh *mesh = MeshSolidMaterial[MAT_NORMAL_MAP][i];
        ListInstancedMatNormalMap::getInstance()->push_back(STK::make_tuple(mesh, instanceData.size()));
    }
}
