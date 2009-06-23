//  $Id: challenge.cpp 1259 2007-09-24 12:28:19Z hiker $
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

#include <iostream>

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
void Challenge::addUnlockTrackReward(const std::string &track_name)
{
    UnlockableFeature feature;
    feature.name = track_name;
    feature.type = UNLOCK_TRACK;
    m_feature.push_back(feature);
}

//-----------------------------------------------------------------------------
void Challenge::addUnlockModeReward(const std::string &internal_mode_name,
                                    const std::string &user_mode_name)
{
    UnlockableFeature feature;
    feature.name = internal_mode_name;
    feature.type = UNLOCK_MODE;
    feature.user_name = user_mode_name;
    m_feature.push_back(feature);
}

//-----------------------------------------------------------------------------
void Challenge::addUnlockGPReward(const std::string &gp_name)
{
    UnlockableFeature feature;
    feature.name = _(gp_name.c_str());
    feature.type = UNLOCK_GP;
    m_feature.push_back(feature);
}

//-----------------------------------------------------------------------------
void Challenge::addUnlockDifficultyReward(const std::string &internal_name, 
                                          const std::string &user_name)
{
    UnlockableFeature feature;
    feature.name = internal_name;
    feature.type = UNLOCK_DIFFICULTY;
    feature.user_name = user_name;
    m_feature.push_back(feature);
}

//-----------------------------------------------------------------------------
void Challenge::addUnlockKartReward(const std::string &internal_name, 
                                    const std::string &user_name)
{
    UnlockableFeature feature;
    feature.name = internal_name;
    feature.type = UNLOCK_KART;
    feature.user_name = user_name;
    m_feature.push_back(feature);
}

//-----------------------------------------------------------------------------
const std::string Challenge::getUnlockedMessage() const
{
    std::string unlocked_message;

    const unsigned int amount = (unsigned int)m_feature.size();
    for(unsigned int n=0; n<amount; n++)
    {
        // add line break if we are showing multiple messages
        if(n>0) unlocked_message+='\n';

        std::string message;

        // write message depending on feature type
        switch(m_feature[n].type)
        {
            case UNLOCK_TRACK:
                {    // {} avoids compiler warning
                    Track* track = track_manager->getTrack( m_feature[n].name );
                    message = StringUtils::insert_values(
                        _("New track '%s'\nnow available"), 
                        _(track->getName()) );
                    break;
                }
            case UNLOCK_MODE:
                message = StringUtils::insert_values( 
                    _("New game mode\n'%s'\nnow available"), 
                    m_feature[n].user_name);
                break;
            case UNLOCK_GP:
            {
                std::string gp_user_name = grand_prix_manager->getGrandPrix(m_feature[n].name)->getName();
                message = StringUtils::insert_values(
                    _("New Grand Prix '%s'\nnow available"),
                    gp_user_name);
                break;
            }
            case UNLOCK_DIFFICULTY:
                message = StringUtils::insert_values(
                    _("New difficulty\n'%s'\nnow available"), 
                    m_feature[n].user_name);
                break;
            case UNLOCK_KART:
                const KartProperties *kp=kart_properties_manager->getKart(m_feature[n].name );
                message = StringUtils::insert_values(
                    _("New kart\n'%s'\nnow available"),
                    kp->getName());
                break;
        }   // switch
        unlocked_message += message;
    }   // for n

    return unlocked_message;
}

//-----------------------------------------------------------------------------
/** Loads the state for a challenge object (esp. m_state), and calls the
 *  virtual function loadAdditionalInfo for additional information
 */
void Challenge::load(const XMLNode* challengesNode)
{
    const XMLNode* node = challengesNode->getNode( getId() );
    if(node == NULL) return;
    
    // See if the challenge is solved (it's activated later from the
    // unlock_manager).
    
    std::string solvedString;
    node->get("solved", &solvedString);
    
    bool finished = (solvedString == "true");
    m_state = finished ? CH_SOLVED : CH_INACTIVE;
    
    if(m_state == CH_SOLVED)
    {
        std::cout << "Solved challenge!! " << getId().c_str() << std::endl;
    }
    
    if(!finished) loadAdditionalInfo(node);
}   // load

//-----------------------------------------------------------------------------
void Challenge::save(std::ofstream& writer)
{
    writer << "        <" << getId() << " solved=\"" << (isSolved() ? "true" : "false") << "\"";
    if(!isSolved()) saveAdditionalInfo(writer);
    writer << " />\n";
}   // save
