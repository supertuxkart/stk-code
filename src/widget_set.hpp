//  $Id$
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

#ifndef HEADER_WIDGETSET_H
#define HEADER_WIDGETSET_H

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#ifdef WIN32
#  include <windows.h>
#endif
#include <GL/gl.h>
#endif
#include <string>

/*---------------------------------------------------------------------------*/

enum WidgetFontSize { GUI_SML = 18, GUI_MED = 24, GUI_LRG = 30};

enum WidgetArea //One of the uses of this, is to say which corners are rounded
{
    GUI_NONE = 0,
    GUI_NW = 1, GUI_SW = 2, GUI_NE = 4, GUI_SE = 8,
    GUI_LFT = (GUI_NW  | GUI_SW),
    GUI_RGT = (GUI_NE  | GUI_SE),
    GUI_TOP = (GUI_NW  | GUI_NE),
    GUI_BOT = (GUI_SW  | GUI_SE),
    GUI_ALL = (GUI_TOP | GUI_BOT)
};

enum WidgetValue { GUI_OFF = 0, GUI_ON = 1 }; //For the widget values, for
                                              // on / off switch.

#define SCREEN_CENTERED_TEXT -10000


/*---------------------------------------------------------------------------*/

#define MAX_WIDGETS 256

#define GUI_TYPE 0xFFFE

enum GuiType
{
    GUI_FREE   = 0, //Unused widget

    GUI_STATE  = 1, //Button that has a state assigned.

    GUI_HARRAY = 2, //Horizantal container for groups of other types, all with
                    //the same size.

    GUI_VARRAY = 4, //Vertical container for groups of other types, all with
                    //the same.

    GUI_HSTACK = 8, //Horizontal container for groups of other types, each one
                    //on top of the other, with individual sizes.

    GUI_VSTACK = 16, //Vertical container for groups of other types, each one
                     //on top the other, with individual sizes.

    GUI_FILLER = 32, //Empty space, cannot be traveled through

    GUI_IMAGE  = 64, //Image widget using an OpenGL texture

    GUI_LABEL  = 128, //Single line text

    GUI_COUNT  = 256,

    GUI_CLOCK  = 512,
    GUI_SPACE  = 1024,//Empty space, can be traveled through
    GUI_PAUSE  = 2048
};

/*---------------------------------------------------------------------------*/

//previously in config.h:

#define MAX_STR 256

/*---------------------------------------------------------------------------*/

//TODO:fix widget set colors, it seems only white works.
const GLfloat gui_wht[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat gui_yel[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
const GLfloat gui_red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
const GLfloat gui_grn[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
const GLfloat gui_blu[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
const GLfloat gui_blk[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat gui_gry[4] = { 0.3f, 0.3f, 0.3f, 1.0f };

struct Widget
{
    int     type;
    int     token;//The token is a number supplied by the programmer to name
                  //recognize a widget. You could also store the id, but
                  //that wouldn't let you use a switch/enum combination.
    int     value;//On/Off switch using the WidgetValue.
    int     size;
    int     rect; //This uses the WidgetAreas, and determines which corners
                  //are rounded, GUI_ALL rounds all corners.
    int     x, y;
    int     w, h;
    int     yOffset;
    int     car;
    int     cdr;

    const char    *_text;
    char    *count_text;
    int     text_width;
    GLuint  text_img;
    GLuint  rect_obj;

    const GLfloat *color0;
    const GLfloat *color1;

    GLfloat  scale;
};

class WidgetSet
{
public:
    /*
     * Initialization/deinitialization functions
     */
    WidgetSet();
    ~WidgetSet();
    void reInit();   // necessary in case of fullscreen/window mode change on windows

    /*
     * Get/set functions
     */
    void set_label(int id, const char *);
    void set_multi(int id, const char *);
    void set_count(int id, int);
    void set_clock(int id, int);

    const char* get_label(int id) { return m_widgets[id]._text; }
    int  get_token(int id) const;
    int  get_value(int id) const;


    /*
     * Creation functions, the first argument is the parent id of the new
     * widget (0 if no parent)
     */
    //arrays, these are used to setup the layout of your widgets
    int  harray(int parent);
    int  varray(int parent);
    int  hstack(int parent);
    int  vstack(int parent);

    //fills up space
    int  filler(int parent);

    //a widget that consists of a texture (which must be completely
    //handled by the application)
    int  image(int parent, int, int, int, int rect=GUI_ALL);

    //a normal text menu entry, except that it is automatically immediately activated
    int  start(int parent, const char *, int, int, int value=GUI_OFF);

    //a normal text menu entry
    int  state(int parent, const char *, int, int, int value=GUI_OFF);

    //a text label (cannot be selected). c0 and c1 are two colours that the text is shaded with
    int  label(int parent, const char *text, int size=GUI_MED, int rect=GUI_ALL,
               const float *c0=0, const float *c1=0);

    //Create  a multi-line  text box  using a  vertical array  of labels.
    //Parse the  text for '\'  characters and treat them  as line-breaks.
    //Preserve the rect specifation across the entire array.
    int  multi(int parent, const char *, int size=GUI_MED, int rect=GUI_ALL,
               const float *c0=0, const float *c1=0);

    //widget is a single number - e.g. an fps counter or whatever
    int  count(int parent, int, int, int);

    //widget consists of a time in minutes and seconds
    int  clock(int parent, int, int, int);

    //just a blank space
    int  space(int parent);

    /*---------------------------------------------------------------------------*/

    /* prints out debugging info */
    void dump(int id, int);

    /* Use this after you have first created your widgets to set their
     * positioning xd and yd have possible values of 1(right/top), 0(center),
     * -1(left/bottom).
     */
    void layout(int id, int xd, int yd);

    int  search(int id, int, int);

    /*
    * Activate a widget, allowing it  to behave as a normal state widget.
    * This may  be used  to create  image buttons, or  cause an  array of
    * widgets to behave as a single state widget.
    */
    int  activate_widget(int id, int, int);

    /* you only need to call this for parents, children will automatically be deleted by their parents */
    int  delete_widget(int id);

    /*---------------------------------------------------------------------------*/

    /* call once a frame to update your widgets on the screen
       You only need to call this for parents, children will automatically be painted by their parents */
    void paint(int id);

    /* call "gui_pulse(gui_point(id, x, y), 1.2f);" whenever the mouse moves to make widgets pulse when the mouse goes over them */
    void pulse(int id, float);

    /* call once a frame, passing on the value given to BaseGUI::update(int)
       You only need to call this for parents, children will automatically be updated by their parents */
    void timer(int id, float);

    /* mouse movement */
    int  point(int id, int x, int y);

    /* joystick movement */
    int  stick(int id, int axis, int dir, int value);

    /* keyboard cursors */
    int cursor(int id, int key);

    /* mouse click */
    int click();

    /* where id's value is being used as a bool, this toggles it */
    void toggle(int id);

    //force id to be the current active widget
    void set_active(int id);

    /*---------------------------------------------------------------------------*/

    void tgl_paused();
    bool get_paused() const { return m_paused; }
    GLuint rect
    (
        const int X,
        const int Y,
        const int WIDTH,
        const int HEIGHT,
        const int RECT,
        const int RADIUS
    );

private:
    int hot(int id);

    GLuint list(int x, int y, int w, int h, const float *c0, const float *c1);
    int add_widget(int parent, int type);

    int pause(int parent);

    /*---------------------------------------------------------------------------*/

    void widget_up(int id);
    void harray_up(int id);
    void varray_up(int id);
    void hstack_up(int id);
    void vstack_up(int id);
    void paused_up(int id);
    void button_up(int id);

    void widget_dn(int id, int x, int y, int w, int h);
    void harray_dn(int id, int x, int y, int w, int h);
    void varray_dn(int id, int x, int y, int w, int h);
    void hstack_dn(int id, int x, int y, int w, int h);
    void vstack_dn(int id, int x, int y, int w, int h);
    void filler_dn(int id, int x, int y, int w, int h);
    void button_dn(int id, int x, int y, int w, int h);

    /*---------------------------------------------------------------------------*/

    void paint_rect(int id, int st);
    void paint_text(int id);;
    void paint_array(int id);
    void paint_image(int id);
    void paint_count(int id);
    void paint_clock(int id);
    void paint_label(int id);

    /*---------------------------------------------------------------------------*/

    int vert_test(int id, int jd);
    int horz_test(int id, int jd);

    int stick_L(int id, int dd);
    int stick_R(int id, int dd);
    int stick_D(int id, int dd);
    int stick_U(int id, int dd);

    /*---------------------------------------------------------------------------*/

    void config_push_persp(float, float, float);
    void config_push_ortho();
    void config_pop_matrix();

    /*---------------------------------------------------------------------------*/

    Widget m_widgets[MAX_WIDGETS];
    int           m_active;
    int           m_radius;

    int m_pause_id;
    bool m_paused;
};

extern WidgetSet *widgetSet;
#endif

/* EOF */
