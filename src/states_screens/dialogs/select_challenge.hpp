//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Marianne Gagnon
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

#ifndef SELECT_CHALLENGE_HPP
#define SELECT_CHALLENGE_HPP

#include "challenges/challenge_status.hpp"
#include "guiengine/event_handler.hpp"
#include "guiengine/modaldialog.hpp"
#include "race/race_manager.hpp"

/**
 * \brief Dialog shown when starting a challenge
 * \ingroup states_screens
 */
class SelectChallengeDialog : public GUIEngine::ModalDialog
{
private:
    bool  m_self_destroy = false;
    std::string m_challenge_id;
    void updateSolvedIcon(const ChallengeStatus* c, RaceManager::Difficulty diff,
                          const char* widget_name, const char* path);
public:
    
    SelectChallengeDialog(const float percentWidth, const float percentHeight,
                          std::string challenge_id);
    virtual ~SelectChallengeDialog();
    
    virtual GUIEngine::EventPropagation processEvent(const std::string& eventSource);
    virtual void onUpdate(float dt);
};

#endif

