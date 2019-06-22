//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#ifndef HEADER_ATTACHMENT_HPP
#define HEADER_ATTACHMENT_HPP

#include "items/attachment_plugin.hpp"
#include "utils/no_copy.hpp"
#include "utils/types.hpp"

using namespace irr;

namespace irr
{
    namespace scene { class IAnimatedMeshSceneNode; }
}

class AbstractKart;
class BareNetworkString;
class ItemState;
class SFXBase;

/** This objects is permanently available in a kart and stores information
 *  about addons. If a kart has no attachment, this object will have the
 *  attachment type ATTACH_NOTHING. This way other tests for attachment
 *  in STK do not have to additionally test if there is an attachment, all
 *  tests for a type will always be valid.
 *  Certain attachments need additional coding, this is supported by
 *  a 'plugin' mechanism: This attachment will forward certain calls to
 *  (see attachment_pluging abstract class). Compared to normal subclassing
 *  (i.e. replacing the attachment object each time an attachment changes)
 *  this has less overhead (since the attachment class always creates
 *  a scene node).
 *  \ingroup items
 */
class Attachment: public NoCopy
{
public:
    // Some loop in attachment.cpp depend on ATTACH_FIRST and ATTACH_MAX.
    // So if new elements are added, make sure to add them in between those values.
    // Also, please note that Attachment::Attachment relies on ATTACH_FIRST being 0.
    enum AttachmentType
    {
        ATTACH_FIRST = 0,
        // It is important that parachute, bomb and anvil stay in this order,
        // since the attachment type is mapped to a random integer (and bomb
        // must be last, since a bomb will not be given in battle mode).
        ATTACH_PARACHUTE = 0,
        ATTACH_ANVIL = 1,
        ATTACH_BOMB = 2,
        // End of fixed order attachments, the rest can be changed.
        ATTACH_SWATTER,
        // Note that the next 2 symbols are only used as an index into the mesh
        // array; it will NEVER be actually assigned as an attachment type
        ATTACH_NOLOKS_SWATTER,
        ATTACH_SWATTER_ANIM,
        ATTACH_BUBBLEGUM_SHIELD,
        ATTACH_NOLOK_BUBBLEGUM_SHIELD,
        ATTACH_MAX,
        ATTACH_NOTHING
    };

private:
    /** Attachment type. */
    AttachmentType  m_type;

    /** Graphical Attachment type (comparing in updateGraphics). */
    AttachmentType m_graphical_type;

    /** Kart the attachment is attached to. */
    AbstractKart   *m_kart;

    /** Time left till attachment expires. */
    int16_t         m_ticks_left;

    /** For parachutes only, stored in cm/s for networking. */
    int16_t         m_initial_speed;

    /** For zoom-in animation */
    int             m_scaling_end_ticks;

    /** Scene node of the attachment, which will be attached to the kart's
     *  scene node. */
    scene::IAnimatedMeshSceneNode
                     *m_node;

    /** Used by bombs so that it's not passed back to previous owner. */
    AbstractKart     *m_previous_owner;

    /** An optional attachment - additional functionality can be implemented
     *  for certain attachments. */
    AttachmentPlugin *m_plugin;

    /** Ticking sound for the bomb */
    SFXBase          *m_bomb_sound;

    /** Sound for exploding bubble gum shield */
    SFXBase          *m_bubble_explode_sound;

public:
          Attachment(AbstractKart* kart);
         ~Attachment();
    void  clear();
    void  hitBanana(ItemState *item);
    void  updateGraphics(float dt);

    void  update(int ticks);
    void  handleCollisionWithKart(AbstractKart *other);
    void  set (AttachmentType type, int ticks,
               AbstractKart *previous_kart=NULL,
               bool set_by_rewind_parachute = false);
    void rewindTo(BareNetworkString *buffer);
    void saveState(BareNetworkString *buffer) const;

    // ------------------------------------------------------------------------
    /** Sets the type of the attachment, but keeps the old time left value. */
    void  set (AttachmentType type) { set(type, m_ticks_left); }
    // ------------------------------------------------------------------------
    /** Returns the type of this attachment. */
    AttachmentType getType() const { return m_type; }
    // ------------------------------------------------------------------------
    /** Returns how much time (in ticks) is left before this attachment is 
     *  removed. */
    int16_t getTicksLeft() const                       { return m_ticks_left; }
    // ------------------------------------------------------------------------
    /** Sets how long this attachment will remain attached. */
    void setTicksLeft(int16_t t)                          { m_ticks_left = t; }
    // ------------------------------------------------------------------------
    /** Returns the previous owner of this attachment, used in bombs that
     *  are being passed between karts. */
    AbstractKart* getPreviousOwner() const { return m_previous_owner; }
    // ------------------------------------------------------------------------
    /** Returns additional weight for the kart. */
    float weightAdjust() const;
    // ------------------------------------------------------------------------
    /** Return the currently associated scene node (used by e.g the swatter) */
    scene::IAnimatedMeshSceneNode* getNode() {return m_node;}
    // ------------------------------------------------------------------------
    void reset()
    {
        clear();
        m_scaling_end_ticks = -1;
    }

};   // Attachment

#endif
