//  $Id: check_structure.hpp 1681 2008-04-09 13:52:48Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2010  Joerg Henrichs
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

#ifndef HEADER_CHECK_STRUCTURE_HPP
#define HEADER_CHECK_STRUCTURE_HPP

#include <vector>

#include "utils/vec3.hpp"

class XMLNode;
class Track;
class CheckManager;

/** 
 * \brief Virtual base class for a check structure.
 *
 * A check structure has a certain ype:
 *  CT_NEW_LAP  : triggering this check structure will cause a new lap to be
 *                counted. If this type is triggered, it will set itselt to
 *                inactive (which means it is not possible to count several
 *                laps by driving over the starting line forwardws and 
 *                backwards)
 *  CT_RESET_NEW_LAP: Activates all lap checks. Each track must have at least
 *                one reset checks somewhere on the track. This is used to 
 *                avoid shortcuts, since karts are forced to cross this reset
 *                check first before a new lap can be counted.
 * Each check structure can be active or inactive. A new_la counter is 
 * initialised as non-active, so that karts have to trigger a reset check
 * before a lap can be counted.
 *
 * \ingroup tracks
 */
class CheckStructure
{
public:
    /** Different types of check structures: 
     *  ACTIVATE: Activates another check structure (independent of
     *            the state that check structure is in)
     *  TOGGLE:   Switches (inverts) the state of another check structure.
     *  NEW_LAP:  On crossing a new lap is counted.
     *  AMBIENT_SPHERE: Modifies the ambient color.
     *  A combination of an activate and new_lap line are used to
     *  avoid shortcuts: a new_lap line is deactivated after crossing it, and
     *  you have to cross a corresponding activate structure to re-activate it,
     *  enabling you to count the lap again.
     */
    enum CheckType {CT_NEW_LAP, CT_ACTIVATE, CT_TOGGLE, CT_AMBIENT_SPHERE};

protected:
    /** Stores the previous position of all karts. This is needed to detect
     *  when e.g. a check point is reached the first time, or a checkline is
     *  crossed. */
    std::vector<Vec3> m_previous_position;
    /** Stores if this check structure is active (for a given kart). Used e.g.
     *  in lap counting. */
    std::vector<bool> m_is_active;
private:
    /** Stores a pointer to the check manager. */
    CheckManager      *m_check_manager;

    /** The type of this checkline. */
    CheckType         m_check_type;

    /** Stores the index of this check structure. This is only used for
     *  debugging (use --check-debug option). */
    unsigned int      m_index;

    /** True if this check structure should be activated at a reset. */
    bool              m_active_at_reset;

    /** If this is a CT_ACTIVATE or CT_SWITCH type, this will contain
     *  the indices of the corresponding check structures that get their
     *  state changed (activated or switched). */
    std::vector<int> m_check_structures_to_change_state;

    /** A list of check lines that should be activated/switched when this
     *  lines is activated/switched. I.e. can be used if more than one lap
     *  counting line is used to make sure they are all in synch, otherwise
     *  players could cross first one then the other lap counting line
     *  as huge shortcuts. */
    std::vector<int> m_same_group;

public:
                CheckStructure(CheckManager *check_manager, const XMLNode &node,
                               unsigned int index);
    virtual    ~CheckStructure() {};
    virtual void update(float dt);
    /** True if going from old_pos to new_pos crosses this checkline. This function
     *  is called from update (of the checkline structure).
     *  \param old_pos  Position in previous frame.
     *  \param new_pos  Position in current frame.
     *  \param indx     Index of the kart, can be used to store kart specific
     *                  additional data.
     */
    virtual bool isTriggered(const Vec3 &old_pos, const Vec3 &new_pos, int indx)=0;
    virtual void trigger(unsigned int kart_index);
    virtual void reset(const Track &track);
    virtual Vec3 getCenterPoint() const=0;

    /** Returns the type of this check structure. */
    CheckType getType() const { return m_check_type; }
};   // CheckStructure

#endif

