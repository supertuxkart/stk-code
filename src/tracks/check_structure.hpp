//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015  Joerg Henrichs
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

#include "utils/aligned_array.hpp"
#include "utils/vec3.hpp"

class BareNetworkString;
class CheckManager;
class Track;
class XMLNode;

/**
 * \brief Virtual base class for a check structure.
 *
 *  A check structure has a certain ype:
 *  CT_NEW_LAP  : triggering this check structure will cause a new lap to be
 *                counted. If this type is triggered, it will set itselt to
 *                inactive (which means it is not possible to count several
 *                laps by driving over the starting line forwardws and
 *                backwards)
 *  CT_ACTIVATE:  Activates the specified other check structures.
 *  CT_TOGGLE:    Toggles the specified other check structures (active to
 *                inactive and vice versa.
 *  CT_CANNON:    A check line that 'shoots' the kart to a specified location.
 *  CT_GOAL:      A goal line in soccer mode.
 *  Each check structure can be active or inactive. Only lap counters are
 *  initialised to be active, all other check structures are inactive.
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
     *  CANNON:   Causes the kart to be shot to a specified point.
     *  GOAL:     Causes a point to be scored when a soccer ball crosses its line
     *  AMBIENT_SPHERE: Modifies the ambient color.
     *  TRIGGER:  Run custom trigger function
     *  A combination of an activate and new_lap line are used to
     *  avoid shortcuts: a new_lap line is deactivated after crossing it, and
     *  you have to cross a corresponding activate structure to re-activate it,
     *  enabling you to count the lap again.
     */
    enum CheckType {CT_NEW_LAP, CT_ACTIVATE, CT_TOGGLE, CT_CANNON,
                    CT_GOAL, CT_AMBIENT_SPHERE, CT_TRIGGER};

protected:
    /** Stores the previous position of all karts. This is needed to detect
     *  when e.g. a check point is reached the first time, or a checkline is
     *  crossed. */
    AlignedArray<Vec3> m_previous_position;
    /** Stores if this check structure is active (for a given kart). */
    std::vector<bool> m_is_active;

    /** True if this check structure should be activated at a reset. */
    bool              m_active_at_reset;

    /** Stores the index of this check structure. This is only used for
     *  debugging (use --check-debug option). */
    unsigned int      m_index;

    /** For CheckTrigger or CheckCylinder */
    CheckStructure();
private:
    /** The type of this checkline. */
    CheckType         m_check_type;

    /** Contains the indices of the corresponding check structures that
     *  get their state changed (activated or switched). */
    std::vector<int> m_check_structures_to_change_state;

    /** A list of check lines that should be activated/switched when this
     *  lines is activated/switched. I.e. can be used if more than one lap
     *  counting line is used to make sure they are all in synch, otherwise
     *  players could cross first one then the other lap counting line
     *  as huge shortcuts. */
    std::vector<int> m_same_group;

    enum ChangeState {CS_DEACTIVATE, CS_ACTIVATE, CS_TOGGLE};

    void changeStatus(const std::vector<int> &indices, int kart_index,
                      ChangeState change_state);

public:
                CheckStructure(const XMLNode &node, unsigned int index);
    virtual    ~CheckStructure() {};
    virtual void update(float dt);
    virtual void resetAfterKartMove(unsigned int kart_index) {}
    virtual void resetAfterRewind(unsigned int kart_index) {}
    virtual void changeDebugColor(bool is_active) {}
    /** True if going from old_pos to new_pos crosses this checkline. This function
     *  is called from update (of the checkline structure).
     *  \param old_pos  Position in previous frame.
     *  \param new_pos  Position in current frame.
     *  \param indx     Index of the kart, can be used to store kart specific
     *                  additional data.
     */
    virtual bool isTriggered(const Vec3 &old_pos, const Vec3 &new_pos,
                             int indx)=0;
    virtual void trigger(unsigned int kart_index);
    virtual void reset(const Track &track);

    // ------------------------------------------------------------------------
    /** Returns the type of this check structure. */
    CheckType getType() const { return m_check_type; }
    // ------------------------------------------------------------------------
    /** Adds the index of a successor check structure which will get triggered
     *  by this check structure. */
    void addSuccessor(unsigned int i)
    {
        m_check_structures_to_change_state.push_back(i);
    }   // addSuccessor
    // ------------------------------------------------------------------------
    virtual bool triggeringCheckline() const { return false; }
    // ------------------------------------------------------------------------
    virtual void saveCompleteState(BareNetworkString* bns);
    // ------------------------------------------------------------------------
    virtual void restoreCompleteState(const BareNetworkString& b);
    // ------------------------------------------------------------------------
    void saveIsActive(int kart_id, BareNetworkString* bns);
    // ------------------------------------------------------------------------
    void restoreIsActive(int kart_id, const BareNetworkString& b);
    // ------------------------------------------------------------------------
    int getIndex() const { return m_index; }
    // ------------------------------------------------------------------------
    /** Clone to child process for server usage (atm no sound or scripting). */
    virtual CheckStructure* clone() = 0;
};   // CheckStructure

#endif

