//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Alejandro Santiago
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

#include "modes/tutorial_race.hpp"

#include "audio/music_manager.hpp"
#include "tutorial/tutorial_manager.hpp"
#include "config/user_config.hpp"
#include "karts/abstract_kart.hpp"
#include "items/powerup_manager.hpp"
#include "states_screens/race_gui_base.hpp"
#include "tracks/track.hpp"
#include "utils/translation.hpp"


//-----------------------------------------------------------------------------
TutorialRace::TutorialRace() : LinearWorld()
{
}   // TutorialRace

//-----------------------------------------------------------------------------
TutorialRace::~TutorialRace()
{
}

//-----------------------------------------------------------------------------
/** The follow the leader race is over if there is only one kart left (plus
 *  the leader), or if all (human) players have been eliminated.
 */
bool TutorialRace::isRaceOver()
{
    // FIXME : add the logic to detect if the user have ended the tutorial. 
    return true;
}   // isRaceOver

//-----------------------------------------------------------------------------
void TutorialRace::restartRace()
{
    LinearWorld::restartRace();
}   // restartRace

//-----------------------------------------------------------------------------
/** Returns the internal identifier for this kind of race. 
 */
const std::string& TutorialRace::getIdent() const
{
    return IDENT_FTL;
}   // getIdent

//-----------------------------------------------------------------------------
/** Sets the title for all karts that is displayed in the icon list. In
 *  this mode the title for the first kart is set to 'leader'.
 */
void TutorialRace::getKartsDisplayInfo(
                           std::vector<RaceGUIBase::KartIconDisplayInfo> *info)
{
    LinearWorld::getKartsDisplayInfo(info);
    (*info)[0].special_title = _("Leader");
}   // getKartsDisplayInfo
