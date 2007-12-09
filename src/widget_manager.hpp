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

#ifndef HEADER_WIDGET_MANAGER_H
#define HEADER_WIDGET_MANAGER_H

#include "widget.hpp"

#include <vector>

/* Here are some big-picture instructions about how to use this widget
 * manager: the extern widget_manager is a global interface to the class. Call
 * add_wgt() to specify the widgets you want, and for each widget specify the
 * details of it with the 'switch features', that can be changed between
 * show/hide, the initial setting for all of them is to be hidden. You will
 * usually have to call it's set_*() function, then the show_*() functions.
 * After you have defined all the widgets in the screen, call layout(), that
 * will do the actual work at creating the widgets. Call the activated
 * functions during the time the widgets are alive, and make sure that
 * update() is called each frame.
 *
 * You can use set_initial_*state() to avoid setting the state of the same
 * switch features with same values over and over; the default states are
 * reset when you call reset() or you can use reset_default_states().
 */


class WidgetManager
{
    struct WidgetID
    {
        int token;
        bool active; //If true, then this widget is interactive(though by
                       //definition, widgets are supposed to be interactive).

        //The percentages of the container this widget takes
        int min_width;
        int min_height;

        Widget *widget;
    };

    //The point of adding 'elements' is to use a vector to keep the order
    enum ElementTypes
    {
        ET_WGT,
        ET_BREAK,
        ET_COLUMN
    };

    /* I decided to waste one integer per break/column with the wgt_pos
     * variable inside the WidgetElement struct, since otherwise we
     * would need 2 vectors for breaks and columns, which would use more
     * memory, be slower and be more complex than this. -Coz
     */
    struct WidgetElement
    {
        ElementTypes type;
        int pos; //If the element is a widget, the position fo the widget
                 //in it's vector

        WidgetElement(ElementTypes _type, int _pos):type(_type), pos(_pos){};
    };

    std::vector<WidgetElement> m_elems;
    std::vector<WidgetID> m_widgets;

    WidgetArea prev_layout_pos;

    int m_x;
    int m_y;

    int m_selected_wgt_token;

    //TODO: change 'default' to 'initial'
    bool m_default_active;
    bool m_default_show_rect;
    bool m_default_rect_round_corners;
    const GLfloat *m_default_rect_color;
    bool m_default_show_texture;
    int m_default_texture;
    bool m_default_show_text;
    std::string m_default_text;
    WidgetFontSize m_default_text_size;

    bool m_default_enable_scroll;
    int m_default_scroll_x_pos;
    int m_default_scroll_y_pos;
    int m_default_scroll_x_speed;
    int m_default_scroll_y_speed;

    bool is_column_break( const int BREAK_POST ) const;

    int find_id(const int TOKEN) const;
    int calc_width() const;
    int calc_height() const;
    int calc_line_width(const int START_ELEM) const;
    int calc_line_height(const int START_ELEM) const;
    int calc_column_width(const int START_ELEM) const;
    int calc_column_height(const int START_ELEM) const;

    int find_left_widget(const int START_WGT) const;
    int find_right_widget(const int START_WGT) const;
    int find_top_widget(const int START_WGT) const;
    int find_bottom_widget(const int START_WGT) const;

	int handle_finish(const int);

public:
    //FIXME: maybe I should get this out of this class?
    static const int WGT_NONE;

    WidgetManager();
    ~WidgetManager();

    bool add_wgt
    (
        const int TOKEN, //A number that names the widget.
        const int MIN_WIDTH, //These values are percentages not pixels. 100%
                             //is the whole screen.
        const int MIN_HEIGHT
    );
    bool insert_column(); //This function changes the orientation from left to
                          //right and top to bottom of the widgets at line
                          //breaks, and switches it, making it from top to
                          //bottom, and left to right at a line break,
                          //until the next line break or reset() call. It can
                          //only be used right at the beginning
                          //of a line (that is, before any widgets have been
                          //created, or just after a line break).
    bool break_line();

    void reset();

    void update(const float DELTA);

    bool layout(); //This calls the other layout() function with the
                   //POSITION given to the previous call to any of the two
                   //layout functions. Fails if no previous call to the
                   //layout(POSITION) function has been done.
    bool layout( const WidgetArea POSITION );

    //TODO: make all get functions const
    int get_selected_wgt() const { return m_selected_wgt_token;}
    void set_selected_wgt(const int TOKEN);

    /* On/off widget switch features. They are all disabled/hidden initially. */
    void set_initial_activation_state( const bool ACTIVE);
    void set_initial_rect_state(const bool SHOW, const WidgetArea ROUND_CORNERS, const GLfloat* const COLOR );
    void set_initial_texture_state(const bool SHOW, const int TEXTURE );
    void set_initial_text_state
    (
        const bool SHOW,
        const std::string TEXT,
        const WidgetFontSize SIZE
    );
    void set_initial_scroll_state
    (
        const bool ENABLE,
        const int X_POS,
        const int Y_POS,
        const int X_SPEED,
        const int Y_SPEED
    );
    void restore_default_states();

    void activate_wgt(const int TOKEN);
    void deactivate_wgt(const int TOKEN);

    //FIXME: maybe this should be set_wgt_rect_color ? and put after the other rect funcs?
    void set_wgt_color(const int TOKEN, const GLfloat* const COLOR);
    void set_wgt_round_corners(const int TOKEN, const WidgetArea CORNERS);
    void show_wgt_rect(const int TOKEN);
    void hide_wgt_rect(const int TOKEN);
//    void toggle_wgt_rect(const int TOKEN);

    void set_wgt_texture(const int TOKEN, const int TEXTURE);
    void show_wgt_texture(const int TOKEN);
    void hide_wgt_texture(const int TOKEN);
//    void toggle_wgt_texture(const int TOKEN);

    void set_wgt_text( const int TOKEN, const char* TEXT );
    void set_wgt_text( const int TOKEN, const std::string TEXT );
    void set_wgt_text_size( const int TOKEN, const WidgetFontSize SIZE);
    void show_wgt_text( const int TOKEN );
    void hide_wgt_text( const int TOKEN );
//    void toggle_wgt_text( const int TOKEN );

    //FIXME: change to enable_wgt_scrolling, since it enables or disables
    //FIXME: maybe all that enabling the scrolling should do, is to allow
    //players to lower/raise it?
    //only the scrolling movement, not setting the scrolling position.
    void enable_wgt_scroll( const int TOKEN );
    void disable_wgt_scroll( const int TOKEN );
    void set_wgt_x_scroll_pos( const int TOKEN, const WidgetScrollPos POS );
    void set_wgt_y_scroll_pos( const int TOKEN, const WidgetScrollPos POS );
    void set_wgt_x_scroll_speed( const int TOKEN, const float SPEED );
    void set_wgt_y_scroll_speed( const int TOKEN, const float SPEED );

    /* Activated widget features. */
    void pulse_wgt( const int TOKEN ) const;

    /* Convenience widget functions. */
    void lighten_wgt_color(const int TOKEN);
    void darken_wgt_color(const int TOKEN);

    /* Input device handling. */
    int handle_pointer( const int X, const int Y );
    int handle_left();
    int handle_right();
    int handle_up();
    int handle_down();

	/* Scrolling modification. */
	void increase_scroll_speed(bool = false);
	void decrease_scroll_speed(bool = false);
};

extern WidgetManager *widget_manager;

#endif

/* EOF */
