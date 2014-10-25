//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2013 SuperTuxKart-Team
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

#include "modes/overworld.hpp"

#include "audio/music_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "input/device_manager.hpp"
#include "input/input.hpp"
#include "input/input_manager.hpp"
#include "input/keyboard_device.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "karts/rescue_animation.hpp"
#include "physics/btKart.hpp"
#include "physics/physics.hpp"
#include "states_screens/dialogs/select_challenge.hpp"
#include "states_screens/offline_kart_selection.hpp"
#include "states_screens/race_gui_overworld.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object_manager.hpp"

//-----------------------------------------------------------------------------
OverWorld::OverWorld() : WorldWithRank()
{
    m_return_to_garage            = false;
    m_stop_music_when_dialog_open = false;
}   // Overworld

//-----------------------------------------------------------------------------
OverWorld::~OverWorld()
{
    Vec3 kart_xyz = getKart(0)->getXYZ();
    race_manager->setKartLastPositionOnOverworld(kart_xyz);
}   // ~OverWorld

//-----------------------------------------------------------------------------
/** Function to simplify the start process */
void OverWorld::enterOverWorld()
{
    race_manager->setNumLocalPlayers(1);
    race_manager->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
    race_manager->setMinorMode (RaceManager::MINOR_MODE_OVERWORLD);
    race_manager->setNumKarts( 1 );
    race_manager->setTrack( "overworld" );
    race_manager->setDifficulty(RaceManager::DIFFICULTY_HARD);

    // Use keyboard 0 by default (FIXME: let player choose?)
    InputDevice* device = input_manager->getDeviceManager()->getKeyboard(0);

    // Create player and associate player with keyboard
    StateManager::get()->createActivePlayer(PlayerManager::getCurrentPlayer(),
                                            device);

    if (!kart_properties_manager->getKart(UserConfigParams::m_default_kart))
    {
        Log::warn("[overworld]", "cannot find kart '%s', "
                  "will revert to default",
                  UserConfigParams::m_default_kart.c_str());

        UserConfigParams::m_default_kart.revertToDefaults();
    }
    race_manager->setLocalKartInfo(0, UserConfigParams::m_default_kart);

    // ASSIGN should make sure that only input from assigned devices
    // is read.
    input_manager->getDeviceManager()->setAssignMode(ASSIGN);
    input_manager->getDeviceManager()
        ->setSinglePlayer( StateManager::get()->getActivePlayer(0) );

    StateManager::get()->enterGameState();
    race_manager->setupPlayerKartInfo();
    race_manager->startNew(false);
    if(race_manager->haveKartLastPositionOnOverworld()){
            OverWorld *ow = (OverWorld*)World::getWorld();
            ow->getKart(0)->setXYZ(race_manager->getKartLastPositionOnOverworld());
            ow->moveKartAfterRescue(ow->getKart(0));
        }
    irr_driver->showPointer(); // User should be able to click on the minimap

}   // enterOverWorld

//-----------------------------------------------------------------------------
/** General update function called once per frame.
 *  \param dt Time step size.
 */
void OverWorld::update(float dt)
{
    // Skip annoying waiting without a purpose
    // Make sure to do all things that would normally happen in the
    // update() method of the base classes.
    if (getPhase() < GO_PHASE)
    {
        setPhase(RACE_PHASE);
        // Normally done in WorldStatus::update(), during phase SET_PHASE,
        // so we have to start music 'manually', since we skip all phases.
        World::getWorld()->getTrack()->startMusic();

        if (music_manager->getCurrentMusic() != NULL &&
            UserConfigParams::m_music)
            music_manager->getCurrentMusic()->startMusic();
        m_karts[0]->startEngineSFX();
    }
    WorldWithRank::update(dt);
    WorldWithRank::updateTrack(dt);
    const unsigned int kart_amount  = (unsigned int)m_karts.size();

    // isn't it cool, on the overworld nitro is free!
    for(unsigned int n=0; n<kart_amount; n++)
    {
        m_karts[n]->setEnergy(100.0f);
    }

    TrackObjectManager* tom = getTrack()->getTrackObjectManager();
    PtrVector<TrackObject>& objects = tom->getObjects();
    for(unsigned int i=0; i<objects.size(); i++)
    {
        TrackObject* obj = objects.get(i);
        if(!obj->isGarage())
            continue;

        float m_distance = obj->getDistance();
        Vec3 m_garage_pos = obj->getPosition();
        Vec3 m_kart_pos = getKart(0)->getXYZ();

        if ((m_garage_pos-m_kart_pos).length_2d() > m_distance)
        {
            obj->reset();
        }
    }

    if (m_return_to_garage)
    {
        m_return_to_garage = false;
        race_manager->exitRace();
        KartSelectionScreen* s = OfflineKartSelectionScreen::getInstance();
        s->setMultiplayer(false);
        s->setFromOverworld(true);
        StateManager::get()->resetAndGoToScreen(s);
        throw AbortWorldUpdateException();
    }
}   // update

//-----------------------------------------------------------------------------
/** This function is not used in the overworld race gui.
 */
void OverWorld::getKartsDisplayInfo(
                       std::vector<RaceGUIBase::KartIconDisplayInfo> *info)
{
    assert(false);
}   // getKartsDisplayInfo

//-----------------------------------------------------------------------------

void OverWorld::createRaceGUI()
{
    m_race_gui = new RaceGUIOverworld();
}   // createRaceGUI

//-----------------------------------------------------------------------------

void OverWorld::onFirePressed(Controller* who)
{
    const std::vector<OverworldChallenge>& challenges =
                                                  m_track->getChallengeList();

    AbstractKart* k = getKart(0);
    Vec3 kart_xyz = k->getXYZ();
    if (dynamic_cast<RescueAnimation*>(k->getKartAnimation()) != NULL)
    {
        // you can't start a race while being rescued
        return;
    }

    for (unsigned int n=0; n<challenges.size(); n++)
    {
        if ( challenges[n].isForceFieldSet() &&
             challenges[n].getForceField().m_is_locked )
            continue;

        if ( (kart_xyz - Vec3(challenges[n].m_position)).length2_2d()
              < CHALLENGE_DISTANCE_SQUARED)
        {
            if (challenges[n].m_challenge_id == "tutorial")
            {
                scheduleTutorial();
                return;
            }
            else
            {
                race_manager->setKartLastPositionOnOverworld(kart_xyz);
                new SelectChallengeDialog(0.8f, 0.8f,
                                          challenges[n].m_challenge_id);
            }
        } // end if
    } // end for
}   // onFirePressed

//-----------------------------------------------------------------------------
/** Called when a mouse click happens. If the click happened while the mouse
 *  was hovering on top of a challenge, the kart will be teleported to
 *  the challenge.
 *  \param x,y Mouse coordinates.
 */
void OverWorld::onMouseClick(int x, int y)
{
    const OverworldChallenge *challenge =
        ((RaceGUIOverworld*)getRaceGUI())->getCurrentChallenge();

    if(challenge)
    {
        // Use the 'get closest start point' rescue function
        // from WorldWithRank by setting the kart's position to
        // be the location of the challenge bubble.
        AbstractKart* kart = getKart(0);
        kart->setXYZ(challenge->m_position);
        kart->getVehicle()->capSpeed(0);

        unsigned int index   = getRescuePositionIndex(kart);
        btTransform s        = getRescueTransform(index);
        const btVector3 &xyz = s.getOrigin();
        float angle          = atan2(challenge->m_position.X - xyz[0],
                                     challenge->m_position.Z - xyz[2]);
        s.setRotation( btQuaternion(btVector3(0.0f, 1.0f, 0.0f), angle) );
        moveKartTo(kart, s);
        return;
    }
}  // onMouseClick
