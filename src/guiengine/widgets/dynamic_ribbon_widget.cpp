//  Supertuxkart - a fun racing game with go-kart
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

#include "config/user_config.hpp"
#include "font/font_manager.hpp"
#include "font/regular_face.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "io/file_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/vs.hpp"

#include <IrrlichtDevice.h>
#include <IGUIEnvironment.h>
#include <iostream>
#include <algorithm>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr::gui;

DynamicRibbonWidget::DynamicRibbonWidget(const bool combo, const bool multi_row) : Widget(WTYPE_DYNAMIC_RIBBON)
{
    m_scroll_offset        = 0;
    m_needed_cols          = 0;
    m_col_amount           = 0;
    m_previous_item_count  = 0;
    m_max_label_length     = 0;
    m_multi_row            = multi_row;
    m_combo                = combo;
    m_has_label            = false;
    m_left_widget          = NULL;
    m_right_widget         = NULL;
    m_check_inside_me      = true;
    m_supports_multiplayer = true;
    m_scrolling_enabled    = true;
    m_animated_contents    = false;
    m_font                 = new ScalableFont(font_manager->getFont<RegularFace>());

    // by default, set all players to have no selection in this ribbon
    for (unsigned int n=0; n<MAX_PLAYER_COUNT; n++)
    {
        m_selected_item[n] = -1;
    }
    m_selected_item[0] = 0; // only player 0 has a selection by default

    m_item_count_hint = 0;

    m_max_label_width = 0;

    m_scroll_callback.callback = NULL;
    m_scroll_callback.data = NULL;
}
// -----------------------------------------------------------------------------
DynamicRibbonWidget::~DynamicRibbonWidget()
{
    m_font->drop();
    m_font = NULL;
    if (m_animated_contents)
    {
        GUIEngine::needsUpdate.remove(this);
    }
}

// -----------------------------------------------------------------------------

/** Function that estimates the area (in squared pixels) that ribbon icons
  * would take given a number of rows (used to estimate the best number of
  * rows)
  * \param[out] visibleItems number of items that can be displayed in this
  *                          configuration
  * \param[out] takenArea    how many square pixels are taken by the icons
  * \param[out] itemHeight   how high each item would be in this configuration
  */
void estimateIconAreaFor(const int rowCount, const int wantedIconWidth,
                         const int width, const int height,
                         const float iconAspectRatio, const int maxIcons,
                         int* visibleItems, int* takenArea, int* itemHeight)
{
    assert(height > 0);
    const int row_height = height / rowCount;

    float icon_height = (float)row_height;
    float icon_width = row_height * iconAspectRatio;
    *itemHeight = int(icon_height);

    const int icons_per_row = std::min(int(width / icon_width), int(width / wantedIconWidth));

    *visibleItems = std::min(maxIcons, icons_per_row * rowCount);
    *takenArea = int(*visibleItems * icon_width * icon_height);
}

void DynamicRibbonWidget::add()
{
    //printf("****DynamicRibbonWidget::add()****\n");

    m_has_label = (m_properties[PROP_LABELS_LOCATION] == "bottom");

    assert( m_properties[PROP_LABELS_LOCATION] == "bottom" ||
            m_properties[PROP_LABELS_LOCATION] == "each" ||
            m_properties[PROP_LABELS_LOCATION] == "none" ||
            m_properties[PROP_LABELS_LOCATION] == "");

    if (m_has_label)
    {
        // m_label_height contains the height of ONE text line
        m_label_height = GUIEngine::getFontHeight();
    }
    else
    {
        m_label_height = 0;
    }

    // ----- add dynamic label at bottom
    if (m_has_label)
    {
        // leave room for many lines, just in case
        rect<s32> label_size = rect<s32>(m_x,
                                         m_y + m_h - m_label_height,
                                         m_x + m_w,
                                         m_y + m_h + m_label_height*5);
        m_label = GUIEngine::getGUIEnv()->addStaticText(L" ", label_size, false, true /* word wrap */, NULL, -1);
        m_label->setTextAlignment( EGUIA_CENTER, EGUIA_UPPERLEFT );
        m_label->setWordWrap(true);
    }

    // ---- add arrow buttons on each side
    if (m_left_widget != NULL)
    {
        m_left_widget->elementRemoved();
        m_right_widget->elementRemoved();
        delete m_left_widget;
        delete m_right_widget;
    }
    m_left_widget  = new IconButtonWidget(IconButtonWidget::SCALE_MODE_KEEP_TEXTURE_ASPECT_RATIO, false);
    m_right_widget = new IconButtonWidget(IconButtonWidget::SCALE_MODE_KEEP_TEXTURE_ASPECT_RATIO, false);

    const int average_y = m_y + (m_h - m_label_height)/2;

    m_arrows_w = GUIEngine::getFontHeight() * 2;
    m_arrows_w = std::max(m_arrows_w, 40);

    const int button_h = m_arrows_w;

    // right arrow
    rect<s32> right_arrow_location = rect<s32>(m_x + m_w - m_arrows_w,
                                               average_y - button_h/2,
                                               m_x + m_w,
                                               average_y + button_h/2);
    m_right_widget->m_x = right_arrow_location.UpperLeftCorner.X;
    m_right_widget->m_y = right_arrow_location.UpperLeftCorner.Y;
    m_right_widget->m_w = right_arrow_location.getWidth();
    m_right_widget->m_h = right_arrow_location.getHeight();
    m_right_widget->m_event_handler = this;
    m_right_widget->m_focusable = false;
    m_right_widget->m_properties[PROP_ID] = "right";
    m_right_widget->setImage(GUIEngine::getSkin()->getImage("right_arrow::neutral"));
    m_right_widget->add();
    m_right_widget->setHighlightedImage(GUIEngine::getSkin()->getImage("right_arrow::focus"));

    m_children.push_back( m_right_widget );

    // left arrow
    rect<s32> left_arrow_location = rect<s32>(m_x,
                                              average_y - button_h/2,
                                              m_x + m_arrows_w,
                                              average_y + button_h/2);
    stringw  lmessage = "<<";
    m_left_widget->m_x = left_arrow_location.UpperLeftCorner.X;
    m_left_widget->m_y = left_arrow_location.UpperLeftCorner.Y;
    m_left_widget->m_w = left_arrow_location.getWidth();
    m_left_widget->m_h = left_arrow_location.getHeight();
    m_left_widget->m_event_handler = this;
    m_left_widget->m_focusable = false;
    m_left_widget->m_properties[PROP_ID] = "left";
    m_left_widget->setImage( GUIEngine::getSkin()->getImage("left_arrow::neutral") );
    m_left_widget->add();
    m_left_widget->setHighlightedImage(GUIEngine::getSkin()->getImage("left_arrow::focus"));

    m_children.push_back( m_left_widget );

    assert( m_left_widget->ok() );
    assert( m_right_widget->ok() );
    m_left_widget->m_element->setVisible(true);

    // ---- Determine number of rows

    // Find children size (and ratio)
    m_child_width  = atoi(m_properties[PROP_CHILD_WIDTH].c_str());
    m_child_height = atoi(m_properties[PROP_CHILD_HEIGHT].c_str());

    if (m_child_width <= 0 || m_child_height <= 0)
    {
        Log::warn("DynamicRibbonWidget", "Ribbon grid widgets require 'child_width' and 'child_height' arguments");
        m_child_width = 256;
        m_child_height = 256;
    }


    if (m_multi_row)
    {
        // determine row amount
        const float aspect_ratio = (float)m_child_width / (float)m_child_height;
        // const int count = m_items.size();

        m_row_amount = -1;

        if (m_h - m_label_height < 0)
        {
            Log::warn("DynamicRibbonWidget", "The widget is too small for anything to fit in it!!");
            m_row_amount = 1;
        }
        else
        {
            float max_score_so_far = -1;
            for (int row_count = 1; row_count < 10; row_count++)
            {
                int visible_items;
                int taken_area;
                int item_height;

                int item_count = m_item_count_hint;

                if (item_count < 1)
                {
                    item_count = (int) m_items.size();
                }

                if (item_count < 1)
                {
                    // No idea so make assumptions
                    item_count = 20;
                }

                estimateIconAreaFor(row_count, m_child_width, m_w, m_h - m_label_height,
                                    aspect_ratio, item_count, &visible_items, &taken_area, &item_height);

                // FIXME: this system to determine the best number of columns is really complicated!
                // the score is computed from taken screen area AND visible item count.
                // A number of rows that allows for the screen space to be used a lot will
                // get a better score. A number of rows that allows showing very few items
                // will be penalized. A configuration that makes items much smaller than
                // requested in the XML file will also be penalized.
                float ratio = (float)item_height / (float)m_child_height;

                // huge icons not so good either
                if (ratio > 1.0f)
                {
                    ratio = 1.0f - ratio/5.0f;
                    if (ratio < 0.0f) ratio = 0.0f;
                }

                float total_area = (float)(m_w * m_h);
                const float score = log(2.0f*visible_items) *
                                      std::min(ratio, 1.0f) * std::min(taken_area/total_area, 1.0f);

                //Log::info("DynamicRibbonWidget", "%d rown: %d visible items; area = %f; "
                //    "size penalty = %f; score = %f", row_count, visible_items, taken_area,
                //    std::min((float)item_height / (float)m_child_height, 1.0f), score);

                if (score > max_score_so_far)
                {
                    m_row_amount = row_count;
                    max_score_so_far = score;
                }
            }
            assert(m_row_amount != -1);
        }

        if (m_properties[PROP_MAX_ROWS].size() > 0)
        {
            const int max_rows = atoi(m_properties[PROP_MAX_ROWS].c_str());
            if (max_rows < 1)
            {
                Log::warn("DynamicRibbonWidget", "The 'max_rows' property must be an integer greater than zero; "
                    "Ignoring current value '%s'", m_properties[PROP_MAX_ROWS].c_str());
            }
            else
            {
                if (m_row_amount > max_rows) m_row_amount = max_rows;
            }
        }
    }
    else
    {
        m_row_amount = 1;
    }

    assert( m_left_widget->ok() );
    assert( m_right_widget->ok() );
    m_left_widget->m_element->setVisible(true);

    // get and build a list of IDs (by now we may not yet know everything about items,
    // but we need to get IDs *now* in order for tabbing to work.
    m_ids.resize(m_row_amount);
    for (int i=0; i<m_row_amount; i++)
    {
        m_ids[i] = getNewID();
        //Log::info("DynamicRibbonWidget", "getNewID returns %d", m_ids[i]);
    }

    buildInternalStructure();
}
// -----------------------------------------------------------------------------
void DynamicRibbonWidget::buildInternalStructure()
{
    //printf("****DynamicRibbonWidget::buildInternalStructure()****\n");

    // ---- Clean-up what was previously there
    for (unsigned int i=0; i<m_children.size(); i++)
    {
        IGUIElement* elem = m_children[i].m_element;
        if (elem != NULL && m_children[i].m_type == WTYPE_RIBBON)
        {
            elem->remove();
            m_children.erase(i);
            i--;
        }
    }
    m_rows.clearWithoutDeleting(); // rows already deleted above, don't double-delete

    m_left_widget->m_element->setVisible(true);
    assert( m_left_widget->ok() );
    assert( m_right_widget->ok() );

    // ---- determine column amount
    const float row_height = (float)(m_h - m_label_height)/(float)m_row_amount;
    float col_width = (float)(row_height * m_child_width / m_child_height);
    
    // Give some margin for columns for better readability
    col_width *= 1.2f;
    
    m_col_amount = std::max((int)floor( m_w / col_width ), 1);

    // ajust column amount to not add more item slots than we actually need
    const int item_count = (int) m_items.size();
    //Log::info("DynamicRibbonWidget", "%d items; %d cells", item_count, row_amount * m_col_amount);
    if (m_row_amount*m_col_amount > item_count)
    {
        m_col_amount = (int)ceil((float)item_count/(float)m_row_amount);
        //Log::info("DynamicRibbonWidget", "Adjusting m_col_amount to be %d", m_col_amount);
    }

    assert( m_left_widget->ok() );
    assert( m_right_widget->ok() );

    // Hide arrows when everything is visible
    if (item_count <= m_row_amount*m_col_amount)
    {
        m_scrolling_enabled = false;
        m_left_widget->m_element->setVisible(false);
        m_right_widget->m_element->setVisible(false);
    }
    else
    {
        m_scrolling_enabled = true;
        m_left_widget->m_element->setVisible(true);
        m_right_widget->m_element->setVisible(true);
    }

    // ---- add rows
    int added_item_count = 0;
    for (int n=0; n<m_row_amount; n++)
    {
        RibbonWidget* ribbon;
        if (m_combo)
        {
            ribbon = new RibbonWidget(RIBBON_COMBO);
        }
        else
        {
            ribbon = new RibbonWidget(RIBBON_TOOLBAR);
        }
        ribbon->setListener(this);
        ribbon->m_reserved_id = m_ids[n];

        ribbon->m_x = m_x + (m_scrolling_enabled ? m_arrows_w : 0);
        ribbon->m_y = m_y + (int)(n*row_height);
        ribbon->m_w = m_w - (m_scrolling_enabled ? m_arrows_w*2 : 0);
        ribbon->m_h = (int)(row_height);
        ribbon->m_type = WTYPE_RIBBON;

        std::stringstream name;
        name << this->m_properties[PROP_ID] << "_row" << n;
        ribbon->m_properties[PROP_ID] = name.str();
        ribbon->m_event_handler = this;

        // add columns
        for (int i=0; i<m_col_amount; i++)
        {
            // stretch the *texture* within the widget (and the widget has the right aspect ratio)
            // (Yeah, that's complicated, but screenshots are saved compressed horizontally so it's hard to be clean)
            IconButtonWidget* icon = new IconButtonWidget(IconButtonWidget::SCALE_MODE_STRETCH, false, true);
            icon->m_properties[PROP_ICON]="textures/transparence.png";

            // set size to get proper ratio (as most textures are saved scaled down to 256x256)
            icon->m_properties[PROP_WIDTH] = m_properties[PROP_CHILD_WIDTH];
            icon->m_properties[PROP_HEIGHT] = m_properties[PROP_CHILD_HEIGHT];
            icon->m_w = atoi(icon->m_properties[PROP_WIDTH].c_str());
            icon->m_h = atoi(icon->m_properties[PROP_HEIGHT].c_str());

            // If we want each icon to have its own label, we must make it non-empty, otherwise
            // it will assume there is no label and none will be created (FIXME: that's ugly)
            if (m_properties[PROP_LABELS_LOCATION] == "each") icon->m_text = " ";

            //Log::info("DynamicRibbonWidget", "Ribbon text = %s", m_properties[PROP_TEXT].c_str());

            ribbon->m_children.push_back( icon );
            added_item_count++;

            // stop adding columns when we have enough items
            if (added_item_count == item_count)
            {
                assert(!m_scrolling_enabled); // we can see all items, so scrolling must be off
                break;
            }
            else if (added_item_count > item_count)
            {
                assert(false);
                break;
            }
        }
        m_children.push_back( ribbon );
        m_rows.push_back( ribbon );
        ribbon->add();

        // stop filling rows when we have enough items
        if (added_item_count == item_count)
        {
            assert(!m_scrolling_enabled); // we can see all items, so scrolling must be off
            break;
        }
    }

#ifdef DEBUG
    if (!m_scrolling_enabled)
    {
        // debug checks
        int childrenCount = 0;
        for (unsigned int n=0; n<m_rows.size(); n++)
        {
            childrenCount += m_rows[n].m_children.size();
        }
        assert(childrenCount == (int)m_items.size());
    }
#endif
}
// -----------------------------------------------------------------------------
void DynamicRibbonWidget::addItem( const irr::core::stringw& user_name, const std::string& code_name,
                                   const std::string& image_file, const unsigned int badges,
                                   IconButtonWidget::IconPathType image_path_type)
{
    ItemDescription desc;
    desc.m_user_name = getUserName(user_name);
    desc.m_code_name = code_name;
    desc.m_sshot_file = image_file;
    desc.m_badges = badges;
    desc.m_animated = false;
    desc.m_image_path_type = image_path_type;

    m_items.push_back(desc);

    setLabelSize(desc.m_user_name);
}

// -----------------------------------------------------------------------------

void DynamicRibbonWidget::addAnimatedItem( const irr::core::stringw& user_name, const std::string& code_name,
                                           const std::vector<std::string>& image_files, const float time_per_frame,
                                           const unsigned int badges, IconButtonWidget::IconPathType image_path_type )
{
    ItemDescription desc;
    desc.m_user_name = getUserName(user_name);
    desc.m_code_name = code_name;
    desc.m_all_images = image_files;
    desc.m_badges = badges;
    desc.m_animated = true;
    desc.m_curr_time = 0.0f;
    desc.m_time_per_frame = time_per_frame;
    desc.m_image_path_type = image_path_type;

    m_items.push_back(desc);

    setLabelSize(desc.m_user_name);

    if (!m_animated_contents)
    {
        m_animated_contents = true;

        /*
         FIXME: remove this unclean thing, I think irrlicht provides this feature:
         virtual void IGUIElement::OnPostRender (u32 timeMs)
         \brief animate the element and its children.
         */
        if (!GUIEngine::needsUpdate.contains(this))
            GUIEngine::needsUpdate.push_back(this);
    }
}

// -----------------------------------------------------------------------------
void DynamicRibbonWidget::clearItems()
{
    if (m_animated_contents)
    {
        GUIEngine::needsUpdate.remove(this);
    }

    m_items.clear();
    m_animated_contents = false;
    m_scroll_offset = 0;
    m_max_label_width = 0;
}

// -----------------------------------------------------------------------------
void DynamicRibbonWidget::setBadge(const std::string &name, BadgeType badge)
{
    for (unsigned int r = 0; r < m_rows.size(); r++)
    {
        for (unsigned int c = 0; c < m_rows[r].m_children.size(); c++)
        {
            if(m_rows[r].m_children[c].m_properties[PROP_ID]==name)
                m_rows[r].m_children[c].setBadge(badge);
            else
                m_rows[r].m_children[c].unsetBadge(badge);
        }
    }
    
    for (unsigned int i = 0; i < m_items.size(); i++)
    {
        if (m_items[i].m_code_name == name)
        {
            m_items[i].m_badges |= int(badge);
        }
        else
        {
            m_items[i].m_badges &= (~int(badge));
        }
    }
}   // setBadge

// -----------------------------------------------------------------------------
void DynamicRibbonWidget::elementRemoved()
{
    Widget::elementRemoved();
    m_previous_item_count = 0;
    m_rows.clearWithoutDeleting();
    m_left_widget = NULL;
    m_right_widget = NULL;

    m_hover_listeners.clearAndDeleteAll();
}


#if 0
#pragma mark -
#pragma mark Getters
#endif

const std::string& DynamicRibbonWidget::getSelectionIDString(const int playerID)
{
    RibbonWidget* row = (RibbonWidget*)(m_rows.size() == 1 ? m_rows.get(0) : getSelectedRibbon(playerID));

    if (row != NULL) return row->getSelectionIDString(playerID);

    static const std::string nothing = "";
    return nothing;
}
// -----------------------------------------------------------------------------

irr::core::stringw DynamicRibbonWidget::getSelectionText(const int playerID)
{
    RibbonWidget* row = (RibbonWidget*)(m_rows.size() == 1 ? m_rows.get(0) : getSelectedRibbon(playerID));

    if (row != NULL) return row->getSelectionText(playerID);

    static const irr::core::stringw nothing = "";
    return nothing;
}

// -----------------------------------------------------------------------------
RibbonWidget* DynamicRibbonWidget::getRowContaining(Widget* w)
{
    for_var_in (RibbonWidget*, row, m_rows)
    {
        if (row != NULL)
        {
            if (row->m_children.contains( w ) ) return row;
        }
    }

    return NULL;
}
// -----------------------------------------------------------------------------
RibbonWidget* DynamicRibbonWidget::getSelectedRibbon(const int playerID)
{
    for_var_in (RibbonWidget*, row, m_rows)
    {
        if (GUIEngine::isFocusedForPlayer(row, playerID))
        {
            return row;
        }
    }

    return NULL;
}

#if 0
#pragma mark -
#pragma mark Event Handling
#endif

void DynamicRibbonWidget::registerHoverListener(DynamicRibbonHoverListener* listener)
{
    m_hover_listeners.push_back(listener);
}
// -----------------------------------------------------------------------------
EventPropagation DynamicRibbonWidget::rightPressed(const int playerID)
{
    if (m_deactivated) return EVENT_LET;

    RibbonWidget* w = getSelectedRibbon(playerID);
    if (w != NULL)
    {
        updateLabel();

        propagateSelection();

        const int listenerAmount = m_hover_listeners.size();
        for (int n=0; n<listenerAmount; n++)
        {
            m_hover_listeners[n].onSelectionChanged(this, getSelectedRibbon(playerID)->getSelectionIDString(playerID),
                                                    getSelectedRibbon(playerID)->getSelectionText(playerID), playerID);
        }
    }
    //Log::info("DynamicRibbonWidget", "Rightpressed %s", m_properties[PROP_ID].c_str());

    assert(m_rows.size() >= 1);
    if (m_rows[0].m_ribbon_type == RIBBON_TOOLBAR) return EVENT_BLOCK;

    //Log::info("DynamicRibbonWidget", "Rightpressed returning EVENT_LET");

    return EVENT_LET;
}
// -----------------------------------------------------------------------------
EventPropagation DynamicRibbonWidget::leftPressed(const int playerID)
{
    if (m_deactivated) return EVENT_LET;

    RibbonWidget* w = getSelectedRibbon(playerID);
    if (w != NULL)
    {
        updateLabel();
        propagateSelection();

        for_var_in (DynamicRibbonHoverListener*, listener, m_hover_listeners)
        {
            listener->onSelectionChanged(this, w->getSelectionIDString(playerID),
                                         w->getSelectionText(playerID), playerID);
        }
    }

    assert(m_rows.size() >= 1);
    if (m_rows[0].m_ribbon_type == RIBBON_TOOLBAR) return EVENT_BLOCK;

    return EVENT_LET;
}
// -----------------------------------------------------------------------------
EventPropagation DynamicRibbonWidget::transmitEvent(Widget* w,
                                                 const std::string& originator,
                                                 const int playerID)
{
    assert(m_magic_number == 0xCAFEC001);


    if (m_deactivated) return EVENT_LET;

    if (originator=="left")
    {
        scroll(-1);
        return EVENT_BLOCK;
    }
    if (originator=="right")
    {
        scroll(1);
        return EVENT_BLOCK;
    }

    // find selection in current ribbon
    if (m_combo)
    {
        RibbonWidget* selected_ribbon = (RibbonWidget*)getSelectedRibbon(playerID);
        if (selected_ribbon != NULL)
        {
            m_selected_item[playerID] = selected_ribbon->m_selection[playerID] + m_scroll_offset;
            if (m_selected_item[playerID] >= (int)m_items.size()) m_selected_item[playerID] -= (int)m_items.size();
        }
    }

    return EVENT_LET;
}
// -----------------------------------------------------------------------------
EventPropagation DynamicRibbonWidget::mouseHovered(Widget* child, const int playerID)
{
    if (m_deactivated) return EVENT_LET;
    //Log::info("DynamicRibbonWidget", "mouseHovered %d", playerID);

    updateLabel();

    bool multitouch_enabled = (UserConfigParams::m_multitouch_active == 1 &&
                               irr_driver->getDevice()->supportsTouchDevice()) ||
                               UserConfigParams::m_multitouch_active > 1;
    // For now disable it to fix the weird "triangle selection" for touch
    // screen
    if (!multitouch_enabled)
        propagateSelection();

    if (getSelectedRibbon(playerID) != NULL)
    {
        for_var_in (DynamicRibbonHoverListener*, listener, m_hover_listeners)
        {
            listener->onSelectionChanged(this, getSelectedRibbon(playerID)->getSelectionIDString(playerID),
                                         getSelectedRibbon(playerID)->getSelectionText(playerID), playerID);
        }
    }

    return EVENT_BLOCK;
}
// -----------------------------------------------------------------------------
EventPropagation DynamicRibbonWidget::focused(const int playerID)
{
    Widget::focused(playerID);
    updateLabel();

    if (getSelectedRibbon(playerID)->getSelectionIDString(playerID) == "")
    {
        //fprintf(stderr, "[DynamicRibbonWidget] WARNING: Can't find selection for player %i, selecting first item\n", playerID);
        getSelectedRibbon(playerID)->setSelection(0, playerID);
    }

    for_var_in (DynamicRibbonHoverListener*, listener, m_hover_listeners)
    {
        listener->onSelectionChanged(this, getSelectedRibbon(playerID)->getSelectionIDString(playerID),
                                     getSelectedRibbon(playerID)->getSelectionText(playerID), playerID);
    }

    return EVENT_LET;
}

// -----------------------------------------------------------------------------

void DynamicRibbonWidget::onRibbonWidgetScroll(const int delta_x)
{
    scroll(delta_x);
}

// -----------------------------------------------------------------------------

void DynamicRibbonWidget::setText(const irr::core::stringw& text)
{
    Widget::setText(text);

    if (m_label != NULL)
        m_label->setText(text);
}

// -----------------------------------------------------------------------------

void DynamicRibbonWidget::onRibbonWidgetFocus(RibbonWidget* emitter, const int playerID)
{
    if (m_deactivated) return;

    if (emitter->m_selection[playerID] >= (int)emitter->m_children.size())
    {
        emitter->m_selection[playerID] = emitter->m_children.size()-1;
    }

    updateLabel(emitter);

    if (emitter->getSelectionIDString(playerID) == "")
    {
        //fprintf(stderr, "[DynamicRibbonWidget] WARNING: Can't find selection for player %i, selecting first item\n", playerID);
        emitter->setSelection(0, playerID);
    }

    for_var_in(DynamicRibbonHoverListener*, listener, m_hover_listeners)
    {
        listener->onSelectionChanged(this, emitter->getSelectionIDString(playerID),
                                     emitter->getSelectionText(playerID), playerID);
    }
}

#if 0
#pragma mark -
#pragma mark Setters / Actions
#endif

void DynamicRibbonWidget::scroll(int x_delta, bool evenIfDeactivated)
{
    if (m_deactivated && !evenIfDeactivated) return;

    // Refuse to scroll when everything is visible
    if ((int)m_items.size() <= m_row_amount*m_col_amount) return;

    m_scroll_offset += x_delta;

    const int max_scroll = std::max(m_col_amount, m_needed_cols) - 1;

    if (m_scroll_offset < 0) m_scroll_offset = max_scroll;
    else if (m_scroll_offset > max_scroll) m_scroll_offset = 0;

    updateItemDisplay();

    if (m_scroll_callback.callback != NULL)
    {
        m_scroll_callback.callback(m_scroll_callback.data);
    }

    // update selection markers in child ribbon
    if (m_combo)
    {
        for (unsigned int n=0; n<MAX_PLAYER_COUNT; n++)
        {
            RibbonWidget* ribbon = m_rows.get(0); // there is a single row when we can select items
            int id = m_selected_item[n] - m_scroll_offset;
            if (id < 0) id += (int) m_items.size();
            ribbon->setSelection(id, n);
        }
    }
}
// -----------------------------------------------------------------------------
/** DynamicRibbonWidget is made of several ribbons; each of them thus has
 its own selection independently of each other. To keep a grid feeling
 (i.e. you remain in the same column when pressing up/down), this method is
 used to ensure that all children ribbons always select the same column */
void DynamicRibbonWidget::propagateSelection()
{
    for (unsigned int p=0; p<MAX_PLAYER_COUNT; p++)
    {
        // find selection in current ribbon
        RibbonWidget* selected_ribbon = (RibbonWidget*)getSelectedRibbon(p);
        if (selected_ribbon == NULL) continue;

        const int relative_selection = selected_ribbon->m_selection[p];
        float where = 0.0f;

        if (selected_ribbon->m_children.size() > 1)
        {
            where = (float)relative_selection / (float)(selected_ribbon->m_children.size() - 1);
        }
        else
        {
            where = 0.0f;
        }

        if (where < 0.0f)      where = 0.0f;
        else if (where > 1.0f) where = 1.0f;

        if (m_combo)
        {
            m_selected_item[p] = relative_selection + m_scroll_offset;
            if (m_selected_item[p] >= (int)m_items.size()) m_selected_item[p] -= (int)m_items.size();
        }

        // set same selection in all ribbons
        for (RibbonWidget* ribbon : m_rows)
        {
            if (ribbon && ribbon != selected_ribbon)
            {
                ribbon->m_selection[p] = (int)roundf(where*(ribbon->m_children.size()-1));
                ribbon->updateSelection();
            }
        }

    }
}
// -----------------------------------------------------------------------------
void DynamicRibbonWidget::updateLabel(RibbonWidget* from_this_ribbon)
{
    if (!m_has_label) return;

    // only the master player can update the label
    const int playerID = PLAYER_ID_GAME_MASTER;

    RibbonWidget* row = from_this_ribbon ? from_this_ribbon : (RibbonWidget*)getSelectedRibbon(playerID);
    if (row == NULL) return;

    std::string selection_id = row->getSelectionIDString(playerID);

    const int amount = (int)m_items.size();
    for (int n=0; n<amount; n++)
    {
        if (m_items[n].m_code_name == selection_id)
        {
            m_label->setText( stringw(m_items[n].m_user_name.c_str()).c_str() );
            return;
        }
    }

    if (selection_id == RibbonWidget::NO_ITEM_ID) m_label->setText( L"" );
    else                                          m_label->setText( L"Unknown Item" );
}

// -----------------------------------------------------------------------------

/** Set to 1 if you wish information about item placement within the ribbon to be printed */
#define CHATTY_ABOUT_ITEM_PLACEMENT 0

void DynamicRibbonWidget::updateItemDisplay()
{
    // ---- Check if we need to update the number of icons in the ribbon
    if ((int)m_items.size() != m_previous_item_count)
    {
        buildInternalStructure();
        m_previous_item_count = (int)m_items.size();
    }

    // ---- some variables
    int icon_id = 0;

    const int row_amount = (int)m_rows.size();
    const int item_amount = (int)m_items.size();

    //FIXME: isn't this set by 'buildInternalStructure' already?
    m_needed_cols = (int)ceil( (float)item_amount / (float)row_amount );

    //const int max_scroll = std::max(m_col_amount, m_needed_cols) - 1;

    // the number of items that fit perfectly the number of rows we have
    // (this value will be useful to compute scrolling)
    int fitting_item_amount = (m_scrolling_enabled ? m_needed_cols * row_amount
                                                   : (int)m_items.size());

    // ---- to determine which items go in which cell of the dynamic ribbon now,
    //      we create a temporary 2D table and fill them with the ID of the item
    //      they need to display.
    //int item_placement[row_amount][m_needed_cols];
    std::vector<std::vector<int> > item_placement;
    item_placement.resize(row_amount);
    for(int i=0; i<row_amount; i++)
        item_placement[i].resize(m_needed_cols);

    int counter = 0;

#if CHATTY_ABOUT_ITEM_PLACEMENT
    std::cout << m_items.size() << " items to be placed:\n{\n";
#endif

    for (int c=0; c<m_needed_cols; c++)
    {
        for (int r=0; r<row_amount; r++)
        {

#if CHATTY_ABOUT_ITEM_PLACEMENT
            std::cout << "    ";
#endif

            const int items_in_row = m_rows[r].m_children.size();
            if (c >= items_in_row)
            {
                item_placement[r][c] = -1;

#if CHATTY_ABOUT_ITEM_PLACEMENT
                std::cout << item_placement[r][c] << "  ";
#endif
                continue;
            }

            int newVal = counter + m_scroll_offset*row_amount;
            while (newVal >= fitting_item_amount) newVal -= fitting_item_amount;
            item_placement[r][c] = newVal;

#if CHATTY_ABOUT_ITEM_PLACEMENT
            std::cout << newVal << "  ";
#endif

            counter++;
        }

#if CHATTY_ABOUT_ITEM_PLACEMENT
        std::cout << "\n";
#endif

    }

#if CHATTY_ABOUT_ITEM_PLACEMENT
    std::cout << "}\n";
#endif

    // ---- iterate through the rows, and set the items of each row to match those of the table
    for (int n=0; n<row_amount; n++)
    {
        RibbonWidget& row = m_rows[n];

        //std::cout << "Row " << n << "\n{\n";

        const unsigned int items_in_row = row.m_children.size();

        // calculate font size
        if (m_col_amount > 0)
        {
            m_font->setScale(GUIEngine::getFont()->getScale() *
                getFontScale((row.m_w / m_col_amount) - 30));
        }

        for (unsigned int i=0; i<items_in_row; i++)
        {
            IconButtonWidget* icon = dynamic_cast<IconButtonWidget*>(&row.m_children[i]);
            assert(icon != NULL);

            //FIXME : it is a bit hackish
            if(i < item_placement[n].size())
            {
                icon_id = item_placement[n][i];
                if (icon_id < item_amount && icon_id != -1)
                {
                    std::string item_icon = (m_items[icon_id].m_animated ?
                                             m_items[icon_id].m_all_images[0] :
                                             m_items[icon_id].m_sshot_file);
                    icon->setImage( item_icon.c_str(), m_items[icon_id].m_image_path_type );

                    icon->m_properties[PROP_ID]   = m_items[icon_id].m_code_name;
                    icon->setLabelFont(m_font);
                    icon->setLabel(m_items[icon_id].m_user_name);
                    icon->m_text                  = m_items[icon_id].m_user_name;
                    icon->m_badges                = m_items[icon_id].m_badges;

                    //std::cout << "    item " << i << " is " << m_items[icon_id].m_code_name << "\n";

                    //std::wcout << L"Setting widget text '" << icon->m_text.c_str() << L"'\n";

                    // if the ribbon has no "ribbon-wide" label, call will do nothing
                    row.setLabel(i, m_items[icon_id].m_user_name);
                }
                else
                {
                    icon->setImage( "textures/transparence.png", IconButtonWidget::ICON_PATH_TYPE_RELATIVE );
                    icon->resetAllBadges();
                    icon->m_properties[PROP_ID] = RibbonWidget::NO_ITEM_ID;
                    icon->setLabel(L"");
                    icon->m_text = L"";
                    //std::cout << "    item " << i << " is a FILLER\n";
                }
            }
        } // next column
    } // next row
}

// -----------------------------------------------------------------------------

void DynamicRibbonWidget::update(float dt)
{
    const int row_amount = m_rows.size();
    for (int n=0; n<row_amount; n++)
    {
        RibbonWidget& row = m_rows[n];

        const int items_in_row = row.m_children.size();
        for (int i=0; i<items_in_row; i++)
        {
            int col_scroll = i + m_scroll_offset;
            int item_id = (col_scroll)*row_amount + n;

            assert(item_id >= 0);

            if (item_id < (int)m_items.size() && m_items[item_id].m_animated)
            {
                const int frameBefore = (int)(m_items[item_id].m_curr_time / m_items[item_id].m_time_per_frame);

                m_items[item_id].m_curr_time += dt;
                int frameAfter = (int)(m_items[item_id].m_curr_time / m_items[item_id].m_time_per_frame);
                if (frameAfter == frameBefore) continue; // no frame change yet

                if (frameAfter >= (int)m_items[item_id].m_all_images.size())
                {
                    m_items[item_id].m_curr_time = 0;
                    frameAfter = 0;
                }

                IconButtonWidget* icon = dynamic_cast<IconButtonWidget*>(&row.m_children[i]);
                icon->setImage( m_items[item_id].m_all_images[frameAfter].c_str() );
            }

        }
    }
}

// -----------------------------------------------------------------------------

bool DynamicRibbonWidget::findItemInRows(const char* name, int* p_row, int* p_id)
{
    int row = -1;
    int id = -1;

    for (unsigned int r=0; r<m_rows.size(); r++)
    {
        id = m_rows[r].findItemNamed(name);
        if (id > -1)
        {
            row = r;
            break;
        }
    }

    *p_row = row;
    *p_id = id;
    return (row != -1);
}

// -----------------------------------------------------------------------------

bool DynamicRibbonWidget::setSelection(int item_id, const int playerID,
                                       const bool focusIt,
                                       bool evenIfDeactivated)
{
    if (m_deactivated && !evenIfDeactivated) return false;

    //printf("****DynamicRibbonWidget::setSelection()****\n");

    if ((unsigned int)item_id >= m_items.size()) return false;

    m_selected_item[playerID] = item_id;

    const std::string& name = m_items[item_id].m_code_name;

    int row;
    int id;

    unsigned int iterations = 0; // a safeguard to avoid infinite loops (should not happen normally)

    while (!findItemInRows(name.c_str(), &row, &id))
    {
        // if we get here it means the item is scrolled out. Try to find it.
        scroll(1, evenIfDeactivated);

        if (iterations > m_items.size())
        {
            Log::error("DynamicRibbonWidget::setSelection", "Cannot find item %d (%s)", item_id, name.c_str());
            return false;
        }

        iterations++;
    }

    //Log::info("DynamicRibbonWidget", "Player %d has item %d (%s) in row %d", playerID, item_id, name.c_str(), row);
    m_rows[row].setSelection(id, playerID);
    if (focusIt)
    {
        m_rows[row].setFocusForPlayer(playerID);
    }
    else
    {
        // focusing the item is enough to trigger the selection listeners; however if we're setting selection
        // without focusing they won't be noticed, which is why we notice them here
        const int listenerAmount = m_hover_listeners.size();
        for (int n=0; n<listenerAmount; n++)
        {
            m_hover_listeners[n].onSelectionChanged(this,
                                                    name,
                                                    m_rows[row].getSelectionText(playerID),
                                                    playerID);
        }
    }

    propagateSelection();
    return true;
}

// ----------------------------------------------------------------------------

bool DynamicRibbonWidget::setSelection(const std::string &item_codename,
                                       const int playerID, const bool focusIt,
                                       bool evenIfDeactivated)
{
    if (m_deactivated && !evenIfDeactivated) return false;

    const int item_count = (int)m_items.size();
    for (int n=0; n<item_count; n++)
    {
        if (m_items[n].m_code_name == item_codename)
        {
            return setSelection(n, playerID, focusIt, evenIfDeactivated);
        }
    }
    return false;
}

// -----------------------------------------------------------------------------

void DynamicRibbonWidget::setLabelSize(const irr::core::stringw& text)
{
    int w = GUIEngine::getFont()->getDimension(text.c_str()).Width;
    if (w > m_max_label_width)
        m_max_label_width = w;
}

// -----------------------------------------------------------------------------

float DynamicRibbonWidget::getFontScale(int icon_width) const
{
    if (m_max_label_width <= icon_width || m_max_label_width == 0 || icon_width == 0)
        return 1.0f;
    else
        return std::max (0.5f, ((float)icon_width / (float)m_max_label_width));
}

// -----------------------------------------------------------------------------

irr::core::stringw DynamicRibbonWidget::getUserName(const irr::core::stringw& user_name) const
{
    if (m_max_label_length == 0 || user_name.size() < m_max_label_length)
        return user_name;
    else
        return (user_name.subString(0, m_max_label_length - 3) + L"...");
}
