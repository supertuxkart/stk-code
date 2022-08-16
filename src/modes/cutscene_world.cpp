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

#include "modes/cutscene_world.hpp"

#include "main_loop.hpp"
#include "animations/animation_base.hpp"
#include "animations/three_d_animation.hpp"
#include "audio/sfx_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "graphics/camera.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "modes/overworld.hpp"
#include "physics/physics.hpp"
#include "states_screens/credits.hpp"
#include "states_screens/cutscene_general.hpp"
#include "states_screens/cutscene_gui.hpp"
#include "states_screens/feature_unlocked.hpp"
#include "states_screens/offline_kart_selection.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/constants.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/string_utils.hpp"

#include <IMeshSceneNode.h>
#include <ISceneManager.h>

#include <algorithm>
#include <string>

bool CutsceneWorld::s_use_duration = false;

//-----------------------------------------------------------------------------
/** Constructor. Sets up the clock mode etc.
 */
CutsceneWorld::CutsceneWorld() : World()
{
    m_time_at_second_reset = 0.0f;
    m_aborted = false;
    WorldStatus::setClockMode(CLOCK_CHRONO);
    m_phase = RACE_PHASE;
    m_use_highscores = false;
    m_play_track_intro_sound = false;
    m_play_ready_set_go_sounds = false;
    m_fade_duration = 1.0f;
    m_camera = NULL;
    m_cleared_cutscene = false;
}   // CutsceneWorld

//-----------------------------------------------------------------------------
/** Initialises the three strikes battle. It sets up the data structure
 *  to keep track of points etc. for each kart.
 */
void CutsceneWorld::init()
{
    // Use real dt even if fps is low. It allows to keep everything synchronized
    main_loop->setAllowLargeDt(true);
    
    m_second_reset = false;
    World::init();

    dynamic_cast<CutsceneGUI*>(m_race_gui)->setFadeLevel(1.0f);

    Track::getCurrentTrack()->startMusic();

    m_duration = -1.0f;

    Camera* stk_cam = Camera::createCamera(NULL, 0);
    m_camera = stk_cam->getCameraSceneNode();
    m_camera->setFOV(stk_config->m_cutscene_fov);
    m_camera->bindTargetAndRotation(true); // no "look-at"

    // --- Build list of sounds to play at certain frames
    PtrVector<TrackObject>& objects = Track::getCurrentTrack()
                                    ->getTrackObjectManager()->getObjects();
    for (TrackObject* curr : objects)
    {
        if (curr->getType() == "particle-emitter" &&
            !curr->getPresentation<TrackObjectPresentationParticles>()->getTriggerCondition().empty())
        {
            const std::string& condition = curr->getPresentation<TrackObjectPresentationParticles>()->getTriggerCondition();

            if (StringUtils::startsWith(condition, "frame "))
            {
                std::string frameStr = condition.substr(6); // remove 'frame ' prefix
                int frame;

                if (!StringUtils::fromString(frameStr, frame))
                {
                    Log::error("[CutsceneWorld]", "Invalid condition '%s'",
                                    condition.c_str());
                    continue;
                }

                float FPS = 25.0f; // for now we assume the cutscene is saved at 25 FPS
                m_particles_to_trigger[frame / FPS].push_back(curr);
            }
        }
        else if (curr->getType() == "sfx-emitter" && !curr->getPresentation<TrackObjectPresentationSound>()->getTriggerCondition().empty())
        {
            const std::string& condition = curr->getPresentation<TrackObjectPresentationSound>()->getTriggerCondition();

            if (StringUtils::startsWith(condition, "frame "))
            {
                std::string frameStr = condition.substr(6); // remove 'frame ' prefix
                int frame;

                if (!StringUtils::fromString(frameStr, frame))
                {
                    Log::error("[CutsceneWorld]", "Invalid condition '%s'",
                                    condition.c_str());
                    continue;
                }

                float FPS = 25.0f; // for now we assume the cutscene is saved at 25 FPS
                m_sounds_to_trigger[frame / FPS].push_back(curr);
            }
            else if (StringUtils::startsWith(condition, "until "))
            {
                std::string frameStr = condition.substr(6); // remove 'until ' prefix
                int frame;

                if (!StringUtils::fromString(frameStr, frame))
                {
                    Log::error("[CutsceneWorld]", "Invalid condition '%s'",
                                    condition.c_str());
                    continue;
                }

                float FPS = 25.0f; // for now we assume the cutscene is saved at 25 FPS
                m_sounds_to_stop[frame / FPS].push_back(curr);
                curr->getPresentation<TrackObjectPresentationSound>()->triggerSound(true);
            }
        }

        if (curr->getAnimator() != NULL)
        {
            m_duration = std::max(m_duration,
                                  (double)curr->getAnimator()->getAnimationDuration());
        }
    }

    if (!s_use_duration)
        m_duration = 999999.0f;

    if (m_duration <= 0.0f)
    {
        Log::error("[CutsceneWorld]", "WARNING: cutscene has no duration");
    }
}   // CutsceneWorld

//-----------------------------------------------------------------------------
/** Destructor. Clears all internal data structures, and removes the tire mesh
 *  from the mesh cache.
 */
CutsceneWorld::~CutsceneWorld()
{
    main_loop->setAllowLargeDt(false);
    clearCutscene();
}   // ~CutsceneWorld
//-----------------------------------------------------------------------------
void CutsceneWorld::reset(bool restart)
{
    World::reset(restart);
    m_phase = RACE_PHASE;
}
//-----------------------------------------------------------------------------
/** Returns the internal identifier for this race.
 */
const std::string& CutsceneWorld::getIdent() const
{
    return IDENT_CUTSCENE;
}   // getIdent

//-----------------------------------------------------------------------------
/** Update the world and the track.
 *  \param ticks Number of physics time steps - should be 1.
 */
void CutsceneWorld::update(int ticks)
{
    /*
    {
    PtrVector<TrackObject>& objects = m_track->getTrackObjectManager()->getObjects();
    TrackObject* curr;
    for_in(curr, objects)
    {
    printf("* %s\n", curr->getType().c_str());
    }
    }
    **/

    if (m_time < 0.0001f)
    {
        //printf("INITIAL TIME for CutsceneWorld\n");

        music_manager->startMusic();

        PtrVector<TrackObject>& objects = Track::getCurrentTrack()
                                        ->getTrackObjectManager()->getObjects();
        TrackObject* curr;
        for_in(curr, objects)
        {
            curr->reset();
        }
        m_time = 0.01f;
        m_time_at_second_reset = StkTime::getRealTime();
        m_second_reset = true;
    }
    else if (m_second_reset)
    {
        m_second_reset = false;

        PtrVector<TrackObject>& objects = Track::getCurrentTrack()
                                        ->getTrackObjectManager()->getObjects();
        TrackObject* curr;
        for_in(curr, objects)
        {
            curr->reset();
        }

        //m_time_at_second_reset = m_time;
        m_time_at_second_reset = StkTime::getRealTime();
        m_time = 0.01f;
    }
    else
    {
        // this way of calculating time and dt is more in line with what
        // irrlicht does and provides better synchronisation
        double now = StkTime::getRealTime();
        m_time = now - m_time_at_second_reset;
    }

    if (m_aborted)
    {
        // We can only set end duration after m_time is updated in the previous
        // step
        if (m_time < m_duration - m_fade_duration)
            m_duration = m_time + m_fade_duration;
    }

    float fade = 0.0f;
    float fadeIn = -1.0f;
    float fadeOut = -1.0f;
    if (m_time < m_fade_duration)
    {
        fadeIn = 1.0f - (float)m_time / m_fade_duration;
    }
    if (m_time > m_duration - m_fade_duration)
    {
        fadeOut = (float)(m_time - (m_duration - m_fade_duration)) / m_fade_duration;
    }

    if (fadeIn >= 0.0f && fadeOut >= 0.0f)
    {
        fade = std::max(fadeIn, fadeOut);
    }
    else if (fadeIn >= 0.0f)
    {
        fade = fadeIn;
    }
    else if (fadeOut >= 0.0f)
    {
        fade = fadeOut;
    }
    dynamic_cast<CutsceneGUI*>(m_race_gui)->setFadeLevel(fade);

    // We assume 25 FPS. Irrlicht starts at frame 0.
    float curr_frame = (float)(m_time*25.0f - 1.0f);

    //printf("Estimated current frame : %f\n", curr_frame);

    const std::vector<Subtitle>& subtitles = Track::getCurrentTrack()
                                           ->getSubtitles();
    bool foundSubtitle = false;
    for (unsigned int n = 0; n < subtitles.size(); n++)
    {
        if (curr_frame >= subtitles[n].getFrom() &&
            curr_frame < subtitles[n].getTo())
        {
            dynamic_cast<CutsceneGUI*>(m_race_gui)->setSubtitle(subtitles[n].getText());
            foundSubtitle = true;
            break;
        }
    }

    if (!foundSubtitle)
    {
        dynamic_cast<CutsceneGUI*>(m_race_gui)->setSubtitle(core::stringw(L""));
    }


    World::update(ticks);
    World::updateTrack(ticks);

    PtrVector<TrackObject>& objects = Track::getCurrentTrack()
                                    ->getTrackObjectManager()->getObjects();
    TrackObject* curr;
    for_in(curr, objects)
    {
        if (curr->getType() == "cutscene_camera")
        {
            Camera *camera = Camera::getActiveCamera();
            if (camera && camera->getType() == Camera::CM_TYPE_NORMAL)
            {
                scene::ISceneNode* anchorNode = curr->getPresentation<TrackObjectPresentationEmpty>()->getNode();
                m_camera->setPosition(anchorNode->getPosition());
                m_camera->updateAbsolutePosition();

                core::vector3df rot = anchorNode->getRotation();
                Vec3 rot2(rot);
                rot2.setPitch(rot2.getPitch() + 90.0f);
                m_camera->setRotation(rot2.toIrrVector());

                irr::core::vector3df up(0.0f, 0.0f, 1.0f);
                irr::core::matrix4 matrix = anchorNode->getAbsoluteTransformation();
                matrix.rotateVect(up);
                m_camera->setUpVector(up);

                SFXManager::get()->positionListener(m_camera->getAbsolutePosition(),
                                              m_camera->getTarget() -
                                                m_camera->getAbsolutePosition(),
                                                Vec3(0,1,0));
            }
            break;
        }
    }

    std::map<float, std::vector<TrackObject*> >::iterator it;
    for (it = m_sounds_to_trigger.begin(); it != m_sounds_to_trigger.end(); )
    {
        if (m_time >= it->first)
        {
            std::vector<TrackObject*> objects = it->second;
            for (unsigned int i = 0; i < objects.size(); i++)
            {
                objects[i]->getPresentation<TrackObjectPresentationSound>()->triggerSound(false);
            }
            m_sounds_to_trigger.erase(it++);
        }
        else
        {
            it++;
        }
     }

     for (it = m_particles_to_trigger.begin();
          it != m_particles_to_trigger.end(); )
     {
        if (m_time >= it->first)
        {
            std::vector<TrackObject*> objects = it->second;
            for (unsigned int i = 0; i < objects.size(); i++)
            {
                objects[i]->getPresentation<TrackObjectPresentationParticles>()->triggerParticles();
            }
            m_particles_to_trigger.erase(it++);
        }
        else
        {
            it++;
        }
     }

     for (it = m_sounds_to_stop.begin(); it != m_sounds_to_stop.end(); )
     {
        if (m_time >= it->first)
        {
            std::vector<TrackObject*> objects = it->second;
            for (unsigned int i = 0; i < objects.size(); i++)
            {
                objects[i]->getPresentation<TrackObjectPresentationSound>()->stopSound();
            }
            m_sounds_to_stop.erase(it++);
        }
        else
        {
            it++;
        }
    }

    //bool isOver = (m_time > m_duration);
    //if (isOver && (s_use_duration || m_aborted))
    //{
    //    GUIEngine::CutsceneScreen* cs = dynamic_cast<GUIEngine::CutsceneScreen*>(
    //        GUIEngine::getCurrentScreen());
    //    if (cs != NULL)
    //        cs->onCutsceneEnd();
    //}
}   // update

//-----------------------------------------------------------------------------

void CutsceneWorld::clearCutscene()
{
    if (m_cleared_cutscene)
        return;

    GUIEngine::CutsceneScreen* cs = dynamic_cast<GUIEngine::CutsceneScreen*>(
        GUIEngine::getCurrentScreen());
    if (cs != NULL)
        cs->onCutsceneEnd();
    m_cleared_cutscene = true;
}

//-----------------------------------------------------------------------------

void CutsceneWorld::enterRaceOverState()
{
    clearCutscene();

    int partId = -1;
    for (int i=0; i<(int)m_parts.size(); i++)
    {
        if (m_parts[i] == RaceManager::get()->getTrackName())
        {
            partId = i;
            break;
        }
    }

    if (m_aborted || partId == -1 || partId == (int)m_parts.size() - 1)
    {
        // TODO: remove hardcoded knowledge of cutscenes, replace with scripting probably
        if (m_parts.size() == 1 && m_parts[0] == "endcutscene")
        {
            CreditsScreen* credits = CreditsScreen::getInstance();
            credits->setVictoryMusic(true);
            MainMenuScreen* mainMenu = MainMenuScreen::getInstance();
            GUIEngine::Screen* newStack[] = { mainMenu, credits, NULL };
            RaceManager::get()->exitRace();
            StateManager::get()->resetAndSetStack(newStack);
        }
        // TODO: remove hardcoded knowledge of cutscenes, replace with scripting probably
        else  if (m_parts.size() == 1 && m_parts[0] == "gpwin")
        {
            RaceManager::get()->exitRace();

            // un-set the GP mode so that after unlocking, it doesn't try to continue the GP
            RaceManager::get()->setMajorMode(RaceManager::MAJOR_MODE_SINGLE);

            //TODO : this code largely duplicate a similar code present in raceResultGUI.
            //       Try to reduce duplication
            std::vector<const ChallengeData*> unlocked =
                PlayerManager::getCurrentPlayer()->getRecentlyCompletedChallenges();

            if (unlocked.size() > 0)
            {
                PlayerManager::getCurrentPlayer()->clearUnlocked();

                StateManager::get()->enterGameState();
                RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
                RaceManager::get()->setNumKarts(0);
                RaceManager::get()->setNumPlayers(0);
                RaceManager::get()->startSingleRace("featunlocked", 999, RaceManager::get()->raceWasStartedFromOverworld());

                FeatureUnlockedCutScene* scene =
                    FeatureUnlockedCutScene::getInstance();
                std::vector<std::string> parts;
                parts.push_back("featunlocked");
                ((CutsceneWorld*)World::getWorld())->setParts(parts);

                assert(unlocked.size() > 0);
                scene->addTrophy(RaceManager::get()->getDifficulty(),true);
                scene->findWhatWasUnlocked(RaceManager::get()->getDifficulty(),unlocked);

                StateManager::get()->replaceTopMostScreen(scene, GUIEngine::INGAME_MENU);
            }
            else
            {
                if (RaceManager::get()->raceWasStartedFromOverworld())
                {
                    //StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
                    OverWorld::enterOverWorld();
                }
                else
                {
                    StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
                    // we assume the main menu was pushed before showing this menu
                    //StateManager::get()->popMenu();
                }
            }
        }
        // TODO: remove hardcoded knowledge of cutscenes, replace with scripting probably
        else if (m_parts.size() == 1 && m_parts[0] == "gplose")
        {
            //RaceManager::get()->exitRace();
            //StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
            //if (RaceManager::get()->raceWasStartedFromOverworld())
            //    OverWorld::enterOverWorld();

            RaceManager::get()->exitRace();

            // un-set the GP mode so that after unlocking, it doesn't try to continue the GP
            RaceManager::get()->setMajorMode(RaceManager::MAJOR_MODE_SINGLE);

            std::vector<const ChallengeData*> unlocked =
                PlayerManager::getCurrentPlayer()->getRecentlyCompletedChallenges();

            if (unlocked.size() > 0)
            {
                PlayerManager::getCurrentPlayer()->clearUnlocked();

                StateManager::get()->enterGameState();
                RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
                RaceManager::get()->setNumKarts(0);
                RaceManager::get()->setNumPlayers(0);
                RaceManager::get()->startSingleRace("featunlocked", 999, RaceManager::get()->raceWasStartedFromOverworld());

                FeatureUnlockedCutScene* scene =
                    FeatureUnlockedCutScene::getInstance();
                std::vector<std::string> parts;
                parts.push_back("featunlocked");
                ((CutsceneWorld*)World::getWorld())->setParts(parts);

                scene->addTrophy(RaceManager::get()->getDifficulty(),true);
                scene->findWhatWasUnlocked(RaceManager::get()->getDifficulty(),unlocked);

                StateManager::get()->replaceTopMostScreen(scene, GUIEngine::INGAME_MENU);
            }
            else
            {
                if (RaceManager::get()->raceWasStartedFromOverworld())
                {
                    //StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
                    OverWorld::enterOverWorld();
                }
                else
                {
                    StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
                    // we assume the main menu was pushed before showing this menu
                    //StateManager::get()->popMenu();
                }
            }
        }
        // TODO: remove hardcoded knowledge of cutscenes, replace with scripting probably
        else if (RaceManager::get()->getTrackName() == "introcutscene" ||
                 RaceManager::get()->getTrackName() == "introcutscene2")
        {
            PlayerProfile *player = PlayerManager::getCurrentPlayer();
            if (player->isFirstTime())
            {
                RaceManager::get()->exitRace();
                StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
                player->setFirstTime(false);
                PlayerManager::get()->save();
                KartSelectionScreen* s = OfflineKartSelectionScreen::getInstance();
                s->setMultiplayer(false);
                s->setGoToOverworldNext();
                s->push();
            } else
            {
                RaceManager::get()->exitRace();
                StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
                OverWorld::enterOverWorld();
            }
        }
        // TODO: remove hardcoded knowledge of cutscenes, replace with scripting probably
        else if (m_parts.size() == 1 && m_parts[0] == "featunlocked")
        {
            if (RaceManager::get()->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
            {
                // in GP mode, continue GP after viewing this screen
                StateManager::get()->popMenu();
                RaceManager::get()->next();
            }
            else
            {
                // back to menu or overworld
                RaceManager::get()->exitRace();
                StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
                //StateManager::get()->popMenu();

                if (RaceManager::get()->raceWasStartedFromOverworld())
                {
                    OverWorld::enterOverWorld();
                }
            }
        }
        else
        {
            RaceManager::get()->exitRace();
            StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
            OverWorld::enterOverWorld();
        }
    }
    else
    {
        // 'exitRace' will destroy this object so get the next part right now
        std::string next_part = m_parts[partId + 1];

        // Save current screen pointer before exitRace
        CutSceneGeneral* csg = dynamic_cast<CutSceneGeneral*>(GUIEngine::getCurrentScreen());

        RaceManager::get()->exitRace();
        RaceManager::get()->startSingleRace(next_part, 999, RaceManager::get()->raceWasStartedFromOverworld());

        // Keep showing cutscene gui if previous scene was using it
        if (csg != NULL)
        {
            CutSceneGeneral* scene = CutSceneGeneral::getInstance();
            scene->push();
        }
    }

}

//-----------------------------------------------------------------------------
/** The battle is over if only one kart is left, or no player kart.
 */
bool CutsceneWorld::isRaceOver()
{
    bool isOver = (m_time > m_duration);

    if (!s_use_duration && !m_aborted)
        return false;

    return isOver;
}   // isRaceOver

//-----------------------------------------------------------------------------

void CutsceneWorld::createRaceGUI()
{
    m_race_gui = new CutsceneGUI();
}   // createRaceGUI

