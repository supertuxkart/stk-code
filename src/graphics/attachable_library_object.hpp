//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015  Joerg Henrichs
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

#ifndef HEADER_ATTACHABLE_LIBRARY_OBJECT_HPP
#define HEADER_ATTACHABLE_LIBRARY_OBJECT_HPP

#include <vector3d.h>

#include "tracks/track_object_presentation.hpp"
#include "utils/cpp2011.hpp"
#include "utils/vec3.hpp"
#include <string>
#include "animations/three_d_animation.hpp"

#include <memory>

namespace GE { class GERenderInfo; }
class ThreeDAnimation;
class XMLNode;

namespace irr
{
    namespace scene
    {
        class IAnimatedMeshSceneNode;
    }
}

using namespace irr;

/**
 * \ingroup graphics
 *  This is a base object for an object that's part of an attachable library
 *  It is graphics-only, with no physics.
 */
class AttachableLibraryObject
{
private:
    /** True if the object is currently being displayed. */
    bool                     m_enabled;

    TrackObjectPresentation* m_presentation;

    std::string m_name;
    std::string m_id;

    std::shared_ptr<GE::GERenderInfo>    m_render_info;

    // Private constructor for cloning
    AttachableLibraryObject(const std::string& name, const std::string& id,
        core::vector3df xyz, core::vector3df hpr, core::vector3df scale,
        bool enabled, const std::string& type, scene::ISceneNode* parent,
        video::SColor color, float distance, float energy,
        const std::string& kind_path, int clip_distance,
        const std::string& trigger_condition, bool auto_emit,
        const std::string& model_path, const std::string& lib_ident,
        unsigned int instance);

protected:

    /** The initial XYZ position of the object. */
    core::vector3df                m_init_xyz;
    /** The initial hpr of the object. */
    core::vector3df                m_init_hpr;
    /** The initial scale of the object. */
    core::vector3df                m_init_scale;

    std::string                    m_type;
    ThreeDAnimation*               m_animator;

    void init(const XMLNode &xml_node, scene::ISceneNode* parent,
              const std::string& lib_ident);

public:
                 AttachableLibraryObject(const XMLNode &xml_node,
                    scene::ISceneNode* parent, const std::string& lib_ident);
    virtual      ~AttachableLibraryObject();
    virtual void updateGraphics(float dt);

    virtual void reset();

    void         setID(std::string obj_id) { m_id = obj_id; }

    // ------------------------------------------------------------------------
    const std::string& getType() const { return m_type; }
    // ------------------------------------------------------------------------
    const std::string getName() const { return m_name; }
    // ------------------------------------------------------------------------
    const std::string getID() const { return m_id; }
    // ------------------------------------------------------------------------
    bool isEnabled() const { return m_enabled; }
    // ------------------------------------------------------------------------
    const core::vector3df getInitXYZ() const { return m_init_xyz; }
    // ------------------------------------------------------------------------
    const core::vector3df getInitRotation() const { return m_init_hpr; }
    // ------------------------------------------------------------------------
    const core::vector3df getInitScale() const { return m_init_scale; }
    // ------------------------------------------------------------------------
    template<typename T>
    T* getPresentation() { return dynamic_cast<T*>(m_presentation); }
    // ------------------------------------------------------------------------
    template<typename T>
    const T* getPresentation() const { return dynamic_cast<T*>(m_presentation); }
    // ------------------------------------------------------------------------
    ThreeDAnimation* getAnimator() { return m_animator; }
    // ------------------------------------------------------------------------
    const ThreeDAnimation* getAnimator() const { return m_animator; }
    // ------------------------------------------------------------------------
    void setPaused(bool mode){ m_animator->setPaused(mode); }
    // ------------------------------------------------------------------------
    AttachableLibraryObject* clone(scene::ISceneNode* parent, const std::string& lib_folder,
                                   const std::string& lib_ident, unsigned int instance);

    // ------------------------------------------------------------------------
    /** To prevent incorrect clearing for library templates, we grab/drop
     * the object's presentation node as needed. */
    void grabNode();
    void dropNode();

    LEAK_CHECK()
};   // AttachableLibraryObject

#endif
