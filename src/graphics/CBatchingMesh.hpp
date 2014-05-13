// A mesh used for batching many other meshes together, to reduce the number
// of draw calls. Simply add meshes into this one, with given transformations
// or positions, and then call update

// TODO: Adapt the VBO interface and integrate setDirty with the current VBO updates.

#include "IMesh.h"
#include "SMeshBuffer.h"

namespace irr
{
namespace scene
{

class CBatchingMesh : public IMesh
{
public:
    CBatchingMesh();

    virtual ~CBatchingMesh();

    //! returns true if new buffers have been added without updating the internal buffers
    bool isDirty(s32 id=-1);

    //! refreshes the internal buffers from source
    void update();

    //! drops all buffers and clears internal states
    void clear();

    //! first updates the mesh, then drops all source buffers.
    /** once this mesh has been finalized, it cannot be changed again! */
    void finalize();

    //! adds a mesh to the buffers with the given offset
    /** \Return: Returns an array of ID numbers */
    core::array<s32> addMesh(IMesh* mesh,
        core::vector3df pos = core::vector3df(0,0,0),
        core::vector3df rot = core::vector3df(0,0,0),
        core::vector3df scale = core::vector3df(1,1,1));

    //! adds a mesh with the given transformation
    /** \Return: Returns an array of ID numbers */
    core::array<s32> addMesh(IMesh* mesh, const core::matrix4 &transform);

    //! adds a mesh buffer with the given transformation
    /** \Return: Returns the ID of this mesh buffer */
    s32 addMeshBuffer(IMeshBuffer* buffer,
        core::vector3df pos = core::vector3df(0,0,0),
        core::vector3df rot = core::vector3df(0,0,0),
        core::vector3df scale = core::vector3df(1,1,1));

    //! adds a mesh with the given transformation
    /** \Return Returns the ID of this mesh buffer */
    s32 addMeshBuffer(IMeshBuffer* buffer, const core::matrix4 &transform);

    //! updates bouding box from internal buffers
    void recalculateBoundingBox();

    //! Moves a mesh,
    /** mesh buffers in clean destination buffers will be moved immediately,
    ones in dirty buffers will be left until the next update */
    core::array<bool> moveMesh(const core::array<s32>& bufferIDs, const core::matrix4 &newMatrix);

    //! Moves a mesh buffer
    /** if the destination buffer is clean it will be moved immediately,
    if a member of a dirty buffer, it will be left until the next update */
    bool moveMeshBuffer(const s32 id, const core::matrix4 &newMatrix);

    //! returns the source buffer, if available
    IMeshBuffer* getSourceBuffer(s32 id);

    //! returns the matrix of the source buffer
    core::matrix4 getSourceBufferMatrix(s32 id);

    //! returns the number of source buffers
    u32 getSourceBufferCount() const;

    /* Standard IMesh functions */

    //! Returns the amount of mesh buffers.
    /** \return Returns the amount of mesh buffers (IMeshBuffer) in this mesh. */
    virtual u32 getMeshBufferCount() const;

    //! Returns pointer to a mesh buffer.
    /** \param nr: Zero based index of the mesh buffer. The maximum value is
    getMeshBufferCount() - 1;
    \return Returns the pointer to the mesh buffer or
    NULL if there is no such mesh buffer. */
    virtual IMeshBuffer* getMeshBuffer(u32 nr) const;

    //! Returns pointer to a mesh buffer which fits a material
    /** \param material: material to search for
    \return Returns the pointer to the mesh buffer or
    NULL if there is no such mesh buffer. */
    virtual IMeshBuffer* getMeshBuffer( const video::SMaterial &material) const;

    //! Returns an axis aligned bounding box of the mesh.
    /** \return A bounding box of this mesh is returned. */
    virtual const core::aabbox3d<f32>& getBoundingBox() const;

    //! set user axis aligned bounding box
    virtual void setBoundingBox( const core::aabbox3df& box);

    //! Sets a flag of all contained materials to a new value.
    /** \param flag: Flag to set in all materials.
     \param newvalue: New value to set in all materials. */
    virtual void setMaterialFlag(video::E_MATERIAL_FLAG flag, bool newvalue);

    virtual void setHardwareMappingHint(E_HARDWARE_MAPPING mapping, E_BUFFER_TYPE type);

    virtual void setDirty(E_BUFFER_TYPE type);

private:

    // add a buffer to the source buffers array if it doesn't already exist
    void addSourceBuffer(IMeshBuffer* source);

    // updates the vertices in dest buffer from the source one
    void updateDestFromSourceBuffer(u32 id);

    // recalculates the bounding box for the given dest buffer
    void recalculateDestBufferBoundingBox(u32 i);

    struct SBufferReference
    {
        SBufferReference()
          : SourceBuffer(0), DestReference(0), FirstVertex(0), VertexCount(0),
            FirstIndex(0), IndexCount(0), Initialized(false) { }

        IMeshBuffer* SourceBuffer;
        u32 DestReference;
        u32 FirstVertex, VertexCount, FirstIndex, IndexCount;
        core::matrix4 Transform;
        bool Initialized;
    };

    struct SMaterialReference
    {
        video::SMaterial Material;
        video::E_VERTEX_TYPE VertexType;
        u32 BufferIndex;
    };

    struct SDestBufferReference
    {
        IMeshBuffer* Buffer;
        video::E_VERTEX_TYPE VertexType;
        u32 VertexCount;
        u32 IndexCount;
        bool IsDirty;
    };

    //! Source mesh buffers, these are locked
    core::array<IMeshBuffer*>         SourceBuffers;

    core::array<SBufferReference>     BufferReferences;
    core::array<SMaterialReference>   MaterialReferences;
    core::array<SDestBufferReference> DestBuffers;

    //! bounding containing all destination buffers
    core::aabbox3d<f32> Box;

    //! does it require an update?
    bool IsDirty;

    //! can it be changed?
    bool IsFinal;
};

} // namespace scene
} // namespace irr

// #endif
