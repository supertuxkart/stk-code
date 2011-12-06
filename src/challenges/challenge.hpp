//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008 Joerg Henrichs
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

#ifndef HEADER_CHALLENGE_HPP
#define HEADER_CHALLENGE_HPP

/**
  * \defgroup challenges
  * This module handles the challenge system, which locks features (tracks, karts
  * modes, etc.) until the user completes some task.
  */

#include <string>
#include <vector>
#include <fstream>
#include <irrString.h>

#include "utils/no_copy.hpp"
#include "utils/translation.hpp"

class XMLNode;
class XMLWriter;
class ChallengeData;

/**
  * \brief The state of a challenge for one player
  * \ingroup challenges
  */
class Challenge : public NoCopy
{
private:
    enum {CH_INACTIVE,                 // challenge not yet possible
          CH_ACTIVE,                   // challenge possible, but not yet solved
          CH_SOLVED}         m_state;  // challenge was solved

    ChallengeData* m_data;
    
public:
             Challenge(ChallengeData* data) : m_state(CH_INACTIVE)
    { m_data = data; }
    virtual ~Challenge() {};
    void  load(const XMLNode* config);
    void  save(XMLWriter& writer);

    // ------------------------------------------------------------------------
    bool  isSolved() const                       {return m_state==CH_SOLVED;  }
    // ------------------------------------------------------------------------
    bool  isActive() const                       {return m_state==CH_ACTIVE;  }
    // ------------------------------------------------------------------------
    void  setSolved()                            {m_state = CH_SOLVED;        }
    // ------------------------------------------------------------------------
    void  setActive()                            {m_state = CH_ACTIVE;        }
    // ------------------------------------------------------------------------
    ChallengeData*  getData() { return m_data; }
};
#endif
