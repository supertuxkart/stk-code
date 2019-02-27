//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Joerg Henrichs
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

#ifndef HEADER_REWINDER_HPP
#define HEADER_REWINDER_HPP

#include <cassert>
#include <functional>
#include <string>
#include <memory>
#include <vector>

class BareNetworkString;

enum RewinderName : char
{
    RN_ITEM_MANAGER = 0x01,
    RN_KART = 0x02,
    RN_RED_FLAG = 0x03,
    RN_BLUE_FLAG = 0x04,
    RN_CAKE = 0x05,
    RN_BOWLING = 0x06,
    RN_PLUNGER = 0x07,
    RN_RUBBERBALL = 0x08,
    RN_PHYSICAL_OBJ = 0x09
};

class Rewinder : public std::enable_shared_from_this<Rewinder>
{
protected:
    void setUniqueIdentity(const std::string& uid)  { m_unique_identity = uid; }
private:
    /** Currently it has 2 usages:
     *  1. Create the required flyable if the firing event missed using this
     *     uid. (see RewindInfoState::restore)
     *  2. Determine the order of restoring state for each rewinder, this is
     *     used as a key in std::string, Rewinder map, which is less than.
     *     So uid of "0x01" (item manager) is restored before "0x02, x" (which
     *     kart id x) and 0x03 / 0x04 (the red / blue flag) is restored after
     *     karts, because the restoreState in CTFFlag read kart transformation.
    */
    std::string m_unique_identity;

public:
    Rewinder(const std::string& ui = "")             { m_unique_identity = ui; }

    virtual ~Rewinder() {}

    /** Called before a rewind. Is used to save the previous position of an
     *  object before a rewind, so that the error due to a rewind can be
     *  computed. */
    virtual void saveTransform() = 0;

    /** Called when a rewind is finished, and is used to compute the error
     *  caused by the rewind (which is then visually smoothed over time). */
    virtual void computeError() = 0;

    /** Provides a copy of the state of the object in one memory buffer.
     *  The memory is managed by the RewindManager.
     *  \param[out] ru The unique identity of rewinder writing to.
     *  \return The address of the memory buffer with the state.
     */
    virtual BareNetworkString* saveState(std::vector<std::string>* ru) = 0;

    /** Called when an event needs to be undone. This is called while going
     *  backwards for rewinding - all stored events will get an 'undo' call.
     */
    virtual void undoEvent(BareNetworkString *buffer) = 0;

    /** Called when an event needs to be replayed. This is called during
     *  rewind, i.e. when going forward in time again.
     */
    virtual void rewindToEvent(BareNetworkString *buffer) = 0;

    /** Called when a state needs to be replayed. This is called during
     *  rewind, i.e. when going forward in time again, and only for confirmed
     *  states.
     */
    virtual void restoreState(BareNetworkString *buffer, int count) = 0;

    /** Undo the effects of the given state, but do not rewind to that
     *  state (which is done by rewindTo). This is called while going
     *  backwards for rewinding - all stored events will get an 'undo' call.
     */
    virtual void undoState(BareNetworkString *buffer) = 0;

    // -------------------------------------------------------------------------
    /** Nothing to do here. */
    virtual void reset() {}
    // -------------------------------------------------------------------------
    virtual std::function<void()> getLocalStateRestoreFunction()
                                                             { return nullptr; }
    // -------------------------------------------------------------------------
    const std::string& getUniqueIdentity() const
    {
        assert(!m_unique_identity.empty() && m_unique_identity.size() < 255);
        return m_unique_identity;
    }
    // -------------------------------------------------------------------------
    bool rewinderAdd();
    // -------------------------------------------------------------------------
    template<typename T> std::shared_ptr<T> getShared()
                    { return std::dynamic_pointer_cast<T>(shared_from_this()); }

};   // Rewinder
#endif

