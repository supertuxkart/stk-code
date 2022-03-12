//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
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

#include "modes/easter_egg_hunt.hpp"

#include "io/file_manager.hpp"
#include "items/item.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/ghost_kart.hpp"
#include "replay/replay_play.hpp"
#include "tracks/track.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

//-----------------------------------------------------------------------------
/** Constructor. Sets up the clock mode etc.
 */
EasterEggHunt::EasterEggHunt() : LinearWorld()
{
    WorldStatus::setClockMode(CLOCK_CHRONO);
    m_use_highscores = true;
    m_eggs_found     = 0;
    m_only_ghosts    = false;
    m_finish_time    = 0;
}   // EasterEggHunt

//-----------------------------------------------------------------------------
/** Initialises the easer egg hunt.
 */
void EasterEggHunt::init()
{
    LinearWorld::init();
    m_display_rank = false;

    unsigned int gk = 0;
    if (RaceManager::get()->hasGhostKarts())
        gk = ReplayPlay::get()->getNumGhostKart();
    // check for possible problems if AI karts were incorrectly added
    if((getNumKarts() - gk) > RaceManager::get()->getNumPlayers())
    {
        Log::error("EasterEggHunt]", "No AI exists for this game mode");
        exit(1);
    }

    if (getNumKarts() == gk)
        m_only_ghosts = true;

    m_eggs_collected.resize(m_karts.size(), 0);

}   // EasterEggHunt

//-----------------------------------------------------------------------------
/** Destructor. Clears all internal data structures, and removes the tire mesh
 *  from the mesh cache.
 */
EasterEggHunt::~EasterEggHunt()
{
}   // ~EasterEggHunt

//-----------------------------------------------------------------------------
/** Check if a file easter_eggs.xml exists in the track directory, and if so
 *  loads that file and makes the easter egg mode available for this track.
*/
void EasterEggHunt::readData(const std::string &filename)
{
    auto easter = std::unique_ptr<XMLNode>(file_manager->createXMLTree(filename));
    if(!easter)
        return;

    if(easter->getName()!="EasterEggHunt")
    {
        Log::error("[EasterEggHunt]", "Can't load easter egg file '%s' - no EasterEggHunt element.",
                filename.c_str());
        return;
    }

    // Search for the most relevant set of egg
    const XMLNode *data = NULL;
    RaceManager::Difficulty difficulty     = RaceManager::get()->getDifficulty();
    RaceManager::Difficulty act_difficulty = RaceManager::DIFFICULTY_COUNT;

    for(int i=RaceManager::DIFFICULTY_FIRST; i<=RaceManager::DIFFICULTY_LAST; i++)
    {
        std::string diff_name = RaceManager::get()->getDifficultyAsString((RaceManager::Difficulty)i);
        const XMLNode * cur_data = easter->getNode(diff_name);
        if (cur_data)
        {
            data = cur_data;
            act_difficulty = (RaceManager::Difficulty)i;
            // Stop at the easiest difficulty which is equal or harder than the desired one.
            // If none is equal or harder, this will default to the hardest defined set.
            if (act_difficulty >= difficulty)
                break;
        }
    }

    if(!data)
    {
        return;
    }
    m_number_of_eggs = 0;
    for(unsigned int i=0; i<data->getNumNodes(); i++)
    {
        const XMLNode *egg = data->getNode(i);
        if(egg->getName()!="easter-egg")
        {
            Log::warn("[EasterEggHunt]", "Unknown node '%s' in easter egg level '%s' - ignored.",
                   egg->getName().c_str(),
                   RaceManager::get()->getDifficultyAsString(act_difficulty).c_str());
            continue;
        }
        Track::getCurrentTrack()->itemCommand(egg);
        m_number_of_eggs++;
    }   // for i <num_nodes

    WorldStatus::setClockMode(CLOCK_CHRONO);

}   // readEasterEggInfo

//-----------------------------------------------------------------------------
/** Returns the internal identifier for this race.
 */
const std::string& EasterEggHunt::getIdent() const
{
    return IDENT_EASTER;
}   // getIdent

//-----------------------------------------------------------------------------
/** Called when a kart has collected an egg.
 *  \param kart The kart that collected an egg.
 */
void EasterEggHunt::collectedItem(const AbstractKart *kart,
                                  const ItemState *item    )
{
    if(item->getType() != ItemState::ITEM_EASTER_EGG) return;

    m_eggs_collected[kart->getWorldKartId()]++;
    m_eggs_found++;
}   // collectedItem

//-----------------------------------------------------------------------------
/** Called when a ghost kart has collected an egg.
 *  \param world_id The world id of the ghost kart that collected an egg.
 */
void EasterEggHunt::collectedEasterEggGhost(int world_id)
{
    m_eggs_collected[world_id]++;
}   // collectedEasterEgg

//-----------------------------------------------------------------------------
/** Update the world and the track.
 *  \param ticks Physics time step size - should be 1.
 */
void EasterEggHunt::update(int ticks)
{
    LinearWorld::update(ticks);
    LinearWorld::updateTrack(ticks);
}   // update

//-----------------------------------------------------------------------------
/** The hunt is over once all eggs are found.
 */
bool EasterEggHunt::isRaceOver()
{
    if(!m_only_ghosts && m_eggs_found == m_number_of_eggs)
    {
        if (m_finish_time == 0)
            m_finish_time = getTime();
        return true;
    }
    else if (m_only_ghosts)
    {
        for (unsigned int i=0 ; i<m_eggs_collected.size();i++)
        {
            if (m_eggs_collected[i] == m_number_of_eggs)
                return true;
        }
    }
    if(m_time<0)
        return true;
    return false;
}   // isRaceOver

//-----------------------------------------------------------------------------
/** Called when an egg hunt is restarted.
 */
void EasterEggHunt::reset(bool restart)
{
    LinearWorld::reset(restart);

    for(unsigned int i=0; i<m_eggs_collected.size(); i++)
        m_eggs_collected[i] = 0;
    m_eggs_found = 0;
    m_finish_time = 0;
}   // reset

//-----------------------------------------------------------------------------
/** Returns the data to display in the race gui.
 */
void EasterEggHunt::getKartsDisplayInfo(
                           std::vector<RaceGUIBase::KartIconDisplayInfo> *info)
{
    const unsigned int kart_amount = getNumKarts();
    for(unsigned int i = 0; i < kart_amount ; i++)
    {
        RaceGUIBase::KartIconDisplayInfo& rank_info = (*info)[i];
        //I18n: number of collected eggs / overall number of eggs
        rank_info.m_text = _("Eggs: %d / %d", m_eggs_collected[i],
                                              m_number_of_eggs);
        rank_info.m_color = video::SColor(255, 255, 255, 255);
    }
}   // getKartDisplayInfo
//-----------------------------------------------------------------------------
/** Override the base class method to change behavior. We don't want wrong
 *  direction messages in the easter egg mode since there is no direction there.
 *  \param i Kart id.
 *  \param dt Time step size.
 */
void EasterEggHunt::checkForWrongDirection(unsigned int i, float dt)
{
}   // checkForWrongDirection

//-----------------------------------------------------------------------------

void EasterEggHunt::terminateRace()
{
    m_karts[0]->getControls().reset();
    WorldWithRank::terminateRace();
}
//-----------------------------------------------------------------------------
/** In Easter Egg mode the finish time is just the time the race is over,
 *  since there are no AI karts and no other players, except for ghosts.
 */
float EasterEggHunt::estimateFinishTimeForKart(AbstractKart* kart)
{
    // For ghost karts, use the replay data
    if (kart->isGhostKart())
    {
        GhostKart* gk = dynamic_cast<GhostKart*>(kart);
        return gk->getGhostFinishTime();
    }

    return m_finish_time;
}   // estimateFinishTimeForKart
