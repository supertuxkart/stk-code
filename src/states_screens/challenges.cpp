//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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


#include "states_screens/challenges.hpp"

#include "challenges/unlock_manager.hpp"
#include "states_screens/state_manager.hpp"

#include <fstream>

#include "irrString.h"
using irr::core::stringw;
using irr::core::stringc;

#include "config/user_config.hpp"
#include "guiengine/engine.hpp"
#include "io/file_manager.hpp"


namespace GUIEngine
{
    
    ChallengesScreen::ChallengesScreen() : Screen("challenges.stkgui")
    {
               
    }
    
    
    void ChallengesScreen::onUpdate(float elapsed_time, irr::video::IVideoDriver*)
    {
    }
    
    void ChallengesScreen::init()
    {
        DynamicRibbonWidget* w = this->getWidget<DynamicRibbonWidget>("challenges");
        assert( w != NULL );
        
        // Re-build track list everytime (accounts for locking changes, etc.)
        w->clearItems();
        
        const std::vector<const Challenge*>& challenges = unlock_manager->getActiveChallenges();
        const std::vector<const Challenge*>& solvedChallenges = unlock_manager->getUnlockedFeatures();
        
        const int challengeAmount = challenges.size();
        const int solvedChallengeAmount = solvedChallenges.size();
        char buffer[64];
        for (int n=0; n<challengeAmount; n++)
        {
            // TODO : temporary icon until we have a 'unsolved challenge' icon
            sprintf(buffer, "challenge%i", n);
            w->addItem(challenges[n]->getName() + L"\n" + challenges[n]->getChallengeDescription(),
                       buffer, file_manager->getTextureFile("speedback.png"));
        }
        for (int n=0; n<solvedChallengeAmount; n++)
        {
            // TODO : add bronze/silver/gold difficulties to challenges
            sprintf(buffer, "solved%i", n);
            w->addItem(solvedChallenges[n]->getName(), buffer, file_manager->getTextureFile("cup_gold.png"));
        }
        
        w->updateItemDisplay();  
    }
    
    void ChallengesScreen::tearDown()
    {
    }
    
    void ChallengesScreen::eventCallback(GUIEngine::Widget* widget, const std::string& name)
    {
        if(name == "back")
        {
            StateManager::get()->escapePressed();
        }
    }
    
} // end namespace
