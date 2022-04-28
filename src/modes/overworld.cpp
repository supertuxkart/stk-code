//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 SuperTuxKart-Team
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
OverWorld::OverWorld() : World()
{
    m_return_to_garage            = false;
    m_stop_music_when_dialog_open = false;
    m_play_track_intro_sound      = false;
}   // Overworld

//-----------------------------------------------------------------------------
OverWorld::~OverWorld()
{
    Vec3 kart_xyz = getKart(0)->getXYZ();
    RaceManager::get()->setKartLastPositionOnOverworld(kart_xyz);
}   // ~OverWorld

//-----------------------------------------------------------------------------
/** Function to simplify the start process */
void OverWorld::enterOverWorld()
{
    // update point count and the list of locked/unlocked stuff
    PlayerManager::getCurrentPlayer()->computeActive();

    RaceManager::get()->setNumPlayers(1);
    RaceManager::get()->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
    RaceManager::get()->setMinorMode (RaceManager::MINOR_MODE_OVERWORLD);
    RaceManager::get()->setNumKarts( 1 );
    RaceManager::get()->setTrack( "overworld" );

    if (PlayerManager::getCurrentPlayer()->isLocked("difficulty_best"))
    {
        RaceManager::get()->setDifficulty(RaceManager::DIFFICULTY_HARD);
    }
    else
    {
        RaceManager::get()->setDifficulty(RaceManager::DIFFICULTY_BEST);
    }

    // Use the last used device
    InputDevice* device = input_manager->getDeviceManager()->getLatestUsedDevice();

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
    RaceManager::get()->setPlayerKart(0, UserConfigParams::m_default_kart);

    // ASSIGN should make sure that only input from assigned devices
    // is read.
    input_manager->getDeviceManager()->setAssignMode(ASSIGN);
    input_manager->getDeviceManager()
        ->setSinglePlayer( StateManager::get()->getActivePlayer(0) );

    StateManager::get()->enterGameState();
    RaceManager::get()->setupPlayerKartInfo();
    RaceManager::get()->startNew(false);
    if(RaceManager::get()->haveKartLastPositionOnOverworld()){
            OverWorld *ow = (OverWorld*)World::getWorld();
            ow->getKart(0)->setXYZ(RaceManager::get()->getKartLastPositionOnOverworld());
            ow->moveKartAfterRescue(ow->getKart(0));
        }
    irr_driver->showPointer(); // User should be able to click on the minimap

}   // enterOverWorld

//-----------------------------------------------------------------------------
/** General update function called once per frame.
 *  \param ticks Number of physics time steps - should be 1.
 */
void OverWorld::update(int ticks)
{
    // Skip annoying waiting without a purpose
    // Make sure to do all things that would normally happen in the
    // update() method of the base classes.
    if (getPhase() < GO_PHASE)
    {
        setPhase(RACE_PHASE);
        // Normally done in WorldStatus::update(), during phase SET_PHASE,
        // so we have to start music 'manually', since we skip all phases.
        MusicInformation* mi = Track::getCurrentTrack()->getTrackMusic();
        Track::getCurrentTrack()->startMusic();

        if (UserConfigParams::m_music)
            music_manager->startMusic(mi);
        m_karts[0]->startEngineSFX();
    }
    World::update(ticks);
    World::updateTrack(ticks);
    const unsigned int kart_amount  = (unsigned int)m_karts.size();

    // isn't it cool, on the overworld nitro is free!
    for(unsigned int n=0; n<kart_amount; n++)
    {
        m_karts[n]->setEnergy(100.0f);
    }

    /*
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
    */

    if (m_return_to_garage)
    {
        m_return_to_garage = false;
        RaceManager::get()->exitRace();
        KartSelectionScreen* s = OfflineKartSelectionScreen::getInstance();
        s->setMultiplayer(false);
        s->setFromOverworld(true);
        StateManager::get()->resetAndGoToScreen(s);
        throw AbortWorldUpdateException();
    }
}   // update

// ----------------------------------------------------------------------------
/** Finds the starting position which is closest to the kart.
 *  \param kart The kart for which a rescue position needs to be determined.
 */
unsigned int OverWorld::getRescuePositionIndex(AbstractKart *kart)
{
    // find closest point to drop kart on
    const int start_spots_amount = getNumberOfRescuePositions();
    assert(start_spots_amount > 0);

    int closest_id = -1;
    float closest_distance = 999999999.0f;

    for (int n=0; n<start_spots_amount; n++)
    {
        const btTransform &s = getStartTransform(n);
        const Vec3 &v = s.getOrigin();

        float abs_distance = (v - kart->getXYZ()).length();

        if (abs_distance < closest_distance)
        {
            closest_distance = abs_distance;
            closest_id = n;
        }
    }

    assert(closest_id != -1);
    return closest_id;
}   // getRescuePositionIndex

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
                                  Track::getCurrentTrack()->getChallengeList();

    AbstractKart* k = getKart(0);
    Vec3 kart_xyz = k->getXYZ();
    if (dynamic_cast<RescueAnimation*>(k->getKartAnimation()) != NULL)
    {
        // you can't start a race while being rescued
        return;
    }

    for (unsigned int n=0; n<challenges.size(); n++)
    {
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
                const ChallengeData* challenge = unlock_manager->getChallengeData(challenges[n].m_challenge_id);
                if (challenge == NULL)
                {
                    Log::error("track", "Cannot find challenge named '%s'\n",
                        challenges[n].m_challenge_id.c_str());
                    continue;
                }

                const unsigned int val = challenge->getNumTrophies();
// Mobile STK may have less challenges available than the main version
#ifdef MOBILE_STK
                bool enough_challenges = true;
#else
                const unsigned int val2 = challenge->getNumChallenges();
                bool enough_challenges = (PlayerManager::getCurrentPlayer()->getNumCompletedChallenges() >= val2);
#endif
                bool unlocked = enough_challenges && (PlayerManager::getCurrentPlayer()->getPoints() >= val);
                
                if (UserConfigParams::m_unlock_everything > 0)
                    unlocked = true;

                if (unlocked)
                {
                    RaceManager::get()->setKartLastPositionOnOverworld(kart_xyz);
                    new SelectChallengeDialog(0.9f, 0.9f,
                        challenges[n].m_challenge_id);
                }
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
        // from World by setting the kart's position to
        // be the location of the challenge bubble.
        AbstractKart* kart = getKart(0);
        kart->setXYZ(challenge->m_position);
        kart->getVehicle()->setMaxSpeed(0);

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
