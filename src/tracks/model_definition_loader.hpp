//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007-2015 Joerg Henrichs
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
class STKInstancedSceneNode;
namespace GE { class GERenderInfo; }
class Track;

#include <cassert>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include "io/xml_node.hpp"

namespace irr
{
    namespace scene
    {
        class IMesh;
        class ISceneNode;
    }
}

struct ModelDefinition
{
    std::string m_model_file;
    bool m_tangent; // obsolete, TODO remove
    const XMLNode* m_xml;

    /** For LOD */
    int m_distance;

    bool m_skeletal_animation;

    /** Constructor to allow storing this in STL containers */
    ModelDefinition()
    {
        m_tangent = false;
        m_skeletal_animation = false;
        m_distance = 0;
        m_xml = NULL;
    }

    ModelDefinition(const XMLNode* xml, int distance, std::string& model, bool tangent, bool skeletal_animation)
    {
        m_model_file = model;
        m_tangent = tangent;
        m_xml = xml;
        m_distance = distance;
        m_skeletal_animation = skeletal_animation;
    }

    ~ModelDefinition()
    {
    }
};

/** Utility class to load level-of-detail nodes and library nodes
 * \ingroup tracks
 */
class ModelDefinitionLoader
{
private:
    std::map<std::string, XMLNode*> m_library_nodes;
    std::map< std::string, std::vector< ModelDefinition > > m_lod_groups;
    std::map< std::string, STKInstancedSceneNode* > m_instancing_nodes;
    Track* m_track;

public:
         ModelDefinitionLoader(Track* track);

    void addModelDefinition(const XMLNode* xml);
    LODNode* instanciateAsLOD(const XMLNode* xml_node,
                              scene::ISceneNode* parent,
                              std::shared_ptr<GE::GERenderInfo> ri);

    void clear();

    scene::IMesh* getFirstMeshFor(const std::string& name);

    std::map<std::string, XMLNode*>& getLibraryNodes()
    {
        return m_library_nodes;
    }

    void cleanLibraryNodesAfterLoad();

    bool containsLibraryNode(const std::string& name) const
    {
        return m_library_nodes.find(name) != m_library_nodes.end();
    }

    void addToLibrary(const std::string& name, XMLNode* xml)
    {
        assert(xml != NULL);
        m_library_nodes[name] = xml;
    }
};  // ModelDefinitionLoader

#endif // HEADER_LOD_NODE_LOADER_HPP
