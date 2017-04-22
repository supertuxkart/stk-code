
#include "graphics/CBatchingMesh.hpp"

namespace irr
{
namespace scene
{

CBatchingMesh::CBatchingMesh()
 : Box(core::vector3df(0,0,0)), IsDirty(false), IsFinal(false)
{

}

CBatchingMesh::~CBatchingMesh()
{
    u32 i;
    for (i=0; i < DestBuffers.size(); ++i)
        DestBuffers[i].Buffer->drop();

    for (i=0; i < SourceBuffers.size(); ++i)
        SourceBuffers[i]->drop();
}

bool CBatchingMesh::isDirty(s32 id)
{
    if ((u32)id > DestBuffers.size())
        return IsDirty;
    else
        return DestBuffers[id].IsDirty;
}

//! refreshes the internal buffers from source
void CBatchingMesh::update()
{
    // allocate the index and vertex arrays
    u32 i;
    for (i=0; i<DestBuffers.size(); ++i)
    {
        if (DestBuffers[i].IndexCount != DestBuffers[i].Buffer->getIndexCount() ||
            DestBuffers[i].VertexCount != DestBuffers[i].Buffer->getVertexCount())
        {
            DestBuffers[i].IsDirty = true;

            switch (DestBuffers[i].VertexType)
            {
            case video::EVT_STANDARD:
            {
                SMeshBuffer* mb = (SMeshBuffer*)DestBuffers[i].Buffer;
                mb->Vertices.set_used(DestBuffers[i].VertexCount);
                mb->Indices.set_used(DestBuffers[i].IndexCount);
                break;
            }
            case video::EVT_2TCOORDS:
            {
                SMeshBufferLightMap* mb = (SMeshBufferLightMap*)DestBuffers[i].Buffer;
                mb->Vertices.set_used(DestBuffers[i].VertexCount);
                mb->Indices.set_used(DestBuffers[i].IndexCount);
                break;
            }
            case video::EVT_TANGENTS:
            {
                SMeshBufferTangents* mb = (SMeshBufferTangents*)DestBuffers[i].Buffer;
                mb->Vertices.set_used(DestBuffers[i].VertexCount);
                mb->Indices.set_used(DestBuffers[i].IndexCount);
                break;
            }
            default: // shouldn't ever happen
                continue;
            }
        }
    }

    // refresh dirty buffers from source
    for (i=0; i<BufferReferences.size(); ++i)
    {
        if (DestBuffers[BufferReferences[i].DestReference].IsDirty)
        {
            updateDestFromSourceBuffer(i);
        }
    }

    // calculate bounding boxes
    for (i=0; i< DestBuffers.size(); ++i)
    {
        if (DestBuffers[i].IsDirty)
        {
            recalculateDestBufferBoundingBox(i);
            // reset dirty state too
            DestBuffers[i].IsDirty = false;
        }
    }

    IsDirty = false;
    recalculateBoundingBox();
}

//! adds a mesh to the buffers with the given offset
/** \Returns Returns an array of ID numbers */
core::array<s32> CBatchingMesh::addMesh(IMesh* mesh, core::vector3df pos, core::vector3df rot, core::vector3df scale)
{
    core::matrix4 m;
    m.setRotationDegrees(rot);
    m.setTranslation(pos);

    core::matrix4 scalem;
    scalem.setScale(scale);
    m *= scalem;

    return addMesh(mesh, m);
}

//! adds a mesh with the given transformation
core::array<s32> CBatchingMesh::addMesh(IMesh* mesh, const core::matrix4 &transform)
{
    core::array<s32> bufferNos;

    if (!mesh)
        return bufferNos;

    u32 i;
    for (i=0; i<mesh->getMeshBufferCount(); ++i)
        bufferNos.push_back(addMeshBuffer(mesh->getMeshBuffer(i), transform));

    return bufferNos;
}

//! adds a mesh buffer with the given transformation
/** \Return Returns the ID of this mesh buffer */
s32 CBatchingMesh::addMeshBuffer(IMeshBuffer* buffer, core::vector3df pos, core::vector3df rot, core::vector3df scale)
{
    core::matrix4 m;
    m.setRotationDegrees(rot);
    m.setTranslation(pos);

    core::matrix4 scalem;
    scalem.setScale(scale);
    m *= scalem;

    return addMeshBuffer(buffer, m);
}

//! adds a mesh with the given transformation
/** \Return Returns the ID of this mesh buffer */
s32 CBatchingMesh::addMeshBuffer(IMeshBuffer* buffer, const core::matrix4 &transform)
{
    if (!buffer || IsFinal)
        return -1;

    u32 i;
    video::SMaterial m = buffer->getMaterial();

    // find material
    bool found=false;
    video::E_VERTEX_TYPE vt = buffer->getVertexType();
    for (i=0; i<MaterialReferences.size(); ++i)
    {
        if (MaterialReferences[i].VertexType == vt &&
            MaterialReferences[i].Material == m)
        {
            // will there be too many vertices in the buffer?
            u32 newTotalI = buffer->getIndexCount() + DestBuffers[ MaterialReferences[i].BufferIndex ].IndexCount;
            u32 newTotalV = buffer->getVertexCount() + DestBuffers[ MaterialReferences[i].BufferIndex ].VertexCount;

            if ( newTotalI < 65536*3 && newTotalV < 65536)
            {
                found = true;
                DestBuffers[ MaterialReferences[i].BufferIndex ].IndexCount = newTotalI;
                DestBuffers[ MaterialReferences[i].BufferIndex ].VertexCount = newTotalV;
                break;
            }
        }
    }

    if (!found)
    {
        // we need a new destination buffer and material reference
        IMeshBuffer *mb=0;

        SMaterialReference r;
        r.Material = m;
        r.VertexType = vt;
        r.BufferIndex = DestBuffers.size();
        switch (vt)
        {
        case video::EVT_STANDARD:
            mb = (IMeshBuffer*)new SMeshBuffer();
            mb->getMaterial() = m;
            break;
        case video::EVT_2TCOORDS:
            mb = (IMeshBuffer*)new SMeshBufferLightMap();
            mb->getMaterial() = m;
            break;
        case video::EVT_TANGENTS:
            mb = (IMeshBuffer*)new SMeshBufferTangents();
            mb->getMaterial() = m;
            break;
        default: // unknown vertex type
            return -1;
        }
        i = MaterialReferences.size();
        MaterialReferences.push_back(r);

        SDestBufferReference db;
        db.Buffer = mb;
        db.IndexCount = buffer->getIndexCount();
        db.VertexCount = buffer->getVertexCount();
        db.IsDirty = true;
        db.VertexType = vt;

        DestBuffers.push_back(db);
    }
    // now we add the mesh reference
    SBufferReference r;
    r.DestReference = i;
    r.SourceBuffer = buffer;
    r.Transform = transform;
    r.IndexCount = buffer->getIndexCount();
    r.VertexCount = buffer->getVertexCount();
    r.FirstIndex = DestBuffers[ MaterialReferences[i].BufferIndex ].IndexCount - r.IndexCount;
    r.FirstVertex = DestBuffers[ MaterialReferences[i].BufferIndex ].VertexCount - r.VertexCount;
    r.Initialized = false;
    BufferReferences.push_back(r);
    addSourceBuffer(buffer);

    IsDirty = true;
    return BufferReferences.size()-1;
}

//! updates bouding box from internal buffers
void CBatchingMesh::recalculateBoundingBox()
{
    if (DestBuffers.size() == 0)
        Box.reset(0,0,0);
    else
    {
        Box.reset(DestBuffers[0].Buffer->getBoundingBox().MinEdge);

        u32 i;
        for (i=0; i < DestBuffers.size(); ++i)
            Box.addInternalBox(DestBuffers[i].Buffer->getBoundingBox());
    }
}


/* Standard IMesh functions */

//! Returns the amount of mesh buffers.
/** \return Returns the amount of mesh buffers (IMeshBuffer) in this mesh. */
u32 CBatchingMesh::getMeshBufferCount() const
{
    return DestBuffers.size();
}

//! Returns pointer to a mesh buffer.
/** \param nr: Zero based index of the mesh buffer. The maximum value is
getMeshBufferCount() - 1;
\return Returns the pointer to the mesh buffer or
NULL if there is no such mesh buffer. */
IMeshBuffer* CBatchingMesh::getMeshBuffer(u32 nr) const
{
    if (nr < DestBuffers.size())
        return DestBuffers[nr].Buffer;
    else
        return 0;
}

//! Returns pointer to a mesh buffer which fits a material
IMeshBuffer* CBatchingMesh::getMeshBuffer( const video::SMaterial &material) const
{
    return 0;
}

//! Returns an axis aligned bounding box of the mesh.
/** \return A bounding box of this mesh is returned. */
const core::aabbox3d<f32>& CBatchingMesh::getBoundingBox() const
{
    return Box;
}

//! set user axis aligned bounding box
void CBatchingMesh::setBoundingBox( const core::aabbox3df& box)
{
    Box = box;
}

//! Sets a flag of all contained materials to a new value.
/** \param flag: Flag to set in all materials.
 \param newvalue: New value to set in all materials. */
void CBatchingMesh::setMaterialFlag(video::E_MATERIAL_FLAG flag, bool newvalue)
{
    for (u32 i=0; i<DestBuffers.size(); ++i)
        DestBuffers[i].Buffer->getMaterial().setFlag(flag, newvalue);
}

//! drops all buffers and clears internal states
void CBatchingMesh::clear()
{
    u32 i;
    for (i=0; i < DestBuffers.size(); ++i)
        DestBuffers[i].Buffer->drop();

    for (i=0; i < SourceBuffers.size(); ++i)
        SourceBuffers[i]->drop();

    BufferReferences.clear();
    MaterialReferences.clear();
    DestBuffers.clear();
    SourceBuffers.clear();

    IsDirty = false;
    IsFinal = false;
}

//! first updates the mesh, then drops all source buffers.
/** once this mesh has been finalized, it cannot be changed again! */
void CBatchingMesh::finalize()
{
    update();

    for (u32 i=0; i < SourceBuffers.size(); ++i)
        SourceBuffers[i]->drop();

    SourceBuffers.clear();

    IsFinal = true;
}

//! Moves a mesh
core::array<bool> CBatchingMesh::moveMesh(const core::array<s32>& bufferIDs, const core::matrix4 &newMatrix)
{
    core::array<bool> result;
    result.reallocate(bufferIDs.size());
    for (u32 i=0; i<bufferIDs.size(); ++i)
        result.push_back(moveMeshBuffer(bufferIDs[i], newMatrix));

    return result;
}


//! Moves a mesh buffer
bool CBatchingMesh::moveMeshBuffer(const s32 id, const core::matrix4 &newMatrix)
{
    if ((u32)id > BufferReferences.size() || IsFinal )
        return false;

    BufferReferences[id].Transform = newMatrix;

    // is the source buffer dirty?
    if (!DestBuffers[BufferReferences[id].DestReference].IsDirty)
    {
        // transform each vertex and normal
        updateDestFromSourceBuffer(id);
        recalculateDestBufferBoundingBox(BufferReferences[id].DestReference);
    }
    return true;
}


//! returns the source buffer, if available
IMeshBuffer* CBatchingMesh::getSourceBuffer(s32 id)
{
    if ((u32)id > BufferReferences.size() || IsFinal)
        return 0;
    else
        return BufferReferences[id].SourceBuffer;
}

//! returns the matrix of the source buffer
core::matrix4 CBatchingMesh::getSourceBufferMatrix(s32 id)
{
    core::matrix4 ret;
    if ((u32)id > BufferReferences.size() || IsFinal)
        ret.makeIdentity();
    else
        ret = BufferReferences[id].Transform;

    return ret;
}


//! returns the number of source buffers
u32 CBatchingMesh::getSourceBufferCount() const
{
    return BufferReferences.size();
}

// private functions

void CBatchingMesh::recalculateDestBufferBoundingBox(u32 i)
{
    switch (DestBuffers[i].VertexType)
    {
    case video::EVT_STANDARD:
        ((SMeshBuffer*)DestBuffers[i].Buffer)->recalculateBoundingBox();
        break;
    case video::EVT_2TCOORDS:
        ((SMeshBufferLightMap*)DestBuffers[i].Buffer)->recalculateBoundingBox();
        break;
    case video::EVT_TANGENTS:
        ((SMeshBufferTangents*)DestBuffers[i].Buffer)->recalculateBoundingBox();
        break;
    default:
        break;
    }
}

void CBatchingMesh::updateDestFromSourceBuffer(u32 i)
{
    u16* ind = BufferReferences[i].SourceBuffer->getIndices();
    void*ver = BufferReferences[i].SourceBuffer->getVertices();
    core::matrix4 m = BufferReferences[i].Transform;
    u32 fi = BufferReferences[i].FirstIndex;
    u32 fv = BufferReferences[i].FirstVertex;
    u32 ic = BufferReferences[i].IndexCount;
    u32 vc = BufferReferences[i].VertexCount;
    u32 x;
    video::E_VERTEX_TYPE vt = DestBuffers[BufferReferences[i].DestReference].VertexType;
    switch (vt)
    {
    case video::EVT_STANDARD:
    {
        SMeshBuffer* dest = (SMeshBuffer*) DestBuffers[BufferReferences[i].DestReference].Buffer;

        for (x=fi; x < fi+ic; ++x)
            dest->Indices[x] = ind[x-fi]+fv;

        video::S3DVertex* vertices= (video::S3DVertex*) ver;

        for (x=fv; x < fv+vc; ++x)
        {
            dest->Vertices[x] = vertices[x-fv];
            m.transformVect(dest->Vertices[x].Pos);
            m.rotateVect(dest->Vertices[x].Normal);
        }
        break;
    }
    case video::EVT_2TCOORDS:
    {
        SMeshBufferLightMap* dest = (SMeshBufferLightMap*) DestBuffers[BufferReferences[i].DestReference].Buffer;

        for (x=fi; x < fi+ic; ++x)
            dest->Indices[x] = ind[x-fi]+fv;

        video::S3DVertex2TCoords* vertices= (video::S3DVertex2TCoords*) ver;

        for (x=fv; x < fv+vc; ++x)
        {
            dest->Vertices[x] = vertices[x-fv];
            m.transformVect(dest->Vertices[x].Pos);
            m.rotateVect(dest->Vertices[x].Normal);
        }
        break;
    }
    case video::EVT_TANGENTS:
    {
        SMeshBufferTangents* dest = (SMeshBufferTangents*) DestBuffers[BufferReferences[i].DestReference].Buffer;

        for (x=fi; x < fi+ic; ++x)
            dest->Indices[x] = ind[x-fi]+fv;

        video::S3DVertexTangents* vertices= (video::S3DVertexTangents*) ver;

        for (x=fv; x < fv+vc; ++x)
        {
            dest->Vertices[x] = vertices[x-fv];
            m.transformVect(dest->Vertices[x].Pos);
            m.rotateVect(dest->Vertices[x].Normal); // are tangents/binormals in face space?
        }
        break;
    }
    default:
        break;
    }
}

void CBatchingMesh::addSourceBuffer(IMeshBuffer *source)
{
    bool found = false;
    for (u32 i=0; i<SourceBuffers.size(); ++i)
    {
        if (SourceBuffers[i] == source)
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        source->grab();
        SourceBuffers.push_back(source);
    }
}

void CBatchingMesh::setHardwareMappingHint(E_HARDWARE_MAPPING mapping, E_BUFFER_TYPE type)
{
    for (u32 i=0; i < DestBuffers.size(); ++i)
        DestBuffers[i].Buffer->setHardwareMappingHint(mapping, type);
}


void CBatchingMesh::setDirty(E_BUFFER_TYPE type)
{
    for (u32 i=0; i < DestBuffers.size(); ++i)
        DestBuffers[i].Buffer->setDirty(type);
}

} // namespace scene
} // namespace irr

