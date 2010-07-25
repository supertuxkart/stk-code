//  $Id: race_result_gui.cpp 5424 2010-05-10 23:53:32Z hikerstk $
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

#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "modes/world.hpp"
#include "states_screens/dialogs/race_over_dialog.hpp"
#include "utils/string_utils.hpp"

/** Constructor, initialises internal data structures.
 */
RaceResultGUI::RaceResultGUI()
{
#define USE_NEW_RACE_RESULT

#ifndef USE_NEW_RACE_RESULT
    // FIXME: for now disable the new race result display
    // by just firing up the old display (which will make sure
    // that the rendering for this object is not called anymore).
    new RaceOverDialog(0.6f, 0.9f);
    return;
#else
    determineLayout();
    m_timer    = 0;
    m_animation_state = RR_BEGIN_FIRST_TABLE;
#endif
}   // RaceResultGUI

//-----------------------------------------------------------------------------
/** Destructor. */
RaceResultGUI::~RaceResultGUI()
{
#ifdef USE_NEW_RACE_RESULT
    m_font->setMonospaceDigits(m_was_monospace);
#endif
}   // ~Racegui

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
    new RaceOverDialog(0.6f, 0.9f);
}   // nextPhase

//-----------------------------------------------------------------------------
/** This determines the layout, i.e. the size of all columns, font size etc.
 */
void RaceResultGUI::determineLayout()
{
    m_font          = GUIEngine::getFont();
    m_was_monospace = m_font->getMonospaceDigits();
    m_font->setMonospaceDigits(true);

    assert(m_font);
    World *world = World::getWorld();
    std::vector<int> order;
    world->raceResultOrder(&order);

    // Determine the kart to display in the right order, 
    // and the maximum width for the kart name column
    // -------------------------------------------------
    m_width_kart_name     = 0;
    float max_finish_time = 0;
    for(unsigned int i=0; i<order.size(); i++)
    {
        if(order[i]==-1) continue;
        Kart *kart = world->getKart(order[i]);
        m_kart_names.push_back(kart->getName());

        video::ITexture *icon = 
            kart->getKartProperties()->getIconMaterial()->getTexture();
        m_kart_icons.push_back(icon);

        const float time  = kart->getFinishTime();
        if(time > max_finish_time) max_finish_time = time;
        std::string time_string = StringUtils::timeToString(time);
        m_finish_time_string.push_back(time_string.c_str());

        core::dimension2d<u32> rect = m_font->getDimension(kart->getName().c_str());
        if(rect.Width > m_width_kart_name)
            m_width_kart_name = rect.Width;
        m_new_points.push_back(race_manager->getPositionScore(i+1));
        int p = race_manager->getKartPrevScore(order[i]);
        m_new_overall_points.push_back(p+m_new_points[i]);
        m_current_displayed_points.push_back((float)p);
    }   // for i < order.size()

    std::string max_time    = StringUtils::timeToString(max_finish_time);
    core::stringw string_max_time(max_time.c_str());
    core::dimension2du r    = m_font->getDimension(string_max_time.c_str());
    m_width_finish_time     = r.Width;

    // Use only the karts that are supposed to be displayed (and
    // ignore e.g. the leader in a FTL race).
    unsigned int num_karts  = m_finish_time_string.size();

    // Top pixel where to display text
    unsigned int top        = (int)(0.15f*UserConfigParams::m_height);

    // Height of the result display
    unsigned int height     = (int)(0.7f *UserConfigParams::m_height);

    // Setup different timing information for the different phases
    // -----------------------------------------------------------
    // How much time between consecutive rows
    m_time_between_rows     = 0.1f;

    // How long it takes for one line to scroll from right to left
    m_time_single_scroll    = 0.2f;

    // Time to rotate the entries to the proper GP position.
    m_time_rotation         = 2.0f;

    // The time the first phase is being displayed: add the start time 
    // of the last kart to the duration of the scroll plus some time
    // of rest before the next phase starts
    m_time_overall_scroll   = (num_karts-1)*m_time_between_rows 
                            + m_time_single_scroll + 2.0f;

    // The time to increase the number of points. Take the
    // overall time for this phase (1 second atm) divided
    // by the maximum number of points increase.
    m_time_for_points       = stk_config->m_scores[0]/1.0f;

    // Determine text height
    r                            = m_font->getDimension(L"Y");
    m_distance_between_rows      = (int)(1.5f*r.Height);

    // If there are too many karts, reduce size between rows
    if(m_distance_between_rows * num_karts > height)
        m_distance_between_rows = height / num_karts;

    m_width_icon = UserConfigParams::m_height<600 
                 ? 27 
                 : (int)(40*(UserConfigParams::m_width/800.0f));

    m_width_column_space  = 20;

    // Determine width of new points column
    core::dimension2du r_new_p = m_font->getDimension(L"+99");
    m_width_new_points = r_new_p.Width;

    // Determine width of overall points column
    core::dimension2du r_all_p = m_font->getDimension(L"9999");
    unsigned int width_all_points = r_all_p.Width;

    unsigned int table_width = m_width_icon        +   m_width_kart_name 
                             + m_width_finish_time + 2*m_width_column_space;

    // Only in GP mode are the points displayed.
    if (race_manager->getMajorMode()==RaceManager::MAJOR_MODE_GRAND_PRIX)
        table_width += m_width_new_points + width_all_points
                     + 2 * m_width_column_space;

    m_leftmost_column = (UserConfigParams::m_width  - table_width)/2;
    if(race_manager->getMajorMode()==RaceManager::MAJOR_MODE_GRAND_PRIX)
        race_manager->computeGPRanks();
    m_start_at.clear();
    for(unsigned int i=0; i<num_karts; i++)
    {
        m_start_at.push_back(m_time_between_rows * i);
        m_x_pos.push_back((float)UserConfigParams::m_width);
        m_y_pos.push_back(top+i*m_distance_between_rows);
        
        if(race_manager->getMajorMode()==RaceManager::MAJOR_MODE_GRAND_PRIX)
        {
            int gp_position = race_manager->getKartGPRank(order[i]);
            if(gp_position<(int)i)
                printf("X");
            m_radius.push_back( (gp_position-(int)i)*(int)m_distance_between_rows*0.5f);
            m_centre_point.push_back(top+(gp_position+i)*m_distance_between_rows*0.5f);
        }
    }   // i < num_karts


}   // determineLayout

//-----------------------------------------------------------------------------
/** Render all global parts of the race gui, i.e. things that are only 
 *  displayed once even in splitscreen.
 *  \param dt Timestep sized.
 */
void RaceResultGUI::renderGlobal(float dt)
{
    m_timer               += dt;
    World *world           = World::getWorld();
    assert(world->getPhase()==WorldStatus::RESULT_DISPLAY_PHASE);
    unsigned int num_karts = world->getNumKarts();


    switch(m_animation_state)
    {
    case RR_BEGIN_FIRST_TABLE:  if(m_timer > m_time_overall_scroll)
                                {
                                    m_animation_state = RR_INCREASE_POINTS;
                                    m_timer           = 0;
                                }
                                break;
    case RR_INCREASE_POINTS:    if(m_timer > 5)
                                {
                                    m_animation_state = RR_RESORT_TABLE;
                                    m_timer           = 0;
                                }
                                break;
    case RR_RESORT_TABLE:       if(m_timer > m_time_rotation)
                                {
                                    m_animation_state = RR_WAIT_TILL_END;
                                    // Make the new row permanent.
                                    for(unsigned int i=0; i<num_karts; i++)
                                        m_y_pos[i] = m_centre_point[i]
                                                   + m_radius[i];
                                }
                                break;
    case RR_WAIT_TILL_END:      break;
    }   // switch

    float v = 0.9f*UserConfigParams::m_width/m_time_single_scroll;
    for(unsigned int i=0; i<m_kart_names.size(); i++)
    {
        float x = m_x_pos[i];
        float y = (float)m_y_pos[i];
        switch(m_animation_state)
        {
        case RR_BEGIN_FIRST_TABLE:
             if(m_timer > m_start_at[i])
             {   // if active
                 m_x_pos[i] -= dt*v;
                 if(m_x_pos[i]<m_leftmost_column)
                     m_x_pos[i] = (float)m_leftmost_column;
                 x = m_x_pos[i];
             }
             break;
        case RR_INCREASE_POINTS:    
            m_current_displayed_points[i] += dt*m_time_for_points;
            if(m_current_displayed_points[i]>m_new_overall_points[i])
                m_current_displayed_points[i] = (float)m_new_overall_points[i];
            break;
        case RR_RESORT_TABLE:
            x = m_x_pos[i]       +m_radius[i]*sin(m_timer/m_time_rotation*M_PI);
            y = m_centre_point[i]+m_radius[i]*cos(m_timer/m_time_rotation*M_PI);
            break;
        case RR_WAIT_TILL_END:
            x = m_x_pos[i];
            y = (float)m_y_pos[i];
            break;
        }   // switch
        displayOneEntry((unsigned int)x, (unsigned int)y, i, true);
    }
}   // renderGlobal

//-----------------------------------------------------------------------------
/** Displays the race results for a single kart.
 *  \param n Index of the kart to be displayed.
 *  \param display_points True if GP points should be displayed, too
 */
void RaceResultGUI::displayOneEntry(unsigned int x, unsigned int y, 
                                    unsigned int n, bool display_points)
{
    World *world = World::getWorld();
    video::SColor color;
    // Display player karts in red
    Kart *kart = world->getKart(n);
    if(kart->getController()->isPlayerController())
        color = video::SColor(255, 255, 0, 0);
    else
        color = video::SColor(255, 255, 255, 255);

    // First draw the icon
    // -------------------
    core::recti source_rect(core::vector2di(0,0), m_kart_icons[n]->getSize());
    core::recti dest_rect(x, y, x+m_width_icon, y+m_width_icon);
    irr_driver->getVideoDriver()->draw2DImage(m_kart_icons[n], dest_rect, 
                                              source_rect, NULL, NULL, true);
    // Draw the name
    // -------------
    core::recti pos_name(x+m_width_icon+m_width_column_space, y,
                         UserConfigParams::m_width, y+m_distance_between_rows);

    m_font->draw(m_kart_names[n], pos_name, color);

    // Draw the time
    // -------------
    unsigned int x_time = x + m_width_icon  + m_width_column_space
                        + m_width_kart_name + m_width_column_space;

    bool mono = m_font->getMonospaceDigits();
    dest_rect = core::recti(x_time, y, x_time+100, y+10);
    m_font->draw(m_finish_time_string[n], dest_rect, color);
    m_font->setMonospaceDigits(mono);

    // Only display points in GP mode.
    // ===============================
    if(race_manager->getMajorMode()!=RaceManager::MAJOR_MODE_GRAND_PRIX)
        return;

    // Draw the new points
    // -------------------
    unsigned int x_point = x + m_width_icon    + m_width_column_space
                         + m_width_kart_name   + m_width_column_space
                         + m_width_finish_time + m_width_column_space;
    dest_rect = core::recti(x_point, y, x_point+100, y+10);
    core::stringw point_string = core::stringw("+")+core::stringw(m_new_points[n]);
    // With mono-space digits space has the same width as each character, so
    // we can simply fill up the string with spaces to get the right aligned.
    while(point_string.size()<3)
        point_string = core::stringw(" ")+point_string;
    m_font->draw(point_string, dest_rect, color);

    // Draw the old_points plus increase value
    // ---------------------------------------
    unsigned int x_point_inc = x + m_width_icon    + m_width_column_space
                             + m_width_kart_name   + m_width_column_space
                             + m_width_finish_time + m_width_column_space
                             + m_width_new_points   +m_width_column_space;
    dest_rect = core::recti(x_point_inc, y, x_point_inc+100, y+10);
    core::stringw point_inc_string = core::stringw((int)(m_current_displayed_points[n]));
    while(point_inc_string.size()<3)
        point_inc_string = core::stringw(" ")+point_inc_string;
    m_font->draw(point_inc_string, dest_rect, color);

}   // displayOneEntry

