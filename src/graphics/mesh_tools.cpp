//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Joerg Henrichs
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

void MeshTools::minMax3D(scene::IMesh* mesh, Vec3 *min, Vec3 *max) {
    
    Vec3 extend;
    *min = Vec3( 999999.9f);
    *max = Vec3(-999999.9f);
    for(unsigned int i=0; i<mesh->getMeshBufferCount(); i++) {
        scene::IMeshBuffer *mb = mesh->getMeshBuffer(i);
        if(mb->getVertexType()!=video::EVT_STANDARD) {
            fprintf(stderr, "Tools::minMax3D: Ignoring type '%d'!", 
                    mb->getVertexType());
            continue;
        }
        u16 *mbIndices = mb->getIndices();
        video::S3DVertex* mbVertices=(irr::video::S3DVertex*)mb->getVertices();
        for(unsigned int j=0; j<mb->getIndexCount(); j+=1) {
            int indx=mbIndices[j];
            Vec3 c(mbVertices[indx].Pos.X,
                   mbVertices[indx].Pos.Y,
                   mbVertices[indx].Pos.Z  );
            min->min(c);
            max->max(c);
        }   // for j
    }  // for i<getMeshBufferCount
}   // minMax3D

