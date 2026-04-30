//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2026 Alayan
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

#ifndef HEADER_ATTACHABLE_LIBRARY_MANAGER_HPP
#define HEADER_ATTACHABLE_LIBRARY_MANAGER_HPP

#include "graphics/attachable_library_object.hpp"
#include "io/xml_node.hpp"
#include "utils/no_copy.hpp"

#include <cassert>
#include <irrlicht.h>
#include <map>


using namespace irr;

class MovingTexture;

class AttachableLibraryManager: public NoCopy
{
protected:
    /** The initial XYZ position of the object. */
    core::vector3df                m_init_xyz;

    /** The initial hpr of the object. */
    core::vector3df                m_init_hpr;

    /** The initial scale of the object. */
    core::vector3df                m_init_scale;

private:
    /** Singleton pointer. */
    static AttachableLibraryManager* m_attachable_lib_manager;

    std::map<std::string, scene::ISceneNode*> m_attachable_lib_nodes;
    std::map<std::string, std::string> m_folder_map;
    std::map<std::string, std::vector<AttachableLibraryObject*>> m_all_objects;

    /** The list of all animated textures. */
    std::map<std::string, std::vector<MovingTexture*>> m_animated_textures;

    AttachableLibraryManager();
    ~AttachableLibraryManager();

    scene::ISceneNode* loadLibraryInstance(const std::string& identifier);
    void add(const XMLNode &xml_node, const std::string& parent_id);
public:
    /** Create the singleton instance. */
    static void create()
    {
        assert(!m_attachable_lib_manager);
        m_attachable_lib_manager = new AttachableLibraryManager();
    }   // create

    // ----------------------------------------------------------------
    /** Returns the singleton.
     *  \pre create has been called to create the singleton.
     */
    static AttachableLibraryManager* get()
    {
        assert(m_attachable_lib_manager);
        return m_attachable_lib_manager;
    }   // get
    // ----------------------------------------------------------------
    /** Destroys the singleton. */
    static void destroy()
    {
        delete m_attachable_lib_manager;
        m_attachable_lib_manager = NULL;
    }   // destroy

    void loadLibraryNode(const std::string& folder_name, const std::string& identifier);
    void handleAnimatedTextures(scene::ISceneNode *node, const XMLNode &xml,
                                const std::string& lib_ident);
    void handleAnimatedTextures(scene::ISceneNode *node, const std::string& lib_ident,
                                unsigned int instance);
    void updateGraphics(float dt);
    scene::ISceneNode* createInstance(const std::string& name);

};  // AttachableLibraryManager

#endif