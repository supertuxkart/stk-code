// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef HEADER_STK_MESH_LOADER_HPP
#define HEADER_STK_MESH_LOADER_HPP

#include "../lib/irrlicht/source/Irrlicht/CSkinnedMesh.h"

#include <IMeshLoader.h>
#include <ISceneManager.h>
#include <IReadFile.h>
#include <string>
#include <unordered_map>

using namespace irr;

namespace SP
{
    class SPMesh;
}


class B3DMeshLoader : public scene::IMeshLoader
{
public:
    static int m_straight_frame;

    //! Constructor
    B3DMeshLoader(scene::ISceneManager* smgr);

    //! returns true if the file maybe is able to be loaded by this class
    //! based on the file extension (e.g. ".bsp")
    virtual bool isALoadableFileExtension(const io::path& filename) const;

    //! creates/loads an animated mesh from the file.
    //! \return Pointer to the created mesh. Returns 0 if loading failed.
    //! If you no longer need the mesh, you should call IAnimatedMesh::drop().
    //! See IReferenceCounted::drop() for more information.
    virtual scene::IAnimatedMesh* createMesh(io::IReadFile* file);

private:
    class JointInfluence
    {
    public:
        int joint_idx;
        float weight;
    };

    static bool sortJointInfluenceFunc(const JointInfluence& a,
        const JointInfluence& b)
    {
        return a.weight > b.weight;
    }

    typedef core::array<core::array <core::array<JointInfluence> > >
        WeightInfluence;

    SP::SPMesh* toSPM(scene::CSkinnedMesh* mesh);

    std::unordered_map<scene::IMeshBuffer*, std::pair<std::string, std::string> > m_texture_string;

    struct SB3dChunkHeader
    {
        c8 name[4];
        s32 size;
    };

    struct SB3dChunk
    {
        SB3dChunk(const SB3dChunkHeader& header, long sp)
            : length(header.size+8), startposition(sp)
        {
            name[0]=header.name[0];
            name[1]=header.name[1];
            name[2]=header.name[2];
            name[3]=header.name[3];
        }

        c8 name[4];
        s32 length;
        long startposition;
    };

    struct SB3dTexture
    {
        core::stringc TextureName;
        s32 Flags;
        s32 Blend;
        f32 Xpos;
        f32 Ypos;
        f32 Xscale;
        f32 Yscale;
        f32 Angle;
    };

    struct SB3dMaterial
    {
        SB3dMaterial() : red(1.0f), green(1.0f),
            blue(1.0f), alpha(1.0f), shininess(0.0f), blend(1),
            fx(0)
        {
            for (u32 i=0; i<video::MATERIAL_MAX_TEXTURES; ++i)
                Textures[i]=0;
        }
        video::SMaterial Material;
        f32 red, green, blue, alpha;
        f32 shininess;
        s32 blend,fx;
        SB3dTexture *Textures[video::MATERIAL_MAX_TEXTURES];
    };

    void computeWeightInfluence(scene::CSkinnedMesh::SJoint* joint,
                                unsigned& index, WeightInfluence& wi);

    void addSPAnimation(SP::SPMesh* spm, scene::CSkinnedMesh::SJoint* joint,
                        unsigned& index, unsigned frame);

    bool load();
    bool readChunkNODE(scene::CSkinnedMesh::SJoint* InJoint);
    bool readChunkMESH(scene::CSkinnedMesh::SJoint* InJoint);
    bool readChunkVRTS(scene::CSkinnedMesh::SJoint* InJoint);
    bool readChunkTRIS(scene::SSkinMeshBuffer *MeshBuffer, u32 MeshBufferID, s32 Vertices_Start);
    bool readChunkBONE(scene::CSkinnedMesh::SJoint* InJoint);
    bool readChunkKEYS(scene::CSkinnedMesh::SJoint* InJoint);
    bool readChunkANIM();
    bool readChunkTEXS();
    bool readChunkBRUS();

    void loadTextures(SB3dMaterial& material, scene::IMeshBuffer* mb);

    void readString(core::stringc& newstring);
    void readFloats(f32* vec, u32 count);

    core::array<SB3dChunk> B3dStack;

    core::array<SB3dMaterial> Materials;
    core::array<SB3dTexture> Textures;

    core::array<s32> AnimatedVertices_VertexID;

    core::array<s32> AnimatedVertices_BufferID;

    core::array<video::S3DVertex2TCoords> BaseVertices;

    scene::ISceneManager* SceneManager;
    scene::CSkinnedMesh* AnimatedMesh;
    io::IReadFile* B3DFile;

    //B3Ds have Vertex ID's local within the mesh I don't want this
    // Variable needs to be class member due to recursion in calls
    u32 VerticesStart;

    bool NormalsInFile;
    bool HasVertexColors;
    bool ShowWarning;
};

#endif

