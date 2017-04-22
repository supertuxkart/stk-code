//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2009-2015 Marianne Gagnon
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


#ifndef HEADER_WIDGET_HPP
#define HEADER_WIDGET_HPP

#include <irrString.h>
namespace irr
{
    namespace gui { class IGUIElement; }
}
#include <map>

#include "guiengine/event_handler.hpp"
#include "guiengine/skin.hpp"
#include "utils/constants.hpp"
#include "utils/ptr_vector.hpp"

namespace GUIEngine
{

    class DynamicRibbonWidget;

    enum WidgetType
    {
        WTYPE_NONE = -1,
        WTYPE_RIBBON,
        WTYPE_SPINNER,
        WTYPE_BUTTON,
        WTYPE_ICON_BUTTON,
        WTYPE_CHECKBOX,
        WTYPE_LABEL,
        WTYPE_BUBBLE,
        WTYPE_SPACER,
        WTYPE_DIV,
        WTYPE_DYNAMIC_RIBBON,
        WTYPE_MODEL_VIEW,
        WTYPE_LIST,
        WTYPE_TEXTBOX,
        WTYPE_PROGRESS,
        WTYPE_RATINGBAR
    };

    enum BadgeType
    {
        /** display a lock on the widget, to mean a certain game feature is locked */
        LOCKED_BADGE   = 1,
        /** display a green check on a widget, useful e.g. to display confirmation */
        OK_BADGE       = 2,
        /** display a red mark badge on the widget, useful e.g. to warn of an invalid choice */
        BAD_BADGE      = 4,
        /** display a trophy badge on the widget, useful e.g. for challenges */
        TROPHY_BADGE   = 8,
        /** A gamepad icon */
        GAMEPAD_BADGE  = 16,
        /** A keyboard icon */
        KEYBOARD_BADGE = 32,
        /** An hourglass badge to indicate loading */
        LOADING_BADGE  = 64,
        /** A zipper badge to indicate that this player receives a boost */
        ZIPPER_BADGE   = 128,
        /** A anchor badge to indicate that this player receives a handicap */
        ANCHOR_BADGE   = 256
    };


    enum Property
    {
        PROP_ID = 100,
        PROP_PROPORTION,
        PROP_WIDTH,
        PROP_HEIGHT,
        PROP_CHILD_WIDTH,
        PROP_CHILD_HEIGHT,
        PROP_WORD_WRAP,
        //PROP_GROW_WITH_TEXT, // yet unused
        PROP_X,
        PROP_Y,
        PROP_LAYOUT,
        PROP_ALIGN,
        // PROP_TEXT, // this one is a bit special, can't go along others since it's wide strings
        PROP_ICON,
        PROP_FOCUS_ICON,
        PROP_TEXT_ALIGN,
        PROP_MIN_VALUE,
        PROP_MAX_VALUE,
        PROP_MAX_WIDTH,
        PROP_MAX_HEIGHT,
        PROP_SQUARE,
        PROP_EXTEND_LABEL,
        PROP_LABELS_LOCATION,
        PROP_MAX_ROWS,
        PROP_WRAP_AROUND,
        PROP_DIV_PADDING,
        PROP_KEEP_SELECTION,
        PROP_CUSTOM_RATIO,
    };

    bool isWithinATextBox();
    void setWithinATextBox(bool in);

    /**
      * \brief The nearly-abstract base of all widgets
      * (not fully abstract since a bare Widget can be created for the sole goal of containing
      * children widgets in a group)
      *
      * Provides basic common functionnality, as well as providing a few callbacks
      * for children to override if they need to do something special on event.
      *
      * Each widget may have an irrlicht parent (most often used to put widgets in dialogs)
      * and also optionally one or many children.
      *
      * Each widget also has a set of properties stored in a map (see enum above)
      *
      * \ingroup guiengine
      */
    class Widget : public SkinWidgetContainer
    {
    protected:
        unsigned int m_magic_number;

        // FIXME: find better ways than hackish "friend"?
        friend class EventHandler;
        friend class Screen;
        friend class Skin;
        friend class RibbonWidget;
        friend class SpinnerWidget;
        friend class ProgressBarWidget;
        friend class DynamicRibbonWidget;
        friend class LayoutManager;
        friend class ModalDialog;
        friend class AbstractTopLevelContainer;

        /** Used during loading, by the layout engine. After layout is done this is not read anymore. */
        int m_absolute_x, m_absolute_y, m_absolute_w, m_absolute_h;
        int m_absolute_reverse_x, m_absolute_reverse_y;
        float m_relative_x, m_relative_y, m_relative_w, m_relative_h;

        /** PROP_TEXT is a special case : since it can be translated it can't
         *  go in the map above, which uses narrow strings */
        irr::core::stringw m_text;

        /** When true, this widget shall use a bigger and more colourful font */
        bool m_title_font;

        /**
          * Can be used in children to indicate whether a widget is selected or not
          * - in widgets where it makes sense (e.g. ribbon children) and where the
          * irrLicht widget can not directly contain this state
          */
        bool m_selected[MAX_PLAYER_COUNT];

        /**
          * Whether to descend in the children of this widget when searching a widget
          * from its ID or name. (children classes can override this value as they please)
          */
        bool m_check_inside_me;

        /**
          * called when right key is pressed and focus is on widget.
          * Returns 'EVENT_LET' if user's event handler should be notified of a change.
          * Override in children to be notified of left/right events and/or make
          * the event propagate to the user's event handler.
          */
        virtual EventPropagation rightPressed(const int playerID) { return EVENT_BLOCK; }

        /**
         * called when left key is pressed and focus is on widget.
         * Returns 'EVENT_LET' if user's event handler should be notified of a change.
         * Override in children to be notified of left/right events and/or make
         * the event propagate to the user's event handler.
         */
        virtual EventPropagation leftPressed (const int playerID) { return EVENT_BLOCK; }

        /** used when you set eventSupervisors - see m_event_handler explainations below
            called when one of a widget's children is hovered.
            \return 'EVENT_LET' if main event handler should be notified of a change, 'EVENT_BLOCK' otherwise */
        virtual EventPropagation mouseHovered(Widget* child, const int playerID) { return EVENT_BLOCK; }

        /** override in children if you need to know when the widget is focused.
          * \return whether to block event */
        virtual EventPropagation focused(const int playerID) { setWithinATextBox(false); return EVENT_LET; }

        /** override in children if you need to know when the widget is unfocused. */
        virtual void unfocused(const int playerID, Widget* new_focus) { }

        /**
         * An irrlicht parent (most often used to put widgets in dialogs)
         */
        irr::gui::IGUIElement* m_parent;

        /**
         * IrrLicht widget created to represent this object.
         */
        irr::gui::IGUIElement* m_element;


        /** numerical ID used by irrLicht to identify this widget
         * (not the same as the string identificator specified in the XML file)
         */
        int m_id;

        /** Usually, only one widget at a time can be focused. There is however a special case where all
            players can move through the screen. This variable will then be used as a bitmask to contain
            which players beyong player 1 have this widget focused. */
        bool m_player_focus[MAX_PLAYER_COUNT];

        /** Whether to reserve an ID in 'm_reserved_id' when widget is added */
        bool m_reserve_id;

        /** Type of this widget */
        WidgetType m_type;

        /**
         * If this widget has any children, they go here. Children can be either
         * specified in the XML file (e.g. Ribbon or Div children), or can also
         * be created automatically for logical widgets built with more than
         * one irrlicht widgets (e.g. Spinner)
         */
        PtrVector<Widget> m_children;

        /** A bitmask of which badges to show, if any; choices are *_BADGE, defined above */
        int m_badges;

        /** A simple flag that can be raised to deactivate this widget */
        bool m_deactivated;

        /** A flag to indicate whether this widget should be visible or not. */
        bool m_is_visible;

        /** Set to false if widget is something that should not receive focus */
        bool m_focusable;

        bool m_bottom_bar;
        bool m_top_bar;

        /** If a badge wouldn't look too pretty on the very side of the widget */
        int m_badge_x_shift;

        bool m_has_tooltip;
        irr::core::stringw m_tooltip_text;

    public:

        /**
         * This is set to NULL by default; set to something else in a widget to mean
         * that events happening on this widget should also be passed to m_event_handler->transmitEvent,
         * which is usually the parent analysing events from its children.
         * This is especially useful with logical widgets built with more than
         * one irrlicht widgets (e.g. Spinner, Ribbon)
         */
        Widget* m_event_handler;

        /**
          * Whether this widget supports multiplayer interaction (i.e. whether this widget can be
          * used by players other than by the game master)
          */
        bool m_supports_multiplayer;

        /** Instead of searching for widget IDs smaller/greater than that of this object, navigation
            through widgets will start from these IDs (if they are set). */
        int m_tab_down_root;

        /** Instead of searching for widget IDs smaller/greater than that of this object, navigation
         through widgets will start from these IDs (if they are set). */
        int m_tab_up_root;

        /** Coordinates of the widget once added (the difference between those x/h and PROP_WIDTH/PROP_HEIGHT is
            that the props are read in raw form from the XML file; PROP_WIDTH can then be e.g. "10%" and w,
            once the widget is added, will be e.g. 80.) */
        int m_x, m_y, m_w, m_h;

        /** Whether to show a bounding box around this widget (used for sections) */
        bool m_show_bounding_box;

        /** Only used if m_show_bounding_box is true */
        bool m_is_bounding_box_round;

        /** Used in two cases :
            1) For 'placeholder' divisions; at the time the layout is created, there is nothing to
               place there yet, but we know there eventually will. So in this case pass 'true' to the
               Widget constructor and it will reserve a widget ID and store it here.
            2) Theorically, in 'add()', derived widgets should checked if this value is set, and use
               it instead of creating a new ID if it is. In practice, it's not widely implemented (FIXME) */
        int m_reserved_id;

        /**
         * A map that holds values for all specified widget properties (in the XML file)
         *
         * \note Changing any of these properties will only take effect the next time
         *       this widget is add()ed (EXCEPT for for x, y, width and height properties,
         *       which are only read on load; after that use method Widget::move).
         * \note Not all widgets use all properties, some widgets may ignore some properties.
         */
        std::map<Property, std::string> m_properties;

        Widget(WidgetType type, bool reserve_id = false);
        virtual ~Widget();

        /**
          * Set the irrlicht widget to be used as parent of this widget next time Widget::add()
          * is invoked on this widget.
          */
        void setParent(irr::gui::IGUIElement* parent);

        /**
         * \brief Sets the widget (and its children, if any) visible or not.
         * Note that setting a widget invisible implicitely calls setDeactivated(), and setting
         * it visible implicitely calls setActive(true). If you mix visiblity and (de)activated calls,
         * undefined behavior may ensue (like invisible but clickable buttons).
         */
        void setVisible(bool visible);

        /** Returns if the element is visible. */
        bool isVisible() const;

        bool isActivated() const;

        virtual EventPropagation onActivationInput(const int playerID) { return EVENT_LET; }

        /**
         * Call to resize/move the widget. Not all widgets can resize gracefully.
         */
        virtual void move(const int x, const int y, const int w, const int h);

        /**
          * Get whether this widget is selected (only makes sense in some cases where
          * a widget is part of a bigger widget, e.g. in ribbons, and a selected item
          * is kept)
          */
        bool isSelected(const int playerID) const { return m_selected[playerID]; }

        bool isBottomBar() const { return m_bottom_bar; }
        bool isTopBar   () const { return m_top_bar;    }

        /**
         * \name Enabling or disabling widgets
         * \{
         */

        /** \brief Sets an widget to be either activated or deactivated 
         *  (i.e. greyed out)
         *  \param active Active (true) or deactive (false). Defaults to 
         *         true. */
        virtual void setActive(bool active=true);

        /**
          * \}
          */

        /**
         * \name Accessing the underlying irrlicht element
         * \{
         */

        /**
          * Get the underlying irrLicht GUI element, casted to the right type.
          */
        template<typename T> T* getIrrlichtElement()
        {
        #if HAVE_RTT
            T* out = dynamic_cast<T*>(m_element);
            return out;
        #else
            return static_cast<T*>(m_element);
        #endif
        }

        /**
         * Get the underlying irrLicht GUI element, casted to the right type; const version.
         */
        template<typename T> const T* getIrrlichtElement() const
        {
            #if HAVE_RTT
                T* out = dynamic_cast<T*>(m_element);
                return out;
            #else
                return static_cast<T*>(m_element);
            #endif
        }

        /**
         * Get the underlying irrLicht GUI element
         */
        irr::gui::IGUIElement* getIrrlichtElement() { return m_element; }

        void moveIrrlichtElement();
        bool isSameIrrlichtWidgetAs(const Widget* ref) const { return m_element == ref->m_element; }

        /**
          * \}
          */

        /**
         * \name Get and set properties
         * \{
         * Note that many properties are read only by the Widget::add method; so, while
         * it will generally work to set the properties before add() is called, modifying
         * the widget after it was added will require other widget-specific calls.
         */

        /**
          * Sets the text of a widget from a wchar_t.
          * Handy for many constant strings used in stk.
          *
          * \note Not all widgets use strings, so some widgets may ignore this text property
          * \note Changing the text property will only take effect the next time this widget
          *       is add()ed
          */
        virtual void setText(const wchar_t *s);

        /**
          * Sets the text of a widget from a stringw.
          * \note This method uses the virtual setText(wchar_t*) function, so only the latter
          *       needs to be overwritten by other classes.
          * \note Not all widgets use strings, so some widgets may ignore this text property
          * \note Changing the text property will only take effect the next time this widget
          *       is add()ed
          */
        virtual void setText(const irr::core::stringw &s) { setText(s.c_str()); }

        /** Returns the text of a widget. */
        const irr::core::stringw &getText() const {return m_text; }

        /** \return Type of this widget */
        WidgetType getType() const { return m_type; }

        /**
          * Get the irrlicht widget ID attributed to this widget
          * \pre Only call this method after the widget has been add()ed
          */
        int getID() const { return m_id; }

        /** Get whether this object is allowed to receive focus */
        bool isFocusable() const { return m_focusable; }

        void setFocusable(bool f) { m_focusable = f; }

        /**
          * \}
          */

        /**
         * \name Focus management
         * \{
         */

        /**
         * Focus the widget for the given player.
         * \param playerID ID of the player you want to set/unset focus for, starting from 0
         */
        void setFocusForPlayer(const int playerID);

        /**
         * Find whether this widget is focused by a given player.
         * \param playerID ID of the player you want to set/unset focus for, starting from 0
         * \return whether this widget is focused by a given player.
         */
        bool isFocusedForPlayer(const int playerID);

        /** Internal method, do not call it. Call the functions in GUIEngine instead to unset focus. */
        void unsetFocusForPlayer(const int playerID);

        /**
          * \}
          */

        /**
         * \name ID Counter Functions
         * Functions used to generate IDs for new widgets. The domain of each ID
         * encodes whether the widget can receive focus or not (this was implemented
         * this way because navigation in irrlicht happens between sequential IDs; so
         * to prevent navigation to a widget, one needs to give an ID that is not
         * sequential with focusable widgets in order not to break keyboard navigation).
         * \{
         */

        static void resetIDCounters();

        /**
          * \brief Provides a new unique ID on each call, for widgets that can be focused.
          */
        static int getNewID();

        /**
          * \brief Provides a new unique ID on each call, for widgets that can not be focused.
          */
        static int getNewNoFocusID();

        /**
          * \brief get whether the given ID represents an ID of a widget that can be focused
          * \return whether the given ID represents an ID of a widget that can be focused
          *         (i.e. whether it was generated by Widget::getNewID() or
          *          Widget::getNewNoFocusID())
          */
        static bool isFocusableId(const int id);

        /**
          * \}
          */

        /**
         * \name Handling children
         * If this widget is a container and has children.
         * \{
         */

        /**
          * \return a read-only view of the childrens of this widget, if any
          */
        const PtrVector<Widget>& getChildren() const { return m_children; }

        PtrVector<Widget>&       getChildren()       { return m_children; }

        /**
          * \brief removes and deletes the child with the given PROP_ID
          * \param id PROP_ID property of the child to remove
          *
          * \note  If the widget has been add()ed, it is moved immediately;
          *        if the widget is not currently visible, it will take the
          *        new position next time it is add()ed.
          *
          * \return whether deletion was successful
          */
        bool deleteChild(const char* id);

        /**
          * \}
          */

        /**
         * \{
         * \name Callbacks for subclasses
         * Classes that subclass Widget to provide actual implementations may override/implement these
         * methods to change behaviour or be notified of some events.
         */

        /**
         * \brief Override in children to possibly receive updates (you may need to register to
         * them first)
         */
        virtual void update(float delta) { }

        /** All widgets, including their parents (m_event_handler) will be notified on event through
         this call. Must return whether main (GUI engine user) event callback should be notified or not.
         Note that in the case of a hierarchy of widgets (with m_event_handler), only the topmost widget
         of the chain decides whether the main handler is notified; return value is not read for others. */
        virtual EventPropagation transmitEvent(Widget* w,
                                               const std::string& originator,
                                               const int playerID)
                { return EVENT_LET; }

        /**
         * \brief Create and add the irrLicht widget(s) associated with this object.
         * Call after Widget was read from XML file and laid out.
         */
        virtual void add();

        /**
          * \brief Called when irrLicht widgets cleared. Forget all references to them, they're no more valid.
          */
        virtual void elementRemoved();

        bool searchInsideMe() const { return m_check_inside_me; }

        /** When inferring widget size from its label length, this method will be called to
         * if/how much space must be added to the raw label's size for the widget to be large enough */
        virtual int getWidthNeededAroundLabel()  const { return 0; }

        /** When inferring widget size from its label length, this method will be called to
         * if/how much space must be added to the raw label's size for the widget to be large enough */
        virtual int getHeightNeededAroundLabel() const { return 0; }

        /**
          * \}
          */

        /**
         * \name Badge support
         * "Badges" are icons that can appear on top of some widgets.
         * \{
         */

        /**
         * \brief adds a particular badge to this widget.
         * The STK widget toolkit has support for "badges". Badges are icon overlays displayed
         * on the corner of a widget; they are useful to convey information visually.
         */
        void setBadge(BadgeType badge_bit)
        {
            m_badges |= int(badge_bit);
        }

        /**
         * \brief removes a particular bade from this widget, if it had it.
         * \see GUIEngine::Widget::setBadge for more info on badge support
         */
        void unsetBadge(BadgeType badge_bit)
        {
            m_badges &= (~int(badge_bit));
        }

        /** \brief sets this widget to have no badge
         * \see GUIEngine::Widget::setBadge for more info on badge support
         */
        void resetAllBadges()
        {
            m_badges = 0;
        }

        /**
         * \brief Get which badges are currently on this widget
         * \return a bitmask of BadgeType values
         */
        int getBadges() const
        {
            return m_badges;
        }

        /**
         * \}
         */

        /**
         * \name Tooltip support
         * \{
         */


        bool hasTooltip() const { return m_has_tooltip; }

        /** Only call if hasTooltip() returned true */
        irr::core::stringw getTooltipText() const { return m_tooltip_text; }

        void setTooltip(irr::core::stringw s) { m_tooltip_text = s; m_has_tooltip = true; }

        /**
         * \}
         */

        bool ok() const { return (m_magic_number == 0xCAFEC001); }

        /** Gets called when the widget is active and got clicked. (Only works for button widgets for now.) */
        virtual void onClick()  { }
    };


}
#endif
