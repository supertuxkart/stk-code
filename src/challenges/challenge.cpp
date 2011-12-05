//  $Id$
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

#include "challenges/challenge.hpp"

#include <fstream>

#include "challenges/challenge_data.hpp"
#include "io/xml_node.hpp"
#include "io/xml_writer.hpp"
#include "karts/kart_properties_manager.hpp"
#include "karts/kart_properties.hpp"
#include "race/grand_prix_manager.hpp"
#include "race/race_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"

//-----------------------------------------------------------------------------
/** Loads the state for a challenge object (esp. m_state)
 */
void Challenge::load(const XMLNode* challengesNode)
{
    const XMLNode* node = challengesNode->getNode( m_data->getId() );
    if(node == NULL) return;
    
    // See if the challenge is solved (it's activated later from the
    // unlock_manager).
    bool finished=false;    
    node->get("solved", &finished);
    m_state = finished ? CH_SOLVED : CH_INACTIVE;
}   // load

//-----------------------------------------------------------------------------
void Challenge::save(XMLWriter& writer)
{
    writer << L"        <" << core::stringw(m_data->getId().c_str()) << L" solved=\"" 
           << (isSolved() ? L"true" : L"false") << L"\"";
    writer << L" />\n";
}   // save
