//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2008-2015 Joerg Henrichs
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
    const XMLNode* node = challenges_node->getNode( m_data->getChallengeId() );
    if(node == NULL)
    {
        Log::info("ChallengeStatus", "Couldn't find node <%s> in challenge list."
                "(If this is the first time you play this is normal)\n",
                m_data->getChallengeId().c_str());
        return;
    }

    m_active = 0;
    m_solved = 0;

    std::string solved;
    if (node->get("solved", &solved))
    {
        // Solving at a difficulty also marks lower difficulties as solved
        if (solved == "easy")
            m_solved = 0x01;
        else if (solved == "medium")
            m_solved = 0x03;
        else if (solved == "hard")
            m_solved = 0x07;
        else if (solved == "best")
            m_solved = 0x0F;
    }   // if has 'solved' attribute
    if (!node->get("best_while_slower", &m_max_req_in_lower_diff))
        m_max_req_in_lower_diff = false;
}   // load

//-----------------------------------------------------------------------------
// Solve not only the current difficulty but all those before
// e.g. if you solved hard then you also get easy.
// Also resets active flag.
void ChallengeStatus::setSolved(RaceManager::Difficulty d)
{
    if ((int) d <= 3)
    {
        m_solved |= (0x0F >> (3 - (int) d)); // Sets the last d+1 bits to 1
        m_active &= ~m_solved; // Sets to 0 all bits which are at 1 in m_solved
    }
} // setSolved

// ------------------------------------------------------------------------
bool ChallengeStatus::isUnlockList()
{
    return m_data->isUnlockList();
} // isUnlockList

// ------------------------------------------------------------------------
bool ChallengeStatus::isGrandPrix()
{
    return m_data->isGrandPrix();
} // isUnlockList

//-----------------------------------------------------------------------------

void ChallengeStatus::save(UTFWriter& writer)
{
    writer << "        <" << m_data->getChallengeId();
    if (isSolved(RaceManager::DIFFICULTY_BEST))
        writer << " solved=\"best\"";
    else if (isSolved(RaceManager::DIFFICULTY_HARD))
        writer << " solved=\"hard\"";
    else if (isSolved(RaceManager::DIFFICULTY_MEDIUM))
        writer << " solved=\"medium\"";
    else if (isSolved(RaceManager::DIFFICULTY_EASY))
        writer << " solved=\"easy\"";
    else
        writer << " solved=\"none\"";

    writer << " best_while_slower=\"" << m_max_req_in_lower_diff << "\"/>\n";
}   // save
