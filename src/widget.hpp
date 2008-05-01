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

/* This file should only be used directly by the widget manager. Also, all
 * the coordinates in the widget.* and widget_manager.* are based on OpenGL,
 * which means that the 0 in the Y-axis is in the bottom, not the top.
 */

#ifndef HEADER_WIDGET_H
#define HEADER_WIDGET_H

// This include strings causes very many warning in the gui subdir:
// xlocnum(590) : warning C4312: 'type cast' : conversion from 'uintptr_t' to 'void *' of greater size
// These can apparently be removed by removing 64 bit compatibility warnings, or 
// just disable the warning during the include:
#if defined(WIN32) && !defined(__CYGWIN__)
#  pragma warning(disable:4312)
#endif
#include <string>
#if defined(WIN32) && !defined(__CYGWIN__)
#  pragma warning(default:4312)
#endif

#include "gui/font.hpp"

#ifdef __APPLE__
#  include <OpenGL/gl.h>
#else
#  ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#  endif
#  include <GL/gl.h>
#endif


enum WidgetFontSize { WGT_FNT_SML = 18, WGT_FNT_MED = 24, WGT_FNT_LRG = 30};

enum WidgetArea //One of the uses of this, is for rounded corners
{
    WGT_AREA_NONE = 0,
    WGT_AREA_NW = 1, WGT_AREA_SW = 2, WGT_AREA_NE = 4, WGT_AREA_SE = 8,
    WGT_AREA_LFT = (WGT_AREA_NW  | WGT_AREA_SW),
    WGT_AREA_RGT = (WGT_AREA_NE  | WGT_AREA_SE),
    WGT_AREA_TOP = (WGT_AREA_NW  | WGT_AREA_NE),
    WGT_AREA_BOT = (WGT_AREA_SW  | WGT_AREA_SE),
    WGT_AREA_ALL = (WGT_AREA_TOP | WGT_AREA_BOT)
};

enum WidgetFont
{
    WGT_FONT_GUI,
    WGT_FONT_RACE
};

//The lowest scroll values here must be bigger than
//Widget::MAX_SCROLL or lower than -Widget::MAX_SCROLL
enum WidgetScrollPos
{
    //For the X axis
    WGT_SCROLL_START_LEFT = 2000001,
    WGT_SCROLL_START_RIGHT = 2000002,
    WGT_SCROLL_END_LEFT = -2000001,
    WGT_SCROLL_END_RIGHT = -2000002,
    //For the Y axis
    WGT_SCROLL_START_TOP = 1000001,
    WGT_SCROLL_START_BOTTOM = 1000002,
    WGT_SCROLL_END_TOP = -1000001,
    WGT_SCROLL_END_BOTTOM = -1000002,
    //Works for both axis
    WGT_SCROLL_CENTER = 3000000
};

//I suggest that you do not use the white or light colors for the rects in
//most cases, because they don't have lighter versions that can be used to
//highlight those rects and then revert them, for example, when you select a
//widget. For textures, you should use WGT_WHITE usually, thought you can get
//nice effects by using other colors.
extern const GLfloat WGT_WHITE  [4];
extern const GLfloat WGT_GRAY  [4];
extern const GLfloat WGT_BLACK  [4];
extern const GLfloat WGT_YELLOW [4];
extern const GLfloat WGT_RED    [4];
extern const GLfloat WGT_GREEN  [4];
extern const GLfloat WGT_BLUE   [4];
extern const GLfloat WGT_TRANS_WHITE  [4];
extern const GLfloat WGT_TRANS_GRAY   [4];
extern const GLfloat WGT_TRANS_BLACK  [4];
extern const GLfloat WGT_TRANS_YELLOW [4];
extern const GLfloat WGT_TRANS_RED    [4];
extern const GLfloat WGT_TRANS_GREEN  [4];
extern const GLfloat WGT_TRANS_BLUE   [4];

extern const GLfloat WGT_LIGHT_GRAY   [4];
extern const GLfloat WGT_LIGHT_BLACK  [4];
extern const GLfloat WGT_LIGHT_YELLOW [4];
extern const GLfloat WGT_LIGHT_RED    [4];
extern const GLfloat WGT_LIGHT_GREEN  [4];
extern const GLfloat WGT_LIGHT_BLUE   [4];
extern const GLfloat WGT_LIGHT_TRANS_GRAY   [4];
extern const GLfloat WGT_LIGHT_TRANS_BLACK  [4];
extern const GLfloat WGT_LIGHT_TRANS_YELLOW [4];
extern const GLfloat WGT_LIGHT_TRANS_RED    [4];
extern const GLfloat WGT_LIGHT_TRANS_GREEN  [4];
extern const GLfloat WGT_LIGHT_TRANS_BLUE   [4];

extern const GLfloat WGT_TRANSPARENT [4];


class Widget
{
    //The only class that can access the Widget class is WidgetManager;
    //they are meant to always be used together, and the widgets should only
    //be used through the WidgetManager class.
    friend class WidgetManager;

    /* Basic widget properties that will always be used. */
    int  m_x, m_y;
    int  m_width, m_height;

    /* Low level features. They are off by default. */
    bool m_enable_rect;
    GLuint  m_rect_list; //A display list number that draws the rectangle with
                         //possibly rounded corners.
    const GLfloat *m_rect_color; //This const cannot change the value it points to, but it
                                 //can change where it points to.
    WidgetArea m_round_corners;

    bool m_enable_border;
    GLuint m_border_list; //Display list for the border
    float m_border_percentage;
    const GLfloat *m_border_color;

    bool m_enable_texture;
    GLuint m_texture;

    bool m_enable_text;
    std::string m_text;
    WidgetFontSize m_text_size;
    Font *m_font;
    const GLfloat *m_text_color;

    //TODO: This variable exists only to go around a bug; should be removed
    //after better restructuration.
    WidgetFont m_curr_widget_font;

    static const int MAX_SCROLL;
    bool m_enable_scroll;
    float m_scroll_pos_x;
    float m_scroll_pos_y;
    int m_scroll_speed_x;
    int m_scroll_speed_y;

    bool m_enable_rotation;
    float m_rotation_angle;
    int m_rotation_speed;


    //The widget calls the Track::drawScaled2D() function to draw a given
    //track, and m_track_num tells which track to draw.
    int m_enable_track;
    int m_track_num;

    /* High level, pattern following features; they deactivate themselves
     * after they follow their pattern, and might use low level features.*/
    static const float MAX_TEXT_SCALE;
    static const float MIN_TEXT_SCALE;
    float m_text_scale; //Used for the pulse effect

    Widget
    (
        const int X_,
        const int Y_,
        const int WIDTH_,
        const int HEIGHT_
    );
    ~Widget();

    void update(const float DELTA);

    /* Time limited features' functions. */
    void pulse() {m_text_scale = MAX_TEXT_SCALE;}

    /* Convenience functions. */
    void resizeToText(); //This checks if the widget is smaller than the
                           //text, and if so, changes the width and height.

    void lightenColor();
    void darkenColor();

    void setFont( const WidgetFont FONT );

    /* Functions created simply to organize the code */
    bool createRect(int radius);
    void updateVariables( const float DELTA );
    void draw();
    void applyTransformations();

};

#endif

/* EOF */
