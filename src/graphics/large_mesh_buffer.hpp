// Copyright (C) 2002-2015 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __LARGE_MESH_BUFFER_H_INCLUDED__
#define __LARGE_MESH_BUFFER_H_INCLUDED__

#include "irrArray.h"
#include "IMeshBuffer.h"
#include "CMeshBuffer.h"

namespace irr
{
namespace scene
{
    //! A SMeshBuffer with 32-bit indices
    class LargeMeshBuffer : public SMeshBuffer
    {
    public:
        //! Get type of index data which is stored in this meshbuffer.
        /** \return Index type of this buffer. */
        virtual video::E_INDEX_TYPE getIndexType() const
        {
            return video::EIT_32BIT;
        }

        //! Get pointer to indices
        /** \return Pointer to indices. */
        virtual const u16* getIndices() const
        {
            return (u16 *) Indices.const_pointer();
        }


        //! Get pointer to indices
        /** \return Pointer to indices. */
        virtual u16* getIndices()
        {
            return (u16 *) Indices.pointer();
        }


        //! Get number of indices
        /** \return Number of indices. */
        virtual u32 getIndexCount() const
        {
            return Indices.size();
        }

        //! Indices into the vertices of this buffer.
        core::array<u32> Indices;
    };
} // end namespace scene
} // end namespace irr

#endif


