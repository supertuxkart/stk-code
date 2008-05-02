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

#ifdef __APPLE__
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

#include <iostream>
#include <cstdlib>
#include <iterator>
#include <algorithm>

WidgetManager *widget_manager;

const int WidgetManager::WGT_NONE = -1;

WidgetManager::WidgetManager() :
m_prev_layout_pos(WGT_AREA_NONE),
m_x( -1 ), m_y( -1 ),
m_selected_wgt_token( WGT_NONE ),
m_selection_change( false )
{
    restoreDefaultStates();
}

//-----------------------------------------------------------------------------
WidgetManager::~WidgetManager()
{
    reset();
}

//-----------------------------------------------------------------------------
bool WidgetManager::addWgt
(
    const int TOKEN,
    const int MIN_WIDTH,
    const int MIN_HEIGHT
)
{
    if( TOKEN != WGT_NONE && findId( TOKEN ) != WGT_NONE )
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
    new_id.min_radius = m_default_rect_radius;

    new_id.last_preset_scroll_x = m_default_scroll_preset_x;
    new_id.last_preset_scroll_y = m_default_scroll_preset_y;

    new_id.resize_to_text = m_default_resize_to_text;

    new_id.widget = new Widget(0, 0, 0, 0);

    new_id.widget->m_enable_rect = m_default_show_rect;
    new_id.widget->m_round_corners = m_default_rect_round_corners;
    new_id.widget->m_rect_color = m_default_rect_color;

    new_id.widget->m_border_percentage = m_default_border_percentage;
    new_id.widget->m_border_color = m_default_border_color;

    new_id.widget->m_enable_texture = m_default_show_texture;
    new_id.widget->m_texture = m_default_texture;

    new_id.widget->m_enable_text = m_default_show_text;
    new_id.widget->m_text.assign(m_default_text);
    new_id.widget->m_text_size = m_default_text_size;
    new_id.widget->setFont( m_default_font );
    new_id.widget->m_text_color = m_default_text_color;

    new_id.widget->m_enable_scroll  = m_default_enable_scroll;
    new_id.widget->m_scroll_pos_x   = (float)m_default_scroll_preset_x;
    new_id.widget->m_scroll_pos_y   = (float)m_default_scroll_preset_y;
    new_id.widget->m_scroll_speed_x = m_default_scroll_x_speed;
    new_id.widget->m_scroll_speed_y = m_default_scroll_y_speed;

    new_id.widget->m_enable_rotation = m_default_enable_rotation;
    new_id.widget->m_rotation_angle = m_default_rotation_angle;
    new_id.widget->m_rotation_speed = m_default_rotation_speed;

    new_id.widget->m_enable_track = m_default_show_track;
    new_id.widget->m_track_num = m_default_track_num;

    m_elems.push_back(WidgetElement(ET_WGT, (int)m_widgets.size()));
    m_widgets.push_back(new_id);

    return true;
}

//-----------------------------------------------------------------------------
bool WidgetManager::insertColumn()
{
    //FIXME: should we add a column specific break? it would be easier to read than
    //two breakLines().

    const int LAST_ELEM = (int)m_elems.size() - 1;
    const int LAST_WGT = (int)m_widgets.size() - 1;

    if( LAST_ELEM > -1 && m_elems[LAST_ELEM].type == ET_COLUMN )
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

    m_elems.push_back( WidgetElement( ET_COLUMN, 0));

    return true;
}

/** isColumnBreak() checks if the line break at the given position marks
 *  the end of a column, assuming that at the position given there is a line
 *  break. If there is no column returns false.
 */
bool WidgetManager::isColumnBreak( const int BREAK_POS) const
{
    for(int i = BREAK_POS - 1; i > -1; --i)
    {
        if (m_elems[i].type == ET_BREAK ) return false;
        else if( m_elems[i].type == ET_COLUMN ) return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool WidgetManager::breakLine()
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
            !isColumnBreak( LAST_ELEM ))
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

    restoreDefaultStates();

    m_selected_wgt_token = WGT_NONE;
    m_selection_change = false;
}

//-----------------------------------------------------------------------------
int WidgetManager::findId(const int TOKEN) const
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
    m_selection_change = false;

    //Enable 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, user_config->m_width, 0.0, user_config->m_height, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    glPushAttrib(GL_LIGHTING_BIT |
        GL_COLOR_BUFFER_BIT |
        GL_DEPTH_BUFFER_BIT |
        GL_STENCIL_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable (GL_STENCIL_TEST);

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

/** The calcWidth() function retrieves the width of the smallest rectangle
 *  that contains all the widgets being handled by the widget manager.
 */
int WidgetManager::calcWidth() const
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
            curr_width = calcColumnWidth(i);

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

/** The calcHeight() function retrieves the height of the smallest rectangle
 *  that contains all the widgets being handled by the widget manager.
 */
int WidgetManager::calcHeight() const
{
    const int NUM_ELEMS = (int)m_elems.size();
    if( NUM_ELEMS < 0 ) return 0;

    int total_height = calcLineHeight(0);
    for( int i = 1; i < NUM_ELEMS; ++i )
    {
        if( m_elems[i-1].type == ET_BREAK &&
            !isColumnBreak( i-1 ))
        {
            total_height += calcLineHeight(i);
        }
    }

    return total_height;
}

/** This argument-less layout() function serves as a recall to the other
 *  layout function in case you want to change the way widgets are put on
 *  the screen. It calls layout(POSITION) with the last given position,
 *  forces the recalculation of the scrolling position (since after the
 *  change the text might not fit properly the widget), and the selected
 *  widget, since layout(POSITION) function changes the selected widget to
 *  the first active widget by default.
 */
bool WidgetManager::layout()
{
    if( m_prev_layout_pos == WGT_AREA_NONE )
    {
        std::cerr << "WARNING: tried to call layout() with the previous " <<
            "layout position, but layout(WidgetArea POSITION) has never " <<
            "been called.\n";
        return false;
    }

    const int NUM_WIDGETS = (int)m_widgets.size();
    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        m_widgets[i].widget->m_scroll_pos_x =
            (float)m_widgets[i].last_preset_scroll_x;

        m_widgets[i].widget->m_scroll_pos_y =
            (float)m_widgets[i].last_preset_scroll_y;
    }

    const int PREV_SELECTED_WGT_TOKEN = m_selected_wgt_token;
    const int RESULT = layout(m_prev_layout_pos);
    if( RESULT == false ) return false;

    if( findId( PREV_SELECTED_WGT_TOKEN ) != WGT_NONE )
    {
        setSelectedWgtToken( PREV_SELECTED_WGT_TOKEN );
    }

    return true;
}

//-----------------------------------------------------------------------------
bool WidgetManager::layout(const WidgetArea POSITION)
{
    if( POSITION == WGT_AREA_NONE )
    {
        std::cerr << "WARNING: called layout with WGT_AREA_NONE.\n";
        return false;
    }

    m_prev_layout_pos = POSITION;

    const int NUM_WIDGETS = (int)m_widgets.size();
    if( NUM_WIDGETS < 1 ) return true;

    const int SCREEN_WIDTH = user_config->m_width;
    const int SCREEN_HEIGHT = user_config->m_height;

    int width, height;
    //Set the widgets' rect shape properties in pixels.
    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        width = (int)(SCREEN_WIDTH * m_widgets[i].min_width * 0.01);
        height = (int)(SCREEN_HEIGHT * m_widgets[i].min_height * 0.01);

        m_widgets[i].widget->m_width = width;
        m_widgets[i].widget->m_height = height;

        if( m_widgets[i].resize_to_text ) m_widgets[i].widget->
            resizeToText();

        if( width < height )
        {
            m_widgets[i].widget->m_radius = (int)( m_widgets[i].min_radius *
                m_widgets[i].widget->m_width * 0.01 );
        }
        else
        {
            m_widgets[i].widget->m_radius = (int)( m_widgets[i].min_radius *
                m_widgets[i].widget->m_height * 0.01 );
        }

        if( m_widgets[i].widget->m_radius < 1 )
        {
            m_widgets[i].widget->m_radius = 1;
        }
    }

    const int WGTS_WIDTH = calcWidth();
    const int WGTS_HEIGHT = calcHeight();

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
    int widget_x = m_x + ( WGTS_WIDTH - calcLineWidth( 0 )) / 2;
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
                const int CENTERED_POS = ( calcColumnWidth( column_pos ) -
                    m_widgets[curr_wgt].widget->m_width) / 2;
                widget_x += CENTERED_POS;
            }

            //Move the position to the widget's bottom left corner, since
            //that's what the createRect() function expects.
            widget_y -= m_widgets[curr_wgt].widget->m_height;

            //Assign the widget's position
            m_widgets[curr_wgt].widget->m_x = widget_x;
            m_widgets[curr_wgt].widget->m_y = widget_y;

            //Create widget's rect
            if( !(m_widgets[curr_wgt].widget->createRect()) )
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
                const int CENTERED_POS = ( calcColumnWidth( column_pos ) -
                    m_widgets[curr_wgt].widget->m_width) / 2;
                widget_x -= CENTERED_POS;
            }

            break;

        case ET_BREAK:
            if( column_pos == -1 )
            {
                //If we are not inside a column, move to the next line
                const int CENTERED_POS = (WGTS_WIDTH -
                    calcLineWidth( i+1 )) / 2;
                widget_x = m_x + CENTERED_POS;

                widget_y -= calcLineHeight(line_pos);

                line_pos = i + 1;
            }
            else
            {
                //If we are inside a column, move to the next widget in the
                //same line
                widget_x += calcColumnWidth(column_pos);
                widget_y += calcColumnHeight(column_pos);

                column_pos = -1;
            }
            break;

        case ET_COLUMN:
            column_pos = i;
            break;
        }
    }

    //Select the first active widget by default
    setSelectedWgtToken( WGT_NONE );

    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        if( m_widgets[i].active )
        {
            setSelectedWgtToken( m_widgets[i].token );
            break;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------
int WidgetManager::calcLineWidth( const int START_ELEM ) const
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
            total_width += calcColumnWidth(i);

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
int WidgetManager::calcLineHeight( const int START_ELEM ) const
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
            column_height = calcColumnHeight(i);

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
int WidgetManager::calcColumnWidth(const int START_ELEM) const
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
int WidgetManager::calcColumnHeight(const int START_ELEM) const
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
void WidgetManager::setSelectedWgt(const int TOKEN)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE )
    {
        setSelectedWgtToken( TOKEN );
    }
    else std::cerr << "WARNING: tried to select unnamed widget with " <<
        "token " << TOKEN << '\n';
}

//-----------------------------------------------------------------------------
bool WidgetManager::addTitleWgt
(
    const int TOKEN,
    const int MIN_WIDTH,
    const int MIN_HEIGHT,
    const std::string TEXT
)
{
    if( !( addWgt( TOKEN, MIN_WIDTH, MIN_HEIGHT ))) return false;

    showWgtRect( TOKEN );
    setWgtTextSize( TOKEN, WGT_FNT_LRG );
    showWgtText( TOKEN );
    setWgtText( TOKEN, TEXT );
    setWgtRoundCorners( TOKEN, WGT_AREA_ALL );
    setWgtCornerRadius( TOKEN, 20 );

    return true;
}

//-----------------------------------------------------------------------------
bool WidgetManager::addTextWgt
(
    const int TOKEN,
    const int MIN_WIDTH,
    const int MIN_HEIGHT,
    const std::string TEXT
)
{
    if( !( addWgt( TOKEN, MIN_WIDTH, MIN_HEIGHT ))) return false;

    showWgtRect( TOKEN );
    setWgtRoundCorners( TOKEN, WGT_AREA_ALL );
    setWgtCornerRadius( TOKEN, 20 );
    showWgtText( TOKEN );
    setWgtText( TOKEN, TEXT );

    return true;
}

//-----------------------------------------------------------------------------
bool WidgetManager::addTextButtonWgt
(
    const int TOKEN,
    const int MIN_WIDTH,
    const int MIN_HEIGHT,
    const std::string TEXT
)
{
    if( !( addWgt( TOKEN, MIN_WIDTH, MIN_HEIGHT ))) return false;

    showWgtRect( TOKEN );
    setWgtRoundCorners( TOKEN, WGT_AREA_ALL );
    setWgtCornerRadius( TOKEN, 20 );
    showWgtText( TOKEN );
    setWgtText( TOKEN, TEXT );
    activateWgt( TOKEN );

    return true;
}

//-----------------------------------------------------------------------------
bool WidgetManager::addImgWgt
(
    const int TOKEN,
    const int MIN_WIDTH,
    const int MIN_HEIGHT,
    const int IMG
)
{
    if( !( addWgt( TOKEN, MIN_WIDTH, MIN_HEIGHT ))) return false;

    setWgtColor( TOKEN, WGT_WHITE );
    showWgtRect( TOKEN );
    setWgtBorderPercentage( TOKEN, 5 );
    setWgtBorderColor( TOKEN, WGT_BLACK );
    showWgtBorder( TOKEN );
    setWgtTexture( TOKEN, IMG );
    showWgtTexture( TOKEN );

    return true;
}

//-----------------------------------------------------------------------------
bool WidgetManager::addImgButtonWgt
(
    const int TOKEN,
    const int MIN_WIDTH,
    const int MIN_HEIGHT,
    const int IMG
)
{
    if( !( addWgt( TOKEN, MIN_WIDTH, MIN_HEIGHT ))) return false;

    setWgtColor( TOKEN, WGT_GRAY );
    showWgtRect( TOKEN );
    setWgtRoundCorners( TOKEN, WGT_AREA_ALL );
    setWgtCornerRadius( TOKEN, 20 );
    setWgtTexture( TOKEN, IMG );
    showWgtTexture( TOKEN );
    activateWgt( TOKEN );

    return true;
}

//-----------------------------------------------------------------------------
void WidgetManager::setInitialActivationState( const bool ACTIVE)
{
    m_default_active = ACTIVE;
}

//-----------------------------------------------------------------------------
void WidgetManager::setInitialRectState
(
    const bool SHOW,
    const WidgetArea ROUND_CORNERS,
    const int RADIUS,
    const GLfloat* const COLOR
)
{
    m_default_show_rect = SHOW;
    m_default_rect_round_corners = ROUND_CORNERS;
    m_default_rect_radius = RADIUS;
    m_default_rect_color = COLOR;
}

//-----------------------------------------------------------------------------
void WidgetManager::setInitialBorderState
(
    const bool SHOW,
    const int PERCENTAGE,
    const GLfloat* const COLOR
)
{
    m_default_show_border = SHOW;
    m_default_border_percentage = PERCENTAGE * 0.01f;
    m_default_border_color = COLOR;
}

//-----------------------------------------------------------------------------
void WidgetManager::setInitialTextureState
(
    const bool SHOW,
    const int TEXTURE
)
{
    m_default_show_texture = SHOW;
    m_default_texture = TEXTURE;
}

//-----------------------------------------------------------------------------
void WidgetManager::setInitialTextState
(
    const bool SHOW,
    const std::string TEXT,
    const WidgetFontSize SIZE,
    const WidgetFont FONT,
    const GLfloat* const COLOR,
    const bool RESIZE
)
{
    m_default_show_text = SHOW;
    m_default_text = TEXT;
    m_default_text_size = SIZE;
    m_default_font = FONT;
    m_default_text_color = COLOR;
    m_default_resize_to_text = RESIZE;
}

//-----------------------------------------------------------------------------
void WidgetManager::setInitialScrollState
(
    const bool ENABLE,
    const WidgetScrollPos X_POS,
    const WidgetScrollPos Y_POS,
    const int X_SPEED,
    const int Y_SPEED
)
{
    m_default_enable_scroll = ENABLE;
    m_default_scroll_preset_x = X_POS;
    m_default_scroll_preset_y = Y_POS;
    m_default_scroll_x_speed = X_SPEED;
    m_default_scroll_y_speed = Y_SPEED;
}

//-----------------------------------------------------------------------------
void WidgetManager::setInitialRotationState
(
        const bool ENABLE,
        const float ANGLE,
        const int SPEED
)
{
    m_default_enable_rotation = ENABLE;
    m_default_rotation_angle = ANGLE;
    m_default_rotation_speed = SPEED;
}

//-----------------------------------------------------------------------------
void WidgetManager::setInitialTrackState
(
    const bool SHOW,
    const int TRACK
)
{
    m_default_show_track = SHOW;
    m_default_track_num = TRACK;
}

//-----------------------------------------------------------------------------
void WidgetManager::restoreDefaultStates()
{
    //FIXME: maybe instead of 'default' these variables should be 'initial'
    m_default_active = false;
    m_default_show_rect = false;
    m_default_rect_color = WGT_TRANS_BLACK;
    m_default_rect_round_corners = WGT_AREA_NONE;
    m_default_rect_radius = 1;
    m_default_show_border = false;
    m_default_border_percentage = 0.0;
    m_default_border_color = WGT_TRANS_WHITE;
    m_default_show_texture = false;
    m_default_texture = 0;
    m_default_show_text = false;
    m_default_text = "";
    m_default_text_size = WGT_FNT_MED;
    m_default_font = WGT_FONT_GUI;
    m_default_text_color = WGT_WHITE;
    m_default_resize_to_text = false;
    m_default_enable_scroll = false;
    m_default_scroll_preset_x = WGT_SCROLL_CENTER;
    m_default_scroll_preset_y = WGT_SCROLL_CENTER;
    m_default_scroll_x_speed = 0;
    m_default_scroll_y_speed = 0;
    m_default_enable_rotation = false;
    m_default_rotation_angle = 0.0f;
    m_default_rotation_speed = 0;
    m_default_show_track = false;
    m_default_track_num = -1;
}

//-----------------------------------------------------------------------------
void WidgetManager::activateWgt(const int TOKEN)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].active = true;
    else
    {
        std::cerr << "WARNING: tried to activate unnamed widget with token "
            << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::deactivateWgt(const int TOKEN)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].active = false;
    else
    {
        std::cerr << "WARNING: tried to deactivate unnamed widget with " <<
            TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtColor(const int TOKEN, const GLfloat *COLOR)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_rect_color = COLOR;
    else
    {
        std::cerr << "WARNING: tried to change the rect color of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtRoundCorners(const int TOKEN, const WidgetArea CORNERS)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_round_corners = CORNERS;
    else
    {
        std::cerr << "WARNING: tried to change the round corners of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtCornerRadius(const int TOKEN, const int RADIUS)
{
    if( RADIUS > 50 )
    {
        std::cerr << "WARNING: tried to set the corner's radius " <<
            "percentage of a widget with token " << TOKEN << " to " <<
            "something bigger than 50% \n";
        return;
    }
    else if( RADIUS < 1 )
    {
        std::cerr << "WARNING: tried to set the corner's radius " <<
            "percentage of a widget with token " << TOKEN << " to " <<
            "something smaller than 1% \n";
        return;
    }

    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].min_radius = RADIUS;
    else
    {
        std::cerr << "WARNING: tried to change the corner radius of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::showWgtRect(const int TOKEN)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_rect = true;
    else
    {
        std::cerr << "WARNING: tried to show the rect of an unnamed widget "
            << "with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::hideWgtRect(const int TOKEN)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_rect = false;
    else
    {
        std::cerr << "WARNING: tried to hide the rect of an unnamed widget "
            << "with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtBorderColor(const int TOKEN, const GLfloat* const COLOR)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_border_color = COLOR;
    else
    {
        std::cerr << "WARNING: tried to change the border color of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtBorderPercentage(const int TOKEN, const int PERCENTAGE)
{
    if( PERCENTAGE > 100 )
    {
        std::cerr << "WARNING: tried to set the border's percentage of " <<
            "widget with token " << TOKEN << " to something bigger than " <<
            "100% \n";
        return;
    }
    else if( PERCENTAGE < 1 )
    {
        std::cerr << "WARNING: tried to set the border's percentage of " <<
            "widget with token " << TOKEN << " to something smaller than " <<
            "1% \n";
        return;
    }

    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_border_percentage = PERCENTAGE * 0.01f;
    else
    {
        std::cerr << "WARNING: tried to change the rect color of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::showWgtBorder(const int TOKEN)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_border = true;
    else
    {
        std::cerr << "WARNING: tried to show the border of an unnamed widget "
            << "with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::hideWgtBorder(const int TOKEN)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_border = false;
    else
    {
        std::cerr << "WARNING: tried to hide the border of an unnamed widget "
            << "with token " << TOKEN << '\n';
    }
}


//-----------------------------------------------------------------------------
void WidgetManager::setWgtTexture(const int TOKEN, const int TEXTURE)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_texture = TEXTURE;
    else
    {
        std::cerr << "WARNING: tried to set the texture of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtTexture(const int TOKEN, const char* FILENAME)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->setTexture( FILENAME );
    else
    {
        std::cerr << "WARNING: tried to set the texture of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::showWgtTexture(const int TOKEN)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_texture = true;
    else
    {
        std::cerr << "WARNING: tried to show the texture of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::hideWgtTexture(const int TOKEN)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_texture = false;
    else
    {
        std::cerr << "WARNING: tried to hide the texture of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtText( const int TOKEN, const char* TEXT )
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE )
    {
        m_widgets[ID].widget->m_text = TEXT;

        //Reset the scroll position, because it will be the wrong value if
        //new text has a different size
        m_widgets[ID].widget->m_scroll_pos_x = (float)m_widgets[ID].last_preset_scroll_x;
        m_widgets[ID].widget->m_scroll_pos_y = (float)m_widgets[ID].last_preset_scroll_y;
    }
    else
    {
        std::cerr << "WARNING: tried to set text to an unnamed widget " <<
            "with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtText( const int TOKEN, const std::string TEXT )
{
    setWgtText( TOKEN, TEXT.c_str());
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtTextSize( const int TOKEN, const WidgetFontSize SIZE)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_text_size = SIZE;
    else
    {
        std::cerr << "WARNING: tried to set the text size of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtFont( const int TOKEN, const WidgetFont FONT )
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->setFont( FONT );
    else
    {
        std::cerr << "WARNING: tried to set the font of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtTextColor( const int TOKEN, const GLfloat* const COLOR)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_text_color = COLOR;
    else
    {
        std::cerr << "WARNING: tried to set the text color of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtResizeToText( const int TOKEN, const bool RESIZE )
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].resize_to_text = RESIZE;
    else
    {
        std::cerr << "WARNING: tried to set the resize to text value of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::showWgtText( const int TOKEN )
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_text = true;
    else
    {
        std::cerr << "WARNING: tried to show the text of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::hideWgtText( const int TOKEN )
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_text = false;
    else
    {
        std::cerr << "WARNING: tried to hide the text of an unnamed widget " <<
            "with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::enableWgtScroll( const int TOKEN )
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_scroll = true;
    else
    {
        std::cerr << "WARNING: tried to enable scrolling of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::disableWgtScroll( const int TOKEN )
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_scroll = false;
    else
    {
        std::cerr << "WARNING: tried to disable scrolling of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtXScrollPos
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

    const int ID = findId(TOKEN);
    if( ID != WGT_NONE )
    {
        m_widgets[ID].widget->m_scroll_pos_x = (float)POS;
        m_widgets[ID].last_preset_scroll_x = POS;
    }
    else
    {
        std::cerr << "WARNING: tried to set the X scroll position of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtYScrollPos
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

    const int ID = findId(TOKEN);
    if( ID != WGT_NONE )
    {
        m_widgets[ID].widget->m_scroll_pos_y = (float)POS;
        m_widgets[ID].last_preset_scroll_y = POS;
    }
    else
    {
        std::cerr << "WARNING: tried to set the Y scroll position of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtXScrollSpeed( const int TOKEN, const int SPEED )
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_scroll_speed_x = SPEED;
    else
    {
        std::cerr << "WARNING: tried to set the X scroll speed of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtYScrollSpeed( const int TOKEN, const int SPEED )
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_scroll_speed_y = SPEED;
    else
    {
        std::cerr << "WARNING: tried to set the Y scroll speed of an " <<
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::enableWgtRotation( const int TOKEN )
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_rotation = true;
    else
    {
        std::cerr << "WARNING: tried to enable rotation of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::disableWgtRotation( const int TOKEN )
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_rotation = false;
    else
    {
        std::cerr << "WARNING: tried to disable rotation of an unnamed " <<
            "widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtRotationAngle( const int TOKEN, const float ANGLE )
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_rotation_angle = ANGLE;
    else
    {
        std::cerr << "WARNING: tried to set the rotation angle of an "
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtRotationSpeed( const int TOKEN, const int SPEED )
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_rotation_speed = SPEED;
    else
    {
        std::cerr << "WARNING: tried to set the rotation speed of an "
            "unnamed widget with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::showWgtTrack( const int TOKEN )
{
    const int ID = findId( TOKEN );
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_track = true;
    else
    {
        std::cerr << "WARNING: tried to show the track of an unnamed widget "
            << "with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::hideWgtTrack( const int TOKEN )
{
    const int ID = findId( TOKEN );
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_enable_track = false;
    else
    {
        std::cerr << "WARNING: tried to hide the track of an unnamed widget "
            << "with token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::setWgtTrackNum( const int TOKEN, const int TRACK )
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->m_track_num = TRACK;
    else
    {
        std::cerr << "WARNING: tried to set the track number of an unnamed "
            << "widget with token " << TOKEN << '\n';
    }
}

/** pulse_widget() passes the pulse order to the right widget.
*/
void WidgetManager::pulseWgt(const int TOKEN) const
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->pulse();
    else
    {
        std::cerr << "WARNING: tried to pulse unnamed widget with token " <<
            TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::lightenWgtColor(const int TOKEN)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->lightenColor();
    else
    {
        std::cerr << "WARNING: tried to lighten an unnamed widget with " <<
            "token " << TOKEN << '\n';
    }
}

//-----------------------------------------------------------------------------
void WidgetManager::darkenWgtColor(const int TOKEN)
{
    const int ID = findId(TOKEN);
    if( ID != WGT_NONE ) m_widgets[ID].widget->darkenColor();
    else
    {
        std::cerr << "WARNING: tried to darken an unnamed widget with " <<
            "token " << TOKEN << '\n';
    }
}

/** The handlePointer() function returns the current widget under the
 *  pointer, if it's different from the selected widget. If the widget under
 *  the pointer is the selected widget, it returns WGT_NONE.
 */
int WidgetManager::handlePointer(const int X, const int Y )
{
    const int NUM_WGTS = (int)m_widgets.size();
    if( NUM_WGTS < 1 ) return WGT_NONE;

    //OpenGL provides a mechanism to select objects; simply 'draw' named
    //objects, and for each non-culled visible object a 'hit' will be saved
    //into a selection buffer. Objects are named by using OpenGL's name
    //stack. The information in each hit is the number of names in the name
    //stack, two hard-to-explain depth values that aren't used in this
    //function, and the contents of the name stack at the time of the hit.

    //This function loads 1 name into the stack (because if you pop an empty
    //stack you get an error), then uses glLoadName (which is a shortcut for
    //popping then pushing) to change the name. That means that each time a
    //hit is recorded, only the last drawn widget will be on the name stack.

    const int HIT_SIZE = 4; //1 Gluint for the number of names, 2 depth
                            //values, and 1 for the token of the widget.
    const int BUFFER_SIZE = HIT_SIZE * NUM_WGTS;

    GLuint* select_buffer = new GLuint[BUFFER_SIZE];
    glSelectBuffer(BUFFER_SIZE, select_buffer);

    glRenderMode(GL_SELECT);

    //Set the viewport to draw only what's under the mouse
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT,viewport);
    gluPickMatrix(X, Y, 1,1,viewport);
    glOrtho(0.0, user_config->m_width, 0.0, user_config->m_height, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);

    glInitNames();
    glPushName(WGT_NONE);

    glPushMatrix();

    for( int i = 0; i < NUM_WGTS; ++i )
    {
        if(!(m_widgets[i].active)) continue;

        glLoadName( i );

        glPushMatrix();
        m_widgets[i].widget->applyTransformations();
        //In case this ever becomes a performance bottleneck:
        //the m_rect_list includes texture coordinates, which are not
        //needed for selection.
        glCallList( m_widgets[i].widget->m_rect_list );
        glPopMatrix();
    }
    glFlush();
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glFlush();

    GLuint* position = select_buffer;
    const GLint NUM_HITS = glRenderMode(GL_RENDER);

    if( NUM_HITS > 0 )
    {
        float dist;
        float near_dist = 9999999.0f;
        int curr_wgt_id;
        int nearest_id = WGT_NONE;
        int wgt_x_center, wgt_y_center;
        for( int i = 0; i < NUM_HITS; ++i )
        {
            position += 3;
            curr_wgt_id = *position;

            wgt_x_center = m_widgets[curr_wgt_id].widget->m_x +
                m_widgets[i].widget->m_width / 2;
            wgt_y_center = m_widgets[curr_wgt_id].widget->m_y +
                m_widgets[i].widget->m_height / 2;

            //Check if it's the closest one to the mouse
            dist = (float)( abs(X - wgt_x_center) + abs(Y - wgt_y_center));
            if(dist < near_dist )
            {
                near_dist = dist;
                nearest_id = curr_wgt_id;
            }

            ++position;
        }
        delete[] select_buffer;

        if( m_widgets[nearest_id].token == m_selected_wgt_token )
        {
            return WGT_NONE;
        }

        setSelectedWgtToken( m_widgets[nearest_id].token );
        return m_selected_wgt_token;
    }

    delete[] select_buffer;
    return WGT_NONE;
}

/** The handle_<direction>() function stores the current widget under
 *  the cursor after receiving input from a key.
 */
int
WidgetManager::handleLeft()
{
    if( m_selected_wgt_token == WGT_NONE ) return WGT_NONE;

	return handleFinish(findLeftWidget(findId(m_selected_wgt_token)));
}

    int
WidgetManager::handleRight()
{
    if( m_selected_wgt_token == WGT_NONE ) return WGT_NONE;

	return handleFinish(findRightWidget(findId(m_selected_wgt_token)));
}

int
WidgetManager::handleUp()
{
    if( m_selected_wgt_token == WGT_NONE ) return WGT_NONE;

	return handleFinish(findTopWidget(findId(m_selected_wgt_token)));
}

int
WidgetManager::handleDown()
{
    if( m_selected_wgt_token == WGT_NONE ) return WGT_NONE;

	return handleFinish(findBottomWidget(findId(m_selected_wgt_token)));
}

int
WidgetManager::handleFinish(const int next_wgt)
{
    if( next_wgt == WGT_NONE)
		return WGT_NONE;

    setSelectedWgtToken( m_widgets[next_wgt].token );
	return m_selected_wgt_token;
}

void
WidgetManager::increaseScrollSpeed(const bool fast)
{
    //FIXME: to increase the scroll speed we substract, and to decrease
    //we add; this goes against logic, making code harder to read.
	const int ID = findId(m_selected_wgt_token);
	if( m_widgets[ID].widget->m_enable_scroll )
	{
        int &speed = m_widgets[ID].widget->m_scroll_speed_y;
		//FIXME: these increases shouldn't be in pixels, but in percentages.
		//This should increase it by 1%, and the page buttons by 5%.
        if( fast )
        {
            if( speed > 0 && speed < 50 ) speed = 0;
            else speed -= 50;
        }
        else
        {
            if( speed > 0 && speed < 10 ) speed = 0;
            else speed -= 10;
        }
	}
}

void
WidgetManager::decreaseScrollSpeed(const bool fast)
{
	const int ID = findId(m_selected_wgt_token);
	if( m_widgets[ID].widget->m_enable_scroll )
	{
        int &speed = m_widgets[ID].widget->m_scroll_speed_y;
		//FIXME: these increases shouldn't be in pixels, but in percentages.
		//This should increase it by 1%, and the page buttons by 5%.
        if( fast )
        {
            if( speed < 0 && speed > -50 ) speed = 0;
            else speed += 50;
        }
        else
        {
            if( speed < 0 && speed > -10 ) speed = 0;
            else speed += 10;
        }
	}
}

/** findLeftWidget() returns the closest widget to the left of START_WGT.
 *  We use the center of the widgets as the reference points; then, we
 *  filter any widget that is not to the left, and favor the ones that are
 *  closest in the Y-axis. If there is only one widget that is closest in the
 *  Y-axis, we pick that one as the closest, but if there is more than one
 *  widget with the same vertical distance, we have to break the tie by
 *  choosing the one closest in the X-axis.
 */
int WidgetManager::findLeftWidget(const int START_WGT) const
{
    const int NUM_WIDGETS = (int)m_widgets.size();
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

/** findRightWidget() returns the closest widget to the right of START_WGT
 */
int WidgetManager::findRightWidget(const int START_WGT) const
{
    const int NUM_WIDGETS = (int)m_widgets.size();
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
        //from the findLeftWidget() function
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
/** findTopWidget() returns the closest widget on top of START_WGT.
 *  Remember that for the widget manager, the value 0 in the y-axis is in
 *  the bottom of the screen.
 */
int WidgetManager::findTopWidget(const int START_WGT) const
{
    const int NUM_WIDGETS = (int)m_widgets.size();
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

/** findBottomWidget() returns the closest widget under START_WGT.
 *  Remember that for the widget manager, the value 0 in the y-axis is in
 *  the bottom of the screen.
 */
int WidgetManager::findBottomWidget(const int START_WGT) const
{
    const int NUM_WIDGETS = (int)m_widgets.size();
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
        //from the findTopWidget() function
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

//-----------------------------------------------------------------------------
void WidgetManager::setSelectedWgtToken(const int TOKEN)
{
    if( m_selected_wgt_token != TOKEN )
    {
        m_selected_wgt_token = TOKEN;
        m_selection_change = true;
    }
}

/** reloadFonts() sets the pointers to the fonts of the guis
 * to their choosen fonts; it's useful in cases where you
 * free the font's memory (which makes the pointers invalid),
 * then reload the fonts, like it happens when you change resolution
 * on the Macs or Windows.
 */
void WidgetManager::reloadFonts()
{
    const int NUM_WIDGETS = (int)m_widgets.size();
    for( int i = 0; i < NUM_WIDGETS; ++i )
    {
        m_widgets[i].widget->setFont(
            m_widgets[i].widget->m_curr_widget_font );
    }
}

