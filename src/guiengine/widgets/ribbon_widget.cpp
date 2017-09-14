//  SuperTuxKart - a fun racing game with go-kart
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

#include "guiengine/widgets/ribbon_widget.hpp"

#include <cmath>

#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/layout_manager.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "input/input_manager.hpp"
#include "io/file_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/vs.hpp"

#include <IGUIElement.h>
#include <IGUIEnvironment.h>
#include <IGUIButton.h>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;

const char RibbonWidget::NO_ITEM_ID[] = "?";


// ----------------------------------------------------------------------------

RibbonWidget::RibbonWidget(const RibbonType type) : Widget(WTYPE_RIBBON)
{
    for (unsigned int n=0; n<MAX_PLAYER_COUNT; n++)
    {
        m_selection[n] = -1;
    }
    m_selection[0] = 0; // only player 0 has a selection by default

    m_ribbon_type = type;
    m_mouse_focus = NULL;
    m_listener    = NULL;

    m_check_inside_me = true;
    m_supports_multiplayer = (type == RIBBON_TOOLBAR);

    updateSelection();
}   // RibbonWidget

// ----------------------------------------------------------------------------

RibbonWidget::~RibbonWidget()
{
    m_active_children.clearWithoutDeleting();
}   // ~RibbonWidget

// ----------------------------------------------------------------------------
void RibbonWidget::add()
{
    assert(m_magic_number == 0xCAFEC001);
    assert(m_x > -10.0f);
    assert(m_y > -10.0f);
    assert(m_w > 0.0f);
    assert(m_h > 0.0f);


    m_labels.clearWithoutDeleting();

    rect<s32> widget_size = rect<s32>(m_x, m_y, m_x + m_w, m_y + m_h);

    int id = (m_reserved_id == -1 ? getNewID() : m_reserved_id);

    IGUIButton * btn = GUIEngine::getGUIEnv()->addButton(widget_size,
                                                         m_parent, id, L"");
    m_element = btn;

    m_active_children.clearWithoutDeleting(); // Is just a copy of m_children without the deactivated children. m_children takes care of memory.
    for (unsigned int i=0; i<m_children.size(); i++)
    {
        if (m_children[i].isVisible())
        {
            m_active_children.push_back(m_children.get(i));
        }
    }
    const int subbuttons_amount = m_active_children.size();

    // For some ribbon types, we can have unequal sizes depending on whether
    // items have labels or not
    int with_label = 0;
    int without_label = 0;

    // ---- check how much space each child button will take and fit
    // them within available space
    int total_needed_space = 0;
    for (int i=0; i<subbuttons_amount; i++)
    {
        // FIXME: why do I manually invoke the Layout Manager here?
        LayoutManager::readCoords(m_active_children.get(i));
        LayoutManager::applyCoords(m_active_children.get(i), NULL, this);

        if (m_active_children[i].m_type != WTYPE_ICON_BUTTON &&
            m_active_children[i].m_type != WTYPE_BUTTON)
        {
            Log::warn("RiggonWidget", "Ribbon widgets can only have "
                            "(icon)button widgets as children");
            continue;
        }

        // ribbon children must not be keyboard navigatable, the parent
        // ribbon takes care of that
        if (m_active_children[i].m_type == WTYPE_ICON_BUTTON)
        {
            IconButtonWidget* icon = ((IconButtonWidget*)m_active_children.get(i));
            icon->m_tab_stop = false;
        }


        bool has_label_underneath = m_active_children[i].m_text.size() > 0;
        if (m_active_children[i].m_properties[PROP_LABELS_LOCATION].size() > 0)
        {
            has_label_underneath = false;
        }

        if (has_label_underneath) with_label++;
        else                      without_label++;

        total_needed_space += m_active_children[i].m_w;
    }

    //int biggest_y = 0;
    const int button_y = 10;

    const int one_button_space = (subbuttons_amount == 0 ? m_w :
        int(roundf((float)m_w / (float)subbuttons_amount)));

    int widget_x = -1;

    // ---- add children
    for (int i=0; i<subbuttons_amount; i++)
    {
        // ---- tab ribbons
        if (getRibbonType() == RIBBON_TABS)
        {
            const int large_tab = (int)((with_label + without_label)
                                        *one_button_space
                                        / (with_label + without_label/2.0f));
            const int small_tab = large_tab/2;

            stringw& message = m_active_children[i].m_text;


            if (message.size() == 0)
            {
                if (widget_x == -1) widget_x = small_tab/2;
                else widget_x += small_tab/2;
            }
            else
            {
                if (widget_x == -1) widget_x = large_tab/2;
                else widget_x += large_tab/2;
            }

            IGUIButton * subbtn = NULL;
            rect<s32> subsize = rect<s32>(widget_x - large_tab/2+2,  0,
                                          widget_x + large_tab/2-2,  m_h);

            if (message.size() == 0)
            {
                subsize = rect<s32>(widget_x - small_tab/2+2,  0,
                                    widget_x + small_tab/2-2,  m_h);
            }

            if (m_active_children[i].m_type == WTYPE_BUTTON)
            {
                subbtn = GUIEngine::getGUIEnv()
                       ->addButton(subsize, btn, getNewNoFocusID(),
                                   message.c_str(), L"");
                subbtn->setTabStop(false);
                subbtn->setTabGroup(false);

                if ((int)GUIEngine::getFont()->getDimension(message.c_str())
                                              .Width > subsize.getWidth()  &&
                    message.findFirst(L' ') == -1                          &&
                    message.findFirst(L'\u00AD') == -1                        )
                {
                    // if message too long and contains no space and no soft
                    // hyphen, make the font smaller
                    subbtn->setOverrideFont(GUIEngine::getSmallFont());
                }
            }
            else if (m_active_children[i].m_type == WTYPE_ICON_BUTTON)
            {
                rect<s32> icon_part = rect<s32>(15,
                                                0,
                                                subsize.getHeight()+15,
                                                subsize.getHeight());

                if (message.size() == 0)
                {
                    const int x = subsize.getWidth()/2 - subsize.getHeight()/2;
                    // no label, only icon, so center the icon
                    icon_part = rect<s32>(x,
                                          0,
                                          x + subsize.getHeight(),
                                          subsize.getHeight());
                }

                // label at the *right* of the icon (for tabs)
                rect<s32> label_part = rect<s32>(subsize.getHeight()+15,
                                                 0,
                                                 subsize.getWidth()-15,
                                                 subsize.getHeight());

                // use the same ID for all subcomponents; since event handling
                // is done per-ID, no matter which one your hover, this
                // widget will get it
                int same_id = getNewNoFocusID();
                subbtn = GUIEngine::getGUIEnv()->addButton(subsize, btn,
                                                           same_id, L"", L"");

                IGUIButton* icon =
                    GUIEngine::getGUIEnv()->addButton(icon_part, subbtn,
                                                      same_id, L"");
                icon->setScaleImage(true);
                std::string filename = file_manager->getAsset(
                                     m_active_children[i].m_properties[PROP_ICON]);
                icon->setImage( irr_driver->getTexture(filename.c_str()) );
                icon->setUseAlphaChannel(true);
                icon->setDrawBorder(false);
                icon->setTabStop(false);

                IGUIStaticText* label =
                    GUIEngine::getGUIEnv()->addStaticText(message.c_str(),
                                                          label_part,
                                                          false /* border */,
                                                          true /* word wrap */,
                                                          subbtn, same_id);

                if ((int)GUIEngine::getFont()->getDimension(message.c_str())
                                              .Width > label_part.getWidth()&&
                    message.findFirst(L' ') == -1                           &&
                    message.findFirst(L'\u00AD') == -1                        )
                {
                    // if message too long and contains no space and no soft
                    // hyphen, make the font smaller
                    label->setOverrideFont(GUIEngine::getSmallFont());
                }
                label->setTextAlignment(EGUIA_CENTER, EGUIA_CENTER);
                label->setTabStop(false);
                label->setNotClipped(true);
                label->setRightToLeft(translations->isRTLText(message));
                m_labels.push_back(label);

                subbtn->setTabStop(false);
                subbtn->setTabGroup(false);
            }
            else
            {
                Log::error("RibbonWidget", "Invalid tab bar contents");
            }

            m_active_children[i].m_element = subbtn;

            if (message.size() == 0) widget_x += small_tab/2;
            else                     widget_x += large_tab/2;
        }
        // ---- icon ribbons
        else if (m_active_children[i].m_type == WTYPE_ICON_BUTTON)
        {
            if (widget_x == -1) widget_x = one_button_space/2;

            // find how much space to keep for the label under the button.
            // consider font size, whether the label is multiline, etc...
            bool has_label = m_active_children[i].m_text.size() > 0;

            if (m_active_children[i].m_properties[PROP_LABELS_LOCATION].size() > 0)
            {
                has_label = false;
            }

            const int needed_space_under_button = has_label
                                                ? GUIEngine::getFontHeight()
                                                : 10;

            float imageRatio =
                (float)m_active_children[i].m_w / (float)m_active_children[i].m_h;

            // calculate the size of the image
            std::string filename =
                file_manager->getAsset(m_active_children[i].m_properties[PROP_ICON]);
            video::ITexture* image =
                irr_driver->getTexture((filename).c_str());
            if(!image)
            {
                std::string file = file_manager->getAsset(FileManager::GUI,"main_help.png");
                image = irr_driver->getTexture(file);
                if(!image)
                    Log::fatal("RibbonWidget",
                        "Can't find fallback texture 'gui/main_help.png, aborting.");
            }

            float image_h = (float)image->getSize().Height;
            float image_w = image_h*imageRatio;
            float zoom = (float) (m_h - button_y - needed_space_under_button) / image_h;
            float zoom_x = (float) one_button_space / image_w;
            if(zoom_x < zoom)
                zoom = zoom_x;

            // ---- add bitmap button part
            // backup and restore position in case the same object is added
            // multiple times (FIXME: unclean)
            int old_x = m_active_children[i].m_x;
            int old_y = m_active_children[i].m_y;
            int old_w = m_active_children[i].m_w;
            int old_h = m_active_children[i].m_h;

            m_active_children[i].m_x = widget_x - int(image_w*zoom/2.0f);
            m_active_children[i].m_y = button_y;
            m_active_children[i].m_w = int(image_w*zoom);
            m_active_children[i].m_h = int(image_h*zoom);

            IconButtonWidget* icon = ((IconButtonWidget*)m_active_children.get(i));

            if (icon->m_properties[PROP_EXTEND_LABEL].size() == 0)
            {
                icon->m_properties[PROP_EXTEND_LABEL] =
                    StringUtils::toString(one_button_space - icon->m_w);
            }

            m_active_children.get(i)->m_parent = btn;
            m_active_children.get(i)->add();

            // restore backuped size and location (see above for more info)
            m_active_children[i].m_x = old_x;
            m_active_children[i].m_y = old_y;
            m_active_children[i].m_w = old_w;
            m_active_children[i].m_h = old_h;

            // the label itself will be added by the icon widget. since it
            // adds the label outside of the widget area it is assigned to,
            // the label will appear in the area we want at the bottom

            widget_x += one_button_space;
        }
        else
        {
            Log::warn("RiggonWidget", "Invalid contents type in ribbon");
        }


        //m_children[i].id = subbtn->getID();
        m_active_children[i].m_event_handler = this;
    }// next sub-button

    id = m_element->getID();
    m_element->setTabOrder(id);
    m_element->setTabGroup(false);
    updateSelection();

    if (!m_is_visible)
        setVisible(false);
}   // add

// ----------------------------------------------------------------------------

void RibbonWidget::addTextChild(const wchar_t* text, const std::string &id)
{
    // This method should only be called BEFORE a widget is added
    assert(m_element == NULL);

    ButtonWidget* item = new ButtonWidget();
    item->m_text = text;
    item->m_properties[PROP_ID] = id;

    m_children.push_back(item);
}   // addTextChild

// ----------------------------------------------------------------------------

void RibbonWidget::addIconChild(const wchar_t* text, const std::string &id,
                         const int w, const int h,
                         const std::string &icon,
                         const IconButtonWidget::IconPathType icon_path_type)
{
    // This method should only be called BEFORE a widget is added
    assert(m_element == NULL);

    IconButtonWidget* ribbon_item = new IconButtonWidget();

    ribbon_item->m_properties[PROP_ID] = id;

    ribbon_item->setImage(icon.c_str(), icon_path_type);

    ribbon_item->m_properties[PROP_WIDTH]  = StringUtils::toString(w);
    ribbon_item->m_properties[PROP_HEIGHT] = StringUtils::toString(h);

    ribbon_item->m_text = text;
    m_children.push_back(ribbon_item);
}   // addIconChild

// ----------------------------------------------------------------------------

void RibbonWidget::clearAllChildren()
{
    // This method should only be called BEFORE a widget is added
    assert(m_element == NULL);

    m_children.clearAndDeleteAll();
}   // clearAllChildren

// ----------------------------------------------------------------------------

void RibbonWidget::removeChildNamed(const char* name)
{
    // This method should only be called BEFORE a widget is added
    assert(m_element == NULL);

    for (Widget* child : m_children)
    {
        if (child->m_properties[PROP_ID] == name)
        {
            m_children.erase(child);
            return;
        }
    }
}

// ----------------------------------------------------------------------------

void RibbonWidget::select(std::string item, const int mousePlayerID)
{
    const int subbuttons_amount = m_active_children.size();

    for (int i=0; i<subbuttons_amount; i++)
    {
        if (m_active_children[i].m_properties[PROP_ID] == item)
        {
            m_selection[mousePlayerID] = i;
            updateSelection();
            return;
        }
    }

}   // select

// ----------------------------------------------------------------------------
EventPropagation RibbonWidget::rightPressed(const int playerID)
{
    EventPropagation result = m_ribbon_type != RIBBON_TOOLBAR ? EVENT_LET : EVENT_BLOCK;
    
    if (m_deactivated) return result;
    // empty ribbon, or only one item (can't move right)
    if (m_active_children.size() < 2) return result;

    m_selection[playerID]++;

    if (m_selection[playerID] >= int(m_active_children.size()))
    {
        if (m_listener != NULL) m_listener->onRibbonWidgetScroll(1);

        m_selection[playerID] = m_event_handler ? m_active_children.size()-1 : 0;
    }
    updateSelection();

    if (m_ribbon_type == RIBBON_COMBO || m_ribbon_type == RIBBON_TABS)
    {
        const int mousePlayerID = input_manager->getPlayerKeyboardID();
        if (playerID == mousePlayerID || playerID == PLAYER_ID_GAME_MASTER)
        {
            m_mouse_focus = m_active_children.get(m_selection[playerID]);
        }
    }

    // if we reached a filler item, move again (but don't wrap)
    if (getSelectionIDString(playerID) == RibbonWidget::NO_ITEM_ID)
    {
        if (m_selection[playerID] + 1 < int(m_active_children.size()))
        {
            rightPressed(playerID);
        }
    }

    return result;
}   // rightPressed

// ----------------------------------------------------------------------------
EventPropagation RibbonWidget::leftPressed(const int playerID)
{
    EventPropagation result = m_ribbon_type != RIBBON_TOOLBAR ? EVENT_LET : EVENT_BLOCK;
    
    if (m_deactivated) return result;
    // empty ribbon, or only one item (can't move left)
    if (m_active_children.size() < 2) return result;

    m_selection[playerID]--;
    if (m_selection[playerID] < 0)
    {
        if (m_listener != NULL) m_listener->onRibbonWidgetScroll(-1);

        m_selection[playerID] = m_event_handler
                              ? 0
                              : m_active_children.size()-1;
    }

    updateSelection();

    if (m_ribbon_type == RIBBON_COMBO)
    {
        const int mousePlayerID = input_manager->getPlayerKeyboardID();
        if (playerID == mousePlayerID || playerID == PLAYER_ID_GAME_MASTER)
        {
            m_mouse_focus = m_active_children.get(m_selection[playerID]);
        }
    }

    // if we reached a filler item, move again (but don't wrap)
    if (getSelectionIDString(playerID) == RibbonWidget::NO_ITEM_ID)
    {
        if (m_selection[playerID] > 0) leftPressed(playerID);
    }

    //if (m_ribbon_type != RIBBON_TOOLBAR)
    //{
        //GUIEngine::transmitEvent( this, m_properties[PROP_ID], playerID );
    //}

    return result;
}   // leftPressed

// ----------------------------------------------------------------------------

EventPropagation RibbonWidget::focused(const int playerID)
{
    Widget::focused(playerID);

    if (m_active_children.size() < 1) return EVENT_LET; // empty ribbon

    if (m_ribbon_type == RIBBON_COMBO || m_ribbon_type == RIBBON_TABS)
    {
        const int mousePlayerID = input_manager->getPlayerKeyboardID();
        if (m_mouse_focus == NULL && m_selection[playerID] != -1  &&
            (playerID == mousePlayerID || playerID == PLAYER_ID_GAME_MASTER))
        {
            m_mouse_focus = m_active_children.get(m_selection[playerID]);
            m_mouse_focus->focused(playerID);
        }
    }
    else
    {
        if (m_selection[playerID] != -1)
        {
            int selection = m_selection[playerID];
            if (selection < (int)m_active_children.size())
                m_active_children.get(selection)->focused(playerID);
        }
    }

    if (m_listener != NULL) m_listener->onRibbonWidgetFocus( this, playerID );


    return EVENT_LET;
}   // focused

// ----------------------------------------------------------------------------

void RibbonWidget::unfocused(const int playerID, Widget* new_focus)
{
    if (new_focus != NULL && new_focus != this && !m_active_children.contains(new_focus))
    {
        if (m_selection[playerID] >= 0 && m_selection[playerID] < int(m_active_children.size()))
        {
            m_active_children.get(m_selection[playerID])->unfocused(playerID, new_focus);
        }
    }

    //if (m_selection[0] != -1) m_children.get(m_selection[0])->unfocused(0);
}   // unfocused

// ----------------------------------------------------------------------------
EventPropagation RibbonWidget::mouseHovered(Widget* child,
                                            const int mousePlayerID)
{
    if (m_deactivated) return EVENT_LET;

    const int subbuttons_amount = m_active_children.size();

    if (m_ribbon_type == RIBBON_COMBO || m_ribbon_type == RIBBON_TABS)
    {
        //Log::info("RibbonWidget", "Setting m_mouse_focus");
        m_mouse_focus = child;
    }

    // In toolbar ribbons, hovering selects
    if (m_ribbon_type == RIBBON_TOOLBAR)
    {
        for (int i=0; i<subbuttons_amount; i++)
        {
            if (m_active_children.get(i) == child)
            {
                // Was already selected, don't send another event
                if (m_selection[mousePlayerID] == i) return EVENT_BLOCK;

                // Don't change selection on hover for others
                m_selection[mousePlayerID] = i;
                break;
            }
        }
    }

    updateSelection();
    return EVENT_BLOCK;
}   // mouseHovered

// ----------------------------------------------------------------------------
const std::string& RibbonWidget::getSelectionIDString(const int playerID)
{
    static std::string empty;
    if (m_selection[playerID] == -1) return empty;
    if (m_active_children.size() == 0)      return empty;

    // This can happen if an addon is removed, which causes a tab group
    // to be removed. If this tab group was previously selected, an
    // invalid array element would be accessed. In this case just pretend
    // that the first child was selected previously.
    if(m_selection[playerID]>=(int)m_active_children.size())
        return m_active_children[0].m_properties[PROP_ID];
    return m_active_children[m_selection[playerID]].m_properties[PROP_ID];
}   // getSelectionIDString

// ----------------------------------------------------------------------------
void RibbonWidget::updateSelection()
{
    const int subbuttons_amount = m_active_children.size();

    // FIXME: m_selection, m_selected, m_mouse_focus... what a mess...

    // Update selection flags for mouse player
    for (unsigned int p=0; p<MAX_PLAYER_COUNT; p++)
    {
        for (int i=0; i<subbuttons_amount; i++)
        {
            bool new_val = (i == m_selection[p]);
            if (!new_val && m_active_children[i].m_selected[p])
            {
                m_active_children[i].unfocused(PLAYER_ID_GAME_MASTER, NULL);
            }
            m_active_children[i].m_selected[p] = new_val;
            if (new_val) m_active_children[i].focused(PLAYER_ID_GAME_MASTER);
        }
    }

    if (m_listener) m_listener->onSelectionChange();
}   // updateSelection

// ----------------------------------------------------------------------------
EventPropagation RibbonWidget::onActivationInput(const int playerID)
{
    assert(m_magic_number == 0xCAFEC001);

    if (m_deactivated)
        return EVENT_BLOCK;

    if (m_selection[playerID] > -1 &&
        m_selection[playerID] < (int)(m_active_children.size()))
    {
        if (m_active_children[m_selection[playerID]].m_deactivated)
        {
            return EVENT_BLOCK;
        }
    }

    return EVENT_LET;
}

// ----------------------------------------------------------------------------
EventPropagation RibbonWidget::transmitEvent(Widget* w,
                                             const std::string& originator,
                                             const int playerID)
{
    assert(m_magic_number == 0xCAFEC001);


    if (!m_deactivated)
    {
        const int subbuttons_amount = m_active_children.size();

        for (int i=0; i<subbuttons_amount; i++)
        {
            if (m_active_children[i].m_properties[PROP_ID] == originator)
            {
                m_selection[playerID] = i;
                break;
            }
        }

        updateSelection();
    }

    // bring focus back to enclosing ribbon widget
    this->setFocusForPlayer( playerID );

    if (m_selection[playerID] > -1 && 
        m_selection[playerID] < (int)(m_active_children.size()))
    {
        if (m_active_children[m_selection[playerID]].m_deactivated)
        {
            GUIEngine::getCurrentScreen()->onDisabledItemClicked(
                      m_active_children[m_selection[playerID]].m_properties[PROP_ID]);

            return EVENT_BLOCK;
        }
    }

    return EVENT_LET;
}   // transmitEvent

// ----------------------------------------------------------------------------
void RibbonWidget::setLabel(const unsigned int id, irr::core::stringw new_name)
{
    if (m_element == NULL)
    {
        // before adding
        m_children[id].setText(new_name);
    }
    else
    {
        // after adding
        // ignore this call for ribbons without labels
        if (m_labels.size() == 0) return;

        assert(id < m_labels.size());
        m_labels[id].setText(new_name.c_str());
        //m_text = new_name;
    }
}   // setLabel

// ----------------------------------------------------------------------------

void RibbonWidget::setItemVisible(const unsigned int id, bool visible)
{
    m_children[id].setVisible(visible);

    // ignore this call for ribbons without labels
    if (m_labels.size() == 0) return;

    m_labels[id].setVisible(visible);
} // RibbonWidget

// ----------------------------------------------------------------------------
int RibbonWidget::findItemNamed(const char* internalName)
{
    const int size = m_children.size();

    //printf("Ribbon : Looking for %s among %i items\n", internalName, size);

    for (int n=0; n<size; n++)
    {
        //printf("     Ribbon : Looking for %s in item %i : %s\n",
        // internalName, n, m_children[n].m_properties[PROP_ID].c_str());
        if (m_children[n].m_properties[PROP_ID] == internalName) return n;
    }
    return -1;
}   // findItemNamed

// ----------------------------------------------------------------------------
Widget* RibbonWidget::findWidgetNamed(const char* internalName)
{
    int id = findItemNamed(internalName);
    if (id >= 0)
        return m_children.get(id);
    return NULL;
}   // findItemNamed
