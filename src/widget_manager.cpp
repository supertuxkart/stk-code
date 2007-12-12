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

#include "gui/font.hpp"

//TEMP
#include <iostream>
#include <cstdlib>
#include <iterator>
#include <algorithm>

WidgetManager *widget_manager;

const int WidgetManager::WGT_NONE = -1;

WidgetManager::WidgetManager() :
prev_layout_pos(WGT_AREA_NONE), m_x( -1 ), m_y( -1 ), m_selected_wgt_token( WGT_NONE )
{
    init_fonts();
    restore_default_states();
}

//-----------------------------------------------------------------------------
WidgetManager::~WidgetManager()
{
    reset();
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

    new_id.widget->m_enable_scroll  = m_default_enable_scroll;
    new_id.widget->m_scroll_pos_x   = (float)m_default_scroll_x_pos;
    new_id.widget->m_scroll_pos_y   = (float)m_default_scroll_y_pos;
    new_id.widget->m_scroll_speed_x = (float)m_default_scroll_x_speed;
    new_id.widget->m_scroll_speed_y = (float)m_default_scroll_y_speed;

    m_elems.push_back(WidgetElement(ET_WGT, (int)m_widgets.size()));
    m_widgets.push_back(new_id);

    return true;
}

//-----------------------------------------------------------------------------
bool WidgetManager::insert_column()
{
    const int LAST_ELEM = (int)m_elems.size() - 1;
    const int LAST_WGT = (int)m_widgets.size() - 1;

    if( LAST_ELEM > -1)
    {
        if( m_elems[LAST_ELEM].type == ET_WGT )
        {
            std::cerr << "WARNING: tried to add a column after widget " <<
                "with token " << m_widgets[LAST_WGT].token << ".\n";
            return false;
        }
        else if ( m_elems[LAST_ELEM].type == ET_WGT )
        {
            if( LAST_WGT > -1 )
            {
                std::cerr << "WARNING: tried to add a column twice after " <<
                    "widget with token " << m_widgets[LAST_WGT].token <<
                    ".\n";
            }
            else
            {
                std::cerr << "WARNING: tried to add a column twice before" <<
                    " the first widget.\n";
            }
            return false;
        }
    }

    m_elems.push_back( WidgetElement( ET_COLUMN, 0));

    return true;
}

/** is_column_break() checks if the line break at the given position marks
 *  the end of a column, assuming that at the position given there is a line
 *  break. If there is no column returns false.
 */
bool WidgetManager::is_column_break( const int BREAK_POS) const
{
    for(int i = BREAK_POS - 1; i > -1; --i)
    {
        if (m_elems[i].type == ET_BREAK ) return false;
        else if( m_elems[i].type == ET_COLUMN ) return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool WidgetManager::break_line()
{
    const int LAST_WGT = (int)m_widgets.size() - 1;

    if( LAST_WGT < 0 )
    {
        std::cerr << "WARNING: tried to add a break before adding any " <<
            "widgets.\n";
        return false;
    }

    const int LAST_ELEM = (int)m_elems.size() - 1;
    if( m_elems[LAST_ELEM].type == ET_COLUMN )
    {
        std::cerr << "WARNING: tried to add a break to end a column " <<
            "with no widgets inside the column, after widget with " <<
            "token " << m_widgets[LAST_WGT].token << ".\n";

        m_elems.pop_back();
        return false;
    }

    if( LAST_ELEM > 0 )//If there are at least two elements
    {
        if( m_elems[LAST_ELEM].type == ET_BREAK &&
            !is_column_break( LAST_ELEM ))
        {
            std::cerr << "WARNING: tried to add an non-column line break "
                "twice after widget with token " <<
                m_widgets[LAST_WGT].token << ".\n";
            return false;
        }
    }

    m_elems.push_back(WidgetElement(ET_BREAK,0));

    return true;
}

//-----------------------------------------------------------------------------
void WidgetManager::reset()
{
    const int NUM_WIDGETS = (int)m_widgets.size();

    for(int i = 0; i < NUM_WIDGETS; ++i)
    {
        delete m_widgets[i].widget;
    }

    m_widgets.clear();
    m_elems.clear();

    restore_default_states();
}

//-----------------------------------------------------------------------------
int WidgetManager::find_id(const int TOKEN) const
{
    const int NUM_WIDGETS = (int)m_widgets.size();

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

    const int NUM_WIDGETS = (int)m_widgets.size();
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
    const int NUM_ELEMS = (int)m_elems.size();
    int curr_width = 0, total_width = 0;

    for( int i = 0; i < NUM_ELEMS; ++i )
    {
        switch( m_elems[i].type)
        {
        case ET_WGT:
            curr_width += m_widgets[ m_elems[i].pos ].widget->m_width;
            break;

        case ET_BREAK:
            if( curr_width > total_width ) total_width = curr_width;
            curr_width = 0;
            break;

        case ET_COLUMN:
            curr_width = calc_column_width(i);

            //Jump to the next line break
            while( i < NUM_ELEMS )
            {
                if( m_elems[i].type == ET_BREAK) break;
                ++i;
            }
            break;
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
    const int NUM_ELEMS = (int)m_elems.size();
    if( NUM_ELEMS < 0 ) return 0;

    int total_height = calc_line_height(0);
    for( int i = 1; i < NUM_ELEMS; ++i )
    {
        if( m_elems[i-1].type == ET_BREAK &&
            !is_column_break( i-1 ))
        {
            total_height += calc_line_height(i);
        }
    }

    return total_height;
}

//-----------------------------------------------------------------------------
bool WidgetManager::layout()
{
    if( prev_layout_pos == WGT_AREA_NONE )
    {
        std::cerr << "WARNING: tried to call layout() with the previous " <<
            "layout position, but layout(WidgetArea POSITION) has never " <<
            "been called.\n";
        return false;
    }

    return layout(prev_layout_pos);
}

//-----------------------------------------------------------------------------
bool WidgetManager::layout(const WidgetArea POSITION)
{
    if( POSITION == WGT_AREA_NONE )
    {
        std::cerr << "WARNING: called layout with WGT_AREA_NONE.\n";
        return false;
    }
    prev_layout_pos = POSITION;

    const int NUM_WIDGETS = (int)m_widgets.size();
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
            "than 100%.\n";
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

    case WGT_AREA_ALL:
        m_x = (int)(SCREEN_WIDTH * 0.5f - WGTS_WIDTH * 0.5f );
        m_y = (int)(SCREEN_HEIGHT * 0.5 + WGTS_HEIGHT * 0.5f );
        break;

    //This is just here to avoid a warning
    case WGT_AREA_NONE:
        break;
    }

    //This formula seems not to have much theory behind it, we pick the
    //smallest from the screen height and width because if we pick the
    //biggest one, it might look bad for the smaller one, but it doesn't
    //happens the other way around, and it's divided by 60, maybe because
    //it results in small enough values to be of use, or maybe because it's
    //divided by 60 minutes? The formula was taken from the old Widget Set.
    const int RADIUS = ( SCREEN_HEIGHT < SCREEN_WIDTH ? SCREEN_HEIGHT : SCREEN_WIDTH ) / 60;

    /* In this loop we give each widget it's true position on the screen and
     * create their rect; we start at the position where the first widget
     * will be, and move right first and down on breaks if the widget is
     * outside of a column, and inside columns we move down first and right
     * at breaks. The position used is the top left corner of the widget or
     * column, and is moved to the bottom left corner just to create the
     * rect, then we move to the top left corner of the next widget. Widgets
     * are centered in the X-axis around their line(if outside a column), or
     * around their column(if inside one), but always are stuck to the top
     * of their line/column.
     */

    //Position the first widget, these coordinates are for the top left
    //corner of the first widget.
    int widget_x = m_x + ( WGTS_WIDTH - calc_line_width( 0 )) / 2;
    int widget_y = m_y;

    const int NUM_ELEMS = (int)m_elems.size();
    int line_pos = 0;
    int column_pos = -1;
    int curr_wgt = 0;

    for( int i = 0; i < NUM_ELEMS; ++i )
    {
        switch( m_elems[i].type )
        {
        case ET_WGT:
            curr_wgt = m_elems[i].pos;

            if( column_pos != -1 )
            {
                //If we are inside a column, we need to recalculate the X
                //position so the widget is centered around the column
                const int CENTERED_POS = ( calc_column_width( column_pos ) -
                    m_widgets[curr_wgt].widget->m_width) / 2;
                widget_x += CENTERED_POS;
            }

            //Move the position to the widget's bottom left corner, since
            //that's what the create_rect() function expects.
            widget_y -= m_widgets[curr_wgt].widget->m_height;

            //Assign the widget's position
            m_widgets[curr_wgt].widget->m_x = widget_x;
            m_widgets[curr_wgt].widget->m_y = widget_y;

            //Create widget's rect
            if( !(m_widgets[curr_wgt].widget->create_rect(RADIUS)) )
            {
                return false;
            }

            if( column_pos == -1 )
            {
                //If we are not inside a column, move the position to the
                //top left corner of the next widget.
                widget_x += m_widgets[curr_wgt].widget->m_width;
                widget_y += m_widgets[curr_wgt].widget->m_height;
            }
            else
            {
                //If we are inside a column, we need to move back to the
                //columns' X position
                const int CENTERED_POS = ( calc_column_width( column_pos ) -
                    m_widgets[curr_wgt].widget->m_width) / 2;
                widget_x -= CENTERED_POS;
            }

            break;

        case ET_BREAK:
            if( column_pos == -1 )
            {
                //If we are not inside a column, move to the next line
                const int CENTERED_POS = (WGTS_WIDTH -
                    calc_line_width( i+1 )) / 2;
                widget_x = m_x + CENTERED_POS;

                widget_y -= calc_line_height(line_pos);

                line_pos = i + 1;
            }
            else
            {
                //If we are inside a column, move to the next widget in the
                //same line
                widget_x += calc_column_width(column_pos);
                widget_y += calc_column_height(column_pos);

                column_pos = -1;
            }
            break;

        case ET_COLUMN:
            column_pos = i;
            break;
        }
    }

    //Always select the first active widget by default
    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        if( m_widgets[i].active)
        {
            m_selected_wgt_token = m_widgets[i].token;
            break;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
int WidgetManager::calc_line_width( const int START_ELEM ) const
{
    int curr_wgt;
    int total_width = 0;
    const int NUM_ELEMS = (int)m_elems.size();

    for( int i = START_ELEM; i < NUM_ELEMS; ++i )
    {
        switch( m_elems[i].type )
        {
        case ET_WGT:
            curr_wgt = m_elems[i].pos;
            total_width += m_widgets[curr_wgt].widget->m_width;
            break;

        case ET_BREAK:
            return total_width;

        case ET_COLUMN:
            total_width += calc_column_width(i);

            while( i < NUM_ELEMS )
            {
                if( m_elems[i].type == ET_BREAK ) break;
                ++i;
            }
            break;
        }
    }

    return total_width;
}

//-----------------------------------------------------------------------------
int WidgetManager::calc_line_height( const int START_ELEM ) const
{
    int curr_wgt;
    int line_height = 0;
    int column_height;
    const int NUM_ELEMS = (int)m_elems.size();

    for( int i = START_ELEM; i < NUM_ELEMS; ++i )
    {
        switch( m_elems[i].type )
        {
        case ET_WGT:
            curr_wgt = m_elems[i].pos;
            if( line_height < m_widgets[curr_wgt].widget->m_height )
            {
                line_height = m_widgets[curr_wgt].widget->m_height;
            }
            break;

        case ET_BREAK:
            return line_height;

        case ET_COLUMN:
            column_height = calc_column_height(i);

            if( line_height < column_height )
            {
                line_height = column_height;
            }

            while( i < NUM_ELEMS )
            {
                if( m_elems[i].type == ET_BREAK ) break;
                ++i;
            }
        }
    }

    return line_height;
}

//-----------------------------------------------------------------------------
int WidgetManager::calc_column_width(const int START_ELEM) const
{
    int curr_wgt;
    int column_width = 0;
    const int NUM_ELEMS = (int)m_elems.size();

    for( int i = START_ELEM; i < NUM_ELEMS; ++i )
    {
        if( m_elems[i].type == ET_WGT )
        {
            curr_wgt = m_elems[i].pos;
            if( column_width < m_widgets[curr_wgt].widget->m_width )
            {
                column_width = m_widgets[curr_wgt].widget->m_width;
            }
        }
        else if (m_elems[i].type == ET_BREAK ) return column_width;
    }

    return column_width;
}

//-----------------------------------------------------------------------------
int WidgetManager::calc_column_height(const int START_ELEM) const
{
    int curr_wgt;
    int total_height = 0;
    const int NUM_ELEMS = (int)m_elems.size();

    for( int i = START_ELEM; i < NUM_ELEMS; ++i )
    {
        if( m_elems[i].type == ET_WGT )
        {
            curr_wgt = m_elems[i].pos;
            total_height += m_widgets[curr_wgt].widget->m_height;
        }
        else if (m_elems[i].type == ET_BREAK ) return total_height;
    }

    return total_height;
}

//-----------------------------------------------------------------------------
void WidgetManager::set_selected_wgt(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE )
    {
        m_selected_wgt_token = TOKEN;
    }
    else std::cerr << "WARNING: tried to select unnamed widget with " <<
        "token " << TOKEN << '\n';
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
    const WidgetFontSize SIZE
)
{
    m_default_show_text = SHOW;
    m_default_text = TEXT;
    m_default_text_size = SIZE;
}

//-----------------------------------------------------------------------------
void WidgetManager::set_initial_scroll_state
(
    const bool ENABLE,
    const int X_POS,
    const int Y_POS,
    const int X_SPEED,
    const int Y_SPEED
)
{
    m_default_enable_scroll = ENABLE;
    m_default_scroll_x_pos = X_POS;
    m_default_scroll_y_pos = Y_POS;
    m_default_scroll_x_speed = X_SPEED;
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
    m_default_enable_scroll = false;
    m_default_scroll_x_pos = WGT_SCROLL_CENTER;
    m_default_scroll_y_pos = WGT_SCROLL_CENTER;
    m_default_scroll_x_speed = 0;
    m_default_scroll_y_speed = 0;
}

//-----------------------------------------------------------------------------
void WidgetManager::activate_wgt(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].active = true;
    else
    {
        std::cerr << "WARNING: tried to activate unnamed widget with token "
            << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::deactivate_wgt(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].active = false;
    else
    {
        std::cerr << "WARNING: tried to deactivate unnamed widget with " <<
            TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_color(const int TOKEN, const GLfloat *COLOR)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_rect_color = COLOR;
    else
    {
        std::cerr << "WARNING: tried to change the rect color of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_round_corners(const int TOKEN, const WidgetArea CORNERS)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_round_corners = CORNERS;
    else
    {
        std::cerr << "WARNING: tried to change the round corners of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}
//-----------------------------------------------------------------------------
void WidgetManager::show_wgt_rect(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_rect = true;
    else
    {
        std::cerr << "WARNING: tried to show the rect of an unnamed widget "
            << "with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::hide_wgt_rect(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_rect = false;
    else
    {
        std::cerr << "WARNING: tried to hide the rect of an unnamed widget "
            << "with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
/*void WidgetManager::toggle_wgt_rect(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->toggle_rect();
    else std::cerr << "Tried to toggle the rect of an unnamed widget with token " << TOKEN << '\n';
}*/

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_texture(const int TOKEN, const int TEXTURE)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_texture = TEXTURE;
    else
    {
        std::cerr << "WARNING: tried to set the texture of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::show_wgt_texture(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_texture = true;
    else
    {
        std::cerr << "WARNING: tried to show the texture of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::hide_wgt_texture(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_texture = false;
    else
    {
        std::cerr << "WARNING: tried to hide the texture of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
/*void WidgetManager::toggle_wgt_texture(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->toggle_texture();
    else std::cerr << "Tried to toggle the texture of an unnamed widget with token " << TOKEN << '\n';
}
*/
//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_text( const int TOKEN, const char* TEXT )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_text = TEXT;
    else
    {
        std::cerr << "WARNING: tried to set text to an unnamed widget " <<
            "with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_text( const int TOKEN, const std::string TEXT )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_text = TEXT;
    else
    {
        std::cerr << "WARNING: tried to set the text of an unnamed widget with " <<
            "token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_text_size( const int TOKEN, const WidgetFontSize SIZE)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_text_size = SIZE;
    else
    {
        std::cerr << "WARNING: tried to set the text size of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::show_wgt_text( const int TOKEN )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_text = true;
    else
    {
        std::cerr << "WARNING: tried to show the text of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::hide_wgt_text( const int TOKEN )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_text = false;
    else
    {
        std::cerr << "WARNING: tried to hide the text of an unnamed widget " <<
            "with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
/*void WidgetManager::toggle_wgt_text( const int TOKEN )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->toggle_text();
    else std::cerr << "WARNING: tried to toggle the text of an unnamed widget with token " << TOKEN << '\n';
}*/

//-----------------------------------------------------------------------------
/*
void WidgetManager::set_wgt_text_x_alignment( const int TOKEN, const Font::FontAlignType ALIGN )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_text_x_alignment = ALIGN;
    else
    {
        std::cerr << "WARNING: tried to set the X alignment of text of " <<
            "an unnamed widget with token " << TOKEN << '\n';
    }
}
*/
//-----------------------------------------------------------------------------
/*void WidgetManager::set_wgt_text_y_alignment( const int TOKEN, const Font::FontAlignType ALIGN )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_text_y_alignment = ALIGN;
    else
    {
        std::cerr << "WARNING: tried to set the Y alignment of text of " <<
            "an unnamed widget with token " << TOKEN << '\n';
    }
}*/

//-----------------------------------------------------------------------------
void WidgetManager::enable_wgt_scroll( const int TOKEN )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_scroll = true;
    else
    {
        std::cerr << "WARNING: tried to enable scrolling of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::disable_wgt_scroll( const int TOKEN )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_scroll = false;
    else
    {
        std::cerr << "WARNING: tried to disable scrolling of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_x_scroll_pos
(
    const int TOKEN,
    const WidgetScrollPos POS
)
{
    if( POS == WGT_SCROLL_START_TOP || POS == WGT_SCROLL_START_BOTTOM ||
        POS == WGT_SCROLL_END_TOP || POS == WGT_SCROLL_END_BOTTOM )
    {
        std::cerr << "WARNING: tried to set the X scroll position to a " <<
            "position for the Y axis, on widget with token " << TOKEN <<
            '\n';
        return;
    }

    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_scroll_pos_x = POS;
    else
    {
        std::cerr << "WARNING: tried to set the X scroll position of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_y_scroll_pos
(
    const int TOKEN,
    const WidgetScrollPos POS
)
{
    if( POS == WGT_SCROLL_START_LEFT || POS == WGT_SCROLL_START_RIGHT ||
        POS == WGT_SCROLL_END_LEFT || POS == WGT_SCROLL_END_RIGHT )
    {
        std::cerr << "WARNING: tried to set the Y scroll position to a " <<
            "position for the X axis, on widget with token " << TOKEN <<
            '\n';
        return;
    }

    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_scroll_pos_y = POS;
    else
    {
        std::cerr << "WARNING: tried to set the Y scroll position of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_x_scroll_speed( const int TOKEN, const float SPEED )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_scroll_speed_x = SPEED;
    else
    {
        std::cerr << "WARNING: tried to set the X scroll speed of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::set_wgt_y_scroll_speed( const int TOKEN, const float SPEED )
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_scroll_speed_y = SPEED;
    else
    {
        std::cerr << "WARNING: tried to set the Y scroll speed of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}

/** pulse_widget() passes the pulse order to the right widget.
 */
void WidgetManager::pulse_wgt(const int TOKEN) const
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->pulse();
    else
    {
        std::cerr << "WARNING: tried to pulse unnamed widget with token " <<
            TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::lighten_wgt_color(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->lighten_color();
    else
    {
        std::cerr << "WARNING: tried to lighten an unnamed widget with " <<
            "token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::darken_wgt_color(const int TOKEN)
{
    const int ID = find_id(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->darken_color();
    else
    {
        std::cerr << "WARNING: tried to darken an unnamed widget with " <<
            "token " << TOKEN << '\n';
    }
}

/** The handle_pointer() function returns the current widget under the
 *  pointer, if it's different from the selected widget. If the widget under
 *  the pointer is the selected widget, it returns WGT_NONE.
 */
int WidgetManager::handle_pointer(const int X, const int Y )
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

/** The handle_<direction>() function stores the current widget under
 *  the cursor after receiving input from a key.
 */
int
WidgetManager::handle_left()
{
    if( m_selected_wgt_token == WGT_NONE ) return WGT_NONE;
	
	return handle_finish(find_left_widget(find_id(m_selected_wgt_token)));
}

int
WidgetManager::handle_right()
{
    if( m_selected_wgt_token == WGT_NONE ) return WGT_NONE;
	
	return handle_finish(find_right_widget(find_id(m_selected_wgt_token)));
}

int
WidgetManager::handle_up()
{
    if( m_selected_wgt_token == WGT_NONE ) return WGT_NONE;
	
	return handle_finish(find_top_widget(find_id(m_selected_wgt_token)));
}

int
WidgetManager::handle_down()
{
    if( m_selected_wgt_token == WGT_NONE ) return WGT_NONE;
	
	return handle_finish(find_bottom_widget(find_id(m_selected_wgt_token)));
}

int
WidgetManager::handle_finish(const int next_wgt)
{
    if( next_wgt == WGT_NONE)
		return WGT_NONE;
	
	m_selected_wgt_token = m_widgets[next_wgt].token;
	
	return m_selected_wgt_token;
}

void
WidgetManager::increase_scroll_speed(const bool fast)
{
	const int ID = find_id(m_selected_wgt_token);
	if( m_widgets[ID].widget->m_enable_scroll )
	{
		//FIXME: these increases shouldn't be in pixels, but in percentages.
		//This should increase it by 1%, and the page buttons by 5%.
		m_widgets[ID].widget->m_scroll_speed_y -= (fast) ? 5 : 1;
	}
}

void
WidgetManager::decrease_scroll_speed(const bool fast)
{
	const int ID = find_id(m_selected_wgt_token);
	if( m_widgets[ID].widget->m_enable_scroll )
	{
		//FIXME: these increases shouldn't be in pixels, but in percentages.
		//This should increase it by 1%, and the page buttons by 5%.
		m_widgets[ID].widget->m_scroll_speed_y += (fast) ? 5 : 1;
	}
}

/** find_left_widget() returns the closest widget to the left of START_WGT.
 *  We use the center of the widgets as the reference points; then, we
 *  filter any widget that is not to the left, and favor the ones that are
 *  closest in the Y-axis. If there is only one widget that is closest in the
 *  Y-axis, we pick that one as the closest, but if there is more than one
 *  widget with the same vertical distance, we have to break the tie by
 *  choosing the one closest in the X-axis.
 */
int WidgetManager::find_left_widget(const int START_WGT) const
{
    const int NUM_WIDGETS = m_widgets.size();
    int closest_wgt = WGT_NONE;
    int closest_x_dist = user_config->m_width;
    int closest_y_dist = user_config->m_height;

    const int START_WGT_Y_CENTER = m_widgets[START_WGT].widget->m_y +
        m_widgets[START_WGT].widget->m_height / 2;
    const int START_WGT_X_CENTER = m_widgets[START_WGT].widget->m_x +
        m_widgets[START_WGT].widget->m_width / 2;

    int curr_wgt_x_center, curr_wgt_y_center;
    int x_dist, y_dist;

    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        if(!(m_widgets[i].active)) continue;

        curr_wgt_y_center = m_widgets[i].widget->m_y + m_widgets[i].widget->m_height / 2;
        curr_wgt_x_center = m_widgets[i].widget->m_x + m_widgets[i].widget->m_width / 2;

        x_dist = START_WGT_X_CENTER - curr_wgt_x_center;
        y_dist = abs( curr_wgt_y_center - START_WGT_Y_CENTER );

        //Filter out all widgets that are not to the left and choose the
        //widget that is closest in the Y-axis
        if( x_dist > 0 && y_dist <= closest_y_dist )
        {
            closest_y_dist = y_dist;

            //If this is the first widget with this vertical distance, pick
            //it as the current closest widget
            if( y_dist != closest_y_dist )
            {
                closest_x_dist = user_config->m_width; //Reset the distance
                closest_wgt = i;
            }
            //If there is more than one widget with the same vertical
            //distance, choose the one that is closest in the X-axis
            else if( x_dist <= closest_x_dist )
            {
                closest_x_dist = x_dist;
                closest_wgt = i;
            }
        }
    }

    return closest_wgt;
}

//FIXME: find_right_widget() doesn't works properly yet
/** find_right_widget() returns the closest widget to the right of START_WGT
 */
int WidgetManager::find_right_widget(const int START_WGT) const
{
    const int NUM_WIDGETS = m_widgets.size();
    int closest_wgt = WGT_NONE;
    int closest_x_dist = user_config->m_width;
    int closest_y_dist = user_config->m_height;

    const int START_WGT_Y_CENTER = m_widgets[START_WGT].widget->m_y +
        m_widgets[START_WGT].widget->m_height / 2;
    const int START_WGT_X_CENTER = m_widgets[START_WGT].widget->m_x +
        m_widgets[START_WGT].widget->m_width / 2;

    int curr_wgt_x_center, curr_wgt_y_center;
    int x_dist, y_dist;

    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        if(!(m_widgets[i].active)) continue;

        curr_wgt_y_center = m_widgets[i].widget->m_y + m_widgets[i].widget->m_height / 2;
        curr_wgt_x_center = m_widgets[i].widget->m_x + m_widgets[i].widget->m_width / 2;

        //Notice that the order of this substraction is the *only* difference
        //from the find_left_widget() function
        x_dist = curr_wgt_x_center - START_WGT_X_CENTER;
        y_dist = abs( curr_wgt_y_center - START_WGT_Y_CENTER );

        if( x_dist > 0 && y_dist <= closest_y_dist )
        {
            closest_y_dist = y_dist;

            if( y_dist != closest_y_dist )
            {
                closest_x_dist = user_config->m_width;
                closest_wgt = i;
            }
            else if( x_dist <= closest_x_dist )
            {
                closest_x_dist = x_dist;
                closest_wgt = i;
            }
        }
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
    int closest_x_dist = user_config->m_width;
    int closest_y_dist = user_config->m_height;

    const int START_WGT_Y_CENTER = m_widgets[START_WGT].widget->m_y +
        m_widgets[START_WGT].widget->m_height / 2;
    const int START_WGT_X_CENTER = m_widgets[START_WGT].widget->m_x +
        m_widgets[START_WGT].widget->m_width / 2;

    int curr_wgt_x_center, curr_wgt_y_center;
    int x_dist, y_dist;

    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        if(!(m_widgets[i].active)) continue;

        curr_wgt_y_center = m_widgets[i].widget->m_y + m_widgets[i].widget->m_height / 2;
        curr_wgt_x_center = m_widgets[i].widget->m_x + m_widgets[i].widget->m_width / 2;

        y_dist = curr_wgt_y_center - START_WGT_Y_CENTER;
        x_dist = abs( curr_wgt_x_center - START_WGT_X_CENTER );

        //Filter out all widgets that are not on top and choose the
        //widget that is closest in the X-axis
        if( y_dist > 0 && x_dist <= closest_x_dist )
        {
            closest_x_dist = x_dist;

            //If this is the first widget with this vertical distance, pick
            //it as the current closest widget
            if( x_dist != closest_x_dist )
            {
                closest_y_dist = user_config->m_height;
                closest_wgt = i;
            }
            //If there is more than one widget with the same horizontal
            //distance, choose the one that is closest in the Y-axis
            else if( y_dist <= closest_y_dist )
            {
                closest_y_dist = y_dist;
                closest_wgt = i;
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
    int closest_x_dist = user_config->m_width;
    int closest_y_dist = user_config->m_height;

    const int START_WGT_Y_CENTER = m_widgets[START_WGT].widget->m_y +
        m_widgets[START_WGT].widget->m_height / 2;
    const int START_WGT_X_CENTER = m_widgets[START_WGT].widget->m_x +
        m_widgets[START_WGT].widget->m_width / 2;

    int curr_wgt_x_center, curr_wgt_y_center;
    int x_dist, y_dist;

    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        if(!(m_widgets[i].active)) continue;

        curr_wgt_y_center = m_widgets[i].widget->m_y + m_widgets[i].widget->m_height / 2;
        curr_wgt_x_center = m_widgets[i].widget->m_x + m_widgets[i].widget->m_width / 2;

        //Notice that the order of this substraction is the *only* difference
        //from the find_top_widget() function
        y_dist = START_WGT_Y_CENTER - curr_wgt_y_center;
        x_dist = abs( curr_wgt_x_center - START_WGT_X_CENTER );

        if( y_dist > 0 && x_dist <= closest_x_dist )
        {
            closest_x_dist = x_dist;

            if( x_dist != closest_x_dist )
            {
                closest_y_dist = user_config->m_height;
                closest_wgt = i;
            }
            else if( y_dist <= closest_y_dist )
            {
                closest_y_dist = y_dist;
                closest_wgt = i;
            }
        }
    }

    return closest_wgt;
}

