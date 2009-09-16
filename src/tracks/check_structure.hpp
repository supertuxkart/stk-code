//  $Id: check_structure.hpp 1681 2008-04-09 13:52:48Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009  Joerg Henrichs
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

/** Virtual base class for a check structure. A check structure has a certain
 *  type:
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
 */
class CheckStructure
{
public:
    /** Different types of check structures: one which triggers couting a new 
     *  lap, and one which resets all lap counters. This is used to avoid
     *  shortcuts: a new lap is only counted if a reset_new_lap check 
     *  structure was triggered in between. So by adding a single reset
     *  structure at the rhoughly halfway mark of the track karts have to
     *  drive there first before a new lap will be counted.
     */
    enum CheckType {CT_NEW_LAP, CT_RESET_NEW_LAP, CT_AMBIENT_SPHERE};

protected:
	/** Stores the previous position of all karts. This is needed to detect
     *  when e.g. a check point is reached the first time, or a checkline is
     *  crossed. */
    std::vector<Vec3> m_previous_position;
    /** Stores if this check structure is active (for a given kart). USed e.g.
     *  in lap counting. */
    std::vector<bool> m_is_active;
private:
    /** Stores a pointer to the check manager. */
    CheckManager      *m_check_manager;

    /** The type of this checkline. */
    CheckType         m_check_type;
    /** True if this check structure should be activated at a reset. */
    bool              m_active_at_reset;
public:
                CheckStructure(CheckManager *check_manager, const XMLNode &node);
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
    virtual void activateNewLapCheck(int kart_index);
    virtual Vec3 getCenterPoint() const=0;
    /** Returns the type of this check structure. */
    CheckType getType() const { return m_check_type; }
};   // CheckStructure

#endif

