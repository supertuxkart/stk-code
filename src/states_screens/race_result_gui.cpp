//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Joerg Henrichs
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

#include "challenges/unlock_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widget.hpp"
#include "io/file_manager.hpp"
#include "modes/world_with_rank.hpp"
#include "states_screens/dialogs/race_over_dialog.hpp"
#include "states_screens/feature_unlocked.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/race_setup_screen.hpp"
#include "utils/string_utils.hpp"

DEFINE_SCREEN_SINGLETON( RaceResultGUI );

/** Constructor, initialises internal data structures.
 */
RaceResultGUI::RaceResultGUI() : Screen("race_result.stkgui", /*pause race*/ false)
{
}   // RaceResultGUI

//-----------------------------------------------------------------------------
/** Besides calling init in the base class this makes all buttons of this 
 *  screen invisible. The buttons will only displayed once the animation is 
 *  over.
 */
void RaceResultGUI::init()
{
    Screen::init();
    determineTableLayout();
    m_animation_state = RR_INIT;

    m_timer           = 0;

    getWidget("top")->setVisible(false);
    getWidget("middle")->setVisible(false);
    getWidget("bottom")->setVisible(false);
}   // init

//-----------------------------------------------------------------------------
void RaceResultGUI::tearDown()
{
    Screen::tearDown();
    m_font->setMonospaceDigits(m_was_monospace);
}   // tearDown
    
//-----------------------------------------------------------------------------
/** Makes the correct buttons visible again, and gives them the right label.
 *  1) If something was unlocked, only a 'next' button is displayed.
 */
void RaceResultGUI::enableAllButtons()
{
    GUIEngine::Widget *top    = getWidget("top");
    GUIEngine::Widget *middle = getWidget("middle");
    GUIEngine::Widget *bottom = getWidget("bottom");

    // If something was unlocked
    // -------------------------
    int n = unlock_manager->getRecentlyUnlockedFeatures().size();
    if(n>0)
    {
        top->setText(n==1 ? _("See unlocked feature") 
                          : _("See unlocked features"));
        top->setVisible(true);
    }
    else if (race_manager->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        // In case of a GP:
        // ----------------
        top->setVisible(false);

        middle->setText( _("Continue") );
        middle->setVisible(true);

        bottom->setText( _("Abort Grand Prix") );
        bottom->setVisible(true);
    }
    else
    {
        // Normal race
        // -----------
        top->setText( _("Setup New Race") );
        top->setVisible(true);

        middle->setText( _("Restart") );
        middle->setVisible(true);

        bottom->setText( _("Back to the menu") );
        bottom->setVisible(true);
    }

    // Make sure that when 'fire' is pressed accidentally (e.g. pressing fire
    // very often to skip all results) that a good default is chosen (to
    // avoid aborting a GP by accident).
    for(int i=0; i<MAX_PLAYER_COUNT; i++)
        top->setFocusForPlayer(i);

}   // enableAllButtons

//-----------------------------------------------------------------------------
void RaceResultGUI::eventCallback(GUIEngine::Widget* widget, 
                                  const std::string& name, const int playerID)
{

    // If something was unlocked, the 'continue' button was 
    // actually used to display "Show unlocked feature(s)" text.
    // ---------------------------------------------------------
    int n = unlock_manager->getRecentlyUnlockedFeatures().size();
    if(n>0)
    {
        if(name=="top")
        {
            std::vector<const ChallengeData*> unlocked = 
                unlock_manager->getRecentlyUnlockedFeatures();
            unlock_manager->clearUnlocked();
            FeatureUnlockedCutScene* scene = FeatureUnlockedCutScene::getInstance();
            scene->addUnlockedThings(unlocked);
            StateManager::get()->popMenu();
            StateManager::get()->pushScreen(scene);
            World::deleteWorld();
            return;
        }
        fprintf(stderr, "Incorrect event '%s' when things are unlocked.\n", 
                name.c_str());
        assert(false);
    }

    // Next check for GP
    // -----------------
    if (race_manager->getMajorMode() == RaceManager::MAJOR_MODE_GRAND_PRIX)
    {
        if (name == "middle")        // Next GP
        {
            StateManager::get()->popMenu();
            race_manager->next();
        }
        else if (name == "bottom")        // Abort
        {
            StateManager::get()->popMenu();
            race_manager->exitRace();
            StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
        }
        else
        {
            fprintf(stderr, "Incorrect event '%s' when things are unlocked.\n",
                            name.c_str());
            assert(false);
        }
        return;
    }

    // This is a normal race, nothing was unlocked
    // -------------------------------------------
    StateManager::get()->popMenu();
    if(name=="top")                 // Setup new race
    {
        race_manager->exitRace();
        Screen* newStack[] = {MainMenuScreen::getInstance(), 
                              RaceSetupScreen::getInstance(), 
                              NULL};
        StateManager::get()->resetAndSetStack( newStack );
    }
    else if (name=="middle")        // Restart
    {
        race_manager->rerunRace();
    }
    else if (name=="bottom")        // Back to main
    {
        race_manager->exitRace();
        StateManager::get()->resetAndGoToScreen(MainMenuScreen::getInstance());
    }
    else
    {
        fprintf(stderr, "Incorrect event '%s' for normal race.\n",
                name.c_str());
    }
    return;
}   // eventCallback

//-----------------------------------------------------------------------------
/** This determines the layout, i.e. the size of all columns, font size etc.
 */
void RaceResultGUI::determineTableLayout()
{
    GUIEngine::Widget *table_area = getWidget("result-table");

    m_font          = GUIEngine::getFont();
    assert(m_font);
    m_was_monospace = m_font->getMonospaceDigits();
    m_font->setMonospaceDigits(true);
    WorldWithRank *rank_world = (WorldWithRank*)World::getWorld();

    unsigned int first_position = 1;
    if(race_manager->getMinorMode()==RaceManager::MINOR_MODE_FOLLOW_LEADER)
        first_position = 2;

    // Use only the karts that are supposed to be displayed (and
    // ignore e.g. the leader in a FTL race).
    unsigned int num_karts =race_manager->getNumberOfKarts()-first_position+1;

    // In FTL races the leader kart is not displayed
    m_all_row_infos.resize(num_karts);


    // Determine the kart to display in the right order, 
    // and the maximum width for the kart name column
    // -------------------------------------------------
    m_width_kart_name     = 0;
    float max_finish_time = 0;


    for(unsigned int position=first_position; 
        position<=race_manager->getNumberOfKarts(); position++)
    {
        const Kart *kart = rank_world->getKartAtPosition(position);

        // Save a pointer to the current row_info entry
        RowInfo *ri              = &(m_all_row_infos[position-first_position]);
        ri->m_is_player_kart     = kart->getController()->isPlayerController();
        ri->m_kart_name          = kart->getName();

        video::ITexture *icon    = 
            kart->getKartProperties()->getIconMaterial()->getTexture();
        ri->m_kart_icon          = icon;

        const float time         = kart->getFinishTime();
        if(time > max_finish_time) max_finish_time = time;
        std::string time_string  = StringUtils::timeToString(time);
        ri->m_finish_time_string = time_string.c_str();

        core::dimension2d<u32> rect = m_font->getDimension(kart->getName());
        if(rect.Width > m_width_kart_name)
            m_width_kart_name = rect.Width;
    }   // for position


    std::string max_time    = StringUtils::timeToString(max_finish_time);
    core::stringw string_max_time(max_time.c_str());
    core::dimension2du r    = m_font->getDimension(string_max_time.c_str());
    m_width_finish_time     = r.Width;

    // Top pixel where to display text
    m_top                   = table_area->m_y;

    // Height of the result display
    unsigned int height     = table_area->m_h;

    // Setup different timing information for the different phases
    // -----------------------------------------------------------
    // How much time between consecutive rows
    m_time_between_rows     = 0.1f;

    // How long it takes for one line to scroll from right to left
    m_time_single_scroll    = 0.2f;

    // Time to rotate the entries to the proper GP position.
    m_time_rotation         = 1.0f;

    // The time the first phase is being displayed: add the start time 
    // of the last kart to the duration of the scroll plus some time
    // of rest before the next phase starts
    m_time_overall_scroll   = (num_karts-1)*m_time_between_rows 
                            + m_time_single_scroll + 2.0f;

    // The time to increase the number of points. 
    m_time_for_points       = 1.0f;

    // Determine text height
    r                       = m_font->getDimension(L"Y");
    m_distance_between_rows = (int)(1.5f*r.Height);

    // If there are too many karts, reduce size between rows
    if(m_distance_between_rows * num_karts > height)
        m_distance_between_rows = height / num_karts;

    m_width_icon = table_area->m_h<600 
                 ? 27 
                 : (int)(40*(table_area->m_w/800.0f));

    m_width_column_space  = 20;

    // Determine width of new points column
    core::dimension2du r_new_p = m_font->getDimension(L"+99");
    m_width_new_points         = r_new_p.Width;

    // Determine width of overall points column
    core::dimension2du r_all_p    = m_font->getDimension(L"999");
    unsigned int width_all_points = r_all_p.Width;

    m_table_width = m_width_icon + m_width_column_space
                  + m_width_kart_name;

    if(race_manager->getMinorMode()!=RaceManager::MINOR_MODE_FOLLOW_LEADER)
        m_table_width += m_width_finish_time + m_width_column_space;

    // Only in GP mode are the points displayed.
    if (race_manager->getMajorMode()==RaceManager::MAJOR_MODE_GRAND_PRIX)
        m_table_width += m_width_new_points + width_all_points
                      + 2 * m_width_column_space;

    m_leftmost_column = table_area->m_x + (table_area->m_w - m_table_width)/2;
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
                                              const unsigned int value,
                                              Input::InputType type, 
                                              int playerId)
{
    if(action!=PA_FIRE) return GUIEngine::EVENT_LET;

    // If the buttons are already visible, let the event go through since
    // it will be triggering eventCallback where this is handles.

    if(m_animation_state == RR_WAIT_TILL_END) return GUIEngine::EVENT_LET;

    nextPhase();
    return GUIEngine::EVENT_BLOCK;
}   // filterActions

//-----------------------------------------------------------------------------
/** Called once a frame, this now triggers the rendering of the actual
 *  race result gui.
 */
void RaceResultGUI::onUpdate(float dt, irr::video::IVideoDriver*)
{
    renderGlobal(dt);
}   // onUpdate

//-----------------------------------------------------------------------------
/** Render all global parts of the race gui, i.e. things that are only 
 *  displayed once even in splitscreen.
 *  \param dt Timestep sized.
 */
void RaceResultGUI::renderGlobal(float dt)
{
    m_timer               += dt;
    assert(World::getWorld()->getPhase()==WorldStatus::RESULT_DISPLAY_PHASE);
    unsigned int num_karts = m_all_row_infos.size();
    
    // First: Update the finite state machine
    // ======================================
    switch(m_animation_state)
    {
    case RR_INIT:        
        for(unsigned int i=0; i<num_karts; i++)
        {
            RowInfo *ri    = &(m_all_row_infos[i]);
            ri->m_start_at = m_time_between_rows * i;
            ri->m_x_pos    = (float)UserConfigParams::m_width;
            ri->m_y_pos    = (float)(m_top+i*m_distance_between_rows);
        }
        m_animation_state = RR_RACE_RESULT;
        break;
    case RR_RACE_RESULT: 
        if(m_timer > m_time_overall_scroll)
        {
            // Make sure that all lines are aligned to the left
            // (in case that the animation was skipped).
            for(unsigned int i=0; i<num_karts; i++)
            {
                RowInfo *ri    = &(m_all_row_infos[i]);
                ri->m_x_pos    = (float)m_leftmost_column;
            }
            if(race_manager->getMajorMode()!=RaceManager::MAJOR_MODE_GRAND_PRIX)
            {
                m_animation_state = RR_WAIT_TILL_END;
                enableAllButtons();
                break;
            }

            determineGPLayout();
            m_animation_state = RR_OLD_GP_RESULTS;
            m_timer           = 0;
        }
        break;
    case RR_OLD_GP_RESULTS:
        if(m_timer > m_time_overall_scroll)
        {
            m_animation_state = RR_INCREASE_POINTS;
            m_timer           = 0;
            for(unsigned int i=0; i<num_karts; i++)
            {
                RowInfo *ri = &(m_all_row_infos[i]);
                ri->m_x_pos = (float)m_leftmost_column;
            }
        }
        break;
    case RR_INCREASE_POINTS: 
        // Have one second delay before the resorting starts.
        if(m_timer > 1+m_time_for_points)
        {
            m_animation_state = RR_RESORT_TABLE;
            if(m_gp_position_was_changed)
                m_timer       = 0;
            else
                // This causes the phase to go to RESORT_TABLE once, and then
                // immediately wait till end. This has the advantage that any
                // phase change settings will be processed properly.
                m_timer       = m_time_rotation+1;
            // Make the new row permanent; necessary in case
            // that the animation is skipped.
            for(unsigned int i=0; i<num_karts; i++)
            {
                RowInfo *ri                    = &(m_all_row_infos[i]);
                ri->m_new_points               = 0;
                ri->m_current_displayed_points = (float)ri->m_new_overall_points;
            }

        }
        break;
    case RR_RESORT_TABLE:
        if(m_timer > m_time_rotation)
        {
            m_animation_state = RR_WAIT_TILL_END;
            // Make the new row permanent.
            for(unsigned int i=0; i<num_karts; i++)
            {
                RowInfo *ri = &(m_all_row_infos[i]);
                ri->m_y_pos = ri->m_centre_point - ri->m_radius;
            }
            enableAllButtons();
        }
        break;
    case RR_WAIT_TILL_END:      break;
    }   // switch

    // Second phase: update X and Y positions for the various animations
    // =================================================================
    float v = 0.9f*UserConfigParams::m_width/m_time_single_scroll;
    for(unsigned int i=0; i<m_all_row_infos.size(); i++)
    {
        RowInfo *ri = &(m_all_row_infos[i]);
        float x = ri->m_x_pos;
        float y = ri->m_y_pos;
        switch(m_animation_state)
        {
        // Both states use the same scrolling:
        case RR_INIT: break;   // Remove compiler warning
        case RR_RACE_RESULT:
        case RR_OLD_GP_RESULTS:
             if(m_timer > ri->m_start_at)
             {   // if active
                 ri->m_x_pos -= dt*v;
                 if(ri->m_x_pos<m_leftmost_column)
                     ri->m_x_pos = (float)m_leftmost_column;
                 x = ri->m_x_pos;
             }
             break;
        case RR_INCREASE_POINTS: 
            ri->m_current_displayed_points += 
                dt*race_manager->getPositionScore(1)/m_time_for_points;
            if(ri->m_current_displayed_points>ri->m_new_overall_points)
                ri->m_current_displayed_points = (float)ri->m_new_overall_points;
            ri->m_new_points -= dt*race_manager->getPositionScore(1)/m_time_for_points;
            if(ri->m_new_points<0)
                ri->m_new_points = 0;
            break;
        case RR_RESORT_TABLE:
            x = ri->m_x_pos       -ri->m_radius*sin(m_timer/m_time_rotation*M_PI);
            y = ri->m_centre_point+ri->m_radius*cos(m_timer/m_time_rotation*M_PI);
            break;
        case RR_WAIT_TILL_END:
            break;
        }   // switch
        displayOneEntry((unsigned int)x, (unsigned int)y, i, true);
    }   // for i
}   // renderGlobal

//-----------------------------------------------------------------------------
/** Determine the layout and fields for the GP table based on the previous
 *  GP results.
 */
void RaceResultGUI::determineGPLayout()
{
    unsigned int num_karts = race_manager->getNumberOfKarts();
    std::vector<int> old_rank(num_karts, 0);
    for(unsigned int kart_id=0; kart_id<num_karts; kart_id++)
    {
        int rank             = race_manager->getKartGPRank(kart_id);
        // In case of FTL mode: ignore the leader
        if(rank<0) continue;
        old_rank[kart_id]    = rank;
        const Kart *kart     = World::getWorld()->getKart(kart_id);
        RowInfo *ri          = &(m_all_row_infos[rank]);
        ri->m_kart_icon      = 
            kart->getKartProperties()->getIconMaterial()->getTexture();
        ri->m_kart_name      = kart->getName();
        ri->m_is_player_kart = kart->getController()->isPlayerController();

        float time           = race_manager->getOverallTime(kart_id);
        ri->m_finish_time_string
                             = StringUtils::timeToString(time).c_str();
        ri->m_start_at       = m_time_between_rows * rank;
        ri->m_x_pos          = (float)UserConfigParams::m_width;
        ri->m_y_pos          = (float)(m_top+rank*m_distance_between_rows);
        int p                = race_manager->getKartPrevScore(kart_id);
        ri->m_current_displayed_points = (float)p;
        ri->m_new_points     = 
            (float)race_manager->getPositionScore(kart->getPosition());
    }

    // Now update the GP ranks, and determine the new position
    // -------------------------------------------------------
    race_manager->computeGPRanks();
    m_gp_position_was_changed = false;
    for(unsigned int i=0; i<num_karts; i++)
    {
        int j                    = old_rank[i];
        int gp_position          = race_manager->getKartGPRank(i);
        m_gp_position_was_changed |= j!=gp_position;
        RowInfo *ri              = &(m_all_row_infos[j]);
        ri->m_radius             = (j-gp_position)*(int)m_distance_between_rows*0.5f;
        ri->m_centre_point       = m_top+(gp_position+j)*m_distance_between_rows*0.5f;
        int p                    = race_manager->getKartScore(i);
        ri->m_new_overall_points = p;
    }   // i < num_karts
}   // determineGPLayout

//-----------------------------------------------------------------------------
/** Displays the race results for a single kart.
 *  \param n Index of the kart to be displayed.
 *  \param display_points True if GP points should be displayed, too
 */
void RaceResultGUI::displayOneEntry(unsigned int x, unsigned int y, 
                                    unsigned int n, bool display_points)
{
    RowInfo *ri = &(m_all_row_infos[n]);
    video::SColor color = ri->m_is_player_kart ? video::SColor(255,255,0,  0  )
                                               : video::SColor(255,255,255,255);

#ifdef USE_PER_LINE_BACKGROUND
    // Draw the background image
    core::rect<s32> dest(x-50, y, 
                         x+50+m_table_width, 
                         (int)(y+m_distance_between_rows));
    ri->m_box_params.setTexture(irr_driver->getTexture( (file_manager->getGUIDir() + "skins/glass/glassbutton_focused.png").c_str() ) );
    GUIEngine::getSkin()->drawBoxFromStretchableTexture(&(ri->m_widget_container),
                                                        dest, 
                                                        ri->m_box_params);
#endif
    unsigned int current_x = x;

    // First draw the icon
    // -------------------
    if(ri->m_kart_icon)
    {
        core::recti source_rect(core::vector2di(0,0), 
                                ri->m_kart_icon->getSize());
        core::recti dest_rect(current_x, y, 
                              current_x+m_width_icon, y+m_width_icon);
        irr_driver->getVideoDriver()->draw2DImage(ri->m_kart_icon, dest_rect,
                                                  source_rect, NULL, NULL, 
                                                  true);
    }

    current_x += m_width_icon + m_width_column_space;

    // Draw the name
    // -------------
    core::recti pos_name(current_x, y,
                         UserConfigParams::m_width, y+m_distance_between_rows);
    m_font->draw(ri->m_kart_name, pos_name, color, false, false, NULL, true /* ignoreRTL */);
    current_x += m_width_kart_name + m_width_column_space;

    // Draw the time except in FTL mode
    // --------------------------------
    if(race_manager->getMinorMode()!=RaceManager::MINOR_MODE_FOLLOW_LEADER)
    {
        core::recti dest_rect = core::recti(current_x, y, current_x+100, y+10);
        m_font->draw(ri->m_finish_time_string, dest_rect, color, false, false, NULL, true /* ignoreRTL */);
        current_x += m_width_finish_time + m_width_column_space;
    }

    // Only display points in GP mode and when the GP results are displayed.
    // =====================================================================
    if(race_manager->getMajorMode()!=RaceManager::MAJOR_MODE_GRAND_PRIX ||
        m_animation_state == RR_RACE_RESULT)
        return;

    // Draw the new points
    // -------------------
    if(ri->m_new_points>0)
    {
        core::recti dest_rect = core::recti(current_x, y, current_x+100, y+10);
        core::stringw point_string = core::stringw("+")
                                   + core::stringw((int)ri->m_new_points);
        // With mono-space digits space has the same width as each digit, so
        // we can simply fill up the string with spaces to get the right 
        // aligned.
        while(point_string.size()<3)
            point_string = core::stringw(" ")+point_string;
        m_font->draw(point_string, dest_rect, color, false, false, NULL, true /* ignoreRTL */);
    }
    current_x += m_width_new_points   +m_width_column_space;

    // Draw the old_points plus increase value
    // ---------------------------------------
    core::recti dest_rect = core::recti(current_x, y, current_x+100, y+10);
    core::stringw point_inc_string = 
        core::stringw((int)(ri->m_current_displayed_points));
    while(point_inc_string.size()<3)
        point_inc_string = core::stringw(" ")+point_inc_string;
    m_font->draw(point_inc_string, dest_rect, color, false, false, NULL, true /* ignoreRTL */);

}   // displayOneEntry

