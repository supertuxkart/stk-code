//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
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

#include "modes/cutscene_world.hpp"

#include <string>
#include <IMeshSceneNode.h>
#include <ISceneManager.h>

#include "audio/music_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "physics/physics.hpp"
#include "states_screens/race_gui_base.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/constants.hpp"

//-----------------------------------------------------------------------------
/** Constructor. Sets up the clock mode etc.
 */
CutsceneWorld::CutsceneWorld() : World()
{
    WorldStatus::setClockMode(CLOCK_NONE);
    m_use_highscores = false;
}   // CutsceneWorld

//-----------------------------------------------------------------------------
/** Initialises the three strikes battle. It sets up the data structure
 *  to keep track of points etc. for each kart.
 */
void CutsceneWorld::init()
{
    World::init();
    
    m_camera = irr_driver->getSceneManager()->addCameraSceneNode(NULL, core::vector3df(-80.0f, 2.0f, 75.0f),
                                core::vector3df(-97.230003, -0.010000, 50.610001));
}   // CutsceneWorld

//-----------------------------------------------------------------------------
/** Destructor. Clears all internal data structures, and removes the tire mesh
 *  from the mesh cache.
 */
CutsceneWorld::~CutsceneWorld()
{
}   // ~CutsceneWorld

//-----------------------------------------------------------------------------
/** Called when a kart is hit. 
 *  \param kart_id The world kart id of the kart that was hit.
 */
void CutsceneWorld::kartHit(const int kart_id)
{
}

//-----------------------------------------------------------------------------
/** Returns the internal identifier for this race.
 */
const std::string& CutsceneWorld::getIdent() const
{
    return IDENT_CUSTSCENE;
}   // getIdent

//-----------------------------------------------------------------------------
/** Update the world and the track.
 *  \param dt Time step size. 
 */
void CutsceneWorld::update(float dt)
{
    World::update(dt);
    World::updateTrack(dt);
}   // update

//-----------------------------------------------------------------------------
/** The battle is over if only one kart is left, or no player kart.
 */
bool CutsceneWorld::isRaceOver()
{
    return false;
}   // isRaceOver

//-----------------------------------------------------------------------------
/** Called when the race finishes, i.e. after playing (if necessary) an
 *  end of race animation. It updates the time for all karts still racing,
 *  and then updates the ranks.
 */
void CutsceneWorld::terminateRace()
{
    World::terminateRace();
}   // terminateRace

//-----------------------------------------------------------------------------
/** Called then a battle is restarted.
 */
void CutsceneWorld::restartRace()
{
    World::restartRace();
}   // restartRace

//-----------------------------------------------------------------------------
/** Returns the data to display in the race gui.
 */
RaceGUIBase::KartIconDisplayInfo* CutsceneWorld::getKartsDisplayInfo()
{
    return NULL;
}   // getKartDisplayInfo

//-----------------------------------------------------------------------------
/** Moves a kart to its rescue position.
 *  \param kart The kart that was rescued.
 */
void CutsceneWorld::moveKartAfterRescue(AbstractKart* kart)
{
}   // moveKartAfterRescue
