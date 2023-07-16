//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Joerg Henrichs
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

#include "states_screens/race_result_gui.hpp"

#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "audio/sfx_base.hpp"
#include "challenges/challenge_status.hpp"
#include "challenges/story_mode_timer.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/2dutils.hpp"
#include "graphics/material.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/message_queue.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen_keyboard.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "io/file_manager.hpp"
#include "karts/controller/controller.hpp"
#include "karts/controller/end_controller.hpp"
#include "karts/controller/local_player_controller.hpp"
#include "karts/ghost_kart.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/cutscene_world.hpp"
#include "modes/demo_world.hpp"
#include "modes/capture_the_flag.hpp"
#include "modes/overworld.hpp"
#include "modes/soccer_world.hpp"
#include "network/network_config.hpp"
#include "network/stk_host.hpp"
#include "network/protocols/client_lobby.hpp"
#include "race/highscores.hpp"
#include "race/highscore_manager.hpp"
#include "replay/replay_play.hpp"
#include "replay/replay_recorder.hpp"
#include "scriptengine/property_animator.hpp"
#include "states_screens/cutscene_general.hpp"
#include "states_screens/feature_unlocked.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/race_setup_screen.hpp"
#include "tips/tips_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <algorithm>

/** Constructor, initialises internal data structures.
 */
RaceResultGUI::RaceResultGUI() : Screen("race_result.stkgui",
    /*pause race*/ false)
{
}   // RaceResultGUI

//-----------------------------------------------------------------------------
/** Besides calling init in the base class this makes all buttons of this
 *  screen invisible. The buttons will be displayed only once the animation is
 *  over.
 */
void RaceResultGUI::init()
{
    Screen::init();
    determineTableLayout();
    m_animation_state = RR_INIT;

    m_timer = 0;

    getWidget("operations")->setActive(false);
    getWidget("left")->setVisible(false);
    getWidget("middle")->setVisible(false);
    getWidget("right")->setVisible(false);

    m_started_race_over_music = false;
    music_manager->stopMusic();

    bool human_win = true;
    bool has_human_players = false;
    bool in_first_place = false;
    unsigned int num_karts = RaceManager::get()->getNumberOfKarts();
    for (unsigned int kart_id = 0; kart_id < num_karts; kart_id++)
    {
        const AbstractKart *kart = World::getWorld()->getKart(kart_id);
        if (kart->getController()->isLocalPlayerController())
        {
            has_human_players = true;
            human_win = human_win && kart->getRaceResult();

            if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_FOLLOW_LEADER)
            {
                // It's possible the winning kart can get ahead of the leader.
                in_first_place = kart->getPosition() <= 2;
            }
            else
            {
                in_first_place = kart->getPosition() == 1;
            }
        }
    }

    m_finish_sound = SFXManager::get()->quickSound(
        human_win ? "race_finish_victory" : "race_finish");

    // Play different result music based on player kart positions.
    if (has_human_players)
    {
        if (human_win)
        {
            if (in_first_place)
            {
                // At least one player kart is in 1st place.
                m_race_over_music = stk_config->m_race_win_music;
            }
            else
            {
                // All player karts finished in winning positions, but none in 1st place.
                m_race_over_music = stk_config->m_race_neutral_music;
            }
        }
        else
        {
            // No player karts finished in winning positions.
            m_race_over_music = stk_config->m_race_lose_music;
        }
    }
    else
    {
        // For races with only AI karts and no human players.
        m_race_over_music = stk_config->m_race_neutral_music;
    }

    if (!m_finish_sound)
    {
        // If there is no finish sound (because sfx are disabled), start
        // the race over music here (since the race over music is only started
        // when the finish sound has been played).
        try
        {music_manager->startMusic(m_race_over_music);}
        catch (std::exception& e)
        {
            Log::error("RaceResultGUI", "Exception caught when "
                "trying to load music: %s", e.what());
        }
    }

    // Calculate how many track screenshots can fit into the "result-table" widget
    GUIEngine::Widget* result_table = getWidget("result-table");
    assert(result_table != NULL);
    m_sshot_height = (int)(UserConfigParams::m_height*0.1275);
    m_max_tracks = std::max(1, ((result_table->m_h - getFontHeight() * 5) /
        (m_sshot_height + SSHOT_SEPARATION))); //Show at least one

    // Calculate screenshot scrolling parameters
    const std::vector<std::string> tracks =
        RaceManager::get()->getGrandPrix().getTrackNames();
    int n_tracks = (int)tracks.size();
    int currentTrack = RaceManager::get()->getTrackNumber();
    m_start_track = currentTrack;
    if (n_tracks > m_max_tracks)
    {
        m_start_track = std::min(currentTrack, n_tracks - m_max_tracks);
        m_end_track = std::min(currentTrack + m_max_tracks, n_tracks);
    }
    else
    {
        m_start_track = 0;
        m_end_track = (int)tracks.size();
    }

#ifndef SERVER_ONLY
    if (!human_win && !NetworkConfig::get()->isNetworking() &&
        !TipsManager::get()->isEmpty())
    {
        std::string tipset = "race";
        if (RaceManager::get()->isSoccerMode())
            tipset = "soccer";
        core::stringw tip = TipsManager::get()->getTip(tipset);
        core::stringw tips_string = _("Tip: %s", tip);
        MessageQueue::add(MessageQueue::MT_GENERIC, tips_string);
    }
#endif
    
    if (RaceManager::get()->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX &&
        !NetworkConfig::get()->isNetworking() &&
        (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_NORMAL_RACE || RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_TIME_TRIAL ||
        RaceManager::get()->isLapTrialMode()))
    {
        const AbstractKart* k = RaceManager::get()->getKartWithGPRank(RaceManager::get()->getLocalPlayerGPRank(PLAYER_ID_GAME_MASTER));
        RaceManager::get()->addGPTotalLaps(World::getWorld()->getFinishedLapsOfKart(k->getWorldKartId()));
        if (RaceManager::get()->getNumOfTracks() == RaceManager::get()->getTrackNumber() + 1
           && !RaceManager::get()->getGrandPrix().isRandomGP() && RaceManager::get()->getSkippedTracksInGP() == 0)
        {
            Highscores* highscores = World::getWorld()->getGPHighscores();
            float full_time;
            if (RaceManager::get()->isLapTrialMode())
                full_time = static_cast<float>(RaceManager::get()->getGPTotalLaps());
            else
                full_time = RaceManager::get()->getOverallTime(RaceManager::get()->getLocalPlayerGPRank(PLAYER_ID_GAME_MASTER));
            std::string gp_name = RaceManager::get()->getGrandPrix().getId();
            highscores->addGPData(k->getIdent(), k->getController()->getName(), gp_name, full_time);
        }
    }
}   // init

//-----------------------------------------------------------------------------
void RaceResultGUI::tearDown()
{
    Screen::tearDown();
    //m_font->setMonospaceDigits(m_was_monospace);

    if (m_finish_sound != NULL &&
        m_finish_sound->getStatus() == SFXBase::SFX_PLAYING)
    {
        m_finish_sound->stop();
    }
}   // tearDown

//-----------------------------------------------------------------------------
/** Makes the correct buttons visible again, and gives them the right label.
 *  1) If something was unlocked, only a 'next' button is displayed.
 */
void RaceResultGUI::enableAllButtons()
{
    GUIEngine::IconButtonWidget *left = getWidget<GUIEngine::IconButtonWidget>("left");
    GUIEngine::IconButtonWidget *middle = getWidget<GUIEngine::IconButtonWidget>("middle");
    GUIEngine::IconButtonWidget *right = getWidget<GUIEngine::IconButtonWidget>("right");
    GUIEngine::RibbonWidget *operations = getWidget<GUIEngine::RibbonWidget>("operations");
    operations->setActive(true);
    operations->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    if (RaceManager::get()->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        enableGPProgress();
    }

    // If we're in a network world, change the buttons text
    if (World::getWorld()->isNetworkWorld())
    {
        right->setLabel(_("Continue"));
        right->setImage("gui/icons/green_check.png");
        right->setVisible(true);
        operations->select("right", PLAYER_ID_GAME_MASTER);
        middle->setVisible(false);
        left->setLabel(_("Quit the server"));
        left->setImage("gui/icons/main_quit.png");
        left->setVisible(true);
        return;
    }

    // If something was unlocked
    // -------------------------

    // Note: Only returns greater than 0 for regular races, despite GPs
    // showing feature unlocked cutscene.  GPs have their own logic.
    int n = (int)PlayerManager::getCurrentPlayer()
        ->getRecentlyCompletedChallenges().size();

    if (n > 0 &&
         (RaceManager::get()->getMajorMode() != RaceManager::MAJOR_MODE_GRAND_PRIX ||
          RaceManager::get()->getTrackNumber() + 1 == RaceManager::get()->getNumOfTracks() ) )
    {
        middle->setLabel(_("Continue"));
        middle->setImage("gui/icons/green_check.png");
        middle->setVisible(true);
        operations->select("middle", PLAYER_ID_GAME_MASTER);
    }
    else if (RaceManager::get()->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        // In case of a GP:
        // ----------------
        middle->setLabel(_("Continue"));
        middle->setImage("gui/icons/green_check.png");
        middle->setVisible(false);
        middle->setFocusable(false);
        left->setVisible(false);
        left->setFocusable(false);

        // Two continue buttons to make sure the buttons in the bar is balanced
        right->setLabel(_("Continue"));
        right->setImage("gui/icons/green_check.png");
        right->setVisible(true);

        if (RaceManager::get()->getTrackNumber() + 1 < RaceManager::get()->getNumOfTracks())
        {
            left->setLabel(_("Abort Grand Prix"));
            left->setImage("gui/icons/race_giveup.png");
            left->setVisible(true);
            left->setFocusable(true);
            operations->select("right", PLAYER_ID_GAME_MASTER);
        }
        else
        {
            right->setVisible(false);
            right->setFocusable(false);
            middle->setVisible(true);
            operations->select("middle", PLAYER_ID_GAME_MASTER);
        }

    }
    else
    {
        // Normal race
        // -----------

        right->setLabel(_("Restart"));
        right->setImage("gui/icons/restart.png");
        right->setVisible(true);
        operations->select("right", PLAYER_ID_GAME_MASTER);
        if (RaceManager::get()->raceWasStartedFromOverworld())
        {
            middle->setVisible(false);
            left->setLabel(_("Back to challenge selection"));
            left->setImage("gui/icons/back.png");
        }
        else
        {
            middle->setImage("gui/icons/main_race.png");
            if (RaceManager::get()->isRecordingRace())
            {
                middle->setLabel(_("Race against the new ghost replay"));
                middle->setVisible(!World::getWorld()->hasRaceEndedEarly());
            }
            else
            {
                middle->setLabel(_("Setup New Race"));
                middle->setVisible(true);
            }
            left->setLabel(_("Back to the menu"));
            left->setImage("gui/icons/back.png");
        }
        left->setVisible(true);
    }
}   // enableAllButtons

//-----------------------------------------------------------------------------
void RaceResultGUI::eventCallback(GUIEngine::Widget* widget,
    const std::string& name, const int playerID)
{
    int n_tracks = RaceManager::get()->getGrandPrix().getNumberOfTracks();
    if (name == "up_button")
    {
        if (n_tracks > m_max_tracks && m_start_track > 0)
        {
            m_start_track--;
            m_end_track--;
            displayScreenShots();
        }
    }
    else if (name == "down_button")
    {
        if (n_tracks > m_max_tracks && m_start_track < (n_tracks - m_max_tracks))
        {
            m_start_track++;
            m_end_track++;
            displayScreenShots();
        }
    }
    else if (name == "operations")
    {
        const std::string& action =
            getWidget<GUIEngine::RibbonWidget>("operations")->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        // User pressed "Continue" button, go from race results to overall GP results
        if (m_animation_state == RR_WAITING_GP_RESULT && action == "middle")
        {
            GUIEngine::IconButtonWidget *middle = getWidget<GUIEngine::IconButtonWidget>("middle");
            middle->setVisible(false);
            getWidget("operations")->setActive(false);
            m_all_row_infos = m_all_row_info_waiting;
            m_animation_state = RR_OLD_GP_RESULTS;
            m_timer = 0;
            return;
        }

        // If we're playing online :
        if (World::getWorld()->isNetworkWorld())
        {
            if (action == "right") // Continue button (return to server lobby)
            {
                // Signal to the server that this client is back in the lobby now.
                auto cl = LobbyProtocol::get<ClientLobby>();
                if (cl)
                    cl->doneWithResults();
                getWidget<GUIEngine::IconButtonWidget>("right")->setLabel(_("Waiting for others"));
            }
            if (action == "left") // Quit server (return to online lan / wan menu)
            {
                RaceManager::get()->clearNetworkGrandPrixResult();
                if (STKHost::existHost())
                {
                    STKHost::get()->shutdown();
                }
                RaceManager::get()->exitRace();
                RaceManager::get()->setAIKartOverride("");
                StateManager::get()->resetAndSetStack(
                    NetworkConfig::get()->getResetScreens().data());
                NetworkConfig::get()->unsetNetworking();
            }
            return;
        }

        // If something was unlocked, the 'continue' button is
        // used to trigger the unlocked feature(s) cutscene.
        // ---------------------------------------------------------
        PlayerProfile *player = PlayerManager::getCurrentPlayer();

        // Note: Only returns greater than 0 for regular races, despite GPs
        // showing feature unlocked cutscene.  GPs have their own logic.
        int n = (int)player->getRecentlyCompletedChallenges().size();

        if (n > 0 &&
             (RaceManager::get()->getMajorMode() != RaceManager::MAJOR_MODE_GRAND_PRIX ||
              RaceManager::get()->getTrackNumber() + 1 == RaceManager::get()->getNumOfTracks() ) )
        {
            if (action == "middle")
            {
                if (RaceManager::get()->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
                {
                    cleanupGPProgress();
                }

                std::vector<const ChallengeData*> unlocked = player->getRecentlyCompletedChallenges();

                bool gameCompleted = false;
                for (unsigned int n = 0; n < unlocked.size(); n++)
                {
                    if (unlocked[n]->getChallengeId() == "fortmagma")
                    {
                        gameCompleted = true;
                        story_mode_timer->stopTimer();
                        player->setFinished();
                        player->setStoryModeTimer(story_mode_timer->getStoryModeTime());
                        if (story_mode_timer->speedrunIsFinished())
                        {
                            player->setSpeedrunTimer(story_mode_timer->getSpeedrunTime());
                            player->setSpeedrunFinished();
                        }
                        break;
                    }
                }

                if (gameCompleted)
                {
                    // clear the race

                    // kart will no longer be available during cutscene, drop reference
                    StateManager::get()->getActivePlayer(playerID)->setKart(NULL);
                    PropertyAnimator::get()->clear();
                    World::deleteWorld();

                    CutsceneWorld::setUseDuration(true);
                    StateManager::get()->enterGameState();
                    RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
                    RaceManager::get()->setNumKarts(0);
                    RaceManager::get()->setNumPlayers(0);
                    RaceManager::get()->startSingleRace("endcutscene", 999, false);

                    std::vector<std::string> parts;
                    parts.push_back("endcutscene");
                    ((CutsceneWorld*)World::getWorld())->setParts(parts);

                    CutSceneGeneral* scene = CutSceneGeneral::getInstance();
                    scene->push();
                }
                else
                {
                    StateManager::get()->popMenu();
                    PropertyAnimator::get()->clear();
                    World::deleteWorld();

                    CutsceneWorld::setUseDuration(false);
                    StateManager::get()->enterGameState();
                    RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_CUTSCENE);
                    RaceManager::get()->setNumKarts(0);
                    RaceManager::get()->setNumPlayers(0);
                    RaceManager::get()->startSingleRace("featunlocked", 999, RaceManager::get()->raceWasStartedFromOverworld());

                    FeatureUnlockedCutScene* scene =
                        FeatureUnlockedCutScene::getInstance();

                    scene->addTrophy(RaceManager::get()->getDifficulty(),false);
                    scene->findWhatWasUnlocked(RaceManager::get()->getDifficulty(),unlocked);
                    scene->push();
                    RaceManager::get()->setAIKartOverride("");

                    std::vector<std::string> parts;
                    parts.push_back("featunlocked");
                    ((CutsceneWorld*)World::getWorld())->setParts(parts);
                }

                PlayerManager::getCurrentPlayer()->clearUnlocked();

                return;
            }
            Log::warn("RaceResultGUI", "Incorrect event '%s' when things are unlocked.",
                action.c_str());
        }

        // Next check for GP
        // -----------------
        if (RaceManager::get()->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
        {
            if (action == "right" || action == "middle")        // Next GP
            {
                cleanupGPProgress();
                StateManager::get()->popMenu();
                RaceManager::get()->next();
            }
            else if (action == "left")        // Abort
            {
                new MessageDialog(_("Do you really want to abort the Grand Prix?"),
                    MessageDialog::MESSAGE_DIALOG_CONFIRM, this,
                    /*delete_listener*/false, /*from_queue*/false,
                    /*width*/0.6f, /*height*/0.6f, /*focus_on_cancel*/true);
            }
            else if (!getWidget(action.c_str())->isVisible())
            {
                Log::warn("RaceResultGUI", "Incorrect event '%s' when things are unlocked.",
                    action.c_str());
            }
            return;
        }

        StateManager::get()->popMenu();
        if (action == "right")        // Restart
        {
            RaceManager::get()->rerunRace();
        }
        else if (action == "middle")                 // Setup new race
        {
            // Save current race data for race against new ghost
            std::string track_name = RaceManager::get()->getTrackName();
            int laps = RaceManager::get()->getNumLaps();
            bool reverse = RaceManager::get()->getReverseTrack();
            bool new_ghost_race = RaceManager::get()->isRecordingRace();

            RaceManager::get()->exitRace();
            RaceManager::get()->setAIKartOverride("");

            //If pressing continue quickly in a losing challenge
            if (RaceManager::get()->raceWasStartedFromOverworld())
            {
                StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
                OverWorld::enterOverWorld();
            }
            // Special case : race against a newly saved ghost
            else if (new_ghost_race)
            {
                ReplayPlay::get()->loadAllReplayFile();
                unsigned long long int last_uid = ReplayRecorder::get()->getLastUID();
                ReplayPlay::get()->setReplayFileByUID(last_uid);

                RaceManager::get()->setRecordRace(true);
                RaceManager::get()->setRaceGhostKarts(true);

                RaceManager::get()->setNumKarts(RaceManager::get()->getNumLocalPlayers());

                // Disable accidentally unlocking of a challenge
                PlayerManager::getCurrentPlayer()->setCurrentChallenge("");

                RaceManager::get()->setReverseTrack(reverse);
                RaceManager::get()->startSingleRace(track_name, laps, false);
            }
            else
            {
                Screen* newStack[] = { MainMenuScreen::getInstance(),
                                      RaceSetupScreen::getInstance(),
                                      NULL };
                StateManager::get()->resetAndSetStack(newStack);
            }
        }
        else if (action == "left")        // Back to main
        {
            RaceManager::get()->exitRace();
            RaceManager::get()->setAIKartOverride("");
            StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());

            if (RaceManager::get()->raceWasStartedFromOverworld())
            {
                OverWorld::enterOverWorld();
            }
        }
    }
    else
        Log::warn("RaceResultGUI", "Incorrect event '%s' for normal race.",
            name.c_str());
    return;
}   // eventCallback

//-----------------------------------------------------------------------------
void RaceResultGUI::displayCTFResults()
{
#ifndef SERVER_ONLY
    //Draw win text
    core::stringw result_text;
    video::SColor color = video::SColor(255, 255, 255, 255);
    video::SColor red_color = video::SColor(255, 255, 0, 0);
    gui::IGUIFont* font = GUIEngine::getTitleFont();
    int team_icon_height = font->getDimension(L"A").Height;
    int current_x = UserConfigParams::m_width / 2;
    RowInfo *ri = &(m_all_row_infos[0]);
    int current_y = (int)ri->m_y_pos;
    CaptureTheFlag* ctf = dynamic_cast<CaptureTheFlag*>(World::getWorld());
    const int red_score = ctf->getRedScore();
    const int blue_score = ctf->getBlueScore();

    GUIEngine::Widget *table_area = getWidget("result-table");
    int height = table_area->m_h + table_area->m_y;

    if (red_score > blue_score)
        result_text = _("Red Team Wins");
    else if (blue_score > red_score)
        result_text = _("Blue Team Wins");
    else
        result_text = _("It's a draw");

    core::rect<s32> pos(current_x, current_y, current_x, current_y);
    font->draw(result_text.c_str(), pos, color, true, true);

    core::dimension2du rect = font->getDimension(result_text.c_str());

    //Draw team scores:
    current_y += rect.Height;
    current_x /= 2;
    irr::video::ITexture* red_icon = irr_driver->getTexture(FileManager::GUI_ICON,
        "red_flag.png");
    irr::video::ITexture* blue_icon = irr_driver->getTexture(FileManager::GUI_ICON,
        "blue_flag.png");

    int team_icon_width = team_icon_height * (red_icon->getSize().Width / red_icon->getSize().Height);
    core::recti source_rect(core::vector2di(0, 0), red_icon->getSize());
    current_x -= team_icon_width/2;
    core::recti dest_rect(current_x, current_y,
        current_x + team_icon_width,
        current_y + team_icon_height);
    draw2DImage(red_icon, dest_rect, source_rect,
        NULL, NULL, true);
    current_x += UserConfigParams::m_width / 2;
    dest_rect = core::recti(current_x, current_y,
        current_x + team_icon_width,
        current_y + team_icon_height);
    draw2DImage(blue_icon, dest_rect, source_rect,
        NULL, NULL, true);

    result_text = StringUtils::toWString(blue_score);
    rect = font->getDimension(result_text.c_str());
    current_x += team_icon_width / 2;
    current_y += team_icon_height + rect.Height / 4;
    pos = core::rect<s32>(current_x, current_y, current_x, current_y);
    font->draw(result_text.c_str(), pos, color, true, false);

    current_x -= UserConfigParams::m_width / 2;
    result_text = StringUtils::toWString(red_score);
    pos = core::rect<s32>(current_x, current_y, current_x, current_y);
    font->draw(result_text.c_str(), pos, color, true, false);

    int center_x = UserConfigParams::m_width / 2;
    pos = core::rect<s32>(center_x, current_y, center_x, current_y);
    font->draw("-", pos, color, true, false);

    // The red team player scores:
    current_y += rect.Height / 2 + rect.Height / 4;
    font = GUIEngine::getSmallFont();
    irr::video::ITexture* kart_icon;

    int prev_y = current_y;
    const unsigned num_karts = ctf->getNumKarts();
    for (unsigned int i = 0; i < num_karts; i++)
    {
        AbstractKart* kart = ctf->getKartAtPosition(i + 1);
        unsigned kart_id = kart->getWorldKartId();
        if (ctf->getKartTeam(kart_id) != KART_TEAM_RED)
            continue;
        result_text = kart->getController()->getName();
        if (RaceManager::get()->getKartGlobalPlayerId(kart_id) > -1)
        {
            const core::stringw& flag = StringUtils::getCountryFlag(
                RaceManager::get()->getKartInfo(kart_id).getCountryCode());
            if (!flag.empty())
            {
                result_text += L" ";
                result_text += flag;
            }
        }
        result_text.append("  ");
        if (kart->isEliminated())
        {
            continue;
        }
        else
        {
            result_text.append(
                StringUtils::toWString(ctf->getKartScore(kart_id)));
        }
        rect = font->getDimension(result_text.c_str());
        current_y += rect.Height;

        if (current_y > height) break;

        pos = core::rect<s32>(current_x, current_y, current_x, current_y);
        font->draw(result_text, pos,
            kart->getController()->isLocalPlayerController() ?
            red_color : color, true, false);
        kart_icon = kart->getKartProperties()->getIconMaterial()->getTexture();
        source_rect = core::recti(core::vector2di(0, 0), kart_icon->getSize());
        irr::u32 offset_x =
            (irr::u32)(font->getDimension(result_text.c_str()).Width / 1.5f);
        dest_rect = core::recti(current_x - offset_x - m_width_icon, current_y,
            current_x - offset_x, current_y + m_width_icon);
        draw2DImage(kart_icon, dest_rect, source_rect, NULL, NULL, true);
    }

    // The blue team player scores:
    current_y = prev_y;
    current_x += UserConfigParams::m_width / 2;
    for (unsigned int i = 0; i < num_karts; i++)
    {
        AbstractKart* kart = ctf->getKartAtPosition(i + 1);
        unsigned kart_id = kart->getWorldKartId();
        if (ctf->getKartTeam(kart_id) != KART_TEAM_BLUE)
            continue;
        result_text = kart->getController()->getName();
        if (RaceManager::get()->getKartGlobalPlayerId(kart_id) > -1)
        {
            const core::stringw& flag = StringUtils::getCountryFlag(
                RaceManager::get()->getKartInfo(kart_id).getCountryCode());
            if (!flag.empty())
            {
                result_text += L" ";
                result_text += flag;
            }
        }
        result_text.append("  ");
        if (kart->isEliminated())
        {
            continue;
        }
        else
        {
            result_text.append(
                StringUtils::toWString(ctf->getKartScore(kart_id)));
        }
        rect = font->getDimension(result_text.c_str());
        current_y += rect.Height;

        if (current_y > height) break;

        pos = core::rect<s32>(current_x, current_y, current_x, current_y);
        font->draw(result_text, pos,
            kart->getController()->isLocalPlayerController() ?
            red_color : color, true, false);
        kart_icon = kart->getKartProperties()->getIconMaterial()->getTexture();
        source_rect = core::recti(core::vector2di(0, 0), kart_icon->getSize());
        irr::u32 offset_x = (irr::u32)
            (font->getDimension(result_text.c_str()).Width / 1.5f);
        dest_rect = core::recti(current_x - offset_x - m_width_icon, current_y,
            current_x - offset_x, current_y + m_width_icon);
        draw2DImage(kart_icon, dest_rect, source_rect, NULL, NULL, true);
    }
#endif
}
//-----------------------------------------------------------------------------
void RaceResultGUI::unload()
{
    cleanupGPProgress();
    Screen::unload();
}

//-----------------------------------------------------------------------------
    void RaceResultGUI::onConfirm()
    {
        //RaceManager::get()->saveGP(); // Save the aborted GP
        GUIEngine::ModalDialog::dismiss();
        cleanupGPProgress();
        StateManager::get()->popMenu();
        RaceManager::get()->exitRace();
        RaceManager::get()->setAIKartOverride("");
        StateManager::get()->resetAndGoToScreen(
            MainMenuScreen::getInstance());

        if (RaceManager::get()->raceWasStartedFromOverworld())
        {
            OverWorld::enterOverWorld();
        }
    }

    //-----------------------------------------------------------------------------
    /** This determines the layout, i.e. the size of all columns, font size etc.
     */
    void RaceResultGUI::determineTableLayout()
    {
        GUIEngine::Widget *table_area = getWidget("result-table");

        m_font = GUIEngine::getFont();
        assert(m_font);
        //m_was_monospace = m_font->getMonospaceDigits();
        //m_font->setMonospaceDigits(true);
        WorldWithRank *rank_world = (WorldWithRank*)World::getWorld();

        unsigned int first_position = 1;
        unsigned int sta = RaceManager::get()->getNumSpareTireKarts();
        if (RaceManager::get()->isFollowMode())
            first_position = 2;

        // Use only the karts that are supposed to be displayed (and
        // ignore e.g. the leader in a FTL race).
        unsigned int num_karts = RaceManager::get()->getNumberOfKarts() - first_position + 1 - sta;

        // Remove previous entries to avoid reserved kart in network being displayed
        m_all_row_infos.clear();
        // In FTL races the leader kart is not displayed
        m_all_row_infos.resize(num_karts);

        // Determine the kart to display in the right order,
        // and the maximum width for the kart name column
        // -------------------------------------------------
        m_width_kart_name = 0;
        float max_finish_time = 0;

        FreeForAll* ffa = dynamic_cast<FreeForAll*>(World::getWorld());

        int time_precision = RaceManager::get()->currentModeTimePrecision();
        bool active_gp = (RaceManager::get()->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX);

        auto cl = LobbyProtocol::get<ClientLobby>();
        for (unsigned int position = first_position;
        position <= RaceManager::get()->getNumberOfKarts() - sta; position++)
        {
            const AbstractKart *kart = rank_world->getKartAtPosition(position);

            if (ffa && kart->isEliminated())
                continue;
            // Save a pointer to the current row_info entry
            RowInfo *ri = &(m_all_row_infos[position - first_position]);
            ri->m_kart_id = kart->getWorldKartId();
            ri->m_is_player_kart = kart->getController()->isLocalPlayerController();
            ri->m_kart_name = kart->getController()->getName();
            if (RaceManager::get()->getKartGlobalPlayerId(kart->getWorldKartId()) > -1)
            {
                const core::stringw& flag = StringUtils::getCountryFlag(
                    RaceManager::get()->getKartInfo(kart->getWorldKartId()).getCountryCode());
                if (!flag.empty())
                {
                    ri->m_kart_name += L" ";
                    ri->m_kart_name += flag;
                }
            }
            video::ITexture *icon =
                kart->getKartProperties()->getIconMaterial()->getTexture();
            ri->m_kart_icon = icon;
            ri->m_kart_color = RaceManager::get()->getKartColor(kart->getWorldKartId());

            // FTL karts will get a time assigned, they are not shown as eliminated
            if (kart->isEliminated() && RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_3_STRIKES)
            {
                ri->m_finish_time_string = core::stringw(_("Eliminated after %s",
                    StringUtils::toWString(StringUtils::timeToString(kart->getFinishTime(), time_precision))));
            }
            else if (kart->isEliminated() && !(RaceManager::get()->isFollowMode()))
            {
                ri->m_finish_time_string = core::stringw(_("Eliminated"));
            }
            else if (   RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_FREE_FOR_ALL
                     || RaceManager::get()->isCTFMode())
            {
                assert(ffa);
                ri->m_finish_time_string =
                    StringUtils::toWString(ffa->getKartScore(kart->getWorldKartId()));
            }
            else
            {
                const float time = kart->getFinishTime();
                if (time > max_finish_time) max_finish_time = time;
                std::string time_string = StringUtils::timeToString(time, time_precision);
                ri->m_finish_time_string = time_string.c_str();
            }
            if (cl && !cl->getRankingChanges().empty())
            {
                unsigned kart_id = kart->getWorldKartId();
                if (kart_id < cl->getRankingChanges().size())
                {
                    ri->m_finish_time_string += L" ";
                    float ranking_change = cl->getRankingChanges()[kart_id];
                    if (ranking_change > 0)
                    {
                        ri->m_finish_time_string += L"+";
                        ri->m_finish_time_string += StringUtils::toWString(ranking_change);
                    }
                    else
                        ri->m_finish_time_string += StringUtils::toWString(ranking_change);
                }
            }
            ri->m_laps = World::getWorld()->raceHasLaps() ?
                World::getWorld()->getFinishedLapsOfKart(ri->m_kart_id) : 0;

            core::dimension2du rect =
                m_font->getDimension(ri->m_kart_name.c_str());
            if (rect.Width > m_width_kart_name)
                m_width_kart_name = rect.Width;
        }   // for position

        std::string max_time = StringUtils::timeToString(max_finish_time, time_precision, true, /*display hours*/ active_gp);
        core::stringw string_max_time(max_time.c_str());
        core::dimension2du r = m_font->getDimension(string_max_time.c_str());
        m_width_finish_time = r.Width;

        // Top pixel where to display text
        m_top = table_area->m_y;

        // Height of the result display
        unsigned int height = table_area->m_h;

        // Setup different timing information for the different phases
        // -----------------------------------------------------------
        // How much time between consecutive rows
        m_time_between_rows = 0.1f;

        // How long it takes for one line to scroll from right to left
        m_time_single_scroll = 0.2f;

        // Time to rotate the entries to the proper GP position.
        m_time_rotation = 1.0f;

        // The time the first phase is being displayed: add the start time
        // of the last kart to the duration of the scroll plus some time
        // of rest before the next phase starts
        m_time_overall_scroll = (num_karts - 1)*m_time_between_rows
            + m_time_single_scroll + 2.0f;

        // The time to increase the number of points.
        m_time_for_points = 1.0f;

        // Determine text height
        r = m_font->getDimension(L"Y");
        m_distance_between_rows = (int)(1.5f*r.Height);
        m_distance_between_meta_rows = m_distance_between_rows;

        // If there are too many highscores, reduce size between rows
        Highscores* scores = World::getWorld()->getHighscores();
        if (scores != NULL &&
            scores->getNumberEntries() * m_distance_between_meta_rows > height * 0.5f)
            m_distance_between_meta_rows *= 0.8f;

        // If there are too many karts, reduce size between rows
        if (m_distance_between_rows * num_karts > height)
            m_distance_between_rows = height / num_karts;

        m_width_icon = std::min((int)(table_area->m_h / num_karts),
                                   GUIEngine::getFontHeight());

        m_width_column_space = 10;

        // Determine width of new points column

        //m_font->setMonospaceDigits(true);
        core::dimension2du r_new_p = m_font->getDimension(L"+99");

        m_width_new_points = r_new_p.Width;

        // Determine width of overall points column
        core::dimension2du r_all_p = m_font->getDimension(L"999");
        //m_font->setMonospaceDigits(false);

        m_width_all_points = r_all_p.Width;

        m_table_width = m_width_icon + m_width_column_space
            + m_width_kart_name;

        if (!RaceManager::get()->isFollowMode())
            m_table_width += m_width_finish_time + m_width_column_space;

        // Only in GP mode are the points displayed.
        if (active_gp)
            m_table_width += m_width_new_points + m_width_all_points
            + 2 * m_width_column_space;

        m_leftmost_column = table_area->m_x;
    }   // determineTableLayout

    //-----------------------------------------------------------------------------
    /** This function is called when one of the player presses 'fire'. The next
     *  phase of the animation will be displayed. E.g.
     *  in a GP: pressing fire while/after showing the latest race result will
     *           start the animation for the current GP result
     *  in a normal race: when pressing fire while an animation is played,
     *           start the menu showing 'rerun, new race, back to main' etc.
     */
    void RaceResultGUI::nextPhase()
    {
        // This will trigger the next phase in the next render call.
        m_timer = 9999;
    }   // nextPhase

    //-----------------------------------------------------------------------------
    /** If escape is pressed, don't do the default option (close the screen), but
     *  advance to the next animation phase.
     */
    bool RaceResultGUI::onEscapePressed()
    {
        nextPhase();
        return false;   // indicates 'do not close'
    }   // onEscapePressed

    //-----------------------------------------------------------------------------
    /** This is called before an event is sent to a widget. Since in this case
     *  no widget is active, the event would be lost, so we act on fire events
     *  here and trigger the next phase.
     */
    GUIEngine::EventPropagation RaceResultGUI::filterActions(PlayerAction action,
        int deviceID,
        const unsigned int value,
        Input::InputType type,
        int playerId)
    {
        if (action != PA_FIRE) return GUIEngine::EVENT_LET;

        // If the buttons are already visible, let the event go through since
        // it will be triggering eventCallback where this is handles.

        if (m_animation_state == RR_WAIT_TILL_END) return GUIEngine::EVENT_LET;

        nextPhase();
        return GUIEngine::EVENT_BLOCK;
    }   // filterActions

    //-----------------------------------------------------------------------------
    /** Called once a frame */
    void RaceResultGUI::onUpdate(float dt)
    {
        // When the finish sound has been played, start the race over music.
        if (!m_started_race_over_music &&
            m_finish_sound && m_finish_sound->getStatus() != SFXBase::SFX_PLAYING)
        {
            try
            {
                // This call is done once each frame, but startMusic() is cheap
                // if the music is already playing.
                music_manager->startMusic(m_race_over_music);
                m_started_race_over_music = true;
            }
            catch (std::exception& e)
            {
                Log::error("RaceResultGUI", "Exception caught when "
                    "trying to load music: %s", e.what());
            }
        }
    }   // onUpdate

    //-----------------------------------------------------------------------------
    /** Called once a frame, this now triggers the rendering of the actual
     *  race result gui.
     */
    void RaceResultGUI::onDraw(float dt)
    {
        renderGlobal(dt);
    }   // onDraw


    //-----------------------------------------------------------------------------
    /** Render all global parts of the race gui, i.e. things that are only
     *  displayed once even in splitscreen.
     *  \param dt Timestep sized.
     */
    void RaceResultGUI::renderGlobal(float dt)
    {
#ifndef SERVER_ONLY
        m_timer += dt;
        assert(World::getWorld()->getPhase() == WorldStatus::RESULT_DISPLAY_PHASE);
        unsigned int num_karts = (unsigned int)m_all_row_infos.size();
        float time_overall_scroll = m_time_overall_scroll;

        // First: Update the finite state machine
        // ======================================
        switch (m_animation_state)
        {
        case RR_INIT:
            for (unsigned int i = 0; i < num_karts; i++)
            {
                RowInfo *ri = &(m_all_row_infos[i]);
                ri->m_start_at = m_time_between_rows * i;
                ri->m_x_pos = (float)UserConfigParams::m_width;
                ri->m_y_pos = (float)(m_top + i*m_distance_between_rows);
            }
            m_animation_state = RR_RACE_RESULT;
            break;
        case RR_RACE_RESULT:
            // GP mode has a continue button so no extra time is needed
            if (RaceManager::get()->getMajorMode() ==
                RaceManager::MAJOR_MODE_GRAND_PRIX)
                time_overall_scroll -= 2.0f;
            if (m_timer > time_overall_scroll)
            {
                // Make sure that all lines are aligned to the left
                // (in case that the animation was skipped).
                for (unsigned int i = 0; i < num_karts; i++)
                {
                    RowInfo *ri = &(m_all_row_infos[i]);
                    ri->m_x_pos = (float)m_leftmost_column;
                }
                if (RaceManager::get()->getMajorMode() !=
                    RaceManager::MAJOR_MODE_GRAND_PRIX)
                {
                    m_animation_state = RR_WAIT_TILL_END;
                    enableAllButtons();
                    break;
                }

                m_animation_state = RR_WAITING_GP_RESULT;
                std::vector<RowInfo> prev_infos = m_all_row_infos;
                determineGPLayout();
                m_all_row_info_waiting = m_all_row_infos;
                m_all_row_infos = prev_infos;
                GUIEngine::IconButtonWidget *middle = getWidget<GUIEngine::IconButtonWidget>("middle");
                GUIEngine::RibbonWidget *operations = getWidget<GUIEngine::RibbonWidget>("operations");
                operations->setActive(true);
                operations->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
                middle->setLabel(_("Continue"));
                middle->setImage("gui/icons/green_check.png");
                middle->setVisible(true);
                operations->select("middle", PLAYER_ID_GAME_MASTER);
            }
            break;
        case RR_WAITING_GP_RESULT:
            break;
        case RR_OLD_GP_RESULTS:
            if (m_timer > m_time_overall_scroll)
            {
                m_animation_state = RR_INCREASE_POINTS;
                m_timer = 0;
                for (unsigned int i = 0; i < num_karts; i++)
                {
                    RowInfo *ri = &(m_all_row_infos[i]);
                    ri->m_x_pos = (float)m_leftmost_column;
                }
            }
            break;
        case RR_INCREASE_POINTS:
            // Have one second delay before the resorting starts.
            if (m_timer > 1 + m_time_for_points)
            {
                m_animation_state = RR_RESORT_TABLE;
                if (m_gp_position_was_changed)
                    m_timer = 0;
                else
                    // This causes the phase to go to RESORT_TABLE once, and then
                    // immediately wait till end. This has the advantage that any
                    // phase change settings will be processed properly.
                    m_timer = m_time_rotation + 1;
                // Make the new row permanent; necessary in case
                // that the animation is skipped.
                for (unsigned int i = 0; i < num_karts; i++)
                {
                    RowInfo *ri = &(m_all_row_infos[i]);
                    ri->m_new_points = 0;
                    ri->m_current_displayed_points =
                        (float)ri->m_new_overall_points;
                }

            }
            break;
        case RR_RESORT_TABLE:
            if (m_timer > m_time_rotation)
            {
                m_animation_state = RR_WAIT_TILL_END;
                // Make the new row permanent.
                for (unsigned int i = 0; i < num_karts; i++)
                {
                    RowInfo *ri = &(m_all_row_infos[i]);
                    ri->m_y_pos = ri->m_centre_point - ri->m_radius;
                }
                enableAllButtons();
            }
            break;
        case RR_WAIT_TILL_END:
            if (RaceManager::get()->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
                displayGPProgress();
            if (m_timer - m_time_rotation > 1.0f &&
                dynamic_cast<DemoWorld*>(World::getWorld()))
            {
                RaceManager::get()->exitRace();
                StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
            }
            break;
        }   // switch

        // Second phase: update X and Y positions for the various animations
        // =================================================================
        float v = 0.9f*UserConfigParams::m_width / m_time_single_scroll;
        if (RaceManager::get()->isSoccerMode())
        {
            displaySoccerResults();
        }
        else if (RaceManager::get()->isCTFMode())
        {
            displayCTFResults();
        }
        else
        {
            for (unsigned int i = 0; i < m_all_row_infos.size(); i++)
            {
                RowInfo *ri = &(m_all_row_infos[i]);
                float x = ri->m_x_pos;
                float y = ri->m_y_pos;
                switch (m_animation_state)
                {
                    // Both states use the same scrolling:
                case RR_INIT: break;   // Remove compiler warning
                case RR_RACE_RESULT:
                case RR_WAITING_GP_RESULT:
                case RR_OLD_GP_RESULTS:
                    if (m_timer > ri->m_start_at)
                    {   // if active
                        ri->m_x_pos -= dt*v;
                        if (ri->m_x_pos < m_leftmost_column)
                            ri->m_x_pos = (float)m_leftmost_column;
                        x = ri->m_x_pos;
                    }
                    break;
                case RR_INCREASE_POINTS:
                {
                    WorldWithRank *wwr = dynamic_cast<WorldWithRank*>(World::getWorld());
                    assert(wwr);
                    int most_points;
                    if (RaceManager::get()->isFollowMode())
                        most_points = wwr->getScoreForPosition(2);
                    else
                        most_points = wwr->getScoreForPosition(1);
                    ri->m_current_displayed_points +=
                        dt*most_points / m_time_for_points;
                    if (ri->m_current_displayed_points > ri->m_new_overall_points)
                    {
                        ri->m_current_displayed_points =
                            (float)ri->m_new_overall_points;
                    }
                    ri->m_new_points -=
                        dt*most_points / m_time_for_points;
                    if (ri->m_new_points < 0)
                        ri->m_new_points = 0;
                    break;
                }
                case RR_RESORT_TABLE:
                    x = ri->m_x_pos
                        - ri->m_radius*sinf(m_timer / m_time_rotation*M_PI);
                    y = ri->m_centre_point
                        + ri->m_radius*cosf(m_timer / m_time_rotation*M_PI);
                    break;
                case RR_WAIT_TILL_END:
                    break;
                }   // switch
                displayOneEntry((unsigned int)x, (unsigned int)y, i, true);
            }   // for i
        }

        // Display highscores
        if (RaceManager::get()->getMajorMode() != RaceManager::MAJOR_MODE_GRAND_PRIX ||
            m_animation_state == RR_WAITING_GP_RESULT)
        {
            displayPostRaceInfo();
        }
#endif
    }   // renderGlobal

    //-----------------------------------------------------------------------------
    /** Determine the layout and fields for the GP table based on the previous
     *  GP results.
     */
    void RaceResultGUI::determineGPLayout()
    {
#ifndef SERVER_ONLY
        unsigned int num_karts = RaceManager::get()->getNumberOfKarts();
        std::vector<int> old_rank(num_karts, 0);

        int time_precision = RaceManager::get()->currentModeTimePrecision();

        float max_time = 0;
        /* Compute highest overall time to know if hours should be displayed */
        for (unsigned int kart_id = 0; kart_id < num_karts; kart_id++)
        {
            max_time = std::max(RaceManager::get()->getOverallTime(kart_id), max_time);
        }

        for (unsigned int kart_id = 0; kart_id < num_karts; kart_id++)
        {
            int rank = RaceManager::get()->getKartGPRank(kart_id);
            // In case of FTL mode: ignore the leader
            if (rank < 0) continue;
            old_rank[kart_id] = rank;
            const AbstractKart *kart = World::getWorld()->getKart(kart_id);
            RowInfo *ri = &(m_all_row_infos[rank]);
            ri->m_kart_icon =
                kart->getKartProperties()->getIconMaterial()->getTexture();
            ri->m_is_player_kart = kart->getController()->isLocalPlayerController();
            ri->m_kart_name = kart->getController()->getName();
            if (RaceManager::get()->getKartGlobalPlayerId(kart->getWorldKartId()) > -1)
            {
                const core::stringw& flag = StringUtils::getCountryFlag(
                    RaceManager::get()->getKartInfo(kart->getWorldKartId()).getCountryCode());
                if (!flag.empty())
                {
                    ri->m_kart_name += L" ";
                    ri->m_kart_name += flag;
                }
            }
            ri->m_kart_color = RaceManager::get()->getKartColor(kart_id);
            // In FTL karts do have a time, which is shown even when the kart
            // is eliminated
            if (kart->isEliminated() && !(RaceManager::get()->isFollowMode()))
            {
                ri->m_finish_time_string = core::stringw(_("Eliminated"));
            }
            else
            {
                float time = RaceManager::get()->getOverallTime(kart_id);
                ri->m_finish_time_string
                    = StringUtils::timeToString(time, time_precision, true, /*display hours*/ (max_time > 3599.99f)).c_str();
            }
            ri->m_start_at = m_time_between_rows * rank;
            ri->m_x_pos = (float)UserConfigParams::m_width;
            ri->m_y_pos = (float)(m_top + rank*m_distance_between_rows);
            int p = RaceManager::get()->getKartPrevScore(kart_id);
            ri->m_current_displayed_points = (float)p;
            if (kart->isEliminated() && !(RaceManager::get()->isFollowMode()))
            {
                ri->m_new_points = 0;
            }
            else
            {
                WorldWithRank *wwr = dynamic_cast<WorldWithRank*>(World::getWorld());
                assert(wwr);
                ri->m_new_points =
                    (float)wwr->getScoreForPosition(kart->getPosition());
            }
        }

        // Now update the GP ranks, and determine the new position
        // -------------------------------------------------------
        RaceManager::get()->computeGPRanks();
        m_gp_position_was_changed = false;
        for (unsigned int i = 0; i < num_karts; i++)
        {
            int j = old_rank[i];
            int gp_position = RaceManager::get()->getKartGPRank(i);
            m_gp_position_was_changed |= j != gp_position;
            RowInfo *ri = &(m_all_row_infos[j]);
            ri->m_radius = (j - gp_position)*(int)m_distance_between_rows*0.5f;
            ri->m_centre_point = m_top + (gp_position + j)*m_distance_between_rows*0.5f;
            int p = RaceManager::get()->getKartScore(i);
            ri->m_new_overall_points = p;
            ri->m_new_gp_rank = gp_position;
            ri->m_laps = World::getWorld()->getFinishedLapsOfKart(i);
        }   // i < num_karts
#endif
    }   // determineGPLayout

    //-----------------------------------------------------------------------------
    /** Displays the race results for a single kart.
     *  \param n Index of the kart to be displayed.
     *  \param display_points True if GP points should be displayed, too
     */
    void RaceResultGUI::displayOneEntry(unsigned int x, unsigned int y,
        unsigned int n, bool display_points)
    {
#ifndef SERVER_ONLY
        RowInfo *ri = &(m_all_row_infos[n]);
        video::SColor color = ri->m_is_player_kart
            ? video::SColor(255, 255, 0, 0)
            : video::SColor(255, 255, 255, 255);

        unsigned int current_x = x;

        // Draw rank order
        // (only when num. of karts >=10 )
        if (RaceManager::get()->getMinorMode() != RaceManager::MINOR_MODE_FREE_FOR_ALL &&
            !ri->m_finish_time_string.empty() &&
            RaceManager::get()->getNumberOfKarts() >= 10)
        {
            int rankNo = (
                RaceManager::get()->getMajorMode()==RaceManager::MAJOR_MODE_GRAND_PRIX &&
                m_animation_state >= RR_RESORT_TABLE
                    ? ri->m_new_gp_rank
                    : n
            ) + 1;

            int pos_rank_width = m_font->getDimension(core::stringw(rankNo).c_str()).Width;
            core::recti pos_rank(current_x, y, pos_rank_width, m_distance_between_rows);
            m_font->draw(core::stringw(rankNo), pos_rank, color);
            current_x += 48;
        }

        // Draw kart color circle if kart has custom color
        if (m_icons_frame && ri->m_kart_color > 0.0)
        {
            const video::SColorHSL kart_colorHSL(ri->m_kart_color * 360.0, 80.0, 50.0);
            video::SColorf kart_colorf;
            kart_colorHSL.toRGB(kart_colorf);
            const video::SColor kart_color = kart_colorf.toSColor();
            const video::SColor colors[4] = {kart_color, kart_color, kart_color, kart_color};
            const core::recti source_rect(core::vector2di(0, 0), m_icons_frame->getSize());
            // make frame bigger than icon to make color visible for all cases
            const int extra_width = std::max((unsigned int)5, m_width_icon / 8);
            core::recti dest_rect(current_x - extra_width, y - extra_width,
                current_x + m_width_icon + extra_width, y + m_width_icon + extra_width);
            draw2DImage(m_icons_frame, dest_rect, source_rect, NULL, colors, true);
        }
        // First draw the icon
        // -------------------
        if (ri->m_kart_icon)
        {
            core::recti source_rect(core::vector2di(0, 0),
                ri->m_kart_icon->getSize());
            core::recti dest_rect(current_x, y,
                current_x + m_width_icon, y + m_width_icon);
            draw2DImage(ri->m_kart_icon, dest_rect,
                source_rect, NULL, NULL,
                true);
        }

        current_x += m_width_icon + m_width_column_space;

        // Draw the name
        // -------------

        core::recti pos_name(current_x, y,
            current_x + m_width_kart_name, y + m_distance_between_rows);
        m_font->draw(ri->m_kart_name, pos_name, color, false, false, NULL,
            true /* ignoreRTL */);
        current_x += m_width_kart_name + m_width_column_space;

        if (RaceManager::get()->isLapTrialMode())
        {
            core::recti pos_laps = core::recti(current_x, y, current_x + 100, y + 10);
            m_font->draw(irr::core::stringw(ri->m_laps), pos_laps, color, false, false,
                NULL, true /* ignoreRTL */);
        }
        else
        {
            core::recti dest_rect = core::recti(current_x, y, current_x + 100, y + 10);
            m_font->draw(ri->m_finish_time_string, dest_rect, color, false, false,
                NULL, true /* ignoreRTL */);
            current_x += m_width_finish_time + m_width_column_space;
        }
        

        current_x += 100 + m_width_column_space;

        // Only display points in GP mode and when the GP results are displayed.
        // =====================================================================
        if (RaceManager::get()->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX &&
            m_animation_state != RR_RACE_RESULT &&
            m_animation_state != RR_WAITING_GP_RESULT)
        {
            // Draw the new points
            // -------------------
            if (ri->m_new_points > 0)
            {
                core::recti dest_rect = core::recti(current_x, y,
                    current_x + 100, y + 10);
                core::stringw point_string = core::stringw("+")
                    + core::stringw((int)ri->m_new_points);
                // With mono-space digits space has the same width as each digit,
                // so we can simply fill up the string with spaces to get the
                // right aligned.
                while (point_string.size() < 3)
                    point_string = core::stringw(" ") + point_string;
                m_font->draw(point_string, dest_rect, color, false, false, NULL,
                    true /* ignoreRTL */);
            }
            current_x += m_width_new_points + m_width_column_space;

            // Draw the old_points plus increase value
            // ---------------------------------------
            core::recti dest_rect = core::recti(current_x, y, current_x + 100, y + 10);
            core::stringw point_inc_string =
                core::stringw((int)(ri->m_current_displayed_points));
            while (point_inc_string.size() < 3)
                point_inc_string = core::stringw(" ") + point_inc_string;
            m_font->draw(point_inc_string, dest_rect, color, false, false, NULL,
                true /* ignoreRTL */);
        }
#endif
    }   // displayOneEntry

    //-----------------------------------------------------------------------------
    void RaceResultGUI::displaySoccerResults()
    {
#ifndef SERVER_ONLY
        //Draw win text
        core::stringw result_text;
        static video::SColor color = video::SColor(255, 255, 255, 255);
        gui::IGUIFont* font = GUIEngine::getTitleFont();
        int team_icon_height = font->getDimension(L"A").Height * 1.5f; // the size of team icon
        int current_x = UserConfigParams::m_width / 2;
        RowInfo *ri = &(m_all_row_infos[0]);
        int current_y = (int)ri->m_y_pos;
        SoccerWorld* sw = (SoccerWorld*)World::getWorld();
        const int red_score = sw->getScore(KART_TEAM_RED);
        const int blue_score = sw->getScore(KART_TEAM_BLUE);

        GUIEngine::Widget *table_area = getWidget("result-table");
        int height = table_area->m_h + table_area->m_y;

        if (red_score > blue_score)
        {
            result_text = _("Red Team Wins");
        }
        else if (blue_score > red_score)
        {
            result_text = _("Blue Team Wins");
        }
        else
        {
            //Cannot really happen now. Only in time limited matches.
            result_text = _("It's a draw");
        }
        core::rect<s32> pos(current_x, current_y, current_x, current_y);
        font->draw(result_text.c_str(), pos, color, true, true);

        core::dimension2du rect = font->getDimension(result_text.c_str());

        //Draw team scores:
        current_y += rect.Height;
        current_x /= 2;
        irr::video::ITexture* red_icon = irr_driver->getTexture(FileManager::GUI_ICON,
            "soccer_ball_red.png");
        irr::video::ITexture* blue_icon = irr_driver->getTexture(FileManager::GUI_ICON,
            "soccer_ball_blue.png");

        int team_icon_width = team_icon_height * (red_icon->getSize().Width / red_icon->getSize().Height);
        core::recti source_rect(core::vector2di(0, 0), red_icon->getSize());
        current_x -= team_icon_width/2;
        core::recti dest_rect(current_x, current_y, current_x + team_icon_width,
            current_y + team_icon_height);
        draw2DImage(red_icon, dest_rect, source_rect,
            NULL, NULL, true);
        current_x += UserConfigParams::m_width / 2;
        dest_rect = core::recti(current_x, current_y, current_x + team_icon_width,
            current_y + team_icon_height);
        draw2DImage(blue_icon, dest_rect, source_rect,
            NULL, NULL, true);

        result_text = StringUtils::toWString(blue_score);
        rect = font->getDimension(result_text.c_str());
        current_x += team_icon_height / 2;
        current_y += team_icon_height + rect.Height / 4;
        pos = core::rect<s32>(current_x, current_y, current_x, current_y);
        font->draw(result_text.c_str(), pos, color, true, false);

        current_x -= UserConfigParams::m_width / 2;
        result_text = StringUtils::toWString(red_score);
        pos = core::rect<s32>(current_x, current_y, current_x, current_y);
        font->draw(result_text.c_str(), pos, color, true, false);

        int center_x = UserConfigParams::m_width / 2;
        pos = core::rect<s32>(center_x, current_y, center_x, current_y);
        font->draw("-", pos, color, true, false);

        //Draw goal scorers:
        //The red scorers:
        current_y += rect.Height / 2 + rect.Height / 4;
        font = GUIEngine::getSmallFont();
        std::vector<SoccerWorld::ScorerData> scorers = sw->getScorers(KART_TEAM_RED);

        // Maximum 10 scorers displayed in result screen
        while (scorers.size() > 10)
        {
            scorers.erase(scorers.begin());
        }

        int prev_y = current_y;

        for (unsigned int i = 0; i < scorers.size(); i++)
        {
            const bool own_goal = !(scorers.at(i).m_correct_goal);

            result_text = scorers.at(i).m_player;
            if (scorers.at(i).m_handicap_level == HANDICAP_MEDIUM)
                result_text = _("%s (handicapped)", result_text);

            if (own_goal)
            {
                result_text.append(" ");
                //I18N: indicates a player that scored in their own goal in result screen
                result_text.append(_("(Own Goal)"));
            }
            if (!scorers.at(i).m_country_code.empty())
            {
                result_text += " ";
                result_text += StringUtils::getCountryFlag(scorers.at(i).m_country_code);
            }

            result_text.append("  ");
            result_text.append(StringUtils::timeToString(scorers.at(i).m_time).c_str());
            rect = font->getDimension(result_text.c_str());

            if (height - prev_y < ((short)scorers.size() + 1)*(short)rect.Height)
                current_y += (height - prev_y) / ((short)scorers.size() + 1);
            else
                current_y += rect.Height;

            if (current_y > height) break;

            pos = core::rect<s32>(current_x, current_y, current_x, current_y);
            font->draw(result_text, pos, (own_goal ?
                video::SColor(255, 255, 0, 0) : color), true, false);
            irr::video::ITexture* scorer_icon = NULL;
            const KartProperties* kp = kart_properties_manager->getKart(scorers.at(i).m_kart);
            // For addon kart online
            if (!kp)
                kp = kart_properties_manager->getKart("tux");
            if (kp)
                scorer_icon = kp->getIconMaterial()->getTexture();
            if (scorer_icon)
            {
                source_rect = core::recti(core::vector2di(0, 0), scorer_icon->getSize());
                irr::u32 offset_x = (irr::u32)(font->getDimension(result_text.c_str()).Width / 1.5f);
                core::recti r = core::recti(current_x - offset_x - m_width_icon, current_y,
                    current_x - offset_x, current_y + m_width_icon);
                draw2DImage(scorer_icon, r, source_rect,
                    NULL, NULL, true);
            }
        }

        //The blue scorers:
        current_y = prev_y;
        current_x += UserConfigParams::m_width / 2;
        scorers = sw->getScorers(KART_TEAM_BLUE);

        while (scorers.size() > 10)
        {
            scorers.erase(scorers.begin());
        }

        for (unsigned int i = 0; i < scorers.size(); i++)
        {
            const bool own_goal = !(scorers.at(i).m_correct_goal);

            result_text = scorers.at(i).m_player;
            if (scorers.at(i).m_handicap_level == HANDICAP_MEDIUM)
                result_text = _("%s (handicapped)", result_text);

            if (own_goal)
            {
                result_text.append(" ");
                //I18N: indicates a player that scored in their own goal in result screen
                result_text.append(_("(Own Goal)"));
            }
            if (!scorers.at(i).m_country_code.empty())
            {
                result_text += " ";
                result_text += StringUtils::getCountryFlag(scorers.at(i).m_country_code);
            }

            result_text.append("  ");
            result_text.append(StringUtils::timeToString(scorers.at(i).m_time).c_str());
            rect = font->getDimension(result_text.c_str());

            if (height - prev_y < ((short)scorers.size() + 1)*(short)rect.Height)
                current_y += (height - prev_y) / ((short)scorers.size() + 1);
            else
                current_y += rect.Height;

            if (current_y > height) break;

            pos = core::rect<s32>(current_x, current_y, current_x, current_y);
            font->draw(result_text, pos, (own_goal ?
                video::SColor(255, 255, 0, 0) : color), true, false);
            irr::video::ITexture* scorer_icon = NULL;
            const KartProperties* kp = kart_properties_manager->getKart(scorers.at(i).m_kart);
            // For addon kart online
            if (!kp)
                kp = kart_properties_manager->getKart("tux");
            if (kp)
                scorer_icon = kp->getIconMaterial()->getTexture();
            if (scorer_icon)
            {
                source_rect = core::recti(core::vector2di(0, 0), scorer_icon->getSize());
                irr::u32 offset_x = (irr::u32)(font->getDimension(result_text.c_str()).Width / 1.5f);
                core::recti r = core::recti(current_x - offset_x - m_width_icon, current_y,
                    current_x - offset_x, current_y + m_width_icon);
                draw2DImage(scorer_icon, r, source_rect,
                    NULL, NULL, true);
            }
        }
#endif
    }

    //-----------------------------------------------------------------------------

    void RaceResultGUI::clearHighscores()
    {
        m_highscore_rank = 0;
    }   // clearHighscores

    //-----------------------------------------------------------------------------

    void RaceResultGUI::setHighscore(int rank)
    {
        m_highscore_rank = rank;
    }   // setHighscore

    // ----------------------------------------------------------------------------
    void RaceResultGUI::enableGPProgress()
    {
        if (RaceManager::get()->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
        {
            GUIEngine::Widget* result_table = getWidget("result-table");
            assert(result_table != NULL);

            int currentTrack = RaceManager::get()->getTrackNumber();
            int font_height = getFontHeight();
            int w = (int)(UserConfigParams::m_width*0.17);
            int x = (int)(result_table->m_x + result_table->m_w - w - 15);
            int y = (m_top + font_height + 5);

            //Current progress
            GUIEngine::LabelWidget* status_label = new GUIEngine::LabelWidget();
            status_label->m_properties[GUIEngine::PROP_ID] = "status_label";
            status_label->m_properties[GUIEngine::PROP_TEXT_ALIGN] = "center";
            status_label->m_x = x;
            status_label->m_y = y;
            status_label->m_w = w;
            status_label->m_h = font_height;
            status_label->add();
            status_label->setText(_("Track %i/%i", currentTrack + 1,
                RaceManager::get()->getGrandPrix().getNumberOfTracks()), true);
            addGPProgressWidget(status_label);
            y = (status_label->m_y + status_label->m_h + 5);

            //Scroll up button
            GUIEngine::IconButtonWidget* up_button = new GUIEngine::IconButtonWidget(
                GUIEngine::IconButtonWidget::SCALE_MODE_KEEP_CUSTOM_ASPECT_RATIO,
                false, false, GUIEngine::IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
            up_button->m_properties[GUIEngine::PROP_ID] = "up_button";
            up_button->m_x = x;
            up_button->m_y = y;
            up_button->m_w = w;
            up_button->m_h = font_height;
            up_button->add();
            up_button->setImage(file_manager->getAsset(FileManager::GUI_ICON, "scroll_up.png"));
            addGPProgressWidget(up_button);
            y = (up_button->m_y + up_button->m_h + SSHOT_SEPARATION);

            //Track screenshots and labels
            int n_sshot = 1;
            for (int i = m_start_track; i < m_end_track; i++)
            {
                //Screenshot
                GUIEngine::IconButtonWidget* screenshot_widget =
                    new GUIEngine::IconButtonWidget(
                        GUIEngine::IconButtonWidget::
                        SCALE_MODE_KEEP_CUSTOM_ASPECT_RATIO,
                        false, false,
                        GUIEngine::IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
                screenshot_widget->setCustomAspectRatio(4.0f / 3.0f);
                screenshot_widget->m_x = x;
                screenshot_widget->m_y = y;
                screenshot_widget->m_w = w;
                screenshot_widget->m_h = m_sshot_height;
                screenshot_widget->m_properties[GUIEngine::PROP_ID] =
                    ("sshot_" + StringUtils::toString(n_sshot));
                screenshot_widget->add();
                addGPProgressWidget(screenshot_widget);

                //Label
                GUIEngine::LabelWidget* sshot_label = new GUIEngine::LabelWidget();
                sshot_label->m_properties[GUIEngine::PROP_ID] =
                    ("sshot_label_" + StringUtils::toString(n_sshot));
                sshot_label->m_properties[GUIEngine::PROP_TEXT_ALIGN] = "left";
                sshot_label->m_x = (x + w + 5);
                sshot_label->m_y = (y + (m_sshot_height / 2) - (font_height / 2));
                sshot_label->m_w = (w / 2);
                sshot_label->m_h = font_height;
                sshot_label->add();
                addGPProgressWidget(sshot_label);

                y += (m_sshot_height + SSHOT_SEPARATION);
                n_sshot++;
            }   // for
            displayScreenShots();

            //Scroll down button
            GUIEngine::IconButtonWidget* down_button = new GUIEngine::IconButtonWidget(
                GUIEngine::IconButtonWidget::SCALE_MODE_KEEP_CUSTOM_ASPECT_RATIO,
                false, false, GUIEngine::IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
            down_button->m_properties[GUIEngine::PROP_ID] = "down_button";
            down_button->m_x = x;
            down_button->m_y = y;
            down_button->m_w = w;
            down_button->m_h = font_height;
            down_button->add();
            down_button->setImage(file_manager->getAsset(FileManager::GUI_ICON, "scroll_down.png"));
            addGPProgressWidget(down_button);

        }   // if MAJOR_MODE_GRAND_PRIX)

    }   // enableGPProgress

    // ----------------------------------------------------------------------------
    void RaceResultGUI::addGPProgressWidget(GUIEngine::Widget* widget)
    {
        m_widgets.push_back(widget);
        m_gp_progress_widgets.push_back(widget);
    }

    // ----------------------------------------------------------------------------
    void RaceResultGUI::displayGPProgress()
    {
        core::stringw msg = _("Grand Prix progress:");

        GUIEngine::Widget* result_table = getWidget("result-table");
        assert(result_table != NULL);

        video::SColor color = video::SColor(255, 255, 0, 0);
        // 0.96 from stkgui
        core::recti dest_rect(
            result_table->m_x + result_table->m_w - m_font->getDimension(msg.c_str()).Width - 5,
            m_top, UserConfigParams::m_width * 0.96f,
            m_top + GUIEngine::getFontHeight());

        m_font->draw(msg, dest_rect, color, false, false, NULL, true);
    }   // displayGPProgress

    // ----------------------------------------------------------------------------
    void RaceResultGUI::cleanupGPProgress()
    {
        for (unsigned int i = 0; i < m_gp_progress_widgets.size(); i++)
            m_widgets.remove(m_gp_progress_widgets.get(i));
        m_gp_progress_widgets.clearAndDeleteAll();
    }   // cleanupGPProgress

    // ----------------------------------------------------------------------------
    void RaceResultGUI::displayPostRaceInfo()
    {
#ifndef SERVER_ONLY
        // This happens in demo world
        if (!World::getWorld())
            return;

        Highscores* scores = World::getWorld()->getHighscores();

        video::SColor white_color = video::SColor(255, 255, 255, 255);

        int x = (int)(UserConfigParams::m_width*0.65f);
        int y = m_top;

        int current_y = y;

        int time_precision = RaceManager::get()->currentModeTimePrecision();

        // In some case for exemple FTL they will be no highscores
        if (scores != NULL)
        {
            // First draw title
            GUIEngine::getFont()->draw(_("Highscores"),
                // 0.96 from stkgui
                core::recti(x, y, UserConfigParams::m_width * 0.96f, y + GUIEngine::getFontHeight()),
                white_color,
                false, false, NULL, true /* ignoreRTL */);

            std::string kart_name;
            irr::core::stringw player_name;

            // prevent excessive long name
            unsigned int max_characters = 15;
            unsigned int max_width = (UserConfigParams::m_width / 2 - 200) / 10;
            if (max_width < 15)
                max_characters = max_width;

            float time;
            for (int i = 0; i < scores->getNumberEntries(); i++)
            {
                scores->getEntry(i, kart_name, player_name, &time);
                if (player_name.size() > max_characters)
                {
                    int begin = (int(m_timer / 0.4f)) % (player_name.size() - max_characters);
                    player_name = player_name.subString(begin, max_characters, false);
                }

                video::SColor text_color = white_color;
                if (m_highscore_rank - 1 == i)
                {
                    text_color = video::SColor(255, 255, 0, 0);
                }

                int current_x = x;
                current_y = y + (int)((i + 1) * m_distance_between_meta_rows);

                const KartProperties* prop = kart_properties_manager->getKart(kart_name);
                if (prop != NULL)
                {
                    const std::string &icon_path = prop->getAbsoluteIconFile();
                    video::ITexture* kart_icon_texture = irr_driver->getTexture(icon_path);

                    if (kart_icon_texture != NULL)
                    {
                        core::recti source_rect(core::vector2di(0, 0),
                            kart_icon_texture->getSize());

                        core::recti dest_rect(current_x, current_y,
                            current_x + m_width_icon, current_y + m_width_icon);

                        draw2DImage(
                            kart_icon_texture, dest_rect,
                            source_rect, NULL, NULL,
                            true);

                        current_x += m_width_icon + m_width_column_space;
                    }
                }

                // draw the player name
                GUIEngine::getSmallFont()->draw(player_name.c_str(),
                    core::recti(current_x, current_y, current_x + 150, current_y + 10),
                    text_color,
                    false, false, NULL, true /* ignoreRTL */);

                current_x = (int)(UserConfigParams::m_width * 0.85f);

                // Finally draw the time
                std::string highscore_string;
                if (RaceManager::get()->isLapTrialMode())
                    highscore_string = std::to_string(static_cast<int>(time));
                else
                    highscore_string = StringUtils::timeToString(time, time_precision);
                GUIEngine::getSmallFont()->draw(highscore_string.c_str(),
                    core::recti(current_x, current_y, current_x + 100, current_y + 10),
                    text_color,
                    false, false, NULL, true /* ignoreRTL */);
            }
        }

        if (!RaceManager::get()->isSoccerMode())
        {
            // display lap count
            if (RaceManager::get()->modeHasLaps())
            {
                core::stringw laps = _("Laps: %i", RaceManager::get()->getNumLaps());
                current_y += int(m_distance_between_meta_rows * 0.8f * 2);
                GUIEngine::getFont()->draw(laps,
                    // 0.96 from stkgui
                    core::recti(x, current_y, UserConfigParams::m_width * 0.96f, current_y + GUIEngine::getFontHeight()),
                    white_color, false, false, nullptr, true);
            }
            // display difficulty
            core::stringw difficulty_name =
                RaceManager::get()->getDifficultyName(RaceManager::get()->getDifficulty());
            core::stringw difficulty_one;
            core::stringw difficulty_two;
            core::recti diff_ghost_one, diff_ghost_two;
            if (RaceManager::get()->hasGhostKarts() && ReplayPlay::get()->isSecondReplayEnabled())
            {
                WorldWithRank* wwr = dynamic_cast<WorldWithRank*>(World::getWorld());
                for (unsigned k = 0; k < wwr->getNumKarts(); k++)
                {
                    GhostKart* gk = dynamic_cast<GhostKart*>(wwr->getKartAtPosition(k + 1));
                    if (!gk)
                        continue;
                    const ReplayPlay::ReplayData& rd = gk->getReplayData();
                    if (difficulty_one.empty())
                    {
                        difficulty_one = RaceManager::get()->getDifficultyName((RaceManager::Difficulty)rd.m_difficulty);
                        core::dimension2d<u32> dim_1 = m_font->getDimension(difficulty_one.c_str());
                        RowInfo* ri_1 = &(m_all_row_infos[k]);
                        diff_ghost_one.UpperLeftCorner.X = ri_1->m_x_pos + m_width_icon + m_width_kart_name + m_width_finish_time + m_width_column_space + 20;
                        diff_ghost_one.UpperLeftCorner.Y = ri_1->m_y_pos;
                        diff_ghost_one.LowerRightCorner.X = diff_ghost_one.UpperLeftCorner.X + dim_1.Width;
                        diff_ghost_one.LowerRightCorner.Y = diff_ghost_one.UpperLeftCorner.Y + dim_1.Height;
                    }
                    else if (difficulty_two.empty())
                    {
                        difficulty_two = RaceManager::get()->getDifficultyName((RaceManager::Difficulty)rd.m_difficulty);
                        core::dimension2d<u32> dim_2 = m_font->getDimension(difficulty_two.c_str());
                        RowInfo* ri_2 = &(m_all_row_infos[k]);
                        diff_ghost_two.UpperLeftCorner.X = ri_2->m_x_pos + m_width_icon + m_width_kart_name + m_width_finish_time + m_width_column_space + 20;
                        diff_ghost_two.UpperLeftCorner.Y = ri_2->m_y_pos;
                        diff_ghost_two.LowerRightCorner.X = diff_ghost_two.UpperLeftCorner.X + dim_2.Width;
                        diff_ghost_two.LowerRightCorner.Y = diff_ghost_two.UpperLeftCorner.Y + dim_2.Height;
                    }
                }
                if (difficulty_one != difficulty_two)
                    difficulty_name = difficulty_one + L" / " + difficulty_two;
                else
                    difficulty_name = difficulty_one;

                // Left side difficulty display
                m_font->draw(difficulty_one, diff_ghost_one, video::SColor(255, 255, 255, 255), false, false, nullptr, true);
                m_font->draw(difficulty_two, diff_ghost_two, video::SColor(255, 255, 255, 255), false, false, nullptr, true);
            }
            core::stringw difficulty_string = _("Difficulty: %s", difficulty_name);
            current_y += int(m_distance_between_meta_rows * 0.8f);
            GUIEngine::getFont()->draw(difficulty_string,
                // 0.96 from stkgui
                core::recti(x, current_y, UserConfigParams::m_width * 0.96f, current_y + GUIEngine::getFontHeight()),
                white_color, false, false, nullptr, true);
            // show fastest lap
            if (RaceManager::get()->modeHasLaps())
            {
                float best_lap_time = static_cast<LinearWorld*>(World::getWorld())->getFastestLap();
                // The fastest lap ticks is set to INT_MAX, so the best_lap_time will be
                // very high when none has been set yet.
                if (best_lap_time <= 3600.0)
                {
                    core::stringw best_lap_string = _("Best lap time: %s",
                        StringUtils::timeToString(best_lap_time, time_precision).c_str());
                    current_y += int(m_distance_between_meta_rows * 0.8f);
                    GUIEngine::getFont()->draw(best_lap_string,
                        // 0.96 from stkgui
                        core::recti(x, current_y, UserConfigParams::m_width * 0.96f, current_y + GUIEngine::getFontHeight()),
                        white_color, false, false,
                        nullptr, true);

                    core::stringw best_lap_by = dynamic_cast<LinearWorld*>(World::getWorld())->getFastestLapKartName();

                    if (best_lap_by != "")
                    {
                        //I18N: is used to indicate who has the bast laptime (best laptime "by kart_name")
                        core::stringw best_lap_by_string = _("by %s", best_lap_by);
                        // Make it closer to the above line
                        current_y += int(GUIEngine::getFontHeight() * 0.8f);
                        GUIEngine::getFont()->draw(best_lap_by_string,
                            // 0.96 from stkgui
                            core::recti(x, current_y, UserConfigParams::m_width * 0.96f, current_y + GUIEngine::getFontHeight()),
                            white_color, false, false,
                            nullptr, true);
                    }
                }
            }   // if mode has laps
        }   // if not soccer mode

        // Display challenge result and goals
        bool is_gp = RaceManager::get()->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX;
        if(RaceManager::get()->raceWasStartedFromOverworld() &&
            (!is_gp ||
            RaceManager::get()->getTrackNumber() + 1 == RaceManager::get()->getNumOfTracks()))
        {
            current_y += int(m_distance_between_meta_rows * 0.4f);

            const ChallengeStatus* c_stat = PlayerManager::getCurrentPlayer()->getCurrentChallengeStatus();
            if (!c_stat)
                return;
            const ChallengeData* c_data = c_stat->getData();
            if (!c_data)
                return;
            RaceManager::Difficulty difficulty = RaceManager::get()->getDifficulty();
            video::SColor win_color = video::SColor(255, 0, 255, 0);
            video::SColor lose_color = video::SColor(255, 255, 0, 0);
            video::SColor special_color = video::SColor(255, 0, 255, 255);
            AbstractKart* kart = World::getWorld()->getPlayerKart(0);
            bool lose_all = false;
            bool position_passed = false;
            bool time_passed = false;
            bool energy_passed = false;

            if (is_gp)
            {
                // GP has no best while slower
                lose_all = true;
                if (c_data->isGPFulfilled())
                {
                    position_passed = true;
                    time_passed = true;
                    energy_passed = true;
                }
            }
            else
            {
                if (kart->isEliminated())
                    lose_all = true;
                position_passed = (kart->getPosition() <= c_data->getMaxPosition(difficulty) && lose_all == false)
                                || c_data->getMaxPosition(difficulty) == -1;
                time_passed = (kart->getFinishTime() <= c_data->getTimeRequirement(difficulty) && lose_all == false)
                                || c_data->getTimeRequirement(difficulty) <= 0.0f;
                energy_passed = (kart->getEnergy() >= c_data->getEnergy(difficulty) && lose_all == false)
                                || c_data->getEnergy(difficulty) <= 0;
            }

            bool all_passed = position_passed && time_passed && energy_passed;

            core::stringw text_string = all_passed ? _("You completed the challenge!") : _("You failed the challenge!");
            video::SColor text_color = all_passed ? win_color : lose_color;
            current_y += int(m_distance_between_meta_rows * 0.8f);
            GUIEngine::getFont()->draw(text_string, core::recti(x, current_y, UserConfigParams::m_width * 0.96f,
                                       y + GUIEngine::getFontHeight()), text_color, false, false, nullptr, true);

            current_y += int(m_distance_between_meta_rows * 0.2f);

            // Display goals
            if (c_data->getMaxPosition(difficulty) != -1)
            {
                int r = c_data->getMaxPosition(difficulty);
                if (c_data->getMinorMode() == RaceManager::MINOR_MODE_FOLLOW_LEADER)
                    r --;

                text_string = _("Required Rank: %i", r);
                text_color = position_passed ? win_color : lose_color;
                current_y += int(m_distance_between_meta_rows * 0.7f);
                GUIEngine::getFont()->draw(text_string, core::recti(x, current_y, UserConfigParams::m_width * 0.96f,
                                       y + GUIEngine::getSmallFontHeight()), text_color, false, false, nullptr, true);
            }
            if (c_data->getTimeRequirement(difficulty) > 0)
            {
                text_string = _("Required Time: %i",
                    StringUtils::timeToString(c_data->getTimeRequirement(difficulty)).c_str());
                text_color = time_passed ? win_color : lose_color;
                current_y += int(m_distance_between_meta_rows * 0.7f);
                GUIEngine::getFont()->draw(text_string, core::recti(x, current_y, UserConfigParams::m_width * 0.96f,
                                       y + GUIEngine::getSmallFontHeight()), text_color, false, false, nullptr, true);
            }
            if (c_data->getEnergy(difficulty) > 0)
            {
                int energy = c_data->getEnergy(difficulty);

                text_string = _("Required Nitro Points: %i", energy);
                text_color = energy_passed ? win_color : lose_color;
                current_y += int(m_distance_between_meta_rows * 0.7f);
                GUIEngine::getFont()->draw(text_string, core::recti(x, current_y, UserConfigParams::m_width * 0.96f,
                                       y + GUIEngine::getSmallFontHeight()), text_color, false, false, nullptr, true);
            }

            if (c_data->isChallengeFulfilled(true) && RaceManager::get()->getDifficulty() != RaceManager::DIFFICULTY_BEST)
            {
                text_string = _("Reached Requirements of SuperTux");
                text_color = special_color;
                current_y += int(m_distance_between_meta_rows * 0.7f);
                GUIEngine::getFont()->draw(text_string, core::recti(x, current_y, UserConfigParams::m_width * 0.96f,
                                       y + GUIEngine::getSmallFontHeight()), text_color, false, false, nullptr, true);
            }

        } // if it's a challenge
#endif
    }

    // ----------------------------------------------------------------------------
    void RaceResultGUI::displayScreenShots()
    {
        const std::vector<std::string> tracks =
            RaceManager::get()->getGrandPrix().getTrackNames();
        int currentTrack = RaceManager::get()->getTrackNumber();

        int n_sshot = 1;
        for (int i = m_start_track; i < m_end_track; i++)
        {
            Track* track = track_manager->getTrack(tracks[i]);
            GUIEngine::IconButtonWidget* sshot = getWidget<GUIEngine::IconButtonWidget>(
                ("sshot_" + StringUtils::toString(n_sshot)).c_str());
            GUIEngine::LabelWidget* label = getWidget<GUIEngine::LabelWidget>(
                ("sshot_label_" + StringUtils::toString(n_sshot)).c_str());
            assert(sshot != NULL && label != NULL);

            // Network grand prix chooses each track 1 by 1
            if (track == NULL)
            {
                sshot->setImage(file_manager->getAsset(FileManager::GUI_ICON,
                    "main_help.png"));
            }
            else
                sshot->setImage(track->getScreenshotFile());
            if (i <= currentTrack)
                sshot->setBadge(GUIEngine::OK_BADGE);
            else
                sshot->resetAllBadges();

            label->setText(StringUtils::toWString(i + 1), true);

            n_sshot++;
        }
    }

    // ----------------------------------------------------------------------------
    int RaceResultGUI::getFontHeight() const
    {
        assert(m_font != NULL);
        return m_font->getDimension(L"A").Height; //Could be any capital letter
    }
