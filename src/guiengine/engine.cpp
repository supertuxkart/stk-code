//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Marianne Gagnon
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

namespace GUIEngine
{
/**


 \page gui_overview GUI Module Overview

 In XML files, widgets are declared in the following fashion :

 \code
 <widget_name property1="value1" property2="value2" />
 \endcode

 or, for widgets of "spawn" type, with children :

 \code
 <widget_name property1="value1" property2="value2" >
 <child1 />
 <child2 />
 </widget_name>
 \endcode

 The first section of this document describes the widgets you can use; the
 second describes the properties widgets can take. Not all properties can be
 applied to all widgets, see the docs for a given widget and a given property
 for full information.

 \n
 <HR>
 \section toc Table of Contents
 <HR>

 \ref widgets
 \li \ref widget1
 \li \ref widget2
 \li \ref widget3
 \li \ref widget4
 \li \ref widget5
 \li \ref widget6
 \li \ref widget7
 \li \ref widget8
 \li \ref widget9
 \li \ref widget10
 \li \ref widget11
 \li \ref widget12
 \li \ref widget13

 \ref props
 \li \ref prop1
 \li \ref prop2
 \li \ref prop3
 \li \ref prop3.1
 \li \ref prop4
 \li \ref prop5
 \li \ref prop6
 \li \ref prop7
 \li \ref prop8
 \li \ref prop9
 \li \ref prop10
 \li \ref prop11
 \li \ref prop12
 \li \ref prop13
 \li \ref prop14
 \li \ref prop15
 \li \ref prop16
 \li \ref prop17
 \li \ref prop18

 \ref code

 \ref internals

 \n
 \n
 <HR>
 \section widgets Widgets
 <HR>

 This section describes the widgets you can use in STK's GUI XML files. The
 upper-case name starting with WTYPE_* is the internal name of the widget
 (see the WidgetType enum).

 \n
 \subsection widget1 WTYPE_RIBBON
 <em> Names in XML files: </em> \c "ribbon", \c "buttonbar", \c "tabs"

 Appears as an horizontal bar containing elements laid in a row, each being
 and icon and/or a label

 \li The "ribbon" subcategory will behave a bit like a radio button group,
     i.e. one element must selected. Events are triggered as soon as a choice
     is selected (can be simply by hovering).
 \li The "buttonbar" subcategory treats children buttons as action buttons,
     which means they can't have a 'selected' state, only focused or not (i.e.
     there is no selection that remains if you leave this area). Events are
     triggered only on enter/fire.
 \li The "tabs" subcategory will show a tab bar. behaviour is same as normal
     ribbon, only looks are different. Orientation of tabs (up or down) is
     automatically inferred from on-screen position

 \note Ribbon widgets are of spawn type (\<ribbon\> ... \</ribbon\>) and may
       contain icon-buttons or buttons as children.
 \note Property PROP_SQUARE can be set to tell the engine if the ribbon's
       contents are rectangular or not (this will affect the type of
       highlighting used)
 \note All elements within a ribbon must have an 'ID' property
 \note Ribbons (e.g. tabs) can have their elements dynamically added at
       runtime, too. Just add no children to the ribbon in the XML file, and
       add them at runtime through the method for this.
 \note The layout algorithm will reserve space for at most one line of text
       (if needed) for ribbon elements. If you have ribbon elements with
       long texts that spawn many lines, 1. give the word_wrap="true" property
       to the icon button widget in the XML file; 2. expect that the extra
       lines will not be accounted for in the sizing algorithms (i.e. extra
       lines will just expand over whatever is located under the ribbon)

 \n
 \subsection widget2 WTYPE_SPINNER
 Names in XML files: </em> \c "spinner", \c "gauge"

 A spinner component (lets you choose numbers).

 Specify PROP_MIN_VALUE and PROP_MAX_VALUE to have control over values
 (default will be from 0 to 99). You can specify an icon; then, include a
 sprintf format string like %i in the name, and at runtime the current number
 will be inserted into the given name to find the right file for each
 possible value the spinner can take. It may also display arbitrary text
 instead of numbers, though this cannot be achieve in the XML file; use
 the -\>addLabel(...) method in code to do this.
 It can also display arbitrary text containing the value; just define the
 PROP_TEXT property to contain the text you want, including a format string %i
 where the value should appear (a string that does not contain %i will result
 in the same text being displayed in the spinner no matter the current value).
 \note The "gauge" variant behaves similarly, but a fill band shows how close
       to the max the value is.

 \n
 \subsection widget3 WTYPE_BUTTON
 <em> Name in XML files: </em> \c "button"

 A plain text button.

 \n
 \subsection widget4 WTYPE_ICON_BUTTON
 <em> Names in XML files: </em> \c "icon-button", \c "icon"

 A component with an image, and optional text to go under it.

 \note The "icon" variant will have no border and will not be clickable.
       PROP_ICON is mandatory for this component. There are three ways to
       place the texture within the allocated space; the default (and only
       way currently accessible through xml files) is to scale the texture to
       fit, while preserving its aspect ratio; other methods, currently only
       accessible through C++ code, are to stretch the texture to fill the
       area without caring for aspect ratio, and another to respect an aspect
       ratio other than the texture's (useful for track screenshots, which
       are 4:3 compressed to fit in a power-of-two 256x256 texture)
 \note Supports property PROP_FOCUS_ICON

 \n
 \subsection widget5 WTYPE_CHECKBOX
 <em> Name in XML files: </em> \c "checkbox"

 A checkbox.

 \n
 \subsection widget6 WTYPE_LABEL
 <em> Names in XML files: </em> \c "label", \c "header" , \c "bright"

 A plain label.

 Supports properties PROP_WORD_WRAP and PROP_TEXT_ALIGN.
 \note The "header" variant uses a bigger and more colourful font.
 \note The "bright" variant uses a more colourful font but is not bigger.


 \c WTYPE_BUBBLE is a variation of the plain label; the difference with a
 bubble widget is that it can be focused, and when focused it will expand
 to show more text, if the label is too long to be displayed in the allocated
 space.

 \n
 \subsection widget7 WTYPE_SPACER
 <em> Name in XML files: </em> \c "spacer"

 Some blank space; not visible on screen.

 \n
 \subsection widget8 WTYPE_DIV
 <em> Name sin XML files: </em> \c "div", \c "box"

 An invisible container.

 \li Divs do not do much on themselves, but are useful to lay out children
     automatically (Supports property PROP_LAYOUT)
 \li Divs can be nested.
 \li Of spawn type (\<div\>...\</div\>, place children within)
 \note "box" is a variant that acts exactly the same but is visible on-screen

 \n
 \subsection widget9 WTYPE_DYNAMIC_RIBBON
 Names in XML files: </em> \c  "ribbon_grid", \c "scrollable_ribbon",
 \c "scrollable_toolbar"

 Builds upon the basic Ribbon to be more dynamic (dynamics contents, possibly
 with scrolling, possibly multi-line)

 \li NOT of spawn type (\<ribbon_grid .../\>), i.e. children are not specified
     in the XML file but programmatically at runtime.
 \li PROP_CHILD_WIDTH and PROP_CHILD_HEIGHT are mandatory (so at least aspect
     ratio of elements that will later be added is known) An interesting
     aspect of PROP_CHILD_WIDTH and PROP_CHILD_HEIGHT is that you can use them
     to show textures to any aspect ratio you want (so you can e.g. save
     textures to a power-of-two size like 256x256, but then show it in 4:3
     ratio).
 \li Property PROP_SQUARE can be set to tell the engine if the ribbon's
     contents are rectangular or icons (this will  affect the type of
     highlighting used).
 \li Supports an optional label at the bottom if PROP_LABELS_LOCATION is set
     (see more on PROP_LABELS_LOCATION below).
 \note The "scrollable_ribbon" and "scrollable_toolbar" subtypes are
       single-line scrollable ribbons. The difference between both is that
       'scrollable_ribbon' always has a value selected (like in a combo box,
       or radio buttons), while 'scrollable_toolbar' is a scrollable list of
       buttons that can be pressed to trigger actions.

 \n
 \subsection widget10 WTYPE_MODEL_VIEW
 <em> Name in XML files: </em> \c "model"

 Displays a 3D model.

 \note Contents must be set programmatically.

 \n
 \subsection widget11 WTYPE_LIST
 <em> Name in XML files: </em> \c "list"

 Displays a list.

 \note Contents must be set programmatically.


 \n
 \subsection widget12 WTYPE_PROGRESS
 <em> Name in XML files: </em> \c "progressbar"

 Display a progress bar (e.g. for downloads).

 \note The value must be set programmatically.

 \n

 \subsection widget13 WTYPE_TEXTBOX
 <em> Name in XML files: </em> \c "textbox"

 A text field where the user can type text

 \n
 \n
 <HR>
 \section props Properties
 <HR>

 \subsection prop1 PROP_ID
 <em> Name in XML files: </em> \c "id"

 Gives a unique internal name to each object using this property. It will be
 used in events callbacks to determine what action occurred. Can be omitted
 on components that do not trigger events (e.g. labels)

 \n
 \subsection prop2 PROP_TEXT
 <em> Name in XML files: </em> \c "text" or "raw_text" ("text" is translated, "raw_text" is not)

 gives text (a label) to the widget where supported. Ribbon-grids give a
 special meaning to this parameter, see ribbon-grid docs above.

 \n
 \subsection prop3 PROP_ICON
 <em> Name in XML files: </em> \c "icon"

 give an icon to the widget. Property contents is the path to the file, by
 default relative to the /data directory of STK (several methods of
 IconButtonWidget and DynamicRibbon can enable you to use absolute paths if
 you wish, however).

 \n
 \subsection prop3.1 PROP_FOCUS_ICON
 <em> Name in XML files: </em> \c "focus_icon"

 For icon buttons. A different icon to show when the item is focused.

 \n
 \subsection prop4 PROP_TEXT_ALIGN
 <em> Name in XML files: </em> \c "text_align"

 used exclusively by label components. Value can be "right" or "center" (left
 used if not specified).

 \n
 \subsection prop5 PROP_WORD_WRAP
 <em> Name in XML files: </em> \c "word_wrap"

 used by label components and icon buttons. Value can be "true" to indicate
 that long text should spawn on multiple lines. Warning, in icon buttons,
 the space under the button's text may be rendered unclickable by the label
 widget overlapping other widgets under.

 Line breaks are done on space characters; if one word is too long to fit
 on one line, then SHY (soft hyphen) characters are searched and breaks
 can be added there.

 Note that for multiline labels, the layout engine is unable to guess their
 width and height on their own so you should explicitely give a width and
 height for labels that use this flag.

 \n
 \subsection prop6 PROP_MIN_VALUE, PROP_MAX_VALUE
 <em> Name in XML files: </em> \c "min_value", \c "max_value"

 used to specify a minimum and maximum value for numeric widgets
 (c.f. spinner)

 \n
 \subsection prop7 PROP_X, PROP_Y
 <em> Name in XML files: </em> \c "x", "y"

 sets the position (location) of a widget, relative to its parent (container
 \<div\> or screen if none). A plain number will be interpreted as an
 aabsolute position in pixels. A '%' sign may be added to the given number
 to mean that the location is specified in terms of a percentage of parent
 size (parent size means the parent \<div\> or the whole screen if none). A
 negative value can also be passed to start coordinate from right and/or
 bottom, instead of starting from top-left corner as usual.
 Note that in many cases, it is not necessary to manually a position. Div
 layouts will often manage that for you (see PROP_LAYOUT). Other widgets will
 also automativally manage the position and size of their children, for
 instance ribbons.

 \n
 \subsection prop8 PROP_WIDTH, PROP_HEIGHT
 <em> Name in XML files: </em> \c "width", \c "height"

 give dimensions to the widget. A plain number will be interpreted as an
 absolute position in pixels. A '%' sign may be added to the given number to
 mean that the size is specified in terms of a percentage of parent size
 (parent size means the parent \<div\> or the whole screen if none).
 Note that in many cases, it is not necessary to manually a size. Div layouts
 will often manage that for you (see PROP_LAYOUT). In addition, sizes are
 automatically calculated for widgets made of icons and/or text like labels
 and plain icons. Other widgets will also automativally manage the position
 and size of their children, for instance ribbons.

 Another possible value is "fit", which will make a \<div\> fit to its
 contents.

 Another possible value is "font", which will use the size of the font
 (useful to insert widgets inside text)

 \n
 \subsection prop9 PROP_MAX_WIDTH, PROP_MAX_HEIGHT
 <em> Names in XML files: </em> \c "max_width", \c "max_height"

 The maximum size a widget can take; especially useful when using percentages
 and proportions.

 \n
 \subsection prop10 PROP_CHILD_WIDTH, PROP_CHILD_HEIGHT
 <em> Names in XML files: </em> \c "child_width", \c "child_height"

 Used exclusively by the ribbon grid widget. See docs for this widget above.

 \n
 \subsection prop11 PROP_LAYOUT
 <em> Name in XML files: </em> \c "layout"

 Valid on 'div' containers. Value can be "horizontal-row" or "vertical-row".
 This means x and y coordinates of all children will automatically be
 calculated at runtime, so they are laid in a row. Width and height can be set
 absolutely as usual, but can also be determined dynamically according to
 available screen space. Also see PROP_ALIGN and PROP_PROPORTION to known
 more about controlling layouts. Note that all components within a layed-out
 div will ignore all x/y coordinates you may give them as parameter.

 \n
 \subsection prop12 PROP_ALIGN
 <em> Name in XML files: </em> \c "align"

 For widgets located inside a vertical-row layout div : Changes how the x
 coord of the widget is determined. Value can be \c "left", \c "center" or
 \c "right".

 For widgets located inside a horizontal-row layout div : Changes how the y
 coord of the widget is determined. Value can be \c "top", \c "center" or
 \c "bottom".

 \note If you want to horizontally center widgets in a horizontal-row layout,
       or vertically center widgets in a vertical-row layout, this property
       is not what you're looking for; instead, add a stretching spacer before
       and after the widget(s) you want to center.

 \note When applied to a label widget, this property will center the text
       widget within its parent. To align the text inside the label widget,
       see \ref prop4

 \n
 \subsection prop13 PROP_PROPORTION
 <em> Name in XML files: </em> \c "proportion"

 Helps  determining widget size dynamically (according to available screen
 space) in layed-out divs. In a vertical row layout, proportion sets the
 height of the item. In an horizontal row, it sets the width of the item.
 Proportions are always evaluated relative to the proportions of other widgets
 in the same div. If one div contains 4 widgets, and their proportions are
 1-2-1-1, it means the second must take twice as much space as the 3 others.
 In this case, 10-20-10-10 would do the exact same effect. 1-1-1-1 would mean
 all take 1/4 of the available space. Note that it is allowed to mix absolute
 widget sizes and proportions; in this case, widgets with absolute size are
 evaluated first, and the dynamically-sized ones split the remaining space
 according to their proportions.

 \n
 \subsection prop14 PROP_SQUARE
 <em> Name in XML files: </em> \c "square_items"

 Valid on Ribbons or RibbonGrids. Can be "true" (omitting it means "false").
 Indicates whether the contents use rectangular icons as opposed to "round"
 icons (this will affect the type of focus/highlighting used)

 \n
 \subsection prop15 PROP_EXTEND_LABEL
 <em> Name in XML files: </em> \c "extend_label"

 How many pixels the label is allowed to expand beyond the boundaries of the
 widget itself. Currently only allowed on icon widgets.

 \n
 \subsection prop16 PROP_LABELS_LOCATION
 <em> Name in XML files: </em> \c "label_location"

 In dynamic ribbons : Decides where the label is. Value
 can be "each", "bottom", or "none" (if ommitted, "none" is the default).
 "each" means that every item has its own label. "bottom" means there is a
 single label for all at the bottom, that displays the name of the current
 item.

 In non-dynamic ribbons, you can also use value "hover" which will make the
 label only visible when the icon is hovered with the mouse.

 \n
 \subsection prop17 PROP_MAX_ROWS
 <em> Name in XML files: </em> \c "max_rows"

 Currently used for ribbon grids only. Indicates the maximum amount of rows
 this ribbon can have.

 \n
 \subsection prop18 PROP_WRAP_AROUND
 <em> Name in XML files: </em> \c "wrap_around"

 Currently used for spinners only. Value can be "true" or "false"


 \n
 \subsection prop19 PROP_DIV_PADDING
 <em> Name in XML files: </em> \c "padding"

 Used on divs, indicate by how many pixels to pad contents


 \n
 \subsection prop20 PROP_KEEP_SELECTION
 <em> Name in XML files: </em> \c "keep_selection"

 Used on lists, indicates that the list should keep showing the selected item
 even when it doesn't have the focus


 \n
 <HR>
 \section code Using the engine in code
 <HR>

 The first thing to do is to derive a class of your own from
 AbstractStateManager. There are a few callbacks you will need to override.
 Once it's done, you have all AbstractStateManager methods ready to be used to
 push/pop/set menus on the screen stack. Once you have instanciated your state
 manager class, call GUIEngine::init and pass it as argument. One of the most
 important callbacks is 'eventCallback', which will be called everytime
 something happens. Events are generally a widget state change. In this case,
 a pointer to the said widget is passed along its name, so you get its new
 state and/or act.

 When you have described the general layout of a Screen in a XML file, as
 described above, you may use it in the code by creating a class deriving
 from GUIEngine::Screen, passing the name of the XML file to the constructor
 of the base class. The derived class will most notably be used for event
 callbacks, to allowcreating interactive menus. The derived class must also
 implement the Screen::init and Screen::tearDown methods,
 that will be called, respectively, when a menu is entered/left. For simple
 menus, it is not unexpected that those methods do nothing. For init and
 tearDown the corresponding function in Screen must be called. Note that init
 is called after the irrlicht elements have been added on screen; if you wish
 to alter elements BEFORE they are actually added, use either
 Screen::loadedFromFile or Screen::beforeAddingWidget ; the
 difference is that the first is called once only upon loading, whereas the
 second is called every time the menu is visited.

 \n
 Summary of callbacks, in order :

 \li (Load the Screen from file : Screen::loadFromFile is called automatically
      the first time you reference this screen through the StateManager)
 \li Screen::loadedFromFile is called (implement it if you need it)
 \li (Ask to visit the screen through the StateManager)
 \li Screen::beforeAddingWidget (implement it if you need it)
 \li Widget::add is called automatically on each Widget of the screen
 \li Screen::init (implement it if you need it)
 \li (Ask to leave the Screen through the StateManager)
 \li Screen::tearDown (implement it if you need it)
 \li Widget::elementRemoved is called automatically on each Widget of the
     screen

 Widget::m_properties contains all the widget properties as loaded
 from the XML file. They are generally only read from the Widget::add
 method, so if you alter a property after 'add()' was called it will not
 appear on screen.

 Note that the same instance of your object may be entered/left more than
 once, so make sure that one instance of your object can be used several
 times if the same screen is visited several times.

 Note that the same instance of your object may be unloaded then loaded back
 later. It is thus important to do set-up in the Screen::loadedFromFile
 callback rather than in the constructor (after the creation of Screen object,
 it may be unloaded then loaded back at will, this is why it's important to
 not rely on the constructor to perform set-up).

 Do not delete a Screen manually, since the GUIEngine caches them; deleting
 a Screen will only result in dangling pointers in the GUIEngine. Instead, let
 the GUIEngine do the cleanup itself on shutdown, or on e.g. resolution
 change.

 You can also explore the various methods in Screen to discover
 more optional callbacks you can use.

 You can also create dialogs by deriving from ModalDialog in a very
 similar way.

 \n
 <HR>
 \section internals Inside the GUI Engine
 <HR>

 \subsection Widget Widget

 SuperTuxKart's GUIEngine::Widget class is a wrapper for the underlying
 irrlicht classes. This is needed for a couple reasons :
 - irrlicht widgets do not do everything we want; so many STK widgets act as
   composite widgets (create multiple irrlicht widgets and adds logic so they
   behave as a whole to the end-user)
 - STK widgets have a longer life-span than their underlying irrlicht
   counterparts. This is simply an optimisation measure to prevent having to
   seek the file to disk everytime a screen switch occurs.

 Each widget contains one (or several) \c irr::gui::IGUIElement instances
 that represent the irrlicht widget that is added to the \c IGUIEnvironment
 if the widget is currently shown; if a widget is not currently shown on
 screen (in irrlicht's \c IGUIEnvironment), then its underlying
 \c IGUIElement pointer will be \c NULL but the widget continues to exist
 and remains ready to re-create its underlying irrlicht widget when the screen
 it is part of is added again. The method \c add() is used to tell a widget
 to create its irrlicht counterpart in the \c IGUIEnvironment - but note that
 unless you start handling stuff manually you do NOT need to invoke \c add()
 on each widget manually, since the parent GUIEngine::Screen object will do
 it automatically when it is shown. When the irrlicht \c IGUIEnvironment
 is cleared (when irrlicht widgets are removed), it is very important to tell
 the Widgets that their pointer to their \cIGUIElement counterpart is no more
 valid; this is done by calling \c elementRemoved() - but again unless you do
 manual manipulation of the widget tree, the GUIEngine::Screen object will
 take care of this for you.

 So, before trying to access the underlying irrlicht element of a
 GUIEngine::Widget, it is thus important to check if the GUIEngine::Widget
 is currently added to the irr \c IGUIEnvironment. This can be done by
 calling \c ->getIrrlichtElement() and checking if the result is \c NULL (if
 non-null, the widget is currently added to the screen). Of course, in some
 circumstances, the check can be skipped because the widget is known to be
 currently visible.

 VERY IMPORTANT: some methods should only be called before Screen::init, and
 some methods should only be called after Screen::init. Unfortunately the
 documentation does not always make this clear at this point :(
 A good hint is that methods that make calls on a IGUIElement* need to be
 called after init(), the others needs to be called before.


 \subsection Screen Screen

 This class holds a tree of GUIEngine::Widget instances. It takes care of
 creating the tree from a XML file upon loading (with the help of others,
 for instane the GUIEngine::LayoutManager); it handles calling \c add() on
 each of its GUIEngine::Widget children when being added - so that the
 corresponding \c IGUIElement irrlicht widgets are added to the irrlicht
 scene. It also takes care of telling its GUIEngine::Widget children when
 their irrlicht \c IGUIElement counterpart was removed from the
 \c IGUIEnvironment so that they don't carry dangling pointers.


 The default behavior of the GUIEngine::Screen object will be just fine for
 most basic purposes, but if you want to build highly dynamic screens, you
 may need to get your hands dirty. Take a look at
 GUIEngine::Screen::manualRemoveWidget() and
 GUIEngine::Screen::manualAddWidget() if you wish to dynamically modify the
 STK widget tree at runtime. If you get into this, be very careful about the
 relationship
 between the STK widget tree and the irrlicht widget tree. If you
 \c manualRemoveWidget() a STK widget that is currently visible on screen,
 this does not remove its associated irrlicht widget; call
 \c widget->getIrrlichtElement()->remove() for that. When you removed a
 widget from a Screen you are also responsible to call
 \c Widget::elementRemoved() on them to avoid dangling pointers.
 Similarly, a GUIEngine::Widget that is not inside a GUIEngine::Screen
 when the screen is added will not have its \c add() method be called
 automatically (so, for instance, if you \c manualAddWidget() a widget
 after a Screen was shown, you will also need to call \c ->add() on the
 widget so that it is added to the irrlicht GUI environment).

 As a final note, note that the GUIEngine::Skin depends on both the irrlicht
 widget and the STK widget to render widgets properly. So adding an irrlicht
 IGUIElement without having its SuperTuxKart GUIEngine::Widget accessible
 through the current GUIEngine::Screen (or a modal dialog) may result in
 rendering glitches.


 */
}

#include "guiengine/engine.hpp"

#include "config/user_config.hpp"
#include "font/bold_face.hpp"
#include "font/digit_face.hpp"
#include "font/font_manager.hpp"
#include "font/font_settings.hpp"
#include "font/regular_face.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#ifndef SERVER_ONLY
#include "graphics/2dutils.hpp"
#endif
#include "graphics/irr_driver.hpp"
#include "guiengine/event_handler.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/message_queue.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/screen_keyboard.hpp"
#include "guiengine/skin.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/dialog_queue.hpp"
#include "modes/demo_world.hpp"
#include "modes/cutscene_world.hpp"
#include "modes/world.hpp"
#include "states_screens/race_gui_base.hpp"

#include <iostream>
#include <assert.h>
#include <irrlicht.h>

using namespace irr::gui;
using namespace irr::video;

namespace GUIEngine
{

    namespace Private
    {
        IGUIEnvironment* g_env;
        Skin* g_skin = NULL;
        ScalableFont *g_font;
        ScalableFont *g_outline_font;
        ScalableFont *g_large_font;
        ScalableFont *g_title_font;
        ScalableFont *g_small_font;
        ScalableFont *g_digit_font;

        IrrlichtDevice* g_device;
        IVideoDriver* g_driver;
        Screen* g_current_screen = NULL;
        AbstractStateManager* g_state_manager = NULL;
        Widget* g_focus_for_player[MAX_PLAYER_COUNT];

        int font_height;
        int large_font_height;
        int small_font_height;
        int title_font_height;
    }
    using namespace Private;

    PtrVector<Widget, REF> needsUpdate;

    PtrVector<Screen, REF> g_loaded_screens;

    float dt = 0;

    // -----------------------------------------------------------------------
    float getLatestDt()
    {
        return dt;
    }   // getLatestDt

    // -----------------------------------------------------------------------
    struct MenuMessage
    {
        irr::core::stringw m_message;
        float m_time;

        MenuMessage(const wchar_t* message, const float time)
                   : m_message(message), m_time(time)
        {
        }
    };   // MenuMessage

    std::vector<MenuMessage> gui_messages;

    // ------------------------------------------------------------------------
    void showMessage(const wchar_t* message, const float time)
    {
        // check for duplicates
        const int count = (int) gui_messages.size();
        for (int n=0; n<count; n++)
        {
            if (gui_messages[n].m_message == message) return;
        }

        // add message
        gui_messages.push_back( MenuMessage(message, time) );

    }   // showMessage

    // ------------------------------------------------------------------------
    Widget* getFocusForPlayer(const unsigned int playerID)
    {
        assert(playerID < MAX_PLAYER_COUNT);

        return g_focus_for_player[playerID];
    }   // getFocusForPlayer

    // ------------------------------------------------------------------------
    void focusNothingForPlayer(const unsigned int playerID)
    {
        Widget* focus = getFocusForPlayer(playerID);
        if (focus != NULL) focus->unsetFocusForPlayer(playerID);

        g_focus_for_player[playerID] = NULL;
    }   // focusNothingForPlayer

    // ------------------------------------------------------------------------
    bool isFocusedForPlayer(const Widget* w, const unsigned int playerID)
    {
        assert(w != NULL);
        assert(playerID < MAX_PLAYER_COUNT);

        // If no focus
        if (g_focus_for_player[playerID] == NULL) return false;

        // otherwise check if the focus is the given widget
        return g_focus_for_player[playerID]->isSameIrrlichtWidgetAs(w);
    }   // isFocusedForPlayer

    // ------------------------------------------------------------------------
    int getTitleFontHeight()
    {
        return Private::title_font_height;
    }   // getTitleFontHeight


    // ------------------------------------------------------------------------
    int getFontHeight()
    {
        return Private::font_height;
    }   // getFontHeight

    // ------------------------------------------------------------------------
    int getSmallFontHeight()
    {
        return Private::small_font_height;
    }   // getSmallFontHeight
 
    // ------------------------------------------------------------------------
    int getLargeFontHeight()
   {

        return Private::large_font_height;
    }   // getSmallFontHeight
        
    // ------------------------------------------------------------------------
    void clear()
    {
        g_env->clear();
        if (g_current_screen != NULL) g_current_screen->elementsWereDeleted();
        g_current_screen = NULL;

        needsUpdate.clearWithoutDeleting();

        gui_messages.clear();
    }   // clear

    // ------------------------------------------------------------------------
    /** Updates all widgets that need to be updated.
     *  \param dt Time step size.
     */
    void update(float dt)
    {
        // Just to mark the begin/end scene block
        GUIEngine::GameState state = StateManager::get()->getGameState();
        if (state != GUIEngine::GAME)
        {
            // This code needs to go outside beginScene() / endScene() since
            // the model view widget will do off-screen rendering there
            for_var_in(GUIEngine::Widget*, widget, GUIEngine::needsUpdate)
            {
                widget->update(dt);
            }
            if (state == GUIEngine::MENU) DialogQueue::get()->update();
        }

        // Hack : on the first frame, irrlicht processes all events that have been queued
        // during the loading screen. So way until the second frame to start processing events.
        // (Events queues during the loading screens are likely the user clicking on the
        // frame to focus it, or similar, and should not be used as a game event)
        static int frame = 0;
        if (frame < 2)
        {
            frame++;
            if (frame == 2)
                GUIEngine::EventHandler::get()->startAcceptingEvents();
        }
    }
    // ------------------------------------------------------------------------

    void cleanForGame()
    {
        clear();

        gui_messages.clear();
    }   // cleanForGame

    // ------------------------------------------------------------------------

    void clearScreenCache()
    {
        Screen* screen;
        for_in (screen, g_loaded_screens)
        {
            screen->unload();
        }

        g_loaded_screens.clearAndDeleteAll();
        g_current_screen = NULL;
    }

    // ------------------------------------------------------------------------

    void switchToScreen(const char* screen_name)
    {
        needsUpdate.clearWithoutDeleting();

        // clean what was left by the previous screen
        g_env->clear();
        if (g_current_screen != NULL) g_current_screen->elementsWereDeleted();
        g_current_screen = NULL;
        Widget::resetIDCounters();

        // check if we already loaded this screen
        const int screen_amount = g_loaded_screens.size();
        for(int n=0; n<screen_amount; n++)
        {
            if (g_loaded_screens[n].getName() == screen_name)
            {
                g_current_screen = g_loaded_screens.get(n);
                break;
            }
        }

        // screen not found in list of existing ones
        if (g_current_screen == NULL)
        {
            assert(false);
            return;
        }

        g_current_screen->beforeAddingWidget();

        // show screen
        g_current_screen->addWidgets();
    }   // switchToScreen

    // ------------------------------------------------------------------------

    void addScreenToList(Screen* cutscene)
    {
        g_loaded_screens.push_back(cutscene);
    }   // addScreenToList

    // ------------------------------------------------------------------------

    void removeScreen(const char* name)
    {
        const int screen_amount = g_loaded_screens.size();
        for(int n=0; n<screen_amount; n++)
        {
            if (g_loaded_screens[n].getName() == name)
            {
                g_current_screen = g_loaded_screens.get(n);
                g_current_screen->unload();
                delete g_current_screen;
                g_current_screen = NULL;
                g_loaded_screens.remove(n);
                break;
            }
        }
    }

    // ------------------------------------------------------------------------
    void reshowCurrentScreen()
    {
        needsUpdate.clearWithoutDeleting();
        g_state_manager->reshowTopMostMenu();
    }   // reshowCurrentScreen

    // ------------------------------------------------------------------------

    /**
     * Clean some of the cached data, either for a shutdown or a reload.
     * If this is a shutdown then you also need to call free().
     */
    void cleanUp()
    {
        // There is no need to delete the skin, the gui environment holds it
        //if (g_skin != NULL) delete g_skin;
        g_skin = NULL;

        for (unsigned int i=0; i<g_loaded_screens.size(); i++)
        {
            g_loaded_screens[i].unload();
        }

        g_current_screen = NULL;
        needsUpdate.clearWithoutDeleting();

        if (ScreenKeyboard::isActive()) ScreenKeyboard::dismiss();
        if (ModalDialog::isADialogActive()) ModalDialog::dismiss();

        //delete g_font;
        g_font->drop();
        g_font = NULL;
        //delete g_title_font;
        g_title_font->drop();
        g_title_font = NULL;
        //delete g_small_font;
        g_small_font->drop();
        g_small_font = NULL;
        g_large_font->drop();
        g_large_font = NULL;
        g_digit_font->drop();
        g_digit_font = NULL;
        g_outline_font->drop();
        g_outline_font = NULL;

        // nothing else to delete for now AFAIK, irrlicht will automatically
        // kill everything along the device
    }   // cleanUp

    // -----------------------------------------------------------------------

    /**
      * To be called after cleanup().
      * The difference between cleanup() and free() is that cleanUp() just
      * removes some cached data but does not actually uninitialize the gui
      * engine. This does.
      */
    void deallocate()
    {
        g_loaded_screens.clearAndDeleteAll();
    }   // deallocate

    // -----------------------------------------------------------------------
    void init(IrrlichtDevice* device_a, IVideoDriver* driver_a,
              AbstractStateManager* state_manager )
    {
        g_env = device_a->getGUIEnvironment();
        g_device = device_a;
        g_driver = driver_a;
        g_state_manager = state_manager;

        for (unsigned int n=0; n<MAX_PLAYER_COUNT; n++)
        {
            g_focus_for_player[n] = NULL;
        }

        /*
         To make the g_font a little bit nicer, we load an external g_font
         and set it as the new default g_font in the g_skin.
         To keep the standard g_font for tool tip text, we set it to
         the built-in g_font.
         */
        try
        {
            g_skin = new Skin(g_env->getSkin());
            g_env->setSkin(g_skin);
            g_skin->drop(); // GUI env grabbed it
            assert(g_skin->getReferenceCount() == 1);
        }
        catch (std::runtime_error& /*err*/)
        {
            Log::error("Engine::init", "Cannot load skin specified in user config. "
                "Falling back to defaults.");
            UserConfigParams::m_skin_file.revertToDefaults();

            try
            {
                g_skin = new Skin(g_env->getSkin());
                g_env->setSkin(g_skin);
                g_skin->drop(); // GUI env grabbed it
                assert(g_skin->getReferenceCount() == 1);
            }
            catch (std::runtime_error& err)
            {
                (void)err;
                Log::fatal("Engine::init", "Canot load default GUI skin");
            }
        }

        RegularFace* regular = font_manager->getFont<RegularFace>();
        BoldFace* bold = font_manager->getFont<BoldFace>();
        DigitFace* digit = font_manager->getFont<DigitFace>();

        ScalableFont* digit_font = new ScalableFont(digit);
        g_digit_font = digit_font;

        ScalableFont* sfont = new ScalableFont(regular);
        g_font = sfont;
        Private::font_height = g_font->getDimension( L"X" ).Height;

        ScalableFont* sfont_larger = new ScalableFont(regular);
        sfont_larger->setScale(1.4f);
        g_large_font = sfont_larger;
        Private::large_font_height = g_large_font->getDimension( L"X" ).Height;

        g_outline_font = new ScalableFont(regular);
        g_outline_font->getFontSettings()->setBlackBorder(true);

        ScalableFont* sfont_smaller = new ScalableFont(regular);
        sfont_smaller->setScale(0.8f);
        g_small_font = sfont_smaller;
        Private::small_font_height = g_small_font->getDimension( L"X" ).Height;

        ScalableFont* sfont2 = new ScalableFont(bold);
        g_title_font = sfont2;
        Private::title_font_height =
            g_title_font->getDimension( L"X" ).Height;

        if (g_font != NULL) g_skin->setFont(g_font);

        // set event receiver
        g_device->setEventReceiver(EventHandler::get());

        g_device->getVideoDriver()
                ->beginScene(true, true, video::SColor(255,100,101,140));
        renderLoading();
        g_device->getVideoDriver()->endScene();
    }   // init

    // -----------------------------------------------------------------------
    void reloadSkin()
    {
        assert(g_skin != NULL);

        irr::gui::IGUISkin* fallbackSkin = g_skin->getFallbackSkin();

        Skin* newSkin;
        try
        {
            // it's important to create the new skin before deleting the old
            // one so that the fallback skin is not dropped
            newSkin = new Skin(fallbackSkin);
        }
        catch (std::runtime_error& /*err*/)
        {
            Log::error("Engine::reloadSkin", "Canot load newly specified skin");
            return;
        }

        assert(g_skin->getReferenceCount() == 1);

        g_skin = newSkin;

        // will also drop (and thus delete) the previous skin
        g_env->setSkin(g_skin);
        g_skin->drop(); // g_env grabbed it
        assert(g_skin->getReferenceCount() == 1);
    }   // reloadSkin

    // -----------------------------------------------------------------------

    void render(float elapsed_time)
    {
#ifndef SERVER_ONLY
        GUIEngine::dt = elapsed_time;

        // Not yet initialized, or already cleaned up
        if (g_skin == NULL) return;

        // ---- menu drawing

        // draw background image and sections

        const GameState gamestate = g_state_manager->getGameState();

        if (gamestate == MENU &&
            GUIEngine::getCurrentScreen() != NULL &&
            !GUIEngine::getCurrentScreen()->needs3D())
        {
            g_skin->drawBgImage();
        }
        else if (gamestate == INGAME_MENU)
        {
            g_skin->drawBGFadeColor();
        }

        g_driver->enableMaterial2D();

        if (gamestate == MENU || gamestate == INGAME_MENU)
        {
            g_skin->renderSections();
        }

        // let irrLicht do the rest (the Skin object will be called for
        // further render)
        g_env->drawAll();

        // ---- some menus may need updating
        if (gamestate != GAME)
        {
            if (ModalDialog::isADialogActive())
                ModalDialog::getCurrent()->onUpdate(dt);
            else
                getCurrentScreen()->onUpdate(elapsed_time);
        }
        else
        {
            if (ModalDialog::isADialogActive())
            {
                ModalDialog::getCurrent()->onUpdate(dt);
            }
            else
            {
                RaceGUIBase* rg = World::getWorld()->getRaceGUI();
                if (rg != NULL) rg->renderGlobal(elapsed_time);
            }
        }

        MessageQueue::update(elapsed_time);

        if (gamestate == INGAME_MENU && dynamic_cast<CutsceneWorld*>(World::getWorld()) != NULL)
        {
            RaceGUIBase* rg = World::getWorld()->getRaceGUI();
            if (rg != NULL) rg->renderGlobal(elapsed_time);
        }

        if (gamestate == MENU || gamestate == INGAME_MENU)
        {
            g_skin->drawTooltips();
        }

        if (gamestate != GAME && !gui_messages.empty())
        {
            core::dimension2d<u32> screen_size = irr_driver->getFrameSize();
            const int text_height = getFontHeight() + 20;
            const int y_from = screen_size.Height - text_height;

            int count = 0;

            std::vector<MenuMessage>::iterator it;
            for (it=gui_messages.begin(); it != gui_messages.end();)
            {
                if ((*it).m_time > 0.0f)
                {
                    (*it).m_time -= dt;

                    core::rect<s32>
                        msgRect(core::position2d<s32>(0,
                                                  y_from - count*text_height),
                                core::dimension2d<s32>(screen_size.Width,
                                                       text_height) );
                    GL32_draw2DRectangle(SColor(255,252,248,230),
                                                       msgRect);
                    Private::g_font->draw((*it).m_message.c_str(),
                                          msgRect,
                                          video::SColor(255, 255, 0, 0),
                                          true /* hcenter */,
                                          true /* vcenter */);
                    count++;
                    it++;
                }
                else
                {
                    it = gui_messages.erase(it);
                }
            }
        }

        // draw FPS if enabled
        if ( UserConfigParams::m_display_fps ) irr_driver->displayFPS();

        g_driver->enableMaterial2D(false);


        if (gamestate == MENU)
        {
            if (DemoWorld::updateIdleTimeAndStartDemo(elapsed_time))
            {
                return;
            }
        }
        else
        {
            DemoWorld::resetIdleTime();
        }

#endif
    }   // render

    // -----------------------------------------------------------------------
    std::vector<irr::video::ITexture*> g_loading_icons;

    void renderLoading(bool clearIcons)
    {
#ifndef SERVER_ONLY
        if (clearIcons) g_loading_icons.clear();

        g_skin->drawBgImage();
        ITexture* loading =
            irr_driver->getTexture(file_manager->getAsset(FileManager::GUI,
                                                          "loading.png"));

        if(!loading)
        {
            Log::fatal("Engine", "Can not find loading.png texture, aborting.");
            exit(-1);
        }
        const int texture_w = loading->getSize().Width;
        const int texture_h = loading->getSize().Height;

        core::dimension2d<u32> frame_size =
            GUIEngine::getDriver()->getCurrentRenderTargetSize();
        const int screen_w = frame_size.Width;
        const int screen_h = frame_size.Height;

        const core::rect< s32 > dest_area =
            core::rect< s32 >(screen_w/2 - texture_w/2,
                              screen_h/2 - texture_h/2,
                              screen_w/2 + texture_w/2,
                              screen_h/2 + texture_h/2);

        const core::rect< s32 > source_area =
            core::rect< s32 >(0, 0, texture_w, texture_h);

        draw2DImage( loading, dest_area, source_area,
                                            0 /* no clipping */, 0,
                                            true /* alpha */);

        // seems like we need to remind irrlicht from time to time to use
        // the Material2D
        irr_driver->getVideoDriver()->enableMaterial2D();
        g_title_font->draw(_("Loading"),
                           core::rect< s32 >( 0, screen_h/2 + texture_h/2,
                                              screen_w, screen_h ),
                           SColor(255,255,255,255),
                           true/* center h */, false /* center v */ );

        const int icon_count = (int)g_loading_icons.size();
        const int icon_size = (int)(screen_w / 16.0f);
        const int ICON_MARGIN = 6;
        int x = ICON_MARGIN;
        int y = screen_h - icon_size - ICON_MARGIN;
        for (int n=0; n<icon_count; n++)
        {
            draw2DImage(g_loading_icons[n],
                              core::rect<s32>(x, y, x+icon_size, y+icon_size),
                              core::rect<s32>(core::position2d<s32>(0, 0),
                                              g_loading_icons[n]->getSize()),
                              NULL, NULL, true
                                  );

            x += ICON_MARGIN + icon_size;
            if (x + icon_size + ICON_MARGIN/2 > screen_w)
            {
                y = y - ICON_MARGIN - icon_size;
                x = ICON_MARGIN;
            }
        }
#endif
    } // renderLoading

    // -----------------------------------------------------------------------

    void addLoadingIcon(irr::video::ITexture* icon)
    {
        if (icon != NULL)
        {
            g_loading_icons.push_back(icon);

            g_device->getVideoDriver()
                    ->beginScene(true, true, video::SColor(255,100,101,140));
            renderLoading(false);
            g_device->getVideoDriver()->endScene();
        }
        else
        {
            Log::warn("Engine::addLoadingIcon", "Given "
                "NULL icon");
        }
    } // addLoadingIcon

    // -----------------------------------------------------------------------

    Widget* getWidget(const char* name)
    {
        if (ScreenKeyboard::isActive())
        {
            Widget* widget = ScreenKeyboard::getCurrent()->getWidget(name);
            if (widget != NULL) 
                return widget;
        }
        
        // if a modal dialog is shown, search within it too
        if (ModalDialog::isADialogActive())
        {
            Widget* widget = ModalDialog::getCurrent()->getWidget(name);
            if (widget != NULL) 
                return widget;
        }

        Screen* screen = getCurrentScreen();

        if (screen == NULL) return NULL;

        return screen->getWidget(name);
    }   // getWidget

    // -----------------------------------------------------------------------
    Widget* getWidget(const int id)
    {
        if (ScreenKeyboard::isActive())
        {
            Widget* widget = ScreenKeyboard::getCurrent()->getWidget(id);
            if (widget != NULL) 
                return widget;
        }
        
        // if a modal dialog is shown, search within it too
        if (ModalDialog::isADialogActive())
        {
            Widget* widget = ModalDialog::getCurrent()->getWidget(id);
            if (widget != NULL) 
                return widget;
        }

        Screen* screen = getCurrentScreen();

        if (screen == NULL) return NULL;

        return screen->getWidget(id);
    }   // getWidget
}   // namespace GUIEngine
