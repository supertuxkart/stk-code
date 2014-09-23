//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013  Joerg Henrichs
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

#ifndef HEADER_CHECK_MANAGER_HPP
#define HEADER_CHECK_MANAGER_HPP

#include "utils/no_copy.hpp"

#include <assert.h>
#include <string>
#include <vector>

class CheckStructure;
class Track;
class XMLNode;
class Vec3;

/**
  * \brief Controls all checks structures of a track.
  * \ingroup tracks
  */
class CheckManager : public NoCopy
{
private:
    std::vector<CheckStructure*> m_all_checks;
    static CheckManager         *m_check_manager;
           /** Private constructor, to make sure it is only called via
            *  the static create function. */
           CheckManager()       {m_all_checks.clear();};
          ~CheckManager();
public:
    void   load(const XMLNode &node);
    void   update(float dt);
    void   reset(const Track &track);
    unsigned int getLapLineIndex() const;
    int    getChecklineTriggering(const Vec3 &from, const Vec3 &to) const;
    // ------------------------------------------------------------------------
    /** Creates an instance of the check manager. */
    static void create()
    {
        assert(!m_check_manager);
        m_check_manager = new CheckManager();
    }   // create
    // ------------------------------------------------------------------------
    /** Returns the instance of the check manager. */
    static CheckManager* get() { return m_check_manager; }
    // ------------------------------------------------------------------------
    /** Destroys the check manager. */
    static void destroy() { delete m_check_manager; m_check_manager = NULL; }
    // ------------------------------------------------------------------------
    /** Returns the number of check structures defined. */
    unsigned int getCheckStructureCount() const { return (unsigned int) m_all_checks.size(); }
    // ------------------------------------------------------------------------
    /** Returns the nth. check structure. */
    CheckStructure *getCheckStructure(unsigned int n) const
    {
        assert(n < m_all_checks.size());
        return m_all_checks[n];
    }
};   // CheckManager

#endif

