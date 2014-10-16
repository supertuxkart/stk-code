//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2013 SuperTuxKart-Team
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

#include "animations/animation_base.hpp"
#include "animations/three_d_animation.hpp"
#include "audio/music_manager.hpp"
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
#include "states_screens/cutscene_gui.hpp"
#include "states_screens/feature_unlocked.hpp"
#include "states_screens/offline_kart_selection.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/constants.hpp"
#include "utils/ptr_vector.hpp"

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
    WorldStatus::setClockMode(CLOCK_NONE);
    m_use_highscores = false;
    m_play_racestart_sounds = false;
    m_fade_duration = 1.0f;
}   // CutsceneWorld

//-----------------------------------------------------------------------------
/** Initialises the three strikes battle. It sets up the data structure
 *  to keep track of points etc. for each kart.
 */
void CutsceneWorld::init()
{
    m_second_reset = false;
    World::init();

    dynamic_cast<CutsceneGUI*>(m_race_gui)->setFadeLevel(1.0f);

    getTrack()->startMusic();

    m_duration = -1.0f;

    //const btTransform &s = getTrack()->getStartTransform(0);
    //const Vec3 &v = s.getOrigin();
    Camera* stk_cam = Camera::createCamera(NULL);
    m_camera = stk_cam->getCameraSceneNode();
    //m_camera = irr_driver->getSceneManager()
    //         ->addCameraSceneNode(NULL, core::vector3df(0.0f, 0.0f, 0.0f),
    //                              core::vector3df(0.0f, 0.0f, 0.0f));
    m_camera->setFOV(0.61f);
    m_camera->bindTargetAndRotation(true); // no "look-at"

    // --- Build list of sounds to play at certain frames
    PtrVector<TrackObject>& objects = m_track->getTrackObjectManager()->getObjects();
    TrackObject* curr;
    for_in(curr, objects)
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
}   // ~CutsceneWorld

//-----------------------------------------------------------------------------
/** Returns the internal identifier for this race.
 */
const std::string& CutsceneWorld::getIdent() const
{
    return IDENT_CUTSCENE;
}   // getIdent

//-----------------------------------------------------------------------------
/** Update the world and the track.
 *  \param dt Time step size.
 */
void CutsceneWorld::update(float dt)
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

        PtrVector<TrackObject>& objects = m_track->getTrackObjectManager()->getObjects();
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

        PtrVector<TrackObject>& objects = m_track->getTrackObjectManager()->getObjects();
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
        // this way of calculating time and  dt is more in line with what
        // irrlicht does andprovides better synchronisation
        double prev_time = m_time;
        double now = StkTime::getRealTime();
        m_time = now - m_time_at_second_reset;
        dt = (float)(m_time - prev_time);
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

    const std::vector<Subtitle>& subtitles = m_track->getSubtitles();
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


    World::update((float)dt);
    World::updateTrack((float)dt);

    PtrVector<TrackObject>& objects = m_track->getTrackObjectManager()->getObjects();
    TrackObject* curr;
    for_in(curr, objects)
    {
        if (curr->getType() == "cutscene_camera")
        {
            scene::ISceneNode* anchorNode = curr->getPresentation<TrackObjectPresentationEmpty>()->getNode();
            m_camera->setPosition(anchorNode->getPosition());
            m_camera->updateAbsolutePosition();

            core::vector3df rot = anchorNode->getRotation();
            Vec3 rot2(rot);
            rot2.setPitch(rot2.getPitch() + 90.0f);
            m_camera->setRotation(rot2.toIrrVector());

            SFXManager::get()->positionListener(m_camera->getAbsolutePosition(),
                                          m_camera->getTarget() -
                                            m_camera->getAbsolutePosition(),
                                            Vec3(0,1,0));

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

void CutsceneWorld::enterRaceOverState()
{
    GUIEngine::CutsceneScreen* cs = dynamic_cast<GUIEngine::CutsceneScreen*>(
        GUIEngine::getCurrentScreen());
    if (cs != NULL)
        cs->onCutsceneEnd();


    int partId = -1;
    for (int i=0; i<(int)m_parts.size(); i++)
    {
        if (m_parts[i] == race_manager->getTrackName())
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
            race_manager->exitRace();
            StateManager::get()->resetAndSetStack(newStack);
            credits->push();
        }
        // TODO: remove hardcoded knowledge of cutscenes, replace with scripting probably
        else  if (m_parts.size() == 1 && m_parts[0] == "gpwin")
        {
            race_manager->exitRace();

            // un-set the GP mode so that after unlocking, it doesn't try to continue the GP
            race_manager->setMajorMode(RaceManager::MAJOR_MODE_SINGLE);

            std::vector<const ChallengeData*> unlocked =
                PlayerManager::getCurrentPlayer()->getRecentlyCompletedChallenges();
            if (unlocked.size() > 0)
            {
                //PlayerManager::getCurrentPlayer()->clearUnlocked();

                StateManager::get()->enterGameState();
                race_manager->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
                race_manager->setNumKarts(0);
                race_manager->setNumPlayers(0);
                race_manager->setNumLocalPlayers(0);
                race_manager->startSingleRace("featunlocked", 999, false);

                FeatureUnlockedCutScene* scene =
                    FeatureUnlockedCutScene::getInstance();
                std::vector<std::string> parts;
                parts.push_back("featunlocked");
                ((CutsceneWorld*)World::getWorld())->setParts(parts);

                assert(unlocked.size() > 0);
                scene->addTrophy(race_manager->getDifficulty());
                scene->findWhatWasUnlocked(race_manager->getDifficulty());

                StateManager::get()->replaceTopMostScreen(scene, GUIEngine::INGAME_MENU);
            }
            else
            {
                if (race_manager->raceWasStartedFromOverworld())
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
            //race_manager->exitRace();
            //StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
            //if (race_manager->raceWasStartedFromOverworld())
            //    OverWorld::enterOverWorld();

            race_manager->exitRace();

            // un-set the GP mode so that after unlocking, it doesn't try to continue the GP
            race_manager->setMajorMode(RaceManager::MAJOR_MODE_SINGLE);

            std::vector<const ChallengeData*> unlocked =
                PlayerManager::getCurrentPlayer()->getRecentlyCompletedChallenges();
            if (unlocked.size() > 0)
            {
                //PlayerManager::getCurrentPlayer()->clearUnlocked();

                StateManager::get()->enterGameState();
                race_manager->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
                race_manager->setNumKarts(0);
                race_manager->setNumPlayers(0);
                race_manager->setNumLocalPlayers(0);
                race_manager->startSingleRace("featunlocked", 999, false);

                FeatureUnlockedCutScene* scene =
                    FeatureUnlockedCutScene::getInstance();
                std::vector<std::string> parts;
                parts.push_back("featunlocked");
                ((CutsceneWorld*)World::getWorld())->setParts(parts);

                scene->addTrophy(race_manager->getDifficulty());
                scene->findWhatWasUnlocked(race_manager->getDifficulty());

                StateManager::get()->replaceTopMostScreen(scene, GUIEngine::INGAME_MENU);
            }
            else
            {
                if (race_manager->raceWasStartedFromOverworld())
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
        else if (race_manager->getTrackName() == "introcutscene" ||
                 race_manager->getTrackName() == "introcutscene2")
        {
            PlayerProfile *player = PlayerManager::getCurrentPlayer();
            if (player->isFirstTime())
            {
                race_manager->exitRace();
                StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());

                player->setFirstTime(false);
                PlayerManager::get()->save();
                KartSelectionScreen* s = OfflineKartSelectionScreen::getInstance();
                s->setMultiplayer(false);
                s->setGoToOverworldNext();
                s->push();
            }
        }
        // TODO: remove hardcoded knowledge of cutscenes, replace with scripting probably
        else if (m_parts.size() == 1 && m_parts[0] == "featunlocked")
        {
            if (race_manager->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
            {
                // in GP mode, continue GP after viewing this screen
                StateManager::get()->popMenu();
                race_manager->next();
            }
            else
            {
                // back to menu or overworld
                race_manager->exitRace();
                StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
                //StateManager::get()->popMenu();

                if (race_manager->raceWasStartedFromOverworld())
                {
                    OverWorld::enterOverWorld();
                }
            }
        }
        else
        {
            race_manager->exitRace();
            StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
            OverWorld::enterOverWorld();
        }
    }
    else
    {
        // 'exitRace' will destroy this object so get the next part right now
        std::string next_part = m_parts[partId + 1];

        race_manager->exitRace();
        race_manager->startSingleRace(next_part, 999, false);
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


