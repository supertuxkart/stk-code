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

#include "network/rewinder.hpp"

#include "network/rewind_manager.hpp"

/** Constructor. It will add this object to the list of all rewindable
 *  objects in the rewind manager.
 */
Rewinder::Rewinder(const std::string& ui, bool can_be_destroyed, bool auto_add)
        : m_unique_identity(ui)
{
    assert(!m_unique_identity.empty() && m_unique_identity.size() < 255);
    m_can_be_destroyed = can_be_destroyed;
    if (auto_add)
        add();
}   // Rewinder

// ----------------------------------------------------------------------------
/** Destructor.
 */
Rewinder::~Rewinder()
{
}   // ~Rewinder

// ----------------------------------------------------------------------------
void Rewinder::add()
{
    RewindManager::get()->addRewinder(this);
}   // Rewinder
