//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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

#include "modes/standard_race.hpp"

#include "challenges/unlock_manager.hpp"
#include "items/powerup_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "karts/controller/ghost_controller.hpp"
#include "network/network_config.hpp"

//-----------------------------------------------------------------------------
StandardRace::StandardRace() : LinearWorld()
{
    WorldStatus::setClockMode(CLOCK_CHRONO);
}   // StandardRace

//-----------------------------------------------------------------------------
/** Returns true if the race is finished, i.e. all player karts are finished.
 */
bool StandardRace::isRaceOver()
{
    if (race_manager->isWatchingReplay())
    {
        return dynamic_cast<GhostController*>
            (m_karts[0]->getController())->isReplayEnd();
    }
    // The race is over if all players have finished the race. Remaining
    // times for AI opponents will be estimated in enterRaceOverState
    return race_manager->allPlayerFinished();
}   // isRaceOver

//-----------------------------------------------------------------------------
void StandardRace::getDefaultCollectibles(int *collectible_type, int *amount)
{
    // in time trial mode, give zippers
    if(race_manager->getMinorMode() == RaceManager::MINOR_MODE_TIME_TRIAL &&
        !race_manager->isWatchingReplay())
    {
        *collectible_type = PowerupManager::POWERUP_ZIPPER;
        *amount = race_manager->getNumLaps();
    }
    else World::getDefaultCollectibles(collectible_type, amount);
}   // getDefaultCollectibles

//-----------------------------------------------------------------------------
/** Returns if this mode supports bonus boxes or not.
 */
bool StandardRace::haveBonusBoxes()
{
    // in time trial mode, don't use bonus boxes
    return race_manager->getMinorMode() != RaceManager::MINOR_MODE_TIME_TRIAL;
}   // haveBonusBoxes

//-----------------------------------------------------------------------------
/** Returns an identifier for this race.
 */
const std::string& StandardRace::getIdent() const
{
    if(race_manager->getMinorMode() == RaceManager::MINOR_MODE_TIME_TRIAL)
        return IDENT_TTRIAL;
    else
        return IDENT_STD;
}   // getIdent

//-----------------------------------------------------------------------------
/** Ends the race early and places still active player karts at the back.
 *  The race immediately goes to the result stage, estimating the time for the
 *  karts still in the race. Still active player karts get a penalty in time
 *  as well as being placed at the back. Players that already finished keep
 *  their position.
 *
 *  End time for the punished players is calculated as follows
 *  end_time = current_time + (estimated_time - current_time)
 *                          + (estimated_time_for_last - current_time)
 *           = estimated_time + estimated_time_for_last - current_time
 *  This will put them at the end at all times. The further you (and the last in
 *  the race) are from the finish line, the harsher the punishment will be.
 */
void StandardRace::endRaceEarly()
{
    const unsigned int kart_amount = (unsigned int)m_karts.size();
    std::vector<int> active_players;
    // Required for debugging purposes
    beginSetKartPositions();
    for (unsigned int i = 1; i <= kart_amount; i++)
    {
        int kartid = m_position_index[i-1];
        AbstractKart* kart = m_karts[kartid];
        if (kart->hasFinishedRace())
        {
            // Have to do this to keep endSetKartPosition happy
            setKartPosition(kartid, kart->getPosition());
            continue;
        }

        if (kart->getController()->isPlayerController())
        {
            // Keep active players apart for now
            active_players.push_back(kartid);
        }
        else
        {
            // AI karts finish
            setKartPosition(kartid, i - (unsigned int) active_players.size());
            kart->finishedRace(estimateFinishTimeForKart(kart));
        }
    } // i <= kart_amount
    // Now make the active players finish
    for (unsigned int i = 0; i < active_players.size(); i++)
    {
        int kartid = active_players[i];
        int position = getNumKarts() - (int) active_players.size() + 1 + i;
        setKartPosition(kartid, position);
        m_karts[kartid]->eliminate();
    } // Finish the active players
    endSetKartPositions();
    setPhase(RESULT_DISPLAY_PHASE);
    if (!isNetworkWorld() || NetworkConfig::get()->isServer())
        terminateRace();
} // endRaceEarly
