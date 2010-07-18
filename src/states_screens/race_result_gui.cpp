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
    // FIXME: for now disable the new race result display
    // by just firing up the old display (which will make sure
    // that the rendering for this object is not called anymore).
    new RaceOverDialog(0.6f, 0.9f);
    return;


    determineLayout();
    m_timer    = 0;
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
    // Top pixel where to display text
    unsigned int top             = (int)(0.15f*UserConfigParams::m_height);

    // Height of the result display
    unsigned int height          = (int)(0.7f *UserConfigParams::m_height);

    // How much time between consecutive rows
    m_time_between_rows          = 0.5f;

    World *world                 = World::getWorld();
    unsigned int num_karts       = world->getNumKarts();

    // Determine text height
    m_font                       = GUIEngine::getFont();
    core::dimension2du text_size = m_font->getDimension(L"a");
    m_distance_between_rows      = (int)(1.5f*text_size.Height);

    // If there are too many karts, reduce size between rows
    if(m_distance_between_rows * num_karts > height)
        m_distance_between_rows = height / num_karts;

    world->raceResultOrder(&m_order);

    m_start_at.clear();
    for(unsigned int i=0; i<num_karts; i++)
    {
        if(m_order[i]==-1) continue;
        m_start_at.push_back(m_time_between_rows * i);
        m_x_pos.push_back((float)UserConfigParams::m_width);
        m_y_pos.push_back(m_distance_between_rows+i*m_distance_between_rows);
        Kart *kart = world->getKart(m_order[i]);
        const core::stringw& kart_name = kart->getName();

        const float time        = kart->getFinishTime();
        std::string time_string = StringUtils::timeToString(time);
        core::stringw kart_results_line = StringUtils::insertValues( L"%i. %s %s",
            kart->getPosition(),
            kart_name.c_str(),
            time_string.c_str());

        m_entry.push_back(core::stringw(kart_results_line.c_str()));
    }

    m_icon_width = UserConfigParams::m_height<600 
                 ? 27 
                 : (int)(40*(UserConfigParams::m_width/800.0f));

    // Determine the maximum width for the kart name column
    unsigned int max_name_width = 0;
    float max_finish_time       = 0;
    for(unsigned int i=0; i<num_karts; i++)
    {
        const Kart *kart = world->getKart(i);
        core::dimension2d<u32> rect = m_font->getDimension(kart->getName().c_str());
        if(rect.Width > max_name_width) max_name_width = rect.Width;
        if(kart->getFinishTime() > max_finish_time) 
            max_finish_time = kart->getFinishTime();
    }   // for i<num_karts

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
    m_column_space_size = 20;
}   // determineLayout

//-----------------------------------------------------------------------------
/** Render all global parts of the race gui, i.e. things that are only 
 *  displayed once even in splitscreen.
 *  \param dt Timestep sized.
 */
void RaceResultGUI::renderGlobal(float dt)
{

    World *world = World::getWorld();
    assert(world->getPhase()==WorldStatus::RESULT_DISPLAY_PHASE);

    m_timer += dt;

    unsigned int num_karts = world->getNumKarts();

    // How long it takes for one line to scroll from right to left
    float scroll_duration = 0.5f;

    float v = 0.9f*UserConfigParams::m_width/scroll_duration;
    for(unsigned int i=0; i<num_karts; i++)
    {
        if(m_order[i]==-1) continue;
        if(m_start_at[i]>m_timer) continue;
        m_x_pos[i] -= dt*v;
        if(m_x_pos[i]<0.1f*UserConfigParams::m_width)
            m_x_pos[i] = 0.1f*UserConfigParams::m_width;
        printf("%d: %f %d\n", i, m_x_pos[i], m_y_pos[i]);
        displayOneEntry((unsigned int)(m_x_pos[i]), m_y_pos[i], 
                         m_order[i], true);
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
    video::ITexture *t = kart->getKartProperties()->getIconMaterial()->getTexture();
    core::recti source_rect(core::vector2di(0,0), t->getSize());
    core::recti dest_rect(x, y, x+m_icon_width, y+m_icon_width);
    irr_driver->getVideoDriver()->draw2DImage(t, dest_rect, source_rect, 
                                              NULL, NULL, true);
    // Draw the name
    // -------------
    core::recti pos(x+m_icon_width+m_column_space_size, y,
                    UserConfigParams::m_width, y+m_distance_between_rows);

    m_font->draw(m_entry[n], pos, color);
}   // displayOneEntry
