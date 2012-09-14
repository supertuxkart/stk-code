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

#include "animations/animation_base.hpp"
#include "audio/music_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "io/file_manager.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "modes/overworld.hpp"
#include "physics/physics.hpp"
#include "states_screens/credits.hpp"
#include "states_screens/cutscene_gui.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/constants.hpp"
#include "utils/ptr_vector.hpp"

//-----------------------------------------------------------------------------
/** Constructor. Sets up the clock mode etc.
 */
CutsceneWorld::CutsceneWorld() : World()
{
    m_aborted = false;
    WorldStatus::setClockMode(CLOCK_NONE);
    m_use_highscores = false;
    m_play_racestart_sounds = false;
}   // CutsceneWorld

//-----------------------------------------------------------------------------
/** Initialises the three strikes battle. It sets up the data structure
 *  to keep track of points etc. for each kart.
 */
void CutsceneWorld::init()
{
    World::init();
    
    dynamic_cast<CutsceneGUI*>(m_race_gui)->setFadeLevel(1.0f);
    
    getTrack()->startMusic();
    
    m_duration = -1.0f;
    
    //const btTransform &s = getTrack()->getStartTransform(0);
    //const Vec3 &v = s.getOrigin();
    m_camera = irr_driver->getSceneManager()
             ->addCameraSceneNode(NULL, core::vector3df(0.0f, 0.0f, 0.0f),
                                  core::vector3df(0.0f, 0.0f, 0.0f));
    m_camera->setFOV(0.61f);
    m_camera->bindTargetAndRotation(true); // no "look-at"
    
    // --- Build list of sounds to play at certain frames
    PtrVector<TrackObject>& objects = m_track->getTrackObjectManager()->getObjects();
    TrackObject* curr;
    for_in(curr, objects)
    {
        if (curr->getType() == "particle-emitter" && !curr->getTriggerCondition().empty())
        {
            const std::string& condition = curr->getTriggerCondition();
            
            if (StringUtils::startsWith(condition, "frame "))
            {
                std::string frameStr = condition.substr(6); // remove 'frame ' prefix
                int frame;
                
                if (!StringUtils::fromString(frameStr, frame))
                {
                    fprintf(stderr, "[CutsceneWorld] Invalid condition '%s'\n", condition.c_str());
                    continue;
                }
                
                float FPS = 25.0f; // for now we assume the cutscene is saved at 25 FPS
                m_particles_to_trigger[frame / FPS].push_back(curr);
            }
        }
        else if (curr->getType() == "sfx-emitter" && !curr->getTriggerCondition().empty())
        {
            const std::string& condition = curr->getTriggerCondition();
            
            if (StringUtils::startsWith(condition, "frame "))
            {
                std::string frameStr = condition.substr(6); // remove 'frame ' prefix
                int frame;
                
                if (!StringUtils::fromString(frameStr, frame))
                {
                    fprintf(stderr, "[CutsceneWorld] Invalid condition '%s'\n", condition.c_str());
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
                    fprintf(stderr, "[CutsceneWorld] Invalid condition '%s'\n", condition.c_str());
                    continue;
                }
                
                float FPS = 25.0f; // for now we assume the cutscene is saved at 25 FPS
                m_sounds_to_stop[frame / FPS].push_back(curr);
                curr->triggerSound(true);
            }
        }
        
        if (dynamic_cast<AnimationBase*>(curr) != NULL)
        {
            m_duration = std::max(m_duration, dynamic_cast<AnimationBase*>(curr)->getAnimationDuration());
        }
    }
    
    if (m_duration <= 0.0f)
    {
        fprintf(stderr, "[CutsceneWorld] WARNING: cutscene has no duration\n");
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
    
    m_time += dt;
    
    if (m_time < 2.0f)
    {
        dynamic_cast<CutsceneGUI*>(m_race_gui)->setFadeLevel(1.0f - m_time / 2.0f);
    }
    else if (m_time > m_duration - 2.0f)
    {
        dynamic_cast<CutsceneGUI*>(m_race_gui)->setFadeLevel((m_time - (m_duration - 2.0f)) / 2.0f);
    }
    else
    {
        dynamic_cast<CutsceneGUI*>(m_race_gui)->setFadeLevel(0.0f);
    }
    
    float currFrame = m_time * 25.0f; // We assume 25 FPS
    
    const std::vector<Subtitle>& subtitles = m_track->getSubtitles();
    bool foundSubtitle = false;
    for (unsigned int n = 0; n < subtitles.size(); n++)
    {
        if (currFrame >= subtitles[n].getFrom() && currFrame < subtitles[n].getTo())
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
    
    
    World::update(dt);
    World::updateTrack(dt);
    
    PtrVector<TrackObject>& objects = m_track->getTrackObjectManager()->getObjects();
    TrackObject* curr;
    for_in(curr, objects)
    {
        if (curr->getType() == "cutscene_camera")
        {
            m_camera->setPosition(curr->getNode()->getPosition());
            m_camera->updateAbsolutePosition();
            
            core::vector3df rot = curr->getNode()->getRotation();
            Vec3 rot2(rot);
            rot2.setPitch(rot2.getPitch() + 90.0f);
            m_camera->setRotation(rot2.toIrrVector());
            
            sfx_manager->positionListener(m_camera->getAbsolutePosition(),
                                          m_camera->getTarget() - m_camera->getAbsolutePosition());
            
            break;
            //printf("Camera %f %f %f\n", curr->getNode()->getPosition().X, curr->getNode()->getPosition().Y, curr->getNode()->getPosition().Z);
        }
    }
    
    for (std::map<float, std::vector<TrackObject*> >::iterator it = m_sounds_to_trigger.begin();
         it != m_sounds_to_trigger.end(); )
    {
        if (m_time >= it->first)
        {
            std::vector<TrackObject*> objects = it->second;
            for (unsigned int i = 0; i < objects.size(); i++)
            {
                objects[i]->triggerSound();
            }
            m_sounds_to_trigger.erase(it++);
        }
        else
        {
            it++;
        }
     }
     
     for (std::map<float, std::vector<TrackObject*> >::iterator it = m_particles_to_trigger.begin();
         it != m_particles_to_trigger.end(); )
     {
        if (m_time >= it->first)
        {
            std::vector<TrackObject*> objects = it->second;
            for (unsigned int i = 0; i < objects.size(); i++)
            {
                objects[i]->triggerParticles();
            }
            m_particles_to_trigger.erase(it++);
        }
        else
        {
            it++;
        }
     }
     
     for (std::map<float, std::vector<TrackObject*> >::iterator it = m_sounds_to_stop.begin();
         it != m_sounds_to_stop.end(); )
     {
        if (m_time >= it->first)
        {
            std::vector<TrackObject*> objects = it->second;
            for (unsigned int i = 0; i < objects.size(); i++)
            {
                objects[i]->stopSound();
            }
            m_sounds_to_stop.erase(it++);
        }
        else
        {
            it++;
        }
    } 
}   // update

//-----------------------------------------------------------------------------

void CutsceneWorld::enterRaceOverState()
{
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
        if (m_parts.size() == 1 && m_parts[0] == "endcutscene")
        {
            CreditsScreen* credits = CreditsScreen::getInstance();
            credits->setVictoryMusic(true);
            MainMenuScreen* mainMenu = MainMenuScreen::getInstance();
            GUIEngine::Screen* newStack[] = { mainMenu, credits, NULL };
            race_manager->exitRace();
            StateManager::get()->resetAndSetStack(newStack);
            StateManager::get()->pushScreen(credits);
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
    return m_time > m_duration;
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

//-----------------------------------------------------------------------------

void CutsceneWorld::createRaceGUI()
{
    m_race_gui = new CutsceneGUI();
}
    
    
