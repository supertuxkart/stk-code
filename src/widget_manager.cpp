//
//  SuperTuxKart - a fun racing game with go-kart
//  This code originally from Neverball copyright (C) 2003 Robert Kooima
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#include "widget_manager.hpp"

#include "user_config.hpp"

//TODO: this include should not be necesary?
#include <SDL/SDL.h>

#include "gui/font.hpp"

//TEMP
#include <iostream>
#include <cstdlib>
#include <iterator>
#include <algorithm>

WidgetManager *widget_manager;

const int WidgetManager::WGT_NONE = -1;

WidgetManager::WidgetManager() :
m_x( -1 ), m_y( -1 ), m_selected_wgt_token( WGT_NONE )
{
    restore_default_states();
    init_fonts();
}

//-----------------------------------------------------------------------------
WidgetManager::~WidgetManager()
{
    delete_wgts();
    delete_fonts();
}

//-----------------------------------------------------------------------------
bool WidgetManager::add_wgt
(
    const int TOKEN,
    const int MIN_WIDTH,
    const int MIN_HEIGHT
)
{
    if( TOKEN != WGT_NONE && find_id( TOKEN ) != WGT_NONE )
    {
        std::cerr << "WARNING: tried to create widget with token " <<
            TOKEN << " but it is already in use.\n";
        return false;
    }

    WidgetID new_id;
    new_id.token = TOKEN;
    //There is no reason to make a token-less widget active, so if the token
    //WGT_NONE, the widget is forced to be inactive, preventing bugs.
    new_id.active = TOKEN != WGT_NONE ? m_default_active : false;
    new_id.min_width = MIN_WIDTH;
    new_id.min_height = MIN_HEIGHT;

    new_id.widget = new Widget(0, 0, 0, 0);

    new_id.widget->m_enable_rect = m_default_show_rect;
    new_id.widget->m_rect_color = m_default_rect_color;

    new_id.widget->m_enable_texture = m_default_show_texture;
    new_id.widget->m_texture = m_default_texture;

    new_id.widget->m_enable_text = m_default_show_text;
    new_id.widget->m_text.assign(m_default_text);
    new_id.widget->m_text_size = m_default_text_size;
    new_id.widget->m_text_x_alignment =  m_default_text_x_alignment;
    new_id.widget->m_text_y_alignment =  m_default_text_y_alignment;

    new_id.widget->m_enable_scroll = m_default_enable_scroll;
/*    new_id.widget->m_scroll_pos_x = m_default_scroll_x_pos;*/
    new_id.widget->m_scroll_pos_y = m_default_scroll_y_pos;
/*    new_id.widget->m_scroll_speed_x = m_default_scroll_x_speed;*/
    new_id.widget->m_scroll_speed_y = m_default_scroll_y_speed;

    m_widgets.push_back(new_id);

    return true;
}

//-----------------------------------------------------------------------------
bool WidgetManager::break_line()
{
    const int LAST_WGT = m_widgets.size() - 1;

    if( LAST_WGT < 0 )
    {
        std::cerr << "Warning: tried to add a break before adding any " <<
            "widgets.\n";
        return false;
    }

    const int NUM_BREAKS = m_breaks.size();

    if( NUM_BREAKS > 0 )
    {
        if( !line_breaks(LAST_WGT) ) m_breaks.push_back(LAST_WGT);
        else
        {
            std::cerr << "Warning: tried to add a break twice after " <<
                "widget with token" << m_widgets[LAST_WGT].token << ".\n";
            return false;
        }
    }
    else
    {
            m_breaks.push_back(LAST_WGT);
    }

    return true;
}


//-----------------------------------------------------------------------------
void WidgetManager::delete_wgts()
{
    const int NUM_WIDGETS = m_widgets.size();

    for(int i = 0; i < NUM_WIDGETS; ++i)
    {
        delete m_widgets[i].widget;
    }

    m_widgets.clear();
}

//-----------------------------------------------------------------------------
bool WidgetManager::line_breaks( const int WGT ) const
{
    const int NUM_BREAKS = m_breaks.size();

    if( NUM_BREAKS > 0)
    {
        for(int i = 0; i < NUM_BREAKS; ++i )
        {
            if( m_breaks[i] == WGT ) return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
int WidgetManager::find_id(const int TOKEN) const
{
    const int NUM_WIDGETS = m_widgets.size();

    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        if( TOKEN == m_widgets[i].token )
            return i;
    }

    return WGT_NONE;
}

//-----------------------------------------------------------------------------
void WidgetManager::update(const float DELTA)
{

    //Enable 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, user_config->m_width, 0.0, user_config->m_height, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);


    glPushAttrib(GL_LIGHTING_BIT |
        GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE  ) ;

    const int NUM_WIDGETS = m_widgets.size();
    for( int i = 0; i < NUM_WIDGETS; ++i)
    {
        m_widgets[i].widget->update(DELTA);
    }

    glPopAttrib();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

}

/** The calc_width() function retrieves the width of the smallest rectangle
 *  that contains all the widgets being handled by the widget manager.
 */
int WidgetManager::calc_width() const
{
    const int NUM_WIDGETS = m_widgets.size();

    int wgt_width;
    int curr_width = 0;
    int total_width = 0;

    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        wgt_width = m_widgets[i].widget->m_width;
        curr_width += wgt_width;

        if( line_breaks(i) )
        {
            if( curr_width > total_width ) total_width = curr_width;
            curr_width = 0;
        }

    }
    if( curr_width > total_width ) total_width = curr_width;

    return total_width;
}

/** The calc_height() function retrieves the height of the smallest rectangle
 *  that contains all the widgets being handled by the widget manager.
 */
int WidgetManager::calc_height() const
{
    const int NUM_WIDGETS = m_widgets.size();
    if( NUM_WIDGETS < 0 ) return 0;

    int total_height = calc_line_height(0);

    for( int i = 1; i < NUM_WIDGETS; ++i )
    {
        if( line_breaks(i-1) )
        {
            total_height += calc_line_height(i);
        }
    }

    return total_height;
}

//-----------------------------------------------------------------------------
bool WidgetManager::layout(const WidgetArea POSITION)
{
    const int NUM_WIDGETS = m_widgets.size();
    if( NUM_WIDGETS < 0 ) return true;

    int SCREEN_WIDTH = user_config->m_width;
    int SCREEN_HEIGHT = user_config->m_height;

    int width;
    int height;
    //Resize the widgets.
    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        width = (SCREEN_WIDTH * m_widgets[i].min_width) / 100;
        height = (SCREEN_HEIGHT * m_widgets[i].min_height) / 100;

        m_widgets[i].widget->m_width = width;
        m_widgets[i].widget->m_height = height;
/* TEMP
 *
            m_widgets[i].widget->resize_to_text();
*/
    }

    const int WGTS_WIDTH = calc_width();
    const int WGTS_HEIGHT = calc_height();

    if( WGTS_WIDTH > SCREEN_WIDTH )
    {
        std::cerr << "WARNING: total width of the widgets is bigger than " <<
            "the screen, because the total minimum width given is bigger " <<
            "than 100%,\n";
    }
    if( WGTS_HEIGHT > SCREEN_HEIGHT )
    {
        std::cerr << "WARNING: total height of the widgets is bigger " <<
            "than the screen, because the total minimum height given is " <<
            "bigger than 100%.\n";
    }

    //To position things on the screen, remember that with OpenGL, in the
    //Y-axis the position 0 is in the bottom of the screen, just like the top
    //right quad of a cartesian plane.
    switch(POSITION)
    {
    case WGT_AREA_NW:
        m_x = 0;
        m_y = SCREEN_HEIGHT;
        break;

    case WGT_AREA_SW:
        m_x = 0;
        m_y = 0;
        break;

    case WGT_AREA_NE:
        m_x = SCREEN_WIDTH - WGTS_WIDTH;
        m_y = SCREEN_HEIGHT;
        break;

    case WGT_AREA_SE:
        m_x = SCREEN_WIDTH - WGTS_WIDTH;
        m_y = 0;
        break;

    case WGT_AREA_LFT:
        m_x = 0;
        m_y = (int)(SCREEN_HEIGHT * 0.5 + WGTS_HEIGHT * 0.5f );
        break;

    case WGT_AREA_RGT:
        m_x = SCREEN_WIDTH - WGTS_WIDTH;
        m_y = (int)(SCREEN_HEIGHT * 0.5 + WGTS_HEIGHT * 0.5f );
        break;

    case WGT_AREA_TOP:
        m_x = (int)(SCREEN_WIDTH * 0.5f - WGTS_WIDTH * 0.5f );
        m_y = SCREEN_HEIGHT;
        break;

    case WGT_AREA_BOT:
        m_x = (int)(SCREEN_WIDTH * 0.5f - WGTS_WIDTH * 0.5f );
        m_y = 0;
        break;

    //A layout of WGT_AREA_NONE should probably just do nothing.
    case WGT_AREA_NONE:
    case WGT_AREA_ALL:
        m_x = (int)(SCREEN_WIDTH * 0.5f - WGTS_WIDTH * 0.5f );
        m_y = (int)(SCREEN_HEIGHT * 0.5 + WGTS_HEIGHT * 0.5f );
        break;
    }

    int line_height = calc_line_height(0);

    //m_y should be the bottom value of the widgets, because that's the way
    //OpenGL handles the y-axis.
    m_y -= line_height;

//TODO: fix if m_x or m_y is bigger than the screen.

    //This formula seems not to have much theory behind it, we pick the
    //smallest from the screen height and width because if we pick the
    //biggest one, it might look bad for the smaller one, but it doesn't
    //happens the other way around, and it's divided by 60, maybe because
    //it results in small enough values to be of use, or maybe because it's
    //divided by 60 minutes?
    const int RADIUS = ( SCREEN_HEIGHT < SCREEN_WIDTH ? SCREEN_HEIGHT : SCREEN_WIDTH ) / 60;

    //The widgets positions given are for the lower left corner.
    int widget_x = m_x + ( WGTS_WIDTH - calc_line_width( 0 )) / 2;
    int widget_y = m_y;
    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        m_widgets[i].widget->m_x = widget_x;
        m_widgets[i].widget->m_y = widget_y + (line_height - m_widgets[i].widget->m_height);

        if( !(m_widgets[i].widget->create_rect(RADIUS)) )
        {
            return false;
        }

        if( i + 1 < NUM_WIDGETS )
        {
            if( line_breaks(i) )
            {
                line_height = calc_line_height(i+1);

                widget_y -= line_height;
                widget_x = m_x + ( WGTS_WIDTH - calc_line_width( i+1 )) / 2;
            }
            else
            {
                widget_x += m_widgets[i].widget->m_width;
            }
        }
    }

    //Always select the first active widget by default
    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        if( m_widgets[i].active) m_selected_wgt_token = m_widgets[i].token;

    }

    //Cleanups
    m_breaks.clear();
    restore_default_states();

    return true;
}

//-----------------------------------------------------------------------------
int WidgetManager::calc_line_width( const int START_WGT ) const
{
    int total_width = 0;
    const int NUM_WIDGETS = m_widgets.size();

    for( int i = START_WGT; i < NUM_WIDGETS; ++i )
    {
        total_width += m_widgets[i].widget->m_width;
        if( line_breaks(i) ) break;
    }

    return total_width;
}

//-----------------------------------------------------------------------------
int WidgetManager::calc_line_height( const int START_WGT ) const
{
    int line_height = 0;
    const int NUM_WIDGETS = m_widgets.size();

    for( int i = START_WGT; i < NUM_WIDGETS; ++i )
    {
        if( line_height < m_widgets[i].widget->m_height )
        {
            line_height = m_widgets[i].widget->m_height;
        }
        if( line_breaks(i) ) break;
    }

    return line_height;
}

//-----------------------------------------------------------------------------
void WidgetManager::set_selected_wgt(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE )
    {
        m_selected_wgt_token = TOKEN;
    }
    else std::cerr << "Tried to select unexistant widget with token " << TOKEN << '\n';
}


//-----------------------------------------------------------------------------
void WidgetManager::set_initial_activation_state( const bool ACTIVE)
{
    m_default_active = ACTIVE;
}

//-----------------------------------------------------------------------------
void WidgetManager::set_initial_rect_state
(
    const bool SHOW,
    const WidgetArea ROUND_CORNERS,
    const GLfloat* const COLOR
)
{
    m_default_show_rect = SHOW;
    m_default_rect_round_corners = ROUND_CORNERS;
    m_default_rect_color = COLOR;
}

//-----------------------------------------------------------------------------
void WidgetManager::set_initial_texture_state
(
    const bool SHOW,
    const int TEXTURE
)
{
    m_default_show_texture = SHOW;
    m_default_texture = TEXTURE;
}

//-----------------------------------------------------------------------------
void WidgetManager::set_initial_text_state
(
    const bool SHOW,
    const std::string TEXT,
    const WidgetFontSize SIZE,
    const Font::FontAlignType X_ALIGN,
    const Font::FontAlignType Y_ALIGN
)
{
    m_default_show_text = SHOW;
    m_default_text = TEXT;
    m_default_text_size = SIZE;
    m_default_text_x_alignment = X_ALIGN;
    m_default_text_y_alignment = Y_ALIGN;
}

//-----------------------------------------------------------------------------
void WidgetManager::set_initial_scroll_state
(
    const bool ENABLE,
/*    const int X_POS,*/
    const int Y_POS,
/*    const int X_SPEED,*/
    const int Y_SPEED
)
{
    m_default_enable_scroll = ENABLE;
/*    m_default_scroll_x_pos = X_POS;*/
    m_default_scroll_y_pos = Y_POS;
/*    m_default_scroll_x_speed = X_SPEED;*/
    m_default_scroll_y_speed = Y_SPEED;
}

//-----------------------------------------------------------------------------
void WidgetManager::restore_default_states()
{
    m_default_active = false;
    m_default_show_rect = false;
    m_default_rect_round_corners = WGT_AREA_NONE;
    m_default_rect_color = WGT_TRANS_BLACK;
    m_default_show_texture = false;
    m_default_texture = 0;
    m_default_show_text = false;
    m_default_text = "";
    m_default_text_size = WGT_FNT_MED;
    m_default_text_x_alignment = Font::ALIGN_CENTER;
    m_default_text_y_alignment = Font::ALIGN_CENTER;
    m_default_enable_scroll = false;
/*    m_default_scroll_x_pos = 0;*/
    m_default_scroll_y_pos = 0;
/*    m_default_scroll_x_speed = 0;*/
    m_default_scroll_y_speed = 0;
}

//-----------------------------------------------------------------------------
void WidgetManager::activate_wgt(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].active = true;
    else std::cerr << "Tried to activate unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
void WidgetManager::deactivate_wgt(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].active = false;
    else std::cerr << "Tried to deactivate unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_color(const int TOKEN, const GLfloat *COLOR)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_rect_color = COLOR;
    else std::cerr << "Tried to change the rect color of an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_round_corners(const int TOKEN, const WidgetArea CORNERS)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_round_corners = CORNERS;
    else std::cerr << "Tried to change the round corners of an unexistant widget with token " << TOKEN << '\n';
}
//-----------------------------------------------------------------------------
void WidgetManager::show_wgt_rect(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_rect = true;
    else std::cerr << "Tried to show the rect of an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
void WidgetManager::hide_wgt_rect(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_rect = false;
    else std::cerr << "Tried to hide the rect of an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
/*void WidgetManager::toggle_wgt_rect(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->toggle_rect();
    else std::cerr << "Tried to toggle the rect of an unexistant widget with token " << TOKEN << '\n';
}*/

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_texture(const int TOKEN, const int TEXTURE)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_texture = TEXTURE;
    else std::cerr << "Tried to set the texture of an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
void WidgetManager::show_wgt_texture(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_texture = true;
    else std::cerr << "Tried to show the texture of an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
void WidgetManager::hide_wgt_texture(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_texture = false;
    else std::cerr << "Tried to hide the texture of an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
/*void WidgetManager::toggle_wgt_texture(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->toggle_texture();
    else std::cerr << "Tried to toggle the texture of an unexistant widget with token " << TOKEN << '\n';
}
*/
//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_text( const int TOKEN, const char* TEXT )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_text = TEXT;
    else std::cerr << "Tried to set text to an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_text( const int TOKEN, const std::string TEXT )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_text = TEXT;
    else std::cerr << "Tried to set the text of an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_text_size( const int TOKEN, const WidgetFontSize SIZE)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_text_size = SIZE;
    else std::cerr << "Tried to set the text size of an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
void WidgetManager::show_wgt_text( const int TOKEN )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_text = true;
    else std::cerr << "Tried to show the text of an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
void WidgetManager::hide_wgt_text( const int TOKEN )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_text = false;
    else std::cerr << "Tried to hide the text of an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
/*void WidgetManager::toggle_wgt_text( const int TOKEN )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->toggle_text();
    else std::cerr << "Tried to toggle the text of an unexistant widget with token " << TOKEN << '\n';
}*/

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_text_x_alignment( const int TOKEN, const Font::FontAlignType ALIGN )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_text_x_alignment = ALIGN;
    else std::cerr << "Tried to set the X alignment of text of an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_text_y_alignment( const int TOKEN, const Font::FontAlignType ALIGN )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_text_y_alignment = ALIGN;
    else std::cerr << "Tried to set the Y alignment of text of an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
void WidgetManager::enable_wgt_scroll( const int TOKEN )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_scroll = true;
    else std::cerr << "Tried to enable scrolling of an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
void WidgetManager::disable_wgt_scroll( const int TOKEN )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_scroll = false;
    else std::cerr << "Tried to disable scrolling of an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
/*void WidgetManager::set_wgt_x_scroll_pos( const int TOKEN, const int POS )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_scroll_pos_x = POS;
    else std::cerr << "Tried to set the X scroll position of an unexistant widget with token " << TOKEN << '\n';
}*/

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_y_scroll_pos( const int TOKEN, const int POS )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_scroll_pos_y = POS;
    else std::cerr << "Tried to set the Y scroll position of an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
/*void WidgetManager::set_wgt_x_scroll_speed( const int TOKEN, const int SPEED )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_scroll_speed_x = SPEED;
    else std::cerr << "Tried to set the X scroll speed of an unexistant widget with token " << TOKEN << '\n';
}*/

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_y_scroll_speed( const int TOKEN, const int SPEED )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_scroll_speed_y = SPEED;
    else std::cerr << "Tried to set the Y scroll speed of an unexistant widget with token " << TOKEN << '\n';
}

/** pulse_widget() passes the pulse order to the right widget.
 */
void WidgetManager::pulse_wgt(const int TOKEN) const
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->pulse();
    else std::cerr << "Tried to pulse unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
void WidgetManager::lighten_wgt_color(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->lighten_color();
    else std::cerr << "Tried to lighten an unexistant widget with token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
void WidgetManager::darken_wgt_color(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->darken_color();
    else std::cerr << "Tried to darken an unexistant widget with token " << TOKEN << '\n';
}

/** The handle_mouse() function returns the current widget under the mouse
 *  pointer, if it's different from the selected widget. If the widget under
 *  the mouse is the selected widget, it returns WGT_NONE.
 */
int WidgetManager::handle_mouse(const int X, const int Y )
{
    //Search if the given x and y positions are on top of any widget. Please
    //note that the bounding box for each widget is used instead of the
    //real widget shape.

    //The search starts with the current selected widget(since it's most
    //probable that the mouse is on top of it )
    const int NUM_WIDGETS = m_widgets.size();

    if( m_selected_wgt_token != WGT_NONE )
    {
        const int SELECTED_WGT_ID = find_id(m_selected_wgt_token);

        if(( X > m_widgets[SELECTED_WGT_ID].widget->m_x ) &&
           ( X < m_widgets[SELECTED_WGT_ID].widget->m_x + m_widgets[SELECTED_WGT_ID].widget->m_width ) &&
           ( Y > m_widgets[SELECTED_WGT_ID].widget->m_y ) &&
           ( Y < m_widgets[SELECTED_WGT_ID].widget->m_y + m_widgets[SELECTED_WGT_ID].widget->m_height ))
        {
            return WGT_NONE;
        }
    }

    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        if(!(m_widgets[i].active)) continue;

        //Simple bounding box test
        if(( X > m_widgets[i].widget->m_x ) &&
           ( X < m_widgets[i].widget->m_x + m_widgets[i].widget->m_width ) &&
           ( Y > m_widgets[i].widget->m_y ) &&
           ( Y < m_widgets[i].widget->m_y + m_widgets[i].widget->m_height ))
        {
            m_selected_wgt_token = m_widgets[i].token;
            return m_selected_wgt_token;
        }
    }

    return WGT_NONE;
}

/** The handle_keyboard() function stores the current widget under the cursor
 *  after receiving input from a key.
 */
int WidgetManager::handle_keyboard(const int KEY)
{
    if( m_selected_wgt_token == WGT_NONE ) return WGT_NONE;

    int next_wgt = find_id(m_selected_wgt_token);
    //FIXME: eventually, the keys should not be hard coded
    switch (KEY)
    {
    case SDLK_LEFT:
        next_wgt = find_left_widget(find_id(m_selected_wgt_token));
        break;

    case SDLK_RIGHT:
        next_wgt = find_right_widget(find_id(m_selected_wgt_token));
        break;

    case SDLK_UP:
        next_wgt = find_top_widget(find_id(m_selected_wgt_token));
        break;

    case SDLK_DOWN:
        next_wgt = find_bottom_widget(find_id(m_selected_wgt_token));
        break;

    //FIXME: apparently, there are different codes for the + and -
    //near the numlock.
    case SDLK_PLUS:
    {
        const int ID = find_id(m_selected_wgt_token);
        if( m_widgets[ID].widget->m_enable_scroll )
        {
            //FIXME: these increases shouldn't be in pixels, but in percentages.
            //This should increase it by 1%, and the page buttons by 5%.
            m_widgets[ID].widget->m_scroll_speed_y -= 1;
        }
        break;
    }

    case SDLK_MINUS:
    {
        const int ID = find_id(m_selected_wgt_token);
        if( m_widgets[ID].widget->m_enable_scroll )
        {
            m_widgets[ID].widget->m_scroll_speed_y += 1;
        }
        break;
    }

    case SDLK_PAGEUP:
    {
        const int ID = find_id(m_selected_wgt_token);
        if( m_widgets[ID].widget->m_enable_scroll )
        {
            m_widgets[ID].widget->m_scroll_speed_y -= 5;
        }
        break;
    }

    case SDLK_PAGEDOWN:
    {
        const int ID = find_id(m_selected_wgt_token);
        if( m_widgets[ID].widget->m_enable_scroll )
        {
            m_widgets[ID].widget->m_scroll_speed_y += 5;
        }
        return WGT_NONE;
    }

    default: return WGT_NONE;
    }

    if( next_wgt == WGT_NONE) return WGT_NONE;

    m_selected_wgt_token = m_widgets[next_wgt].token;
    return m_selected_wgt_token;
}

/** The handle_joystick() function stores the current widget under the cursor
 *  after receiving input from the joystick.
 */
//FIXME: shouldn't direction and value be merged?
int WidgetManager::handle_joystick
(
    const int axis,
    const int direction,
    int value
)
{
    if( m_selected_wgt_token == WGT_NONE ) return WGT_NONE;

    int next_wgt = WGT_NONE; //This asignment is to prevent a compiler warning
    switch (axis)
    {
    case 0:
        if( direction == 0 )
        {
            next_wgt = find_left_widget(find_id(m_selected_wgt_token));
        }
        else if( direction == 1 )
        {
            next_wgt = find_right_widget(find_id(m_selected_wgt_token));
        }
        break;

    case 1:
        if( direction == 0 )
        {
            next_wgt = find_top_widget(find_id(m_selected_wgt_token));
        }
        else if( direction == 1 )
        {
            next_wgt = find_bottom_widget(find_id(m_selected_wgt_token));
        }
        break;

    default: return WGT_NONE;
    }

    if( next_wgt == find_id(m_selected_wgt_token) ) return WGT_NONE;

    m_selected_wgt_token = m_widgets[next_wgt].token;
    return m_selected_wgt_token;
}

/** find_left_widget() returns the closest widget to the left of START_WGT.
 */
int WidgetManager::find_left_widget(const int START_WGT) const
{
    const int NUM_WIDGETS = m_widgets.size();
    int closest_wgt = WGT_NONE;
    int closest_dist = user_config->m_width;
    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        if(!(m_widgets[i].active)) continue;

        if( m_widgets[i].widget->m_x + m_widgets[i].widget->m_width < m_widgets[START_WGT].widget->m_x + m_widgets[START_WGT].widget->m_width)
        {
            const int START_WGT_Y_CENTER = m_widgets[START_WGT].widget->m_y + m_widgets[START_WGT].widget->m_height / 2;
            const int START_WGT_X_CENTER = m_widgets[START_WGT].widget->m_x + m_widgets[START_WGT].widget->m_width / 2;
            const int CURR_WGT_Y_CENTER = m_widgets[i].widget->m_y + m_widgets[i].widget->m_height / 2;
            const int CURR_WGT_X_CENTER = m_widgets[i].widget->m_x + m_widgets[i].widget->m_width / 2;

            const int X_DIST = START_WGT_X_CENTER - CURR_WGT_X_CENTER;
            if( X_DIST > abs(CURR_WGT_Y_CENTER - START_WGT_Y_CENTER ))
            {
                if( closest_dist > X_DIST )
                {
                    closest_dist = X_DIST;
                    closest_wgt = i;
                }

            }
        }
#if 0
        //Check if the widget with the id i is to the left of the start widget.
        //To do so, simply checking if the X position of the i widget is smaller
        //than the start widget(divides the screen in a left and right blocks),
        //and that the distance between the two widgets in the X axis is bigger
        //than the distance in the Y axis(so that widgets on top and the
        //bottom are ignored).
        if( m_widgets[i].widget->m_x <= m_widgets[START_WGT].widget->m_x &&
            m_widgets[START_WGT].widget->m_x - m_widgets[i].widget->m_x >=
            abs(m_widgets[START_WGT].widget->m_y - m_widgets[i].widget->m_y ))
            {
                //FIXME: instead of using plib's routines, we should use functions
                //that works on integers that don't depend on external libraries.
                sgVec2 a;
                a[0] = m_widgets[START_WGT].widget->m_x;
                a[1] = m_widgets[START_WGT].widget->m_y;

                sgVec2 b;
                b[0] = m_widgets[i].widget->m_x;
                b[1] = m_widgets[i].widget->m_y;
                //Now that we know that the i widget is to the left, we have
                //to find if it's the closest one.
                if( closest_dist * closest_dist > sgDistanceSquaredVec2( a,b ) )
                {
                    closest_dist = (int)sgDistanceSquaredVec2( a,b );
                    closest_wgt = i;
                }

            }
#endif
    }

    return closest_wgt;
}

/** find_right_widget() returns the closest widget to the right of START_WGT
 */
int WidgetManager::find_right_widget(const int START_WGT) const
{
    const int NUM_WIDGETS = m_widgets.size();
    int closest_wgt = WGT_NONE;
    int closest_dist = user_config->m_width;
    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        if(!(m_widgets[i].active)) continue;

        //Check if the widget with the id i is on top of the start widget.
        //First it is checked if the y value of the i widget is higher than
        //the start widget(remember that for the widget manager a higher
        //value means that it is more on top). Then we add more precision:
        //if the vertical distance of the center of the two widgets is
        //smaller than the horizontal distance between the center of the two
        //widgets, then the i widget is definitely on top of the START_WGT
        //widget.
        if( m_widgets[i].widget->m_x > m_widgets[START_WGT].widget->m_x )
        {
            const int START_WGT_Y_CENTER = m_widgets[START_WGT].widget->m_y + m_widgets[START_WGT].widget->m_height / 2;
            const int START_WGT_X_CENTER = m_widgets[START_WGT].widget->m_x + m_widgets[START_WGT].widget->m_width / 2;
            const int CURR_WGT_Y_CENTER = m_widgets[i].widget->m_y + m_widgets[i].widget->m_height / 2;
            const int CURR_WGT_X_CENTER = m_widgets[i].widget->m_x + m_widgets[i].widget->m_width / 2;

            const int X_DIST = CURR_WGT_X_CENTER - START_WGT_X_CENTER;
            if( X_DIST > abs(CURR_WGT_Y_CENTER - START_WGT_Y_CENTER ))
            {
                if( closest_dist > X_DIST )
                {
                    closest_dist = X_DIST;
                    closest_wgt = i;
                }

            }
        }
#if 0
        //Check if the widget with the id i is to the right of the start widget.
        if( m_widgets[i].widget->m_x >= m_widgets[START_WGT].widget->m_x &&
            m_widgets[i].widget->m_x - m_widgets[START_WGT].widget->m_x >=
            abs(m_widgets[START_WGT].widget->m_y - m_widgets[i].widget->m_y ))
            {
                //FIXME: instead of using plib's routines, we should use functions
                //that works on integers that don't depend on external libraries.
                sgVec2 a;
                a[0] = m_widgets[START_WGT].widget->m_x;
                a[1] = m_widgets[START_WGT].widget->m_y;

                sgVec2 b;
                b[0] = m_widgets[i].widget->m_x;
                b[1] = m_widgets[i].widget->m_y;

                //Check if the widget found is closer than the closest
                //widget we have found
                if( closest_dist * closest_dist > sgDistanceSquaredVec2( a,b ) )
                {
                    closest_dist = (int)sgDistanceSquaredVec2( a,b );
                    closest_wgt = i;
                }

            }
#endif
    }

    return closest_wgt;
}
//FIXME: fix find_left_widget and find_right_widget.
/** find_top_widget() returns the closest widget on top of START_WGT.
 *  Remember that for the widget manager, the value 0 in the y-axis is in
 *  the bottom of the screen.
 */
int WidgetManager::find_top_widget(const int START_WGT) const
{
    const int NUM_WIDGETS = m_widgets.size();
    int closest_wgt = WGT_NONE;
    int closest_dist = user_config->m_height;
    for(int i = 0; i < NUM_WIDGETS; ++i)
    {
        if(!(m_widgets[i].active)) continue;

        //Check if the widget with the ID i is on top of the start widget.
        //First it is checked if the y value of the i widget is higher than
        //the start widget(remember that for the widget manager a higher
        //value means that it closer to the top of the screen). Then we add
        //more precision: if the vertical distance of the center of the two
        //widgets is smaller than the horizontal distance between the center
        //of the two widgets, then the i widget is definitely on top of the
        //START_WGT widget.
        if( m_widgets[i].widget->m_y > m_widgets[START_WGT].widget->m_y )
        {
            const int START_WGT_Y_CENTER = m_widgets[START_WGT].widget->m_y + m_widgets[START_WGT].widget->m_height / 2;
            const int START_WGT_X_CENTER = m_widgets[START_WGT].widget->m_x + m_widgets[START_WGT].widget->m_width / 2;
            const int CURR_WGT_Y_CENTER = m_widgets[i].widget->m_y + m_widgets[i].widget->m_height / 2;
            const int CURR_WGT_X_CENTER = m_widgets[i].widget->m_x + m_widgets[i].widget->m_width / 2;

            const int Y_DIST = CURR_WGT_Y_CENTER - START_WGT_Y_CENTER;
            if( Y_DIST > abs(CURR_WGT_X_CENTER - START_WGT_X_CENTER ))
            {
                if( closest_dist > Y_DIST )
                {
                    closest_dist = Y_DIST;
                    closest_wgt = i;
                }

            }
        }
    }

    return closest_wgt;
}

/** find_bottom_widget() returns the closest widget under START_WGT.
 *  Remember that for the widget manager, the value 0 in the y-axis is in
 *  the bottom of the screen.
 */
int WidgetManager::find_bottom_widget(const int START_WGT) const
{
    const int NUM_WIDGETS = m_widgets.size();
    int closest_wgt = WGT_NONE;
    int closest_dist = user_config->m_height;
    for(int i = 0; i < NUM_WIDGETS; ++i)
    {
        if(!(m_widgets[i].active)) continue;

        if( m_widgets[i].widget->m_y + m_widgets[i].widget->m_height < m_widgets[START_WGT].widget->m_y + m_widgets[START_WGT].widget->m_height)
        {
            const int START_WGT_Y_CENTER = m_widgets[START_WGT].widget->m_y + m_widgets[START_WGT].widget->m_height / 2;
            const int START_WGT_X_CENTER = m_widgets[START_WGT].widget->m_x + m_widgets[START_WGT].widget->m_width / 2;
            const int CURR_WGT_Y_CENTER = m_widgets[i].widget->m_y + m_widgets[i].widget->m_height / 2;
            const int CURR_WGT_X_CENTER = m_widgets[i].widget->m_x + m_widgets[i].widget->m_width / 2;

            const int Y_DIST = START_WGT_Y_CENTER - CURR_WGT_Y_CENTER;
            if( Y_DIST > abs(CURR_WGT_X_CENTER - START_WGT_X_CENTER ))
            {
                if( closest_dist > Y_DIST )
                {
                    closest_dist = Y_DIST;
                    closest_wgt = i;
                }

            }
        }
    }

    return closest_wgt;
}

