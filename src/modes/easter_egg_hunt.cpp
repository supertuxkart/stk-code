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
#include "karts/abstract_kart.hpp"
#include "tracks/track.hpp"

//-----------------------------------------------------------------------------
/** Constructor. Sets up the clock mode etc.
 */
EasterEggHunt::EasterEggHunt() : LinearWorld()
{
    WorldStatus::setClockMode(CLOCK_CHRONO);
    m_use_highscores = true;
    m_eggs_found     = 0;
}   // EasterEggHunt

//-----------------------------------------------------------------------------
/** Initialises the easer egg hunt.
 */
void EasterEggHunt::init()
{
    LinearWorld::init();
    m_display_rank = false;

    // check for possible problems if AI karts were incorrectly added
    if(getNumKarts() > race_manager->getNumPlayers())
    {
        Log::error("EasterEggHunt]", "No AI exists for this game mode");
        exit(1);
    }

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
    XMLNode *easter = file_manager->createXMLTree(filename);
    if(!easter)
        return;

    if(easter->getName()!="EasterEggHunt")
    {
        Log::error("[EasterEggHunt]", "Can't load easter egg file '%s' - no EasterEggHunt element.",
                filename.c_str());
        delete easter;
        return;
    }

    // Search for the closest difficulty set of egg.
    const XMLNode *data = NULL;
    RaceManager::Difficulty difficulty     = race_manager->getDifficulty();
    RaceManager::Difficulty act_difficulty = RaceManager::DIFFICULTY_COUNT;
    for(int i=difficulty; i<=RaceManager::DIFFICULTY_LAST; i++)
    {
        std::string diff_name=
            race_manager->getDifficultyAsString((RaceManager::Difficulty)i);
        const XMLNode * cur_data = easter->getNode(diff_name);
        if (cur_data)
        {
            data = cur_data;
            act_difficulty = (RaceManager::Difficulty)i;
            break;
        }
    }
    // If there is no data for an equal or harder placement,
    // check for the most difficult placement that is easier:
    if(!data)
    {
        for(int i=difficulty-1; i>=RaceManager::DIFFICULTY_FIRST; i--)
        {
            std::string diff_name=
               race_manager->getDifficultyAsString((RaceManager::Difficulty)i);
            const XMLNode * cur_data = easter->getNode(diff_name);
            if (cur_data)
            {
                data = cur_data;
                act_difficulty = (RaceManager::Difficulty)i;
                break;
            }
        }   // for i
    }   // if !data

    if(!data)
    {
        delete easter;
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
                   race_manager->getDifficultyAsString(act_difficulty).c_str());
            continue;
        }
        Track::getCurrentTrack()->itemCommand(egg);
        m_number_of_eggs++;
    }   // for i <num_nodes

    delete easter;

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
void EasterEggHunt::collectedEasterEgg(const AbstractKart *kart)
{
    m_eggs_collected[kart->getWorldKartId()]++;
    m_eggs_found++;
}   // collectedEasterEgg

//-----------------------------------------------------------------------------
/** Update the world and the track.
 *  \param dt Time step size.
 */
void EasterEggHunt::update(float dt)
{
    LinearWorld::update(dt);
    LinearWorld::updateTrack(dt);
}   // update

//-----------------------------------------------------------------------------
/** The hunt is over once all eggs are found.
 */
bool EasterEggHunt::isRaceOver()
{
    if(m_eggs_found == m_number_of_eggs)
        return true;
    if(m_time<0)
        return true;
    return false;
}   // isRaceOver

//-----------------------------------------------------------------------------
/** Called then a battle is restarted.
 */
void EasterEggHunt::reset()
{
    LinearWorld::reset();

    for(unsigned int i=0; i<m_eggs_collected.size(); i++)
        m_eggs_collected[i] = 0;
    m_eggs_found = 0;
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
 *  since there are no AI karts.
 */
float EasterEggHunt::estimateFinishTimeForKart(AbstractKart* kart)
{
    return getTime();
}   // estimateFinishTimeForKart
