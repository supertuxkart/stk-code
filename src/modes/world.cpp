//
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

#include "modes/world.hpp"

#include "audio/music_manager.hpp"
#include "audio/sfx_base.hpp"
#include "audio/sfx_manager.hpp"
#include "config/player_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/central_settings.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "graphics/render_info.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen_keyboard.hpp"
#include "io/file_manager.hpp"
#include "input/device_manager.hpp"
#include "input/keyboard_device.hpp"
#include "items/projectile_manager.hpp"
#include "karts/controller/battle_ai.hpp"
#include "karts/ghost_kart.hpp"
#include "karts/controller/end_controller.hpp"
#include "karts/controller/local_player_controller.hpp"
#include "karts/controller/skidding_ai.hpp"
#include "karts/controller/soccer_ai.hpp"
#include "karts/controller/spare_tire_ai.hpp"
#include "karts/controller/test_ai.hpp"
#include "karts/controller/network_ai_controller.hpp"
#include "karts/controller/network_player_controller.hpp"
#include "karts/kart.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties_manager.hpp"
#include "karts/kart_rewinder.hpp"
#include "main_loop.hpp"
#include "modes/overworld.hpp"
#include "modes/profile_world.hpp"
#include "network/protocols/client_lobby.hpp"
#include "network/network_config.hpp"
#include "network/rewind_manager.hpp"
#include "physics/btKart.hpp"
#include "physics/physics.hpp"
#include "physics/triangle_mesh.hpp"
#include "race/highscore_manager.hpp"
#include "race/history.hpp"
#include "race/race_manager.hpp"
#include "replay/replay_play.hpp"
#include "replay/replay_recorder.hpp"
#include "scriptengine/script_engine.hpp"
#include "states_screens/dialogs/race_paused_dialog.hpp"
#include "states_screens/race_gui_base.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/race_gui.hpp"
#include "states_screens/race_result_gui.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/check_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/constants.hpp"
#include "utils/profiler.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"

#include <algorithm>
#include <assert.h>
#include <ctime>
#include <sstream>
#include <stdexcept>


World* World::m_world = NULL;

/** The main world class is used to handle the track and the karts.
 *  The end of the race is detected in two phases: first the (abstract)
 *  function isRaceOver, which must be implemented by all game modes,
 *  must return true. In which case enterRaceOverState is called. At
 *  this time a winning (or losing) animation can be played. The WorldStatus
 *  class will in its enterRaceOverState switch to DELAY_FINISH_PHASE,
 *  but the remaining AI kart will keep on racing during that time.
 *  After a time period specified in stk_config.xml WorldStatus will
 *  switch to FINISH_PHASE and call terminateRace. Now the finishing status
 *  of all karts is set (i.e. in a normal race the arrival time for karts
 *  will be estimated), highscore is updated, and the race result gui
 *  is being displayed.
 *  Rescuing is handled via the three functions:
 *  getNumberOfRescuePositions() - which returns the number of rescue
 *           positions defined.
 *  getRescuePositionIndex(AbstractKart *kart) - which determines the
 *           index of the rescue position to be used for the given kart.
 *  getRescueTransform(unsigned int index) - which returns the transform
 *           (i.e. position and rotation) for the specified rescue
 *           position.
 *  This allows the world class to do some tests to make sure all rescue
 *  positions are valid (when started with --track-debug). It tries to
 *  place all karts on all rescue positions. If there are any problems
 *  (e.g. a rescue position not over terrain (perhaps because it is too
 *  low); or the rescue position is on a texture which will immediately
 *  trigger another rescue), a warning message will be printed.
 */

//-----------------------------------------------------------------------------
/** Constructor. Note that in the constructor it is not possible to call any
 *  functions that use World::getWorld(), since this is only defined
 *  after the constructor. Those functions must be called in the init()
 *  function, which is called immediately after the constructor.
 */
World::World() : WorldStatus()
{
    RewindManager::setEnable(NetworkConfig::get()->isNetworking());
#ifdef DEBUG
    m_magic_number = 0xB01D6543;
#endif

    m_race_gui           = NULL;
    m_saved_race_gui     = NULL;
    m_use_highscores     = true;
    m_schedule_pause     = false;
    m_schedule_unpause   = false;
    m_schedule_exit_race = false;
    m_schedule_tutorial  = false;
    m_is_network_world   = false;

    m_stop_music_when_dialog_open = true;

    WorldStatus::setClockMode(CLOCK_CHRONO);

}   // World

// ----------------------------------------------------------------------------
/** This function is called after instanciating. The code here can't be moved
 *  to the contructor as child classes must be instanciated, otherwise
 *  polymorphism will fail and the results will be incorrect . Also in init()
 *  functions can be called that use World::getWorld().
 */
void World::init()
{
    m_ended_early         = false;
    m_faster_music_active = false;
    m_fastest_kart        = 0;
    m_eliminated_karts    = 0;
    m_eliminated_players  = 0;
    m_num_players         = 0;
    unsigned int gk       = 0;
    m_red_ai = m_blue_ai = 0;
    if (race_manager->hasGhostKarts())
        gk = ReplayPlay::get()->getNumGhostKart();

    // Create the race gui before anything else is attached to the scene node
    // (which happens when the track is loaded). This allows the race gui to
    // do any rendering on texture. Note that this function can NOT be called
    // in the World constuctor, since it might be overwritten by a the game
    // mode class, which would not have been constructed at the time that this
    // constructor is called, so the wrong race gui would be created.
    createRaceGUI();
    main_loop->renderGUI(1000);
    RewindManager::create();
    main_loop->renderGUI(1100);
    // Grab the track file
    Track *track = track_manager->getTrack(race_manager->getTrackName());
    Scripting::ScriptEngine::getInstance<Scripting::ScriptEngine>();
    if(!track)
    {
        std::ostringstream msg;
        msg << "Track '" << race_manager->getTrackName()
            << "' not found.\n";
        throw std::runtime_error(msg.str());
    }

    std::string script_path = track->getTrackFile("scripting.as");
    Scripting::ScriptEngine::getInstance()->loadScript(script_path, true);
    main_loop->renderGUI(1200);
    // Create the physics
    Physics::getInstance<Physics>();
    main_loop->renderGUI(1300);
    unsigned int num_karts = race_manager->getNumberOfKarts();
    //assert(num_karts > 0);

    // Load the track models - this must be done before the karts so that the
    // karts can be positioned properly on (and not in) the tracks.
    // This also defines the static Track::getCurrentTrack function.
    track->loadTrackModel(race_manager->getReverseTrack());

    // Shuffles the start transforms with playing 3-strikes or free for all battles.
    if ((race_manager->getMinorMode() == RaceManager::MINOR_MODE_3_STRIKES ||
         race_manager->getMinorMode() == RaceManager::MINOR_MODE_FREE_FOR_ALL) &&
         !NetworkConfig::get()->isNetworking())
    {
        track->shuffleStartTransforms();
    }

    main_loop->renderGUI(6998);
    if (gk > 0)
    {
        ReplayPlay::get()->load();
        for (unsigned int k = 0; k < gk; k++)
            m_karts.push_back(ReplayPlay::get()->getGhostKart(k));
    }
    main_loop->renderGUI(6999);

    // Assign team of AIs for team mode before createKart
    if (hasTeam())
        setAITeam();

    for(unsigned int i=0; i<num_karts; i++)
    {
        main_loop->renderGUI(7000, i, num_karts);
        if (race_manager->getKartType(i) == RaceManager::KT_GHOST) continue;
        std::string kart_ident = history->replayHistory()
                               ? history->getKartIdent(i)
                               : race_manager->getKartIdent(i);
        int local_player_id  = race_manager->getKartLocalPlayerId(i);
        int global_player_id = race_manager->getKartGlobalPlayerId(i);
        std::shared_ptr<AbstractKart> new_kart;
        if (hasTeam())
        {
            new_kart = createKartWithTeam(kart_ident, i, local_player_id,
                global_player_id, race_manager->getKartType(i),
                race_manager->getPlayerHandicap(i));
        }
        else
        {
            new_kart = createKart(kart_ident, i, local_player_id,
                global_player_id, race_manager->getKartType(i),
                race_manager->getPlayerHandicap(i));
        }
        new_kart->setBoostAI(race_manager->hasBoostedAI(i));
        m_karts.push_back(new_kart);
    }  // for i

    main_loop->renderGUI(7050);
    // Load other custom models if needed
    loadCustomModels();
    main_loop->renderGUI(7100);
    // Must be called after all karts are created
    m_race_gui->init();

    powerup_manager->computeWeightsForRace(race_manager->getNumberOfKarts());
    main_loop->renderGUI(7200);
    if (UserConfigParams::m_particles_effects > 1)
    {
        Weather::getInstance<Weather>();   // create Weather instance
    }

    if (Camera::getNumCameras() == 0)
    {
        auto cl = LobbyProtocol::get<ClientLobby>();
        if ( (NetworkConfig::get()->isServer() && 
              !ProfileWorld::isNoGraphics()       ) ||
            race_manager->isWatchingReplay()        ||
            (cl && cl->isSpectator()))
        {
            // In case that the server is running with gui, watching replay or
            // spectating the game, create a camera and attach it to the first
            // kart.
            Camera::createCamera(World::getWorld()->getKart(0), 0);

        }   // if server with graphics of is watching replay
    } // if getNumCameras()==0

    const unsigned int kart_amount = (unsigned int)m_karts.size();
    for (unsigned int i = 0; i < kart_amount; i++)
        initTeamArrows(m_karts[i].get());

    main_loop->renderGUI(7300);
}   // init

//-----------------------------------------------------------------------------
void World::initTeamArrows(AbstractKart* k)
{
    if (!hasTeam())
        return;
#ifndef SERVER_ONLY
    //Loading the indicator textures
    std::string red_path =
            file_manager->getAsset(FileManager::GUI_ICON, "red_arrow.png");
    std::string blue_path =
            file_manager->getAsset(FileManager::GUI_ICON, "blue_arrow.png");

    // Assigning indicators
    scene::ISceneNode *arrow_node = NULL;

    KartModel* km = k->getKartModel();
    // Color of karts can be changed using shaders if the model supports
    if (km->supportColorization() && CVS->isGLSL())
        return;

    float arrow_pos_height = km->getHeight() + 0.5f;
    KartTeam team = getKartTeam(k->getWorldKartId());

    arrow_node = irr_driver->addBillboard(
        core::dimension2d<irr::f32>(0.3f,0.3f),
        team == KART_TEAM_BLUE ? blue_path : red_path,
        k->getNode());

    arrow_node->setPosition(core::vector3df(0, arrow_pos_height, 0));
#endif
}   // initTeamArrows

//-----------------------------------------------------------------------------
/** This function is called before a race is started (i.e. either after
 *  calling init() when starting a race for the first time, or after
 *  restarting a race, in which case no init() is called.
 */
void World::reset(bool restart)
{
    RewindManager::get()->reset();

    // If m_saved_race_gui is set, it means that the restart was done
    // when the race result gui was being shown. In this case restore the
    // race gui (note that the race result gui is cached and so never really
    // destroyed).
    if(m_saved_race_gui)
    {
        m_race_gui       = m_saved_race_gui;
        m_saved_race_gui = NULL;
    }

    m_ended_early = false;
    m_schedule_pause = false;
    m_schedule_unpause = false;

    WorldStatus::reset(restart);
    m_faster_music_active = false;
    m_eliminated_karts    = 0;
    m_eliminated_players  = 0;
    m_is_network_world = false;

    for ( KartList::iterator i = m_karts.begin(); i != m_karts.end() ; ++i )
    {
        (*i)->reset();
        if ((*i)->getController()->canGetAchievements())
        {
            updateAchievementModeCounters(true /*start*/);

            PlayerManager::resetKartHits(getNumKarts());
            if (race_manager->isLinearRaceMode())
            {
                PlayerManager::trackEvent(race_manager->getTrackName(), AchievementsStatus::TR_STARTED);
                AchievementsStatus::AchievementData diff;
                diff = (race_manager->getDifficulty() == RaceManager::DIFFICULTY_EASY)   ? AchievementsStatus::EASY_STARTED :
                       (race_manager->getDifficulty() == RaceManager::DIFFICULTY_MEDIUM) ? AchievementsStatus::MEDIUM_STARTED :
                       (race_manager->getDifficulty() == RaceManager::DIFFICULTY_HARD)   ? AchievementsStatus::HARD_STARTED :
                                                                                           AchievementsStatus::BEST_STARTED;
                PlayerManager::increaseAchievement(diff,1);
            }
            else if (race_manager->isEggHuntMode())
            {
                PlayerManager::trackEvent(race_manager->getTrackName(), AchievementsStatus::TR_EGG_HUNT_STARTED);
            }
            if (restart)
                PlayerManager::onRaceEnd(true /* previous race aborted */);
        }
    }

    Camera::resetAllCameras();

    if(race_manager->hasGhostKarts())
        ReplayPlay::get()->reset();

    // Remove all (if any) previous game flyables before reset karts, so no
    // explosion animation will be created
    projectile_manager->cleanup();
    resetAllKarts();
    // Note: track reset must be called after all karts exist, since check
    // objects need to allocate data structures depending on the number
    // of karts.
    Track::getCurrentTrack()->reset();

    // Reset the race gui.
    m_race_gui->reset();

    // Start music from beginning
    music_manager->stopMusic();

    // Enable SFX again
    SFXManager::get()->resumeAll();

    RewindManager::get()->reset();
    race_manager->reset();
    // Make sure to overwrite the data from the previous race.
    if(!history->replayHistory()) history->initRecording();
    if(race_manager->isRecordingRace())
    {
        Log::info("World", "Start Recording race.");
        ReplayRecorder::get()->init();
    }

    // Reset all data structures that depend on number of karts.
    irr_driver->reset();
    m_unfair_team = false;
}   // reset

//-----------------------------------------------------------------------------

void World::createRaceGUI()
{
    m_race_gui = new RaceGUI();
}

//-----------------------------------------------------------------------------
/** Creates a kart, having a certain position, starting location, and local
 *  and global player id (if applicable).
 *  \param kart_ident Identifier of the kart to create.
 *  \param index Index of the kart.
 *  \param local_player_id If the kart is a player kart this is the index of
 *         this player on the local machine.
 *  \param global_player_id If the kart is a player kart this is the index of
 *         this player globally (i.e. including network players).
 */
std::shared_ptr<AbstractKart> World::createKart
    (const std::string &kart_ident, int index, int local_player_id,
    int global_player_id, RaceManager::KartType kart_type,
    HandicapLevel handicap)
{
    unsigned int gk = 0;
    if (race_manager->hasGhostKarts())
        gk = ReplayPlay::get()->getNumGhostKart();

    std::shared_ptr<RenderInfo> ri = std::make_shared<RenderInfo>();
    core::stringw online_name;
    if (global_player_id > -1)
    {
        ri->setHue(race_manager->getKartInfo(global_player_id)
            .getDefaultKartColor());
        online_name = race_manager->getKartInfo(global_player_id)
            .getPlayerName();
    }

    int position           = index+1;
    btTransform init_pos   = getStartTransform(index - gk);
    std::shared_ptr<AbstractKart> new_kart;
    if (RewindManager::get()->isEnabled())
    {
        auto kr = std::make_shared<KartRewinder>(kart_ident, index, position,
            init_pos, handicap, ri);
        kr->rewinderAdd();
        new_kart = kr;
    }
    else
    {
        new_kart = std::make_shared<Kart>(kart_ident, index, position,
            init_pos, handicap, ri);
    }

    new_kart->init(race_manager->getKartType(index));
    Controller *controller = NULL;
    switch(kart_type)
    {
    case RaceManager::KT_PLAYER:
    {
        if (NetworkConfig::get()->isNetworkAITester())
        {
            AIBaseController* ai = NULL;
            if (race_manager->isBattleMode())
                ai = new BattleAI(new_kart.get());
            else
                ai = new SkiddingAI(new_kart.get());
            controller = new NetworkAIController(new_kart.get(),
                local_player_id, ai);
        }
        else
        {
            controller = new LocalPlayerController(new_kart.get(),
                local_player_id, handicap);
            const PlayerProfile* p = StateManager::get()
                ->getActivePlayer(local_player_id)->getConstProfile();
            if (p && p->getDefaultKartColor() > 0.0f)
            {
                ri->setHue(p->getDefaultKartColor());
            }
        }
        m_num_players ++;
        break;
    }
    case RaceManager::KT_NETWORK_PLAYER:
    {
        controller = new NetworkPlayerController(new_kart.get());
        if (!online_name.empty())
            new_kart->setOnScreenText(online_name.c_str());
        m_num_players++;
        break;
    }
    case RaceManager::KT_AI:
    {
        controller = loadAIController(new_kart.get());
        break;
    }
    case RaceManager::KT_GHOST:
    case RaceManager::KT_LEADER:
    case RaceManager::KT_SPARE_TIRE:
        break;
    }

    new_kart->setController(controller);
    race_manager->setKartColor(index, ri->getHue());
    return new_kart;
}   // createKart

//-----------------------------------------------------------------------------
/** Returns the start coordinates for a kart with a given index.
 *  \param index Index of kart ranging from 0 to kart_num-1. */
const btTransform &World::getStartTransform(int index)
{
    return Track::getCurrentTrack()->getStartTransform(index);
}   // getStartTransform

//-----------------------------------------------------------------------------
/** Creates an AI controller for the kart.
 *  \param kart The kart to be controlled by an AI.
 */
Controller* World::loadAIController(AbstractKart* kart)
{
    Controller *controller;
    int turn=0;

    if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_3_STRIKES
        || race_manager->getMinorMode()==RaceManager::MINOR_MODE_FREE_FOR_ALL)
        turn=1;
    else if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_SOCCER)
        turn=2;
    // If different AIs should be used, adjust turn (or switch randomly
    // or dependent on difficulty)
    switch(turn)
    {
        case 0:
            // If requested, start the test ai
            if( (AIBaseController::getTestAI()!=0                       ) && 
                ( (kart->getWorldKartId()+1) % AIBaseController::getTestAI() )==0)
                controller = new TestAI(kart);
            else
                controller = new SkiddingAI(kart);
            break;
        case 1:
            controller = new BattleAI(kart);
            break;
        case 2:
            controller = new SoccerAI(kart);
            break;
        default:
            Log::warn("[World]", "Unknown AI, using default.");
            controller = new SkiddingAI(kart);
            break;
    }

    return controller;
}   // loadAIController

//-----------------------------------------------------------------------------
World::~World()
{
    material_manager->unloadAllTextures();
    RewindManager::destroy();

    irr_driver->onUnloadWorld();

    projectile_manager->cleanup();

    // In case that a race is aborted (e.g. track not found) track is 0.
    if(Track::getCurrentTrack())
        Track::getCurrentTrack()->cleanup();

    // Delete the in-race-gui:
    if(m_saved_race_gui)
    {
        // If there is a save race gui, this means that the result gui is
        // currently being shown. The race result gui is a screen and so
        // is deleted by the state manager. So we only have to delete
        // the actual race gui:
        delete m_saved_race_gui;
    }
    else
    {
        // No race result gui is shown, so m_race_gui is the in-race
        // gui and this must be deleted.
        delete m_race_gui;
    }
    
    Weather::kill();

    m_karts.clear();
    if(race_manager->hasGhostKarts() || race_manager->isRecordingRace())
    {
        // Destroy the old replay object, which also stored the ghost
        // karts, and create a new one (which means that in further
        // races the usage of ghosts will still be enabled).
        // It can allow auto recreation of ghost replay file lists
        // when next time visit the ghost replay selection screen.
        ReplayPlay::destroy();
        ReplayPlay::create();
    }
    if(race_manager->isRecordingRace())
        ReplayRecorder::get()->reset();
    race_manager->setRaceGhostKarts(false);
    race_manager->setRecordRace(false);
    race_manager->setWatchingReplay(false);
    race_manager->setTimeTarget(0.0f);
    race_manager->setSpareTireKartNum(0);

    Camera::removeAllCameras();

    // In case that the track is not found, Physics was not instantiated,
    // but kill handles this correctly.
    Physics::kill();

    Scripting::ScriptEngine::kill();

    m_world = NULL;

    irr_driver->getSceneManager()->clear();

#ifdef DEBUG
    m_magic_number = 0xDEADBEEF;
#endif

}   // ~World

//-----------------------------------------------------------------------------
/** Called when 'go' is being displayed for the first time. Here the brakes
 *  of the karts are released.
 */
void World::onGo()
{
    // Reset the brakes now that the prestart
    // phase is over (braking prevents the karts
    // from sliding downhill)
    for(unsigned int i=0; i<m_karts.size(); i++)
    {
        if (m_karts[i]->isGhostKart()) continue;
        m_karts[i]->getVehicle()->setAllBrakes(0);
    }
    // Reset track objects 1 more time to make sure all instances of moveable
    // fall at the same instant when race start in network
    if (NetworkConfig::get()->isNetworking())
    {
        PtrVector<TrackObject>& objs = Track::getCurrentTrack()
            ->getTrackObjectManager()->getObjects();
        for (TrackObject* curr : objs)
        {
            if (curr->getPhysicalObject())
            {
                curr->reset();
                curr->resetEnabled();
            }
        }
    }
}   // onGo

//-----------------------------------------------------------------------------
/** Called at the end of a race. Updates highscores, pauses the game, and
 *  informs the unlock manager about the finished race. This function must
 *  be called after all other stats were updated from the different game
 *  modes.
 */
void World::terminateRace()
{
    // In case the user opened paused dialog in network
    GUIEngine::ScreenKeyboard::dismiss();
    GUIEngine::ModalDialog::dismiss();

    m_schedule_pause = false;
    m_schedule_unpause = false;

    // Update the estimated finishing time for all karts that haven't
    // finished yet.
    const unsigned int kart_amount = getNumKarts();
    for(unsigned int i = 0; i < kart_amount ; i++)
    {
        if(!m_karts[i]->hasFinishedRace() && !m_karts[i]->isEliminated())
        {
            m_karts[i]->finishedRace(
                estimateFinishTimeForKart(m_karts[i].get()));

        }
    }   // i<kart_amount

    // Update highscores, and retrieve the best highscore if relevant
    // to show it in the GUI
    int best_highscore_rank = -1;
    std::string highscore_who = "";
    if (!isNetworkWorld())
    {
        updateHighscores(&best_highscore_rank);
    }

    updateAchievementDataEndRace();

    PlayerManager::getCurrentPlayer()->raceFinished();

    if (m_race_gui) m_race_gui->clearAllMessages();
    // we can't delete the race gui here, since it is needed in case of
    // a restart: the constructor of it creates some textures which assume
    // that no scene nodes exist. In case of a restart there are scene nodes,
    // so we can't create the race gui again, so we keep it around
    // and save the pointer.
    assert(m_saved_race_gui==NULL);
    m_saved_race_gui = m_race_gui;

    RaceResultGUI* results = RaceResultGUI::getInstance();
    m_race_gui       = results;

    if (best_highscore_rank > 0)
    {
        results->setHighscore(best_highscore_rank);
    }
    else
    {
        results->clearHighscores();
    }

    results->push();
    WorldStatus::terminateRace();
}   // terminateRace

//-----------------------------------------------------------------------------
/** Waits till each kart is resting on the ground
 *
 * Does simulation steps still all karts reach the ground, i.e. are not
 * moving anymore
 */
void World::resetAllKarts()
{
    // Reset the physics 'remaining' time to 0 so that the number
    // of timesteps is reproducible if doing a physics-based history run
    Physics::getInstance()->getPhysicsWorld()->resetLocalTime();

    // If track checking is requested, check all rescue positions if
    // they are high enough.
    if(UserConfigParams::m_track_debug)
    {
        // Loop over all karts, in case that some karts are dfferent
        for(unsigned int kart_id=0; kart_id<(unsigned int)m_karts.size(); kart_id++)
        {
            if (m_karts[kart_id]->isGhostKart()) continue;
            for(unsigned int rescue_pos=0;
                rescue_pos<getNumberOfRescuePositions();
                rescue_pos++)
            {
                btTransform t = getRescueTransform(rescue_pos);
                // This will print out warnings if there is no terrain under
                // the kart, or the kart is being dropped on a reset texture
                moveKartTo(m_karts[kart_id].get(), t);

            }   // rescue_pos<getNumberOfRescuePositions

            // Reset the karts back to the original start position.
            // This call is a bit of an overkill, but setting the correct
            // transforms, positions, motion state is a bit of a hassle.
            m_karts[kart_id]->reset();
        }   // for kart_id<m_karts.size()


    }   // if m_track_debug

    m_schedule_pause = false;
    m_schedule_unpause = false;

    //Project karts onto track from above. This will lower each kart so
    //that at least one of its wheel will be on the surface of the track
    for ( KartList::iterator i=m_karts.begin(); i!=m_karts.end(); i++)
    {
        if ((*i)->isGhostKart()) continue;
        Vec3 xyz = (*i)->getXYZ();
        //start projection from top of kart
        Vec3 up_offset = (*i)->getNormal() * (0.5f * ((*i)->getKartHeight()));
        (*i)->setXYZ(xyz+up_offset);

        bool kart_over_ground = Track::getCurrentTrack()->findGround(i->get());

        if (!kart_over_ground)
        {
            Log::error("World",
                       "No valid starting position for kart %d on track %s.",
                       (int)(i - m_karts.begin()),
                       Track::getCurrentTrack()->getIdent().c_str());
            if (UserConfigParams::m_artist_debug_mode)
            {
                Log::warn("World", "Activating fly mode.");
                (*i)->flyUp();
                continue;
            }
            else
            {
                exit(-1);
            }
        }
    }

    // Do a longer initial simulation, which should be long enough for all
    // karts to be firmly on ground.
    float g = Track::getCurrentTrack()->getGravity();
    for (KartList::iterator i = m_karts.begin(); i != m_karts.end(); i++)
    {
        if ((*i)->isGhostKart()) continue;
        (*i)->getBody()->setGravity(
            (*i)->getMaterial() && (*i)->getMaterial()->hasGravity() ?
            (*i)->getNormal() * -g : Vec3(0, -g, 0));
    }
    for(int i=0; i<stk_config->getPhysicsFPS(); i++) 
        Physics::getInstance()->update(1);

    for ( KartList::iterator i=m_karts.begin(); i!=m_karts.end(); i++)
    {
        (*i)->kartIsInRestNow();
    }

    // Initialise the cameras, now that the correct kart positions are set
    for(unsigned int i=0; i<Camera::getNumCameras(); i++)
    {
        Camera::getCamera(i)->setInitialTransform();
    }
}   // resetAllKarts

// ----------------------------------------------------------------------------
/** Places a kart that is rescued. It calls getRescuePositionIndex to find
 *  to which rescue position the kart should be moved, then getRescueTransform
 *  to get the position and rotation of this rescue position, and then moves
 *  the kart.
 *  \param kart The kart that is rescued.
 */
void World::moveKartAfterRescue(AbstractKart* kart)
{
    unsigned int index = getRescuePositionIndex(kart);
    btTransform t      = getRescueTransform(index);
    moveKartTo(kart, t);
}  // moveKartAfterRescue

// ----------------------------------------------------------------------------
/** Places the kart at a given position and rotation.
 *  \param kart The kart to be moved.
 *  \param transform
 */
void World::moveKartTo(AbstractKart* kart, const btTransform &transform)
{
    btTransform pos(transform);

    // Move the kart
    Vec3 xyz = pos.getOrigin() +
        pos.getBasis() * Vec3(0, 0.5f*kart->getKartHeight(), 0);
    pos.setOrigin(xyz);
    kart->setXYZ(xyz);
    kart->setRotation(pos.getRotation());

    kart->getBody()->setCenterOfMassTransform(pos);
    // The raycast to determine the terrain underneath the kart is done from
    // the centre point of the 4 wheel positions. After a rescue, the wheel
    // positions need to be updated (otherwise the raycast will be done from
    // the previous position, which might be the position that triggered
    // the rescue in the first place).
    kart->getVehicle()->updateAllWheelPositions();

    // Project kart to surface of track
    // This will set the physics transform
    Track::getCurrentTrack()->findGround(kart);
    CheckManager::get()->resetAfterKartMove(kart);

}   // moveKartTo

// ----------------------------------------------------------------------------
void World::schedulePause(Phase phase)
{
    if (m_schedule_unpause)
    {
        m_schedule_unpause = false;
    }
    else
    {
        m_schedule_pause = true;
        m_scheduled_pause_phase = phase;
    }
}   // schedulePause

// ----------------------------------------------------------------------------
void World::scheduleUnpause()
{
    if (m_schedule_pause)
    {
        m_schedule_pause = false;
    }
    else
    {
        m_schedule_unpause = true;
    }
}   // scheduleUnpause

//-----------------------------------------------------------------------------
/** This is the main interface to update the world. This function calls
 *  update(), and checks then for the end of the race. Note that race over
 *  handling can not necessarily be done in update(), since not all
 *  data structures might have been updated (e.g.LinearWorld must
 *  call World::update() first, to get updated kart positions. If race
 *  over would be handled in World::update, LinearWorld had no opportunity
 *  to update its data structures before the race is finished).
 *  \param ticks Number of physics time steps - should be 1.
 */
void World::updateWorld(int ticks)
{
#ifdef DEBUG
    assert(m_magic_number == 0xB01D6543);
#endif


    if (m_schedule_pause)
    {
        pause(m_scheduled_pause_phase);
        m_schedule_pause = false;
    }
    else if (m_schedule_unpause)
    {
        unpause();
        m_schedule_unpause = false;
    }

    // Don't update world if a menu is shown or the race is over.
    if (getPhase() == FINISH_PHASE ||
        (!NetworkConfig::get()->isNetworking() &&
        getPhase() == IN_GAME_MENU_PHASE))
        return;

    try
    {
        update(ticks);
    }
    catch (AbortWorldUpdateException& e)
    {
        (void)e;   // avoid compiler warning
        return;
    }

#ifdef DEBUG
    assert(m_magic_number == 0xB01D6543);
#endif

    if( (!isFinishPhase()) && isRaceOver())
    {
        enterRaceOverState();
    }
    else
    {
        if (m_schedule_exit_race)
        {
            m_schedule_exit_race = false;
            race_manager->exitRace(false);
            race_manager->setAIKartOverride("");

            StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());

            if (m_schedule_tutorial)
            {
                m_schedule_tutorial = false;
                race_manager->setNumPlayers(1);
                race_manager->setMajorMode (RaceManager::MAJOR_MODE_SINGLE);
                race_manager->setMinorMode (RaceManager::MINOR_MODE_TUTORIAL);
                race_manager->setNumKarts( 1 );
                race_manager->setTrack( "tutorial" );
                race_manager->setDifficulty(RaceManager::DIFFICULTY_EASY);
                race_manager->setReverseTrack(false);

                // Use keyboard 0 by default (FIXME: let player choose?)
                InputDevice* device = input_manager->getDeviceManager()->getKeyboard(0);

                // Create player and associate player with keyboard
                StateManager::get()->createActivePlayer(PlayerManager::getCurrentPlayer(),
                                                        device);

                if (!kart_properties_manager->getKart(UserConfigParams::m_default_kart))
                {
                    Log::warn("[World]",
                              "Cannot find kart '%s', will revert to default.",
                              UserConfigParams::m_default_kart.c_str());
                    UserConfigParams::m_default_kart.revertToDefaults();
                }
                race_manager->setPlayerKart(0, UserConfigParams::m_default_kart);

                // ASSIGN should make sure that only input from assigned devices
                // is read.
                input_manager->getDeviceManager()->setAssignMode(ASSIGN);
                input_manager->getDeviceManager()
                    ->setSinglePlayer( StateManager::get()->getActivePlayer(0) );

                delete this;

                StateManager::get()->enterGameState();
                race_manager->setupPlayerKartInfo();
                race_manager->startNew(true);
            }
            else
            {
                delete this;

                if (race_manager->raceWasStartedFromOverworld())
                {
                    OverWorld::enterOverWorld();
                }

            }
        }
    }
}   // updateWorld

#define MEASURE_FPS 0

//-----------------------------------------------------------------------------

void World::scheduleTutorial()
{
    m_schedule_exit_race = true;
    m_schedule_tutorial = true;
}   // scheduleTutorial

//-----------------------------------------------------------------------------
/** This updates all only graphical elements. It is only called once per
 *  rendered frame, not once per time step.
 *  float dt Time since last frame.
 */
void World::updateGraphics(float dt)
{
    if (auto cl = LobbyProtocol::get<ClientLobby>())
    {
        // Reset all smooth network body of rewinders so the rubber band effect
        // of moveable does not exist during firstly live join.
        if (cl->hasLiveJoiningRecently())
            RewindManager::get()->resetSmoothNetworkBody();
    }

    PROFILER_PUSH_CPU_MARKER("World::update (weather)", 0x80, 0x7F, 0x00);
    if (UserConfigParams::m_particles_effects > 1 && Weather::getInstance())
    {
        Weather::getInstance()->update(dt);
    }
    PROFILER_POP_CPU_MARKER();

    // Update graphics of karts, e.g. visual suspension, skid marks
    const int kart_amount = (int)m_karts.size();
    for (int i = 0; i < kart_amount; ++i)
    {
        // Update all karts that are visible
        if (m_karts[i]->isVisible())
        {
            m_karts[i]->updateGraphics(dt);
        }
    }

    PROFILER_PUSH_CPU_MARKER("World::updateGraphics (camera)", 0x60, 0x7F, 0);
    for (unsigned int i = 0; i < Camera::getNumCameras(); i++)
        Camera::getCamera(i)->update(dt);
    PROFILER_POP_CPU_MARKER();

    Scripting::ScriptEngine *script_engine =
        Scripting::ScriptEngine::getInstance();
    if (script_engine)
        script_engine->update(dt);

    projectile_manager->updateGraphics(dt);
    Track::getCurrentTrack()->updateGraphics(dt);
}   // updateGraphics

//-----------------------------------------------------------------------------
/** Updates the physics, all karts, the track, and projectile manager.
 *  \param ticks Number of physics time steps - should be 1.
 */
void World::update(int ticks)
{
#ifdef DEBUG
    assert(m_magic_number == 0xB01D6543);
#endif

    PROFILER_PUSH_CPU_MARKER("World::update()", 0x00, 0x7F, 0x00);

#if MEASURE_FPS
    static int time = 0.0f;
    time += ticks;
    if (time > stk_config->time2Ticks(5.0f))
    {
        time -= stk_config->time2Ticks(5.0f);
        printf("%i\n",irr_driver->getVideoDriver()->getFPS());
    }
#endif

    PROFILER_PUSH_CPU_MARKER("World::update (sub-updates)", 0x20, 0x7F, 0x00);
    WorldStatus::update(ticks);
    PROFILER_POP_CPU_MARKER();
    PROFILER_PUSH_CPU_MARKER("World::update (RewindManager)", 0x20, 0x7F, 0x40);
    RewindManager::get()->update(ticks);
    PROFILER_POP_CPU_MARKER();

    PROFILER_PUSH_CPU_MARKER("World::update (Track object manager)", 0x20, 0x7F, 0x40);
    Track::getCurrentTrack()->getTrackObjectManager()->update(stk_config->ticks2Time(ticks));
    PROFILER_POP_CPU_MARKER();

    PROFILER_PUSH_CPU_MARKER("World::update (Kart::upate)", 0x40, 0x7F, 0x00);

    // Update all the karts. This in turn will also update the controller,
    // which causes all AI steering commands set. So in the following 
    // physics update the new steering is taken into account.
    const int kart_amount = (int)m_karts.size();
    for (int i = 0 ; i < kart_amount; ++i)
    {
        SpareTireAI* sta =
            dynamic_cast<SpareTireAI*>(m_karts[i]->getController());
        // Update all karts that are not eliminated
        if(!m_karts[i]->isEliminated() || (sta && sta->isMoving()))
            m_karts[i]->update(ticks);
        if (isStartPhase())
            m_karts[i]->makeKartRest();
    }
    PROFILER_POP_CPU_MARKER();
    if(race_manager->isRecordingRace()) ReplayRecorder::get()->update(ticks);

    PROFILER_PUSH_CPU_MARKER("World::update (projectiles)", 0xa0, 0x7F, 0x00);
    projectile_manager->update(ticks);
    PROFILER_POP_CPU_MARKER();

    PROFILER_PUSH_CPU_MARKER("World::update (physics)", 0xa0, 0x7F, 0x00);
    Physics::getInstance()->update(ticks);
    PROFILER_POP_CPU_MARKER();

    PROFILER_POP_CPU_MARKER();

#ifdef DEBUG
    assert(m_magic_number == 0xB01D6543);
#endif
}   // update

// ----------------------------------------------------------------------------
/** Only updates the track. The order in which the various parts of STK are
 *  updated is quite important (i.e. the track can't be updated as part of
 *  the standard update call):
 *  the track must be updated after updating the karts (otherwise the
 *  checklines would be using the previous kart positions to determine
 *  new laps, but linear world which determines distance along track would
 *  be using the new kart positions --> the lap counting line will be
 *  triggered one frame too late, potentially causing strange behaviour of
 *  the icons.
 *  Similarly linear world must update the position of all karts after all
 *  karts have been updated (i.e. World::update() must be called before
 *  updating the position of the karts). The check manager (which is called
 *  from Track::update()) needs the updated distance along track, so track
 *  update has to be called after updating the race position in linear world.
 *  That's why there is a separate call for trackUpdate here.
 */
void World::updateTrack(int ticks)
{
    Track::getCurrentTrack()->update(ticks);
}   // update Track

// ----------------------------------------------------------------------------
Highscores* World::getHighscores() const
{
    if (isNetworkWorld() || !m_use_highscores) return NULL;

    const Highscores::HighscoreType type = "HST_" + getIdent();

    Highscores * highscores =
        highscore_manager->getHighscores(type,
                                         race_manager->getNumNonGhostKarts(),
                                         race_manager->getDifficulty(),
                                         race_manager->getTrackName(),
                                         race_manager->getNumLaps(),
                                         race_manager->getReverseTrack());

    return highscores;
}   // getHighscores

// ----------------------------------------------------------------------------
/** Called at the end of a race. Checks if the current times are worth a new
 *  score, if so it notifies the HighscoreManager so the new score is added
 *  and saved.
 */
void World::updateHighscores(int* best_highscore_rank)
{
    *best_highscore_rank = -1;

    if(!m_use_highscores) return;

    // Add times to highscore list. First compute the order of karts,
    // so that the timing of the fastest kart is added first (otherwise
    // someone might get into the highscore list, only to be kicked out
    // again by a faster kart in the same race), which might be confusing
    // if we ever decide to display a message (e.g. during a race)
    unsigned int *index = new unsigned int[m_karts.size()];

    const unsigned int kart_amount = (unsigned int) m_karts.size();
    for (unsigned int i=0; i<kart_amount; i++ )
    {
        index[i] = 999; // first reset the contents of the array
    }
    for (unsigned int i=0; i<kart_amount; i++ )
    {
        const int pos = m_karts[i]->getPosition()-1;
        if(pos < 0 || pos >= (int)kart_amount) continue; // wrong position
        index[pos] = i;
    }

    for (unsigned int pos=0; pos<kart_amount; pos++)
    {
        if(index[pos] == 999)
        {
            // no kart claimed to be in this position, most likely means
            // the kart location data is wrong

#ifdef DEBUG
            Log::error("[World]", "Incorrect kart positions:");
            for (unsigned int i=0; i<m_karts.size(); i++ )
            {
                Log::error("[World]", "i=%d position %d.",i,
                           m_karts[i]->getPosition());
            }
#endif
            continue;
        }

        // Only record times for local player karts and only if
        // they finished the race
        if(!m_karts[index[pos]]->getController()->isLocalPlayerController())
            continue;
        if (!m_karts[index[pos]]->hasFinishedRace()) continue;
        if (m_karts[index[pos]]->isEliminated()) continue;

        assert(index[pos] < m_karts.size());
        Kart *k = (Kart*)m_karts[index[pos]].get();

        Highscores* highscores = getHighscores();

        int highscore_rank = 0;
        // The player is a local player, so there is a name:
        highscore_rank = highscores->addData(k->getIdent(),
                                             k->getController()->getName(),
                                             k->getFinishTime()    );

        if (highscore_rank > 0)
        {
            if (*best_highscore_rank == -1 ||
                highscore_rank < *best_highscore_rank)
            {
                *best_highscore_rank = highscore_rank;
            }

            highscore_manager->saveHighscores();
        }
    } // next position
    delete []index;

}   // updateHighscores

//-----------------------------------------------------------------------------
/** Returns the n-th player kart. Note that this function is O(N), not O(1),
 *  so it shouldn't be called inside of loops.
 *  \param n Index of player kart to return.
 */
AbstractKart *World::getPlayerKart(unsigned int n) const
{
    unsigned int count = -1;

    for(unsigned int i = 0; i < m_karts.size(); i++)
    {
        if (m_karts[i]->getController()->isPlayerController())
        {
            count++;
            if (count == n)
                return m_karts[i].get();
        }
    }
    return NULL;
}   // getPlayerKart

//-----------------------------------------------------------------------------
/** Returns the nth local player kart, i.e. a kart that has a camera.
 *  Note that in profile mode this means a non player kart could be returned
 *  (since an AI kart will have the camera).
 *  \param n Index of player kart to return.
 */
AbstractKart *World::getLocalPlayerKart(unsigned int n) const
{
    if(n>=Camera::getNumCameras()) return NULL;
    return Camera::getCamera(n)->getKart();
}   // getLocalPlayerKart

//-----------------------------------------------------------------------------
/** Remove (eliminate) a kart from the race */
void World::eliminateKart(int kart_id, bool notify_of_elimination)
{
    assert(kart_id < (int)m_karts.size());
    AbstractKart *kart = m_karts[kart_id].get();
    if (kart->isGhostKart()) return;

    // Display a message about the eliminated kart in the race gui
    if (notify_of_elimination)
    {
        for(unsigned int i=0; i<Camera::getNumCameras(); i++)
        {
            Camera *camera = Camera::getCamera(i);
            if(camera->getKart()==kart)
                m_race_gui->addMessage(_("You have been eliminated!"), kart,
                                       2.0f);
            else
            {
                // Store the temporary string because clang would mess this up
                // (remove the stringw before the wchar_t* is used).
                const core::stringw &kart_name = kart->getController()->getName();
                m_race_gui->addMessage(_("'%s' has been eliminated.",
                                       kart_name),
                                       camera->getKart(),
                                       2.0f);
            }
        }  // for i < number of cameras
    }   // if notify_of_elimination

    if(kart->getController()->isLocalPlayerController())
    {
        for(unsigned int i=0; i<Camera::getNumCameras(); i++)
        {
            // Change the camera so that it will be attached to the leader
            // and facing backwards.
            Camera *camera = Camera::getCamera(i);
            if(camera->getKart()==kart)
                camera->setMode(Camera::CM_LEADER_MODE);
        }
        m_eliminated_players++;
    }

    // The kart can't be really removed from the m_kart array, since otherwise
    // a race can't be restarted. So it's only marked to be eliminated (and
    // ignored in all loops). Important:world->getCurrentNumKarts() returns
    // the number of karts still racing. This value can not be used for loops
    // over all karts, use race_manager->getNumKarts() instead!
    kart->eliminate();
    m_eliminated_karts++;

}   // eliminateKart

//-----------------------------------------------------------------------------
/** Called to determine the default collectibles to give each player at the
 *  start for this kind of race. Both parameters are of 'out' type.
 *  \param collectible_type The type of collectible each kart.
 *  \param amount The number of collectibles.
 */
void World::getDefaultCollectibles(int *collectible_type, int *amount )
{
    *collectible_type = PowerupManager::POWERUP_NOTHING;
    *amount = 0;
}   // getDefaultCollectibles

//-----------------------------------------------------------------------------
/** Pauses the music (and then pauses WorldStatus).
 */
void World::pause(Phase phase)
{
    if (m_stop_music_when_dialog_open)
        music_manager->pauseMusic();
    SFXManager::get()->pauseAll();

    WorldStatus::pause(phase);
}   // pause

//-----------------------------------------------------------------------------
void World::unpause()
{
    if (m_stop_music_when_dialog_open)
        music_manager->resumeMusic();
    SFXManager::get()->resumeAll();

    WorldStatus::unpause();

    for(unsigned int i=0; i<m_karts.size(); i++)
    {
        // Note that we can not test for isPlayerController here, since
        // an EndController will also return 'isPlayerController' if the
        // kart belonged to a player.
        LocalPlayerController *pc =
            dynamic_cast<LocalPlayerController*>(m_karts[i]->getController());
        if(pc)
            pc->resetInputState();
    }
}   // pause

//-----------------------------------------------------------------------------
void World::escapePressed()
{
    for (unsigned i = 0; i < m_karts.size(); i++)
    {
        for (unsigned j = 0; j < PA_PAUSE_RACE; j++)
        {
            if (m_karts[i]->isEliminated() || !m_karts[i]->getController()
                ->isLocalPlayerController())
                continue;
            m_karts[i]->getController()->action((PlayerAction)j, 0);
        }
    }

    new RacePausedDialog(0.8f, 0.6f);
}   // escapePressed

// ----------------------------------------------------------------------------
/** Returns the start transform with the give index.
 *  \param rescue_pos Index of the start position to be returned.
 *  \returns The transform of the corresponding start position.
 */
btTransform World::getRescueTransform(unsigned int rescue_pos) const
{
    return Track::getCurrentTrack()->getStartTransform(rescue_pos);
}   // getRescueTransform

//-----------------------------------------------------------------------------
/** Uses the start position as rescue positions, override if necessary
 */
unsigned int World::getNumberOfRescuePositions() const
{
    return Track::getCurrentTrack()->getNumberOfStartPositions();
}   // getNumberOfRescuePositions

//-----------------------------------------------------------------------------
std::shared_ptr<AbstractKart> World::createKartWithTeam
    (const std::string &kart_ident, int index, int local_player_id,
    int global_player_id, RaceManager::KartType kart_type,
    HandicapLevel handicap)
{
    int cur_red = getTeamNum(KART_TEAM_RED);
    int cur_blue = getTeamNum(KART_TEAM_BLUE);
    int pos_index = 0;
    int position  = index + 1;
    KartTeam team = KART_TEAM_BLUE;

    if (kart_type == RaceManager::KT_AI)
    {
        if (index < m_red_ai)
            team = KART_TEAM_RED;
        else
            team = KART_TEAM_BLUE;
        m_kart_team_map[index] = team;
    }
    else if (NetworkConfig::get()->isNetworking())
    {
        m_kart_team_map[index] = race_manager->getKartInfo(index).getKartTeam();
        team = race_manager->getKartInfo(index).getKartTeam();
    }
    else
    {
        int rm_id = index -
            (race_manager->getNumberOfKarts() - race_manager->getNumPlayers());

        assert(rm_id >= 0);
        team = race_manager->getKartInfo(rm_id).getKartTeam();
        m_kart_team_map[index] = team;
    }

    core::stringw online_name;
    if (global_player_id > -1)
    {
        online_name = race_manager->getKartInfo(global_player_id)
            .getPlayerName();
    }

    // Notice: In blender, please set 1,3,5,7... for blue starting position;
    // 2,4,6,8... for red.
    if (team == KART_TEAM_BLUE)
    {
        pos_index = 1 + 2 * cur_blue;
    }
    else
    {
        pos_index = 2 + 2 * cur_red;
    }

    btTransform init_pos = getStartTransform(pos_index - 1);
    m_kart_position_map[index] = (unsigned)(pos_index - 1);

    std::shared_ptr<RenderInfo> ri = std::make_shared<RenderInfo>();
    ri = (team == KART_TEAM_BLUE ? std::make_shared<RenderInfo>(0.66f) :
        std::make_shared<RenderInfo>(1.0f));

    std::shared_ptr<AbstractKart> new_kart;
    if (RewindManager::get()->isEnabled())
    {
        auto kr = std::make_shared<KartRewinder>(kart_ident, index, position,
            init_pos, handicap, ri);
        kr->rewinderAdd();
        new_kart = kr;
    }
    else
    {
        new_kart = std::make_shared<Kart>(kart_ident, index, position,
            init_pos, handicap, ri);
    }

    new_kart->init(race_manager->getKartType(index));
    Controller *controller = NULL;

    switch(kart_type)
    {
    case RaceManager::KT_PLAYER:
        controller = new LocalPlayerController(new_kart.get(), local_player_id, handicap);
        m_num_players ++;
        break;
    case RaceManager::KT_NETWORK_PLAYER:
        controller = new NetworkPlayerController(new_kart.get());
        if (!online_name.empty())
            new_kart->setOnScreenText(online_name.c_str());
        m_num_players++;
        break;
    case RaceManager::KT_AI:
        controller = loadAIController(new_kart.get());
        break;
    case RaceManager::KT_GHOST:
    case RaceManager::KT_LEADER:
    case RaceManager::KT_SPARE_TIRE:
        break;
    }

    new_kart->setController(controller);

    return new_kart;
}   // createKartWithTeam

//-----------------------------------------------------------------------------
int World::getTeamNum(KartTeam team) const
{
    int total = 0;
    if (m_kart_team_map.empty()) return total;

    for (unsigned int i = 0; i < (unsigned)m_karts.size(); ++i)
    {
        if (team == getKartTeam(m_karts[i]->getWorldKartId())) total++;
    }

    return total;
}   // getTeamNum

//-----------------------------------------------------------------------------
KartTeam World::getKartTeam(unsigned int kart_id) const
{
    std::map<int, KartTeam>::const_iterator n =
        m_kart_team_map.find(kart_id);

    assert(n != m_kart_team_map.end());
    return n->second;
}   // getKartTeam

//-----------------------------------------------------------------------------
void World::setAITeam()
{
    m_red_ai  = race_manager->getNumberOfRedAIKarts();
    m_blue_ai = race_manager->getNumberOfBlueAIKarts();
    
    for (int i = 0; i < (int)race_manager->getNumLocalPlayers(); i++)
    {
        KartTeam team = race_manager->getKartInfo(i).getKartTeam();

        // Happen in profiling mode
        if (team == KART_TEAM_NONE)
        {
            race_manager->setKartTeam(i, KART_TEAM_BLUE);
            team = KART_TEAM_BLUE;
            continue; //FIXME, this is illogical
        }
    }

    Log::debug("World", "Blue AI: %d red AI: %d", m_blue_ai, m_red_ai);

}   // setAITeam

// As a class name can't be skipped with "using", we use a preprocessor macro
// to clean up the two following functions
#define ACS AchievementsStatus

//-----------------------------------------------------------------------------
/* This function takes care to update all relevant achievements
 * and statistics counters related to a finished race. */
void World::updateAchievementDataEndRace()
{
    const unsigned int kart_amount = getNumKarts();

    for(unsigned int i = 0; i < kart_amount; i++)
    {
        // TODO : does this work in multiplayer ?
        // TODO : check what happens when abandonning a race in a GP
        // Retrieve the current player
        if (m_karts[i]->getController()->canGetAchievements())
        {
            // Increment won races counts and track finished counts
            if (race_manager->isLinearRaceMode())
            {
                ACS::AchievementData diff;
                diff = (race_manager->getDifficulty() == RaceManager::DIFFICULTY_EASY)   ? ACS::EASY_FINISHED :
                       (race_manager->getDifficulty() == RaceManager::DIFFICULTY_MEDIUM) ? ACS::MEDIUM_FINISHED :
                       (race_manager->getDifficulty() == RaceManager::DIFFICULTY_HARD)   ? ACS::HARD_FINISHED :
                                                                                           ACS::BEST_FINISHED;
                PlayerManager::increaseAchievement(diff,1);

                PlayerManager::trackEvent(race_manager->getTrackName(), ACS::TR_FINISHED);
                if (race_manager->getReverseTrack())
                    PlayerManager::trackEvent(race_manager->getTrackName(), ACS::TR_FINISHED_REVERSE);

                if (race_manager->modeHasLaps())
                {
                    Track* track = track_manager->getTrack(race_manager->getTrackName());
                    int default_lap_num = track->getDefaultNumberOfLaps();
                    if (race_manager->getNumLaps() < default_lap_num)
                    {
                        PlayerManager::trackEvent(race_manager->getTrackName(), ACS::TR_LESS_LAPS);
                    }
                    else if (race_manager->getNumLaps() > default_lap_num)
                    {
                        PlayerManager::trackEvent(race_manager->getTrackName(), ACS::TR_MORE_LAPS);
                        if (race_manager->getNumLaps() >= 2*default_lap_num)
                            PlayerManager::trackEvent(race_manager->getTrackName(), ACS::TR_MIN_TWICE_LAPS);
                    }
                }

                int winner_position = 1;
                //TODO : check this always work : what happens if the leader is overtaken between the last elimination
                //       and the results screen ?
                if (race_manager->isFollowMode()) winner_position = 2;
                // Check if the player has won
                if (m_karts[i]->getPosition() == winner_position)
                {
                    if (race_manager->getNumNonGhostKarts() >= 2)
                        PlayerManager::trackEvent(race_manager->getTrackName(), ACS::TR_WON);
                    else
                        PlayerManager::trackEvent(race_manager->getTrackName(), ACS::TR_FINISHED_ALONE);
                    if (race_manager->getNumberOfAIKarts() >= 3)
                    {
                        PlayerManager::increaseAchievement(ACS::WON_RACES,1);
                        PlayerManager::increaseAchievement(ACS::CONS_WON_RACES,1);
                        if (race_manager->isTimeTrialMode())
                            PlayerManager::increaseAchievement(ACS::WON_TT_RACES,1);
                        else if (race_manager->isFollowMode())
                            PlayerManager::increaseAchievement(ACS::WON_FTL_RACES,1);
                        else // normal race
                            PlayerManager::increaseAchievement(ACS::WON_NORMAL_RACES,1);
                    }
                    if (race_manager->getNumberOfAIKarts() >= 5 &&
                        (race_manager->getDifficulty() == RaceManager::DIFFICULTY_HARD ||
                         race_manager->getDifficulty() == RaceManager::DIFFICULTY_BEST))
                        PlayerManager::increaseAchievement(ACS::CONS_WON_RACES_HARD,1);
                }
                // Race lost, reset the consecutive wins counters
                else if (m_karts[i]->getPosition() > winner_position)
                {
                    PlayerManager::resetAchievementData(ACS::CONS_WON_RACES);
                    PlayerManager::resetAchievementData(ACS::CONS_WON_RACES_HARD);
                }
            } // if isLinearMode

            // Increment egg hunt finished count
            else if (race_manager->isEggHuntMode())
            {
                PlayerManager::trackEvent(race_manager->getTrackName(), ACS::TR_EGG_HUNT_FINISHED);
            }

            updateAchievementModeCounters(false /*start*/);
         } // if m_karts[i]->getController()->canGetAchievements()
    } // for i<kart_amount
} // updateAchievementDataEndRace

//-----------------------------------------------------------------------------
/* This function updates the race mode start and finish counters.
 * \param start - true if start, false if finish */
void World::updateAchievementModeCounters(bool start)
{
    if (race_manager->isTimeTrialMode())
        PlayerManager::increaseAchievement(start ? ACS::TT_STARTED : ACS::TT_FINISHED,1);
    else if (race_manager->isFollowMode())
        PlayerManager::increaseAchievement(start ? ACS::FTL_STARTED : ACS::FTL_FINISHED,1);
    else if (race_manager->isEggHuntMode())
        PlayerManager::increaseAchievement(start ? ACS::EGG_HUNT_STARTED : ACS::EGG_HUNT_FINISHED,1);
    else if (race_manager->isSoccerMode())
        PlayerManager::increaseAchievement(start ? ACS::SOCCER_STARTED : ACS::SOCCER_FINISHED,1);
    else if (race_manager->isBattleMode())
    {
        if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_3_STRIKES)
            PlayerManager::increaseAchievement(start ? ACS::THREE_STRIKES_STARTED : ACS::THREE_STRIKES_FINISHED,1);
        else if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_CAPTURE_THE_FLAG)
            PlayerManager::increaseAchievement(start ? ACS::CTF_STARTED : ACS::CTF_FINISHED,1);
        else if (race_manager->getMinorMode() == RaceManager::MINOR_MODE_FREE_FOR_ALL)
            PlayerManager::increaseAchievement(start ? ACS::FFA_STARTED : ACS::FFA_FINISHED,1);
    }
    else // normal races
        PlayerManager::increaseAchievement(start ? ACS::NORMAL_STARTED : ACS::NORMAL_FINISHED,1);

    if (race_manager->hasGhostKarts())
        PlayerManager::increaseAchievement(start ? ACS::WITH_GHOST_STARTED : ACS::WITH_GHOST_FINISHED,1);
} // updateAchievementModeCounters
#undef ACS
