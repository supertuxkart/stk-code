//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2019 dumaosen
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

#ifndef HEADER_TIPS_MANAGER_HPP
#define HEADER_TIPS_MANAGER_HPP

#include "tips/tip_set.hpp"

#include <assert.h>
#include <string>
#include <map>

/** This class manages the list of all tips. It reads the
 *  data/tips.xml file, which contains the contents for
 *  each tip.
  */
class TipsManager
{
private:
    /** Pointer to the single instance. */
    static TipsManager* m_tips_manager;

    std::map<std::string, TipSet *> m_all_tip_sets;

    TipsManager      ();
    ~TipsManager     ();

public:
    /** Static function to create the instance of the tips manager. */
    static void create()
    {
        assert(!m_tips_manager);
        m_tips_manager = new TipsManager();
    }   // create
    // ------------------------------------------------------------------------
    /** Static function to get the tips manager. */
    static TipsManager* get()
    {
        assert(m_tips_manager);
        return m_tips_manager;
    }   // get
    // ------------------------------------------------------------------------
    static void destroy()
    {
        delete m_tips_manager;
        m_tips_manager = NULL;
    }   // destroy
    // ========================================================================

    TipSet* getTipSet(std::string id) const;
    // ------------------------------------------------------------------------
};   // class TipsManager

#endif
