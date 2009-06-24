//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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


/*
+---------+
+ Widgets +
+---------+

_______________________________________________________
Internal constant       Name in XML files
_______________________________________________________
WTYPE_RIBBON            "ribbon", "buttonbar", "tabs"
appears an horizontal bar containing elements laid in a row, each being and icon and/or a label
the "ribbon" subcategory will behave a bit like a radio button group, i.e. one element must selected.
events are triggered as soon as a choice is selected (can be simply by hovering).
the "buttonbar" subcategory treats children buttons as action buttons, which means they can't have a
'selected' state, only focused or not (i.e. there is no selection that remains if you leave this area).
events are triggered only on enter/fire.
the "tabs" subcategory will show a tab bar. behaviour is same as normal ribbon, only looks are different.
Orientation of tabs (up or down) is automatically inferred from on-screen position
Ribbon widgets are of spawn type (<ribbon> ... </ribbon>) and may contain icon-buttons or buttons as children.
Property PROP_SQUARE can be set to tell the engine if the ribbon's contents are rectangular or not (this will
affect the type of highlighting used)
 * Note : all elements within a ribbon must have an 'ID' property

WTYPE_SPINNER           "spinner", "gauge"
A spinner component (lets you choose numbers). Sprecify PROP_MIN_VALUE and PROP_MAX_VALUE to have control
over values (default will be from 0 to 10). You can specify an icon; then, include a sprintf format string
like %i in the name, and at runtime the current number will be inserted into the given name to find the
right file for each possible value the spinner can take. It may also display arbitrary text instead of
numbers, though this cannot be achieve in the XML file; use the ->addLabel(...) method in code to do this.
The "gauge" variant behaves similarly, but a fill band shows how close to the max the value is.

WTYPE_BUTTON            "button"
A plain text buttons.

WTYPE_ICON_BUTTON       "icon-button", "icon"
A component with an image, and optional text to go under it. The "icon" variant will have no border and will not
be clickable. PROP_ICON is mandatory for this component.

WTYPE_CHECKBOX          "checkbox"
A checkbox. Not used at the moment.

WTYPE_LABEL             "label"
A plain label. Supports properties PROP_WORD_WRAP and PROP_TEXT_ALIGN.

WTYPE_SPACER            "spacer"
Some blank space; not visible on screen.

WTYPE_DIV               "div", "box"
A container. Does not do much on itself, but is useful to lay out children automatically. Divs can be nested.
Supports property PROP_LAYOUT. Of spawn type (<div>...</div>, place children within)
 "box" is a variant that acts exactly the same but is visible on-screen

WTYPE_RIBBON_GRID       "ribbon_grid", "scrollable_ribbon", "scrollable_toolbar"
Shows a scrollable grid of icons. NOT of spawn type (<ribbon_grid .../>), contents must be programmatically set at runtime.
Property PROP_SQUARE can be set to tell the engine if the ribbon's contents are rectangular or not (this will
affect the type of highlighting used). Supports an optional label at the bottom if PROP_TEXT is set.
PROP_CHILD_WIDTH and PROP_CHILD_HEIGHT are mandatory (so at least aspect ratio of elements that will later be added isk nown)
An interesting aspect of PROP_CHILD_WIDTH and PROP_CHILD_HEIGHT is that you can use them to show textures to any aspect ratio
you want (so you can e.g. save textures to a power-of-two size like 256x256, but then show it in 4:3 ratio).
Gives a special meaning to the text parameter. A value of "bottom" means to display the name of the selected icon at the bottom.
A value of "all" means that each icon shall have its name under it.
The "scrollable_ribbon" and "scrollable_toolbar" subtypes are single-line scrollable ribbons; they use the ribbon-grid
implementation since it already supports scrolling so no need to duplicate code... The difference between both is that
'scrollable_ribbon always has a value selected (like in a combo box, or radio buttons), while 'scrollable_toolbar' is a
scrollable list of buttons that can be pressed to trigger actions.
 
WTYPE_MODEL_VIEW        "model"
Displays a model. Currently incomplete. Contents must be set programmatically.

WTYPE_LIST              "list"
Displays a list. Currently incomplete. Contents must be set programmatically.

+------------+
+ Properties +
+------------+

_______________________________________________________
Internal constant       Name in XML files
_______________________________________________________

PROP_ID                 "id"
gives a unique internal name to each object using this property. It will be
used in events callbacks to determine what action occurred. Can be omitted
on components that do not trigger events (e.g. labels)

PROP_TEXT               "text"
gives text (a label) to the widget where supported. Ribbon-grids give a special meaning
to this parameter, see ribbon-grid docs above.

PROP_ICON               "icon"
give an icon to the widget. Property contents is the path to the file, relative
relative to the /data directory of STK.

PROP_TEXT_ALIGN         "text_align"
used exclusively by label components. Value can be "right" or "center" (left used if not specified).
PROP_WORD_WRAP          "word_wrap"
used exclusively by label components. Value can be "true" to indicate that long text should spawn on
multiple lines.

PROP_MIN_VALUE          "min_value"
PROP_MAX_VALUE          "max_value"
used to specify a minimum and maximum value for numeric widgets (c.f. spinner)

PROP_X                  "x"
PROP_Y                  "y"
sets the position (location) of a widget, relative to its parent (container <div> or screen if none).
A plain number will be interpreted as an aabsolute position in pixels. A '%' sign may be added to the
given number to mean that the location is specified in terms of a percentage of parent size (parent size
means the parent <div> or the whole screen if none). A negative value can also be passed to start coordinate
from right and/or bottom, instead of starting from top-left corner as usual.
Note that in many cases, it is not necessary to manually a position. Div layouts will often manage that
for you (see PROP_LAYOUT). Other widgets will also automativally manage the position and size of their children,
for instance ribbons.

PROP_WIDTH              "width"
PROP_HEIGHT             "height"
give dimensions to the widget. A plain number will be interpreted as an aabsolute position in pixels.
A '%' sign may be added to the given number to mean that the size is specified in terms of a percentage
of parent size (parent size means the parent <div> or the whole screen if none).
Note that in many cases, it is not necessary to manually a size. Div layouts will often manage that
for you (see PROP_LAYOUT). In addition, sizes are automatically calculated for widgets made of icons
and/or text like labels and plain icons. Other widgets will also automativally manage the position and
size of their children, for instance ribbons.

PROP_MAX_WIDTH          "max_width"
PROP_MAX_HEIGHT         "max_height"
The maximum size a widget can take; especially useful when using percentages and proportions.
 
PROP_CHILD_WIDTH        "child_width"
PROP_CHILD_HEIGHT       "child_height"
Used exclusively by the ribbon grid widget. See docs for this widget above.

PROP_GROW_WITH_TEXT     "grow_with_text"
Reserved, but currently unimplemented and unused.

PROP_LAYOUT             "layout"
Valid on 'div' containers. Value can be "horizontal-row" or "vertical-row". This means x and y coordinates
of all children will automatically be calculated at runtime, so they are laid in a row. Width and height can
be set absolutely as usual, but can also be determined dynamically according to available screen space. Also
see PROP_ALIGN and PROP_PROPORTION to known more about controlling layouts. Note that all components within a
layed-out div will ignore all x/y coordinates you may give them as parameter.

PROP_ALIGN              "align"
For widgets located inside a vertical-row layout div. Value can be "left", "center" or "right". Determines how
the x coord of the widget will be determined.

PROP_PROPORTION         "proportion"
Helps  determining widget size dynamically (according to available screen space) in layed-out divs. In a
vertical row layout, proportion sets the height of the item. In an horizontal row, it sets the width of
the item. Proportions are always evaluated relative to the proportions of other widgets in the same div.
If one div contains 4 widgets, and their proportions are 1-2-1-1, it means the second must take twice as
much space as the 3 others. In this case, 10-20-10-10 would do the exact same effect. 1-1-1-1 would mean
all take 1/4 of the available space. Note that it is allowed to mix absolute widget sizes and proportions;
in this case, widgets with absolute size are evaluated first, and the dynamically-sized ones split the
remaining space according to their proportions.

PROP_SQUARE             "square_items"
Valid on Ribbons or RibbonGrids. Can be "true" (omitting it means "false"). Indicates whether the contents
use rectangular icons (this will affect the type of focus/highlighting used)

*/

#ifndef HEADER_ENGINE_HPP
#define HEADER_ENGINE_HPP

#include <irrlicht.h>
#include <string>

#include "gui/widget.hpp"
#include "utils/ptr_vector.hpp"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


namespace GUIEngine
{
    class Screen;
    class Widget;
    
    extern IrrlichtDevice* getDevice();
    extern IGUIEnvironment* getGUIEnv();
    extern IVideoDriver* getDriver();
    extern IGUIFont* getFont();

    float getLatestDt();
    
    // Widgets that need to be notified at every frame can add themselves there
    extern ptr_vector<Widget, REF> needsUpdate;
    
    void init(irr::IrrlichtDevice* device, irr::video::IVideoDriver* driver, void (*eventCallback)(Widget* widget, std::string& name) );
    void cleanUp();
    void switchToScreen(const char* );
    void clear();
    void cleanForGame();
    
    Screen* getCurrentScreen();
    void reshowCurrentScreen();
    
    void render(float dt);
    void transmitEvent(Widget* widget, std::string& name);
     
}

#endif
