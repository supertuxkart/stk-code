//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Joerg Henrichs
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

#include "graphics/mesh_tools.hpp"
#include <irrlicht.h>
#include <IMesh.h>
#include <IMeshBuffer.h>
#include "utils/log.hpp"
#include "graphics/irr_driver.hpp"
#include "modes/world.hpp"
#include "tracks/track.hpp"

void MeshTools::minMax3D(scene::IMesh* mesh, Vec3 *min, Vec3 *max) {

    Vec3 extend;
    *min = Vec3( 999999.9f);
    *max = Vec3(-999999.9f);
    for(unsigned int i=0; i<mesh->getMeshBufferCount(); i++)
    {
        scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);

        if (mb->getVertexType() == video::EVT_STANDARD)
        {
            u16 *mbIndices = mb->getIndices();
            video::S3DVertex* mbVertices=(irr::video::S3DVertex*)mb->getVertices();
            for (unsigned int j=0; j<mb->getIndexCount(); j+=1)
            {
                int indx=mbIndices[j];
                Vec3 c(mbVertices[indx].Pos.X,
                       mbVertices[indx].Pos.Y,
                       mbVertices[indx].Pos.Z  );
                min->min(c);
                max->max(c);
            }   // for j
        }
        else if (mb->getVertexType() == video::EVT_2TCOORDS)
        {
            u16 *mbIndices = mb->getIndices();
            video::S3DVertex2TCoords* mbVertices=(irr::video::S3DVertex2TCoords*)mb->getVertices();
            for (unsigned int j=0; j<mb->getIndexCount(); j+=1)
            {
                int indx=mbIndices[j];
                Vec3 c(mbVertices[indx].Pos.X,
                       mbVertices[indx].Pos.Y,
                       mbVertices[indx].Pos.Z  );
                min->min(c);
                max->max(c);
            }   // for j
        }
        else if (mb->getVertexType() == video::EVT_TANGENTS)
        {
            u16 *mbIndices = mb->getIndices();
            video::S3DVertexTangents* mbVertices=(irr::video::S3DVertexTangents*)mb->getVertices();
            for (unsigned int j=0; j<mb->getIndexCount(); j+=1)
            {
                int indx=mbIndices[j];
                Vec3 c(mbVertices[indx].Pos.X,
                       mbVertices[indx].Pos.Y,
                       mbVertices[indx].Pos.Z  );
                min->min(c);
                max->max(c);
            }   // for j
        }
        else
        {
            Log::warn("Tools", "minMax3D: Ignoring type '%d'!\n",
                      mb->getVertexType());
        }
    }  // for i<getMeshBufferCount
}   // minMax3D

// Copied from irrlicht
void calculateTangents(
    core::vector3df& normal,
    core::vector3df& tangent,
    core::vector3df& binormal,
    const core::vector3df& vt1, const core::vector3df& vt2, const core::vector3df& vt3, // vertices
    const core::vector2df& tc1, const core::vector2df& tc2, const core::vector2df& tc3) // texture coords
{
    core::vector3df v1 = vt1 - vt2;
    core::vector3df v2 = vt3 - vt1;
    normal = v2.crossProduct(v1);
    normal.normalize();

    // binormal

    f32 deltaX1 = tc1.X - tc2.X;
    f32 deltaX2 = tc3.X - tc1.X;
    binormal = (v1 * deltaX2) - (v2 * deltaX1);
    binormal.normalize();

    // tangent

    f32 deltaY1 = tc1.Y - tc2.Y;
    f32 deltaY2 = tc3.Y - tc1.Y;
    tangent = (v1 * deltaY2) - (v2 * deltaY1);
    tangent.normalize();

    // adjust

    core::vector3df txb = tangent.crossProduct(binormal);
    if (txb.dotProduct(normal) < 0.0f)
    {
        tangent *= -1.0f;
        binormal *= -1.0f;
    }
}

// Copied from irrlicht
static inline core::vector3df getAngleWeight(const core::vector3df& v1,
    const core::vector3df& v2,
    const core::vector3df& v3)
{
    // Calculate this triangle's weight for each of its three vertices
    // start by calculating the lengths of its sides
    const f32 a = v2.getDistanceFromSQ(v3);
    const f32 asqrt = sqrtf(a);
    const f32 b = v1.getDistanceFromSQ(v3);
    const f32 bsqrt = sqrtf(b);
    const f32 c = v1.getDistanceFromSQ(v2);
    const f32 csqrt = sqrtf(c);

    // use them to find the angle at each vertex
    return core::vector3df(
        acosf((b + c - a) / (2.f * bsqrt * csqrt)),
        acosf((-b + c + a) / (2.f * asqrt * csqrt)),
        acosf((b - c + a) / (2.f * bsqrt * asqrt)));
}

// Copied from irrlicht
template <typename T>
void recalculateTangentsT(scene::IMeshBuffer* buffer, bool recalculateNormals, bool smooth, bool angleWeighted)
{
    if (!buffer || (buffer->getVertexType() != video::EVT_TANGENTS))
        return;

    const u32 vtxCnt = buffer->getVertexCount();
    const u32 idxCnt = buffer->getIndexCount();

    T* idx = reinterpret_cast<T*>(buffer->getIndices());
    video::S3DVertexTangents* v =
        (video::S3DVertexTangents*)buffer->getVertices();

    if (smooth)
    {
        u32 i;

        for (i = 0; i != vtxCnt; ++i)
        {
            if (recalculateNormals)
                v[i].Normal.set(0.f, 0.f, 0.f);
            v[i].Tangent.set(0.f, 0.f, 0.f);
            v[i].Binormal.set(0.f, 0.f, 0.f);
        }

        //Each vertex gets the sum of the tangents and binormals from the faces around it
        for (i = 0; i<idxCnt; i += 3)
        {
            // if this triangle is degenerate, skip it!
            if (v[idx[i + 0]].Pos == v[idx[i + 1]].Pos ||
                v[idx[i + 0]].Pos == v[idx[i + 2]].Pos ||
                v[idx[i + 1]].Pos == v[idx[i + 2]].Pos
                /*||
                v[idx[i+0]].TCoords == v[idx[i+1]].TCoords ||
                v[idx[i+0]].TCoords == v[idx[i+2]].TCoords ||
                v[idx[i+1]].TCoords == v[idx[i+2]].TCoords */
                )
                continue;

            //Angle-weighted normals look better, but are slightly more CPU intensive to calculate
            core::vector3df weight(1.f, 1.f, 1.f);
            if (angleWeighted)
                weight = getAngleWeight(v[i + 0].Pos, v[i + 1].Pos, v[i + 2].Pos);
            core::vector3df localNormal;
            core::vector3df localTangent;
            core::vector3df localBinormal;

            calculateTangents(
                localNormal,
                localTangent,
                localBinormal,
                v[idx[i + 0]].Pos,
                v[idx[i + 1]].Pos,
                v[idx[i + 2]].Pos,
                v[idx[i + 0]].TCoords,
                v[idx[i + 1]].TCoords,
                v[idx[i + 2]].TCoords);

            if (recalculateNormals)
                v[idx[i + 0]].Normal += localNormal * weight.X;
            v[idx[i + 0]].Tangent += localTangent * weight.X;
            v[idx[i + 0]].Binormal += localBinormal * weight.X;

            calculateTangents(
                localNormal,
                localTangent,
                localBinormal,
                v[idx[i + 1]].Pos,
                v[idx[i + 2]].Pos,
                v[idx[i + 0]].Pos,
                v[idx[i + 1]].TCoords,
                v[idx[i + 2]].TCoords,
                v[idx[i + 0]].TCoords);

            if (recalculateNormals)
                v[idx[i + 1]].Normal += localNormal * weight.Y;
            v[idx[i + 1]].Tangent += localTangent * weight.Y;
            v[idx[i + 1]].Binormal += localBinormal * weight.Y;

            calculateTangents(
                localNormal,
                localTangent,
                localBinormal,
                v[idx[i + 2]].Pos,
                v[idx[i + 0]].Pos,
                v[idx[i + 1]].Pos,
                v[idx[i + 2]].TCoords,
                v[idx[i + 0]].TCoords,
                v[idx[i + 1]].TCoords);

            if (recalculateNormals)
                v[idx[i + 2]].Normal += localNormal * weight.Z;
            v[idx[i + 2]].Tangent += localTangent * weight.Z;
            v[idx[i + 2]].Binormal += localBinormal * weight.Z;
        }

        // Normalize the tangents and binormals
        if (recalculateNormals)
        {
            for (i = 0; i != vtxCnt; ++i)
                v[i].Normal.normalize();
        }
        for (i = 0; i != vtxCnt; ++i)
        {
            v[i].Tangent.normalize();
            v[i].Binormal.normalize();
        }
    }
    else
    {
        core::vector3df localNormal;
        for (u32 i = 0; i<idxCnt; i += 3)
        {
            calculateTangents(
                localNormal,
                v[idx[i + 0]].Tangent,
                v[idx[i + 0]].Binormal,
                v[idx[i + 0]].Pos,
                v[idx[i + 1]].Pos,
                v[idx[i + 2]].Pos,
                v[idx[i + 0]].TCoords,
                v[idx[i + 1]].TCoords,
                v[idx[i + 2]].TCoords);
            if (recalculateNormals)
                v[idx[i + 0]].Normal = localNormal;

            calculateTangents(
                localNormal,
                v[idx[i + 1]].Tangent,
                v[idx[i + 1]].Binormal,
                v[idx[i + 1]].Pos,
                v[idx[i + 2]].Pos,
                v[idx[i + 0]].Pos,
                v[idx[i + 1]].TCoords,
                v[idx[i + 2]].TCoords,
                v[idx[i + 0]].TCoords);
            if (recalculateNormals)
                v[idx[i + 1]].Normal = localNormal;

            calculateTangents(
                localNormal,
                v[idx[i + 2]].Tangent,
                v[idx[i + 2]].Binormal,
                v[idx[i + 2]].Pos,
                v[idx[i + 0]].Pos,
                v[idx[i + 1]].Pos,
                v[idx[i + 2]].TCoords,
                v[idx[i + 0]].TCoords,
                v[idx[i + 1]].TCoords);
            if (recalculateNormals)
                v[idx[i + 2]].Normal = localNormal;
        }
    }
}

// Copied from irrlicht
void recalculateTangents(scene::IMeshBuffer* buffer, bool recalculateNormals, bool smooth, bool angleWeighted)
{
    if (buffer && (buffer->getVertexType() == video::EVT_TANGENTS))
    {
        if (buffer->getIndexType() == video::EIT_16BIT)
            recalculateTangentsT<u16>(buffer, recalculateNormals, smooth, angleWeighted);
        else
            recalculateTangentsT<u32>(buffer, recalculateNormals, smooth, angleWeighted);
    }
}

// Copied from irrlicht
void recalculateTangents(scene::IMesh* mesh, bool recalculateNormals, bool smooth, bool angleWeighted)
{
    if (!mesh)
        return;

    const u32 meshBufferCount = mesh->getMeshBufferCount();
    for (u32 b = 0; b<meshBufferCount; ++b)
    {
        recalculateTangents(mesh->getMeshBuffer(b), recalculateNormals, smooth, angleWeighted);
    }
}

bool MeshTools::isNormalMap(scene::IMeshBuffer* mb)
{
    if (!irr_driver->isGLSL())
        return false;
    return (mb->getMaterial().MaterialType == irr_driver->getShader(ES_NORMAL_MAP) &&
        mb->getVertexType() != video::EVT_TANGENTS);
}

// Copied from irrlicht
scene::IMesh* MeshTools::createMeshWithTangents(scene::IMesh* mesh, bool(*predicate)(scene::IMeshBuffer*),
    bool recalculateNormals, bool smooth, bool angleWeighted, bool calculateTangents)
{
    if (!mesh)
        return 0;

    // copy mesh and fill data into SMeshBufferTangents

    scene::SMesh* clone = new scene::SMesh();
    const u32 meshBufferCount = mesh->getMeshBufferCount();

    bool needsNormalMap = false;
    for (u32 b = 0; b < meshBufferCount; ++b)
    {
        scene::IMeshBuffer* original = mesh->getMeshBuffer(b);
        if (predicate(original))
        {
            needsNormalMap = true;
            break;
        }
    }

    if (!needsNormalMap)
    {
        return mesh;
    }

    for (u32 b = 0; b<meshBufferCount; ++b)
    {
        scene::IMeshBuffer* original = mesh->getMeshBuffer(b);
        const u32 idxCnt = original->getIndexCount();
        const u16* idx = original->getIndices();

        if (!predicate(original))
        {
            clone->addMeshBuffer(original);
            continue;
        }

        scene::SMeshBufferTangents* buffer = new scene::SMeshBufferTangents();

        buffer->Material = original->getMaterial();
        buffer->Vertices.reallocate(idxCnt);
        buffer->Indices.reallocate(idxCnt);

        core::map<video::S3DVertexTangents, int> vertMap;
        int vertLocation;

        // copy vertices

        const video::E_VERTEX_TYPE vType = original->getVertexType();
        video::S3DVertexTangents vNew;
        for (u32 i = 0; i<idxCnt; ++i)
        {
            switch (vType)
            {
                case video::EVT_STANDARD:
                {
                    const video::S3DVertex* v =
                        (const video::S3DVertex*)original->getVertices();
                    vNew = video::S3DVertexTangents(
                        v[idx[i]].Pos, v[idx[i]].Normal, v[idx[i]].Color, v[idx[i]].TCoords);
                }
                break;
                case video::EVT_2TCOORDS:
                {
                    const video::S3DVertex2TCoords* v =
                        (const video::S3DVertex2TCoords*)original->getVertices();
                    vNew = video::S3DVertexTangents(
                        v[idx[i]].Pos, v[idx[i]].Normal, v[idx[i]].Color, v[idx[i]].TCoords);
                }
                break;
                case video::EVT_TANGENTS:
                {
                        const video::S3DVertexTangents* v =
                            (const video::S3DVertexTangents*)original->getVertices();
                        vNew = v[idx[i]];
                }
                break;
            }
            core::map<video::S3DVertexTangents, int>::Node* n = vertMap.find(vNew);
            if (n)
            {
                vertLocation = n->getValue();
            }
            else
            {
                vertLocation = buffer->Vertices.size();
                buffer->Vertices.push_back(vNew);
                vertMap.insert(vNew, vertLocation);
            }

            // create new indices
            buffer->Indices.push_back(vertLocation);
        }
        buffer->recalculateBoundingBox();

        // add new buffer
        clone->addMeshBuffer(buffer);
        buffer->drop();
    }

    clone->recalculateBoundingBox();
    if (calculateTangents)
        recalculateTangents(clone, recalculateNormals, smooth, angleWeighted);
    
    int mbcount = clone->getMeshBufferCount();
    for (int i = 0; i < mbcount; i++)
    {
        scene::IMeshBuffer* mb = clone->getMeshBuffer(i);

        for (int t = 0; t < video::MATERIAL_MAX_TEXTURES; t++)
        {
            video::ITexture* texture = mb->getMaterial().TextureLayer[t].Texture;
            if (texture != NULL)
                texture->grab();
        }
    }

    scene::IMeshCache* meshCache = irr_driver->getSceneManager()->getMeshCache();
    io::SNamedPath path = meshCache->getMeshName(mesh);
    irr_driver->removeMeshFromCache(mesh);

    scene::SAnimatedMesh* amesh = new scene::SAnimatedMesh(clone);
    meshCache->addMesh(path, amesh);

    World::getWorld()->getTrack()->addCachedMesh(amesh);

    return clone;
}

