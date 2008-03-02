//  $Id: widget_set.cpp 1094 2007-05-21 06:49:06Z hiker $
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

#include "widget.hpp"

//FIXME: this should be removed when the scrolling is cleaned
#include "user_config.hpp"

#include "constants.hpp"
#include "track_manager.hpp"
#include "track.hpp"
#include <cmath>
#include <iostream>

const int Widget::MAX_SCROLL = 1000000;

const float Widget::MAX_TEXT_SCALE = 1.2f;
const float Widget::MIN_TEXT_SCALE = 1.0f;

const GLfloat WGT_WHITE  [4] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat WGT_GRAY   [4] = { 0.5f, 0.5f, 0.5f, 1.0f };
const GLfloat WGT_BLACK  [4] = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat WGT_YELLOW [4] = { 1.0f, 1.0f, 0.0f, 1.0f };
const GLfloat WGT_RED    [4] = { 1.0f, 0.0f, 0.0f, 1.0f };
const GLfloat WGT_GREEN  [4] = { 0.0f, 1.0f, 0.0f, 1.0f };
const GLfloat WGT_BLUE   [4] = { 0.0f, 0.0f, 1.0f, 1.0f };
const GLfloat WGT_TRANS_WHITE  [4] = { 1.0f, 1.0f, 1.0f, 0.5f };
const GLfloat WGT_TRANS_GRAY   [4] = { 0.5f, 0.5f, 0.5f, 0.5f };
const GLfloat WGT_TRANS_BLACK  [4] = { 0.0f, 0.0f, 0.0f, 0.5f };
const GLfloat WGT_TRANS_YELLOW [4] = { 1.0f, 1.0f, 0.0f, 0.5f };
const GLfloat WGT_TRANS_RED    [4] = { 1.0f, 0.0f, 0.0f, 0.5f };
const GLfloat WGT_TRANS_GREEN  [4] = { 0.0f, 1.0f, 0.0f, 0.5f };
const GLfloat WGT_TRANS_BLUE   [4] = { 0.0f, 0.0f, 1.0f, 0.5f };

//FIXME: I should change 'LIGHT' for 'LIT'.
const GLfloat WGT_LIGHT_GRAY   [4] = {1.0f, 1.0f, 1.0f, 1.0f};
const GLfloat WGT_LIGHT_BLACK  [4] = {0.5f, 0.5f, 0.5f, 1.0f};
const GLfloat WGT_LIGHT_YELLOW [4] = {1.0f, 1.0f, 0.5f, 1.0f};
const GLfloat WGT_LIGHT_RED    [4] = {1.0f, 0.5f, 0.5f, 1.0f};
const GLfloat WGT_LIGHT_GREEN  [4] = {0.5f, 1.0f, 0.5f, 1.0f};
const GLfloat WGT_LIGHT_BLUE   [4] = {0.5f, 0.5f, 1.0f, 1.0f};
const GLfloat WGT_LIGHT_TRANS_GRAY   [4] = {1.0f, 1.0f, 1.0f, 0.8f};
const GLfloat WGT_LIGHT_TRANS_BLACK  [4] = {0.5f, 0.5f, 0.5f, 0.8f};
const GLfloat WGT_LIGHT_TRANS_YELLOW [4] = {1.0f, 1.0f, 0.5f, 0.8f};
const GLfloat WGT_LIGHT_TRANS_RED    [4] = {1.0f, 0.5f, 0.5f, 0.8f};
const GLfloat WGT_LIGHT_TRANS_GREEN  [4] = {0.5f, 1.0f, 0.5f, 0.8f};
const GLfloat WGT_LIGHT_TRANS_BLUE   [4] = {0.5f, 0.5f, 1.0f, 0.8f};

const GLfloat WGT_TRANSPARENT [4] = {1.0f, 1.0f, 1.0f, 0.0f};

Widget::Widget
(
    const int X_,
    const int Y_,
    const int WIDTH_,
    const int HEIGHT_
) :
//Switch features are not set here to sane defaults because the WidgetManager
//handles that.
    m_x(X_), m_y(Y_),
    m_width(WIDTH_), m_height(HEIGHT_),
    m_rect_list(0),
    m_round_corners(WGT_AREA_ALL),
    m_scroll_pos_x(0), m_scroll_pos_y(0),
    m_text_scale(1.0f)
{
}

//-----------------------------------------------------------------------------
Widget::~Widget()
{
    if(glIsList(m_rect_list))
    {
        glDeleteLists(m_rect_list, 1);
    }
}

//-----------------------------------------------------------------------------
void Widget::update(const float DELTA)
{
    glPushMatrix();

    /* OpenGL transformations are affected by the order of the calls; but the
     * operations must be called in the inverse order that you want them to
     * be applied, since the calls are stacked, and the one at the top is
     * done first, till the one at the bottom.
     */

    glClear( GL_STENCIL_BUFFER_BIT );
    glTranslatef ( (GLfloat)(m_x + m_width * 0.5f), (GLfloat)(m_y + m_height * 0.5f), 0);

    m_rotation_angle += m_rotation_speed * DELTA;
    if( m_enable_rotation )
    {
        glRotatef( (GLfloat)m_rotation_angle, 0.0f, 0.0f, (GLfloat)1.0f );
    }



    /*Handle delta time dependant features*/
    if(m_text_scale > MIN_TEXT_SCALE)
    {
        m_text_scale -= MIN_TEXT_SCALE * DELTA;
        if(m_text_scale < MIN_TEXT_SCALE) m_text_scale = MIN_TEXT_SCALE;
    }

    /*Start handling of on/off features*/
    if(m_enable_texture)
    {
        glEnable(GL_TEXTURE_2D);
        if(glIsTexture(m_texture))
        {
            glBindTexture(GL_TEXTURE_2D, m_texture);
        }
        else
        {
            std::cerr << "Warning: widget tried to draw null texture.\n";
            std::cerr << "(Did you set the texture?)\n";
        }
    }
    else
    {
        //This ensures that a texture from another module doesn't affects the widget
        glDisable(GL_TEXTURE_2D);
    }

    if(glIsList(m_rect_list))
    {
        //FIXME: maybe there is some sort of stacking method to disable/enable
        //color masking
        if(!m_enable_rect)
        {
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        }
        else
        {
            glColor4fv(m_rect_color);
        }

        //FIXME: I should probably revert the values to the defaults within the widget manager
        //(if glPushAttrib() doesn't), but right now this is the only thing using the
        //stencil test anyways.
        glStencilFunc(GL_ALWAYS, 0x1, 0x1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glCallList(m_rect_list);

        if(!m_enable_rect)
        {
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
    }
    else
    {
        std::cerr << "Warning: widget tried to draw null rect list.\n";
        std::cerr << "(Did you created the rect?)\n";
    }

    if( m_enable_track )
    {
        if( m_track_num > (int)(track_manager->getTrackCount()) - 1)
        {
            std::cerr << "Warning: widget tried to draw a track with a " <<
                "number bigger than the amount of tracks available.\n";
        }

        if( m_track_num != -1 )
        {
            track_manager->getTrack( m_track_num )->drawScaled2D( 0.0f, 
                0.0f, (float)m_width, (float)m_height);
        }
        else
        {
            std::cerr << "Warning: widget tried to draw an unset track.\n";
        }
    }

    //For multilines we have to do a *very* ugly workaround for a plib
    //bug which causes multiline strings to move to the left, at least
    //while centering, and also gives wrong values for the size of the
    //text when there are multiple lines. Hopefully this work around will
    //be removed when we move away from plib; the scrolling and the other
    //text handling should be cleaned. Also, for some reason, different
    //positions are needed if the text is centered, and on top of that,
    //it's not 100% exact. Sorry for the mess.
    size_t line_end = 0;
    int lines = 0;

    do
    {
        line_end = m_text.find_first_of('\n', line_end + 1);
        ++lines;
    } while( line_end != std::string::npos );

    /* Handle preset scrolling positions */
    // In the Y-axis, a scroll position of 0 leaves the text centered, and
    // positive values lowers the text, and negatives (obviously) raise the
    // text, in the X-axis, a position of 0 leaves the text aligned to the
    // left; positive values move to the right and negative
    // values to the left.

    float left, right;
    m_font->getBBox(m_text.c_str(), m_text_size, false, &left, &right, NULL, NULL);
    int text_width = (int)(right - left + 0.99);


    const int Y_LIMIT = lines * m_text_size + m_height;

    //A work around for yet another bug with multilines: we get the wrong
    //width when using multilines.
    if( text_width > m_width )
    {
        text_width = m_width;
    }

    //With the preset positions, we do comparations with the equal sign on
    //floating point variables; however, no operations are done of the
    //variables between the assignment of these integer values and the
    //comparation and the values are small enough to fit in a few bytes,
    //so no inaccuracies because of floating point rounding should happen.
    //X-axis preset positions
    if( m_scroll_pos_x == WGT_SCROLL_START_LEFT )
    {
        m_scroll_pos_x = 0;
    }
    else if( m_scroll_pos_x == WGT_SCROLL_START_RIGHT )
    {
        m_scroll_pos_x = (float)m_width;
    }
    else if( m_scroll_pos_x == WGT_SCROLL_CENTER )
    {
        m_scroll_pos_x = (float)( (m_width - text_width) / 2 );
    }
    else if( m_scroll_pos_x == WGT_SCROLL_END_LEFT )
    {
        m_scroll_pos_x = (float)(-text_width);
    }
    else if( m_scroll_pos_x == WGT_SCROLL_END_RIGHT )
    {
        m_scroll_pos_x = (float)(m_width - text_width);
    }
    else if( m_scroll_pos_x > MAX_SCROLL )
    {
        std::cerr << "WARNING: text position is too much to the right to " <<
            "scroll!.\n";
    }
    else if( m_scroll_pos_x < -MAX_SCROLL )
    {
        std::cerr << "WARNING: text position is too much to the left to " <<
            "to scroll!.\n";
    }

    //Y-axis preset positions
    if( m_scroll_pos_y == WGT_SCROLL_START_TOP )
    {
        m_scroll_pos_y =(float)(Y_LIMIT / 2 - m_height);
    }
    else if( m_scroll_pos_y == WGT_SCROLL_START_BOTTOM )
    {
        m_scroll_pos_y = (float)(Y_LIMIT / 2);
    }
    else if( m_scroll_pos_y == WGT_SCROLL_CENTER )
    {
        m_scroll_pos_y = 0;
    }
    else if( m_scroll_pos_y == WGT_SCROLL_END_TOP )
    {
        m_scroll_pos_y = (float)(-Y_LIMIT / 2);
    }
    else if( m_scroll_pos_y == WGT_SCROLL_END_BOTTOM )
    {
        m_scroll_pos_y = (float)(-Y_LIMIT / 2 + m_height);
    }
    else if( m_scroll_pos_y > MAX_SCROLL )
    {
        std::cerr << "WARNING: text position too high to scroll!.\n";
    }
    else if( m_scroll_pos_y < -MAX_SCROLL )
    {
        std::cerr << "WARNING: text position too low to scroll!.\n";
    }

    if(m_enable_scroll)
    {
        //TODO: constrain speed to sane values
        m_scroll_pos_x += m_scroll_speed_x * DELTA;
        m_scroll_pos_y += m_scroll_speed_y * DELTA;

        //Y-axis wrapping
        if(m_scroll_pos_y * 2 > Y_LIMIT)
        {
            m_scroll_pos_y = WGT_SCROLL_END_TOP;
        }
        else if(-m_scroll_pos_y * 2 > Y_LIMIT)
        {
            m_scroll_pos_y = WGT_SCROLL_START_BOTTOM;
        }

        //X-axis wrapping
        if(m_scroll_pos_x > m_width )
        {
            m_scroll_pos_x = WGT_SCROLL_END_LEFT;
        }
        else if(m_scroll_pos_x < -text_width )
        {
            m_scroll_pos_x = WGT_SCROLL_START_RIGHT;
        }

    }

    if(m_enable_text)
    {
        if(m_text.empty())
        {
            std::cerr << "Warning: widget tried to print an empty string.\n";
            std::cerr << "(Did you set the text?)\n";
        }

        int x_pos = (int)(m_scroll_pos_x - m_width * 0.5f);
        int y_pos = - (int)m_scroll_pos_y + (lines - 1 )* m_text_size / 2;

        size_t line_start = 0;
        bool draw;

        glStencilFunc(GL_EQUAL,0x1,0x1);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        do
        {
            draw = true;
            if(y_pos + m_text_size / 2 > m_height / 2 )
            {
                if(y_pos - m_text_size / 2 >  m_height / 2) draw = false;
            }
            else if(y_pos + (m_height - m_text_size) / 2 < 0)
            {
                if(y_pos + (m_height + m_text_size) / 2 < 0) draw = false;
            }

            line_end = m_text.find_first_of('\n', line_start);

            if( draw )
            {
                glScalef(m_text_scale, m_text_scale, 1.0f);
                m_font->Print(m_text.substr(line_start, line_end - line_start).c_str(), m_text_size,
                    x_pos, y_pos - m_text_size / 2,
                    255, 255, 255, 1.0f, 1.0f);
                glScalef(1.0f/m_text_scale, 1.0f/m_text_scale, 1.0f);
            }

            y_pos -= m_text_size;
            line_start = line_end + 1;

        } while( line_end != std::string::npos );
    }
    glPopMatrix();
}

/** Initialize a display list containing a rectangle that can have rounded
 *  corners, with texture coordinates to properly apply a texture
 *  map to the rectangle as though the corners were not rounded . Returns
 *  false if the call to glGenLists failed, otherwise it returns true.
 */
bool Widget::createRect(int radius)
{
    //TODO: show warning if text > rect
    if(radius > m_width * 0.5)
    {
      //std::cerr << "Warning: widget's radius > half width.\n";
    }
    if(radius > m_height * 0.5)
    {
      // std::cerr << "Warning: widget's radius > half height.\n";
    }
    if(radius < 1)
    {
        std::cerr << "Warning: widget's radius < 1, setting to 1.\n";
        radius = 1;
    }

    if(m_width == 0)
    {
        std::cerr << "Warning: creating widget rect with width 0, " <<
            "setting to 1.\n";
        m_width = 1;
    }
    if(m_height == 0)
    {
        std::cerr << "Warning: creating widget rect with height 0, " <<
            "setting to 1.\n";
        m_height = 1;
    }

    if(!glIsList(m_rect_list))
    {
        m_rect_list = glGenLists(1);
        if(m_rect_list == 0)
        {
            std::cerr << "Error: could not create a widget's rect list.\n";
            return false;
        }
    }

    //Calculate the number of quads each side should have. The algorithm
    //isn't based on pure logic, instead it's based on the perception of
    //roundness and some visual testing.
    const int MIN_QUADS = 2;
    const float QUAD_RADIUS_RAISE = 0.413f;
    const int NUM_QUADS = MIN_QUADS + (int)(pow((float)radius, QUAD_RADIUS_RAISE));

    int i;

    glNewList(m_rect_list, GL_COMPILE);
    {
        //To create a rounded rectangle, we generate lines that are
        //progressively bigger, which are given as vertex points for each
        //quad.
        glBegin(GL_QUAD_STRIP);
        {
            //These are used to center the widget; without centering, the
            //widget's 0,0 coordinates are at the lower left corner.
            float half_width = m_width * 0.5f;
            float half_height = m_height * 0.5f;

            float angle;
            float circle_x, circle_y;
            float vertex_x, vertex_ya, vertex_yb;

            //Draw the left side of a rectangle.
            for (i = 0; i <= NUM_QUADS; ++i)
            {
                //To find the position in the X and Y axis of each point of
                //the quads, we use the property of the unit circle (a circle
                //with radius = 1) that at any given angle, cos(angle) is the
                //position of the unit circle at that angle in the X axis,
                //and that sin(angle) is the position of the unit circle at
                //that angle in the Y axis. Then the values from cos(angle)
                //and sin(angle) are multiplied by the radius.
                //
                //First we find the angle: since 2 * pi is the number of
                //radians in an entire circle, 0.5 * pi is a quarter of the
                //circle, which is a corner of the rounded rectangle. Based
                //on that, we just split the radians in a corner in NUM_QUADS
                //+ 1 parts, and use the angles at those parts to find the
                //X and Y position of the points.
                angle = 0.5f * M_PI * (float)i / (float)NUM_QUADS;
                circle_x = radius * cos(angle);
                circle_y = radius * sin(angle);

                //After we generate the positions in circle for the angles,
                //we have to position each rounded corner properly depending
                //on the position of the rectangle and the radius. The y
                //position for the circle is dependant on rect; if a corner
                //wasn't given, then the y position is computed as if it was
                //for a rectangle without rounder corners.
                vertex_x  = radius - circle_x;

                if( m_round_corners & WGT_AREA_NW )
                {
                    vertex_ya = m_height + circle_y - radius;
                }
                else
                {
                    vertex_ya = (float)m_height;
                }

                if( m_round_corners & WGT_AREA_SW )
                {
                    vertex_yb = radius - circle_y;
                }
                else
                {
                    vertex_yb = 0;
                }

                glTexCoord2f(vertex_x / m_width, vertex_ya / m_height);
                glVertex2f(vertex_x - half_width, vertex_ya - half_height);

                glTexCoord2f(vertex_x / m_width, vertex_yb / m_height);
                glVertex2f(vertex_x - half_width, vertex_yb - half_height);
            }

            //Draw the right side of a rectangle
            for (i = 0; i <= NUM_QUADS; ++i)
            {
                angle = 0.5f * M_PI * (float) i / (float) NUM_QUADS;

                //By inverting the use of sin and cos we get corners that are
                //drawn from left to right instead of right to left
                circle_x = radius * sin(angle);
                circle_y = radius * cos(angle);

                vertex_x  = m_width - radius + circle_x;

                if( m_round_corners & WGT_AREA_NE )
                {
                    vertex_ya = m_height + circle_y - radius;
                }
                else
                {
                    vertex_ya = (float)m_height;
                }

                if( m_round_corners & WGT_AREA_SE )
                {
                    vertex_yb = radius - circle_y;
                }
                else
                {
                    vertex_yb = 0;
                }


                glTexCoord2f(vertex_x / m_width, vertex_ya / m_height);
                glVertex2f(vertex_x - half_width, vertex_ya - half_height);

                glTexCoord2f(vertex_x / m_width, vertex_yb / m_height);
                glVertex2f(vertex_x - half_width, vertex_yb - half_height);
            }

        }
        glEnd();
    }
    glEndList();

    return true;
}

//-----------------------------------------------------------------------------
void Widget::resizeToText()
{
    if( !m_text.empty() )
    {
        float left, right, bottom, top;
        m_font->getBBox(m_text.c_str(), m_text_size, false, &left, &right, &bottom, &top);

        const int TEXT_WIDTH = (int)(right - left);
        const int TEXT_HEIGHT = (int)(top - bottom);

        if( TEXT_WIDTH > m_width ) m_width = TEXT_WIDTH;
        if( TEXT_HEIGHT > m_height ) m_height = TEXT_HEIGHT;
    }
}

//-----------------------------------------------------------------------------
/* Please note that this function only lightens 'non-light' colors */
void Widget::lightenColor()
{
    if(m_rect_color == WGT_GRAY)
    {
        m_rect_color = WGT_LIGHT_GRAY;
    }
    if(m_rect_color == WGT_BLACK)
    {
        m_rect_color = WGT_LIGHT_BLACK;
    }
    else if (m_rect_color == WGT_YELLOW)
    {
        m_rect_color = WGT_LIGHT_YELLOW;
    }
    else if (m_rect_color == WGT_RED)
    {
        m_rect_color = WGT_LIGHT_RED;
    }
    else if (m_rect_color == WGT_GREEN)
    {
        m_rect_color = WGT_LIGHT_GREEN;
    }
    else if (m_rect_color == WGT_BLUE)
    {
        m_rect_color = WGT_LIGHT_BLUE;
    }
    else if (m_rect_color == WGT_TRANS_GRAY)
    {
        m_rect_color = WGT_LIGHT_TRANS_GRAY;
    }
    else if (m_rect_color == WGT_TRANS_BLACK)
    {
        m_rect_color = WGT_LIGHT_TRANS_BLACK;
    }
    else if (m_rect_color == WGT_TRANS_YELLOW)
    {
        m_rect_color = WGT_LIGHT_TRANS_YELLOW;
    }
    else if (m_rect_color == WGT_TRANS_RED)
    {
        m_rect_color = WGT_LIGHT_TRANS_RED;
    }
    else if (m_rect_color == WGT_TRANS_GREEN)
    {
        m_rect_color = WGT_LIGHT_TRANS_GREEN;
    }
    else if (m_rect_color == WGT_TRANS_BLUE)
    {
        m_rect_color = WGT_LIGHT_TRANS_BLUE;
    }
}

//-----------------------------------------------------------------------------
/* Please note that this function only darkens 'light' colors. */
void Widget::darkenColor()
{
    if(m_rect_color == WGT_LIGHT_GRAY)
    {
        m_rect_color = WGT_GRAY;
    }
    if(m_rect_color == WGT_LIGHT_BLACK)
    {
        m_rect_color = WGT_BLACK;
    }
    else if (m_rect_color == WGT_LIGHT_YELLOW)
    {
        m_rect_color = WGT_YELLOW;
    }
    else if (m_rect_color == WGT_LIGHT_RED)
    {
        m_rect_color = WGT_RED;
    }
    else if (m_rect_color == WGT_LIGHT_GREEN)
    {
        m_rect_color = WGT_GREEN;
    }
    else if (m_rect_color == WGT_LIGHT_BLUE)
    {
        m_rect_color = WGT_BLUE;
    }
    else if (m_rect_color == WGT_LIGHT_TRANS_GRAY)
    {
        m_rect_color = WGT_TRANS_GRAY;
    }
    else if (m_rect_color == WGT_LIGHT_TRANS_BLACK)
    {
        m_rect_color = WGT_TRANS_BLACK;
    }
    else if (m_rect_color == WGT_LIGHT_TRANS_YELLOW)
    {
        m_rect_color = WGT_TRANS_YELLOW;
    }
    else if (m_rect_color == WGT_LIGHT_TRANS_RED)
    {
        m_rect_color = WGT_TRANS_RED;
    }
    else if (m_rect_color == WGT_LIGHT_TRANS_GREEN)
    {
        m_rect_color = WGT_TRANS_GREEN;
    }
    else if (m_rect_color == WGT_LIGHT_TRANS_BLUE)
    {
        m_rect_color = WGT_TRANS_BLUE;
    }
}

//-----------------------------------------------------------------------------
void Widget::setFont( const WidgetFont FONT )
{
    switch( FONT )
    {
        case WGT_FONT_GUI:
            m_font = font_gui;
            break;

        case WGT_FONT_RACE:
            m_font = font_race;
            break;
    };

    //TODO: the curr_widget_font variable exists only for a bug around; after
    //some restructuration, it should be fine to remove this.
    m_curr_widget_font = FONT;
}
