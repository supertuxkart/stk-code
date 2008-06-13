//
//  SuperTuxKart - a fun racing game with go-kart
//  This code originally from Neverball copyright (C) 2003 Robert Kooima
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

#ifndef HEADER_WIDGET_MANAGER_H
#define HEADER_WIDGET_MANAGER_H

#include "widget.hpp"
#include <vector>


/* Big-picture instructions: the extern widget_manager is a global interface
 * to the class. Call add*Wgt() to insert new widgets, and change details
 * using the 'switch features'. By default no feature besides the one for
 * each widget type are enabled.
 *
 * After you defined the entire screen, call layout(), that creates and
 * places the widgets. Call the activated functions during the time the
 * widgets are alive, and make sure that update() is called each frame.
 *
 * To remove the widgets, you have to call reset().
 */

class WidgetManager
{
    struct WidgetID
    {
        int token;
        bool active; //If true, then this widget is interactive(though by
                       //definition, widgets are supposed to be interactive).

        //The percentages for the widget's rect
        int min_width;
        int min_height;
        int min_radius;

        //The last given preset scroll position is stored, to restore it in
        //case that the text is changed it needs to be restored.
        WidgetScrollPos last_preset_scroll_x;
        WidgetScrollPos last_preset_scroll_y;

        bool resize_to_text; //This has to do with layout, so it shouldn't
                             //inside the Widget class, even thought
                             //conceptually most people will asociate it with
                             //text

        Widget *widget;
    };

    enum ElementTypes
    {
        ET_WGT,
        ET_BREAK,
        ET_SWITCH //Switch orientation
    };

    /* I decided to waste one integer per break/switch with the pos
     * variable inside the WidgetElement struct, since otherwise we
     * would need 2 vectors for breaks and columns, which would use more
     * memory, be slower and more complex than this; another approach
     * is to use classes for each ElementType, but this most likely will also
     * waste more resources. -Coz
     */
    struct WidgetElement
    {
        ElementTypes type;
        int pos; //If the element is a widget, the position of the widget
                 //in it's vector

        WidgetElement(ElementTypes _type, int _pos):type(_type), pos(_pos){};
    };

    std::vector<WidgetElement> m_elems;
    std::vector<WidgetID> m_widgets;

    WidgetArea m_prev_layout_pos;

    int m_x;
    int m_y;

    int m_selected_wgt_token;
    bool m_selection_change;

    bool m_default_active;
    bool m_default_resize_to_text;

    bool m_default_show_rect;
    WidgetArea m_default_rect_round_corners;
    int m_default_rect_radius;
    const GLfloat *m_default_rect_color;

    bool m_default_show_border;
    float m_default_border_percentage;
    const GLfloat *m_default_border_color;

    bool m_default_show_texture;
    int m_default_texture;

    bool m_default_show_text;
    std::string m_default_text;
    WidgetFontSize m_default_text_size;
    WidgetFont m_default_font;
    const GLfloat* m_default_text_color;

    bool m_default_enable_scroll;
    WidgetScrollPos m_default_scroll_preset_x;
    WidgetScrollPos m_default_scroll_preset_y;
    int m_default_scroll_x_speed;
    int m_default_scroll_y_speed;

    bool m_default_enable_rotation;
    float m_default_rotation_angle;
    int m_default_rotation_speed;

    int m_default_show_track;
    int m_default_track_num;

    int findId(const int TOKEN) const;

    int calcLineWidth( const int POS );
    int calcLineHeight( const int POS );
    int calcColumnWidth( const int POS );
    int calcColumnHeight( const int POS );

    //These get* functions return the same thing as the above functions, but
    //they modify pos and set it to the position of the last element
    int getLineWidth( int& pos);
    int getLineHeight( int& pos );
    int getColumnWidth( int& pos );
    int getColumnHeight( int& pos );

    int calcLineX( const int POS );
    int calcColumnX( const int POS );

    int calcWidth();
    int calcHeight();

    bool layoutLine( int& x, int& y, int& pos );
    bool layoutColumn( int& x, int& y, int& pos );

    int findLeftWidget(const int START_WGT) const;
    int findRightWidget(const int START_WGT) const;
    int findTopWidget(const int START_WGT) const;
    int findBottomWidget(const int START_WGT) const;

	int handleFinish(const int);

    void setSelectedWgtToken(const int TOKEN);

public:
    //TODO: remove return values that we don't check
    static const int WGT_NONE;

    WidgetManager();
    ~WidgetManager();

    bool isEmpty() { return m_widgets.empty(); }

    bool addWgt
    (
        const int TOKEN, //A number that names the widget.
        const int MIN_WIDTH, //These values are percentages not pixels. 100%
                             //is the whole screen.
        const int MIN_HEIGHT
    );

    bool breakLine();
    void switchOrder(); //This changes the orientation from horizontal to
                        //vertical. It's reverted at line breaks. There are
                        //no situations where you cannot use a switch order
                        //so it should always succeed.

    void reset();

    void update(const float DELTA);

    bool layout(); //This calls the other layout() function with the
                   //POSITION given to the previous call to any of the two
                   //layout functions. Besides the conditions under the other
                   //layour() function fails, itm ay also fail if no previous
                   //call to the layout(POSITION) function has been done.
    bool layout( const WidgetArea POSITION );

    //TODO: make all get functions const
    int getSelectedWgt() const { return m_selected_wgt_token; }
    void setSelectedWgt(const int TOKEN);

    //Checks if the selected widget changed since the last call to update()
    bool selectionChanged() const { return m_selection_change; }

    /* Macro functions. They are widgets with special predefined values. */

    //FIXME: Temporal, till I rename addWgt() to addEmptyWgt()
    bool addEmptyWgt(const int TOKEN, const int MIN_WIDTH, const int MIN_HEIGHT) {return addWgt(TOKEN,MIN_WIDTH,MIN_HEIGHT);}

    //Widget that adds visible rect & text, rounded corners with 20% radius,
    //sets the text, and large font
    bool addTitleWgt
    (
        const int TOKEN,
        const int MIN_WIDTH,
        const int MIN_HEIGHT,
        const std::string TEXT
    );

    //Widget that adds visible rect & text, rounded corners with 20% radius,
    //and sets the text
    bool addTextWgt
    (
        const int TOKEN,
        const int MIN_WIDTH,
        const int MIN_HEIGHT,
        const std::string TEXT
    );

    //Widget that adds visible rect & text, rounded corners with 20% radius,
    //sets the text and is selectable
    bool addTextButtonWgt
    (
        const int TOKEN,
        const int MIN_WIDTH,
        const int MIN_HEIGHT,
        const std::string TEXT
    );

    //Widget that adds visible rect & image, white rect, 5% black
    //border, and sets the texture
    bool addImgWgt
    (
        const int TOKEN,
        const int MIN_WIDTH,
        const int MIN_HEIGHT,
        const int IMG
    );

    bool addImgWgt
    (
        const int TOKEN,
        const int MIN_WIDTH,
        const int MIN_HEIGHT,
        const char* FILENAME
    );


    //Selectable widget with visible rect & image, rounded corners with 20% radius,
    //gray rect and texture
    bool addImgButtonWgt
    (
        const int TOKEN,
        const int MIN_WIDTH,
        const int MIN_HEIGHT,
        const int IMG
    );

    bool addImgButtonWgt
    (
        const int TOKEN,
        const int MIN_WIDTH,
        const int MIN_HEIGHT,
        const std::string& FILENAME
    );

    /* On/off widget switch features. They are all disabled/hidden initially. */
    void setInitialActivationState( const bool ACTIVE);

    void setInitialRectState
    (
        const bool SHOW,
        const WidgetArea ROUND_CORNERS,
        const int RADIUS,
        const GLfloat* const COLOR
    );

    void setInitialTextureState(const bool SHOW, const int TEXTURE );

    void setInitialBorderState
    (
        const bool SHOW,
        const int PERCENTAGE,
        const GLfloat* const COLOR
    );

    void setInitialTextState
    (
        const bool SHOW,
        const std::string TEXT,
        const WidgetFontSize SIZE,
        const WidgetFont FONT,
        const GLfloat* const COLOR,
        const bool RESIZE_WGT
    );

    void setInitialScrollState
    (
        const bool ENABLE,
        const WidgetScrollPos X_POS,
        const WidgetScrollPos Y_POS,
        const int X_SPEED,
        const int Y_SPEED
    );

    void setInitialRotationState
    (
        const bool ENABLE,
        const float ANGLE,
        const int SPEED
    );


    void setInitialTrackState
    (
        const bool SHOW,
        const int TRACK
    );

    void restoreDefaultStates();

    void activateWgt(const int TOKEN);
    void deactivateWgt(const int TOKEN);

    //FIXME: maybe this should be setWgtRectColor ? and put after the other rect funcs?
    void setWgtColor(const int TOKEN, const GLfloat* const COLOR);
    void setWgtRoundCorners(const int TOKEN, const WidgetArea CORNERS);
    //The radius given is the percentage of the height or the width of the
    //widget, whatever is smaller; however, the maximum is 50% (the corner's
    //size).
    void setWgtCornerRadius(const int TOKEN, const int RADIUS);
    void showWgtRect(const int TOKEN);
    void hideWgtRect(const int TOKEN);

    void setWgtBorderColor(const int TOKEN, const GLfloat* const COLOR);
    void setWgtBorderPercentage(const int TOKEN, const int PERCENTAGE);
    void showWgtBorder(const int TOKEN);
    void hideWgtBorder(const int TOKEN);
    //TODO: add initial border colors, if I don't erase those functions.

    void setWgtTexture(const int TOKEN, const int TEXTURE);
    void setWgtTexture(const int TOKEN, const std::string& FILENAME, const bool is_full_path=true);
    void showWgtTexture(const int TOKEN);
    void hideWgtTexture(const int TOKEN);

    void setWgtText( const int TOKEN, const char* TEXT );
    void setWgtText( const int TOKEN, const std::string TEXT );
    void setWgtTextSize( const int TOKEN, const WidgetFontSize SIZE );
    void setWgtFont( const int TOKEN, const WidgetFont FONT );
    void setWgtTextColor( const int TOKEN, const GLfloat* const COLOR );
    void setWgtResizeToText( const int TOKEN, const bool RESIZE );
    void showWgtText( const int TOKEN );
    void hideWgtText( const int TOKEN );
    void resizeWgtToText( const int TOKEN );
    void reloadFonts();

    //FIXME: change to enableWgtScrolling, since it enables or disables
    //FIXME: maybe all that enabling the scrolling should do, is to allow
    //players to lower/raise it?
    //only the scrolling movement, not setting the scrolling position.
    void enableWgtScroll( const int TOKEN );
    void disableWgtScroll( const int TOKEN );
    void setWgtXScrollPos( const int TOKEN, const WidgetScrollPos POS );
    void setWgtYScrollPos( const int TOKEN, const WidgetScrollPos POS );
    void setWgtXScrollSpeed( const int TOKEN, const int SPEED );
    void setWgtYScrollSpeed( const int TOKEN, const int SPEED );

    void enableWgtRotation( const int TOKEN );
    void disableWgtRotation( const int TOKEN );
    void setWgtRotationAngle( const int TOKEN, const float ANGLE );
    void setWgtRotationSpeed( const int TOKEN, const int SPEED );

    void showWgtTrack( const int TOKEN );
    void hideWgtTrack( const int TOKEN );
    void setWgtTrackNum( const int TOKEN, const int TRACK );

    /* Activated widget features. */
    void pulseWgt( const int TOKEN ) const;

    /* Convenience widget functions. */
    void lightenWgtColor(const int TOKEN);
    void darkenWgtColor(const int TOKEN);

    /* Input device handling. */
    int handlePointer( const int X, const int Y );
    int handleLeft();
    int handleRight();
    int handleUp();
    int handleDown();

	/* Scrolling modification. */
	void increaseScrollSpeed(bool = false);
	void decreaseScrollSpeed(bool = false);
};

extern WidgetManager *widget_manager;

#endif

/* EOF */
