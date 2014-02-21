//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2013 Joerg Henrichs
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

#include "challenges/challenge_status.hpp"

#include "challenges/challenge_data.hpp"
#include "io/utf_writer.hpp"
#include "io/xml_node.hpp"
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
void ChallengeStatus::load(const XMLNode* challenges_node)
{
    const XMLNode* node = challenges_node->getNode( m_data->getId() );
    if(node == NULL)
    {
        Log::info("ChallengeStatus", "Couldn't find node <%s> in challenge list."
                "(If this is the first time you play this is normal)\n",
                m_data->getId().c_str());
        return;
    }
    const XMLNode* easy   = node->getNode("easy");
    const XMLNode* medium = node->getNode("medium");
    const XMLNode* hard   = node->getNode("hard");

    m_state[0] = CH_INACTIVE;
    m_state[1] = CH_INACTIVE;
    m_state[2] = CH_INACTIVE;

    if (easy != NULL)
    {
        bool finished = false;
        easy->get("solved", &finished);

        if (finished) m_state[0] = CH_SOLVED;
    }
    if (medium != NULL)
    {
        bool finished = false;
        medium->get("solved", &finished);

        if (finished) m_state[1] = CH_SOLVED;
    }
    if (hard != NULL)
    {
        bool finished = false;
        hard->get("solved", &finished);

        if (finished) m_state[2] = CH_SOLVED;
    }
}   // load

//-----------------------------------------------------------------------------

void ChallengeStatus::setSolved(RaceManager::Difficulty d)
{
    // solve not only the current difficulty but all those before
    // e.g. if you solved hard then you also get easy
    for (int curr = 0; curr <= d; curr++)
    {
        m_state[curr] = CH_SOLVED;
    }
}

//-----------------------------------------------------------------------------

void ChallengeStatus::save(UTFWriter& writer)
{
    writer << L"        <"<< m_data->getId() << L">\n"
           << L"            <easy   solved=\"" 
           << isSolved(RaceManager::DIFFICULTY_EASY)
           << L"\"/>\n"
           << L"            <medium solved=\"" 
           << isSolved(RaceManager::DIFFICULTY_MEDIUM)
           << L"\"/>\n"
           << L"            <hard   solved=\"" 
           << isSolved(RaceManager::DIFFICULTY_HARD)
           << L"\"/>\n"
           << L"        </" << m_data->getId() << L">\n";
}   // save
