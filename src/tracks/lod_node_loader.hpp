//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2013 Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTe ABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#ifndef HEADER_LOD_NODE_LOADER_HPP
#define HEADER_LOD_NODE_LOADER_HPP

class LODNode;
class Track;

#include <map>
#include <vector>
#include <string>
#include "io/xml_node.hpp"

namespace irr
{
    namespace scene
    {
        class IMesh;
        class ISceneNode;
    }
}

struct LodInstance
{
    const XMLNode* m_xml_node;
    scene::ISceneNode* m_parent;

    /** Constructor to allow storing this in STL containers */
    LodInstance()
    {
        m_parent = NULL;
        m_xml_node = NULL;
    }

    LodInstance(const XMLNode* xml_node, scene::ISceneNode* parent)
    {
        m_xml_node = xml_node;
        m_parent = parent;
    }
};

struct LodModel
{
    std::string m_model_file;
    bool m_tangent;
    const XMLNode* m_xml;

    /** Constructor to allow storing this in STL containers */
    LodModel()
    {
        m_tangent = false;
        m_xml = NULL;
    }

    LodModel(const XMLNode* xml, std::string& model, bool tangent)
    {
        m_model_file = model;
        m_tangent = tangent;
        m_xml = xml;
    }

    ~LodModel()
    {
    }
};

/** Utility class to load level-of-detail nodes
 * \ingroup tracks
 */
class LodNodeLoader
{
private:
    std::map< std::string, std::map< int, LodModel > > lod_groups;
    std::map< std::string, std::vector< LodInstance > > lod_instances;

public:
         LodNodeLoader();

    bool check(const XMLNode* xml, scene::ISceneNode* parent);
    void done(Track* track,
              std::string directory,
              std::vector<irr::scene::IMesh*>& cache,
              std::vector<LODNode*>& out);

    void clear();

};  // LodNodeLoader

#endif // HEADER_LOD_NODE_LOADER_HPP
