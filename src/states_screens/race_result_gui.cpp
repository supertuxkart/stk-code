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
#include "modes/world.hpp"
#include "states_screens/dialogs/race_over_dialog.hpp"
#include "utils/string_utils.hpp"

/** Constructor, initialises internal data structures.
 */
RaceResultGUI::RaceResultGUI()
{
#undef USE_NEW_RACE_RESULT

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
    m_font       = dynamic_cast<gui::ScalableFont*>(GUIEngine::getFont());
    assert(m_font);
    World *world = World::getWorld();
    world->raceResultOrder(&m_order);

    // Determine the kart to display in the right order, and the maximum 
    // width for the kart name column
    m_max_kart_name_width = 0;
    float max_finish_time = 0;
    for(unsigned int i=0; i<m_order.size(); i++)
    {
        if(m_order[i]==-1) continue;
        Kart *kart = world->getKart(m_order[i]);
        m_kart_names.push_back(kart->getName());

        video::ITexture *icon = 
            kart->getKartProperties()->getIconMaterial()->getTexture();
        m_kart_icons.push_back(icon);

        const float time  = kart->getFinishTime();
        if(time > max_finish_time) max_finish_time = time;
        std::string time_string = StringUtils::timeToString(time);
        m_time_strings.push_back(time_string.c_str());

        core::dimension2d<u32> rect = m_font->getDimension(kart->getName().c_str());
        if(rect.Width > m_max_kart_name_width) 
            m_max_kart_name_width = rect.Width;
        m_new_points.push_back(race_manager->getPositionScore(i+1));
        m_old_overall_points.push_back(race_manager->getKartPrevScore(m_order[i]));
    }

    std::string max_time = StringUtils::timeToString(max_finish_time);
    core::stringw string_max_time(max_time.c_str());
    core::dimension2du r_time = m_font->getDimension(string_max_time.c_str());
    m_time_width = r_time.Width;

    // Use only the karts that are supposed to be displayed (and
    // ignore e.g. the leader in a FTL race).
    unsigned int num_karts       = m_time_strings.size();

    // Top pixel where to display text
    unsigned int top             = (int)(0.15f*UserConfigParams::m_height);

    // Height of the result display
    unsigned int height          = (int)(0.7f *UserConfigParams::m_height);

    // Setup different timing information for the different phases
    // -----------------------------------------------------------
    // How much time between consecutive rows
    m_time_between_rows          = 0.5f;

    // How long it takes for one line to scroll from right to left
    m_time_single_scroll         = 0.5f;

    // The time the first phase is being displayed: add the start time 
    // of the last kart to the duration of the scroll plus some time
    // of rest before the next phase starts
    m_time_overall_scroll        = (num_karts-1)*m_time_between_rows 
                                 + m_time_single_scroll + 2.0f;

    // Determine text height
    core::dimension2du text_size = m_font->getDimension(L"Y");
    m_distance_between_rows      = (int)(1.5f*text_size.Height);

    // If there are too many karts, reduce size between rows
    if(m_distance_between_rows * num_karts > height)
        m_distance_between_rows = height / num_karts;

    m_start_at.clear();
    for(unsigned int i=0; i<num_karts; i++)
    {
        m_start_at.push_back(m_time_between_rows * i);
        m_x_pos.push_back((float)UserConfigParams::m_width);
        m_y_pos.push_back(top+i*m_distance_between_rows);
    }

    m_icon_width = UserConfigParams::m_height<600 
                 ? 27 
                 : (int)(40*(UserConfigParams::m_width/800.0f));

    // Not all digits have the same width. So to properly align the times 
    // and points, we have to do the layout one digit at a time, and we
    // need the maximum size of one digit for that:
    m_max_digit_width = 0;
    for(char c='0'; c<='9'; c++)
    {
        wchar_t s[2];
        s[0] = c; s[1]=0;
        core::dimension2du r = m_font->getDimension(s);
        if(r.Width > m_max_digit_width) m_max_digit_width = r.Width;
    }
    core::dimension2du r = m_font->getDimension(L":");
    m_colon_width        = r.Width;

    m_column_space_size  = 20;
}   // determineLayout

//-----------------------------------------------------------------------------
/** Render all global parts of the race gui, i.e. things that are only 
 *  displayed once even in splitscreen.
 *  \param dt Timestep sized.
 */
void RaceResultGUI::renderGlobal(float dt)
{
    m_timer += dt;
    switch(m_animation_state)
    {
    case RR_BEGIN_FIRST_TABLE: 
        if(m_timer > m_time_overall_scroll)
        {
            m_animation_state = RR_INCREASE_POINTS;
            m_timer           = 0;
        }
        break;
    case RR_INCREASE_POINTS:
        if(m_timer > 5)
        {
            m_animation_state = RR_RESORT_TABLE;
            m_timer           = 0;
        }
        break;
    case RR_RESORT_TABLE:
        break;
    case RR_WAIT_TILL_END:
        break;
    }   // switch
    World *world = World::getWorld();
    assert(world->getPhase()==WorldStatus::RESULT_DISPLAY_PHASE);


    unsigned int num_karts = world->getNumKarts();

    float v = 0.9f*UserConfigParams::m_width/m_time_single_scroll;
    for(unsigned int i=0; i<m_kart_names.size(); i++)
    {
        if(m_start_at[i]>m_timer) continue;
        m_x_pos[i] -= dt*v;
        if(m_x_pos[i]<0.1f*UserConfigParams::m_width)
            m_x_pos[i] = 0.1f*UserConfigParams::m_width;
        displayOneEntry((unsigned int)(m_x_pos[i]), m_y_pos[i], 
                         i, true);
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
    core::recti dest_rect(x, y, x+m_icon_width, y+m_icon_width);
    irr_driver->getVideoDriver()->draw2DImage(m_kart_icons[n], dest_rect, 
                                              source_rect, NULL, NULL, true);
    // Draw the name
    // -------------
    core::recti pos_name(x+m_icon_width+m_column_space_size, y,
                         UserConfigParams::m_width, y+m_distance_between_rows);

    m_font->draw(m_kart_names[n], pos_name, color);

    // Draw the time
    // -------------
    unsigned int x_time = x + m_icon_width + m_column_space_size
                        + m_max_kart_name_width + m_column_space_size;

    bool mono = m_font->getMonospaceDigits();
    //m_font->setMonospaceDigits(true);
    dest_rect = core::recti(x_time, y, x_time+100, y+10);
    m_font->draw(m_time_strings[n], dest_rect, color);
    m_font->setMonospaceDigits(mono);

    // Draw the new points
    // -------------------
    unsigned int x_point = x + m_icon_width + m_column_space_size
                         + m_max_kart_name_width + m_column_space_size
                         + m_time_width + m_column_space_size;
    dest_rect = core::recti(x_point, y, x_point+100, y+10);
    core::stringw point_string = core::stringw("+")+core::stringw(m_new_points[n]);
    while(point_string.size()<3)
        point_string = core::stringw(" ")+point_string;
    m_font->draw(point_string, dest_rect, color);
    
}   // displayOneEntry

//-----------------------------------------------------------------------------
void RaceResultGUI::drawNumber(const core::stringw &number_string, 
                               unsigned int *x, unsigned int y,
                               const video::SColor &color)
{
    for(unsigned int i=0; i<number_string.size(); i++)
    {
        core::recti p(*x, y, *x+m_max_digit_width, y+10);
        m_font->draw(number_string.subString(i,1), p, color);
        if(number_string[i]==':')
            *x+=m_colon_width;
        else
            *x+=m_max_digit_width;
    }   // for i<number_string.size
}   // drawNumber
