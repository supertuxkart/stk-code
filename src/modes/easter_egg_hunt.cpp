//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012 Joerg Henrichs
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
EasterEggHunt::EasterEggHunt() : WorldWithRank()
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
    WorldWithRank::init();
    m_display_rank = false;
    
    // check for possible problems if AI karts were incorrectly added
    if(getNumKarts() > race_manager->getNumPlayers())
    {
        fprintf(stderr, "No AI exists for this game mode\n");
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
        printf("Can't load easter egg file '%s' - no EasterEggHunt element.",
                filename.c_str());
        delete easter;
        return;
    }

    // Search for the closest difficulty set of egg.
    const XMLNode *data = NULL;
    std::string difficulty_name;
    RaceManager::Difficulty diff = race_manager->getDifficulty();
    for(unsigned int i=0; i<RaceManager::DIFFICULTY_COUNT; i++)
    {
        difficulty_name = race_manager->getDifficultyAsString(diff);
        data = easter->getNode(difficulty_name);
        if(data) break;
        diff = (RaceManager::Difficulty)(diff+1);
        if(diff==RaceManager::DIFFICULTY_LAST)
            diff = RaceManager::DIFFICULTY_FIRST;
    }
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
            printf("Unknown node '%s' in easter egg level '%s' - ignored.\n",
                    egg->getName().c_str(), difficulty_name.c_str());
            continue;
        }
        World::getTrack()->itemCommand(egg);
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
    WorldWithRank::update(dt);
    WorldWithRank::updateTrack(dt);
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
void EasterEggHunt::restartRace()
{
    WorldWithRank::restartRace();
    
    for(unsigned int i=0; i<m_eggs_collected.size(); i++)
        m_eggs_collected[i] = 0;
    m_eggs_found = 0;
}   // restartRace

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
        std::ostringstream o;
        //I18n: number of collected eggs / overall number of eggs
        rank_info.m_text = StringUtils::insertValues(_("Eggs: %d / %d"), 
                                                    m_eggs_collected[i],
                                                    m_number_of_eggs);
        rank_info.m_color = video::SColor(255, 255, 255, 255);
    }
}   // getKartDisplayInfo

//-----------------------------------------------------------------------------
/** Moves a kart to its rescue position.
 *  \param kart The kart that was rescued.
 */
void EasterEggHunt::moveKartAfterRescue(AbstractKart* kart)
{
    int start_position = kart->getInitialPosition();
    btTransform start_pos = getTrack()->getStartTransform(start_position-1);

    kart->getBody()->setCenterOfMassTransform(start_pos);

}   // moveKartAfterRescue
