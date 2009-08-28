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

#include "guiengine/engine.hpp"
#include "guiengine/widgets/ribbon_grid_widget.hpp"
#include "io/file_manager.hpp"

#include <sstream>

using namespace GUIEngine;

#ifndef round
#  define round(x)  (floor(x+0.5f))
#endif

RibbonGridWidget::RibbonGridWidget(const bool combo, const int max_rows)
{
    m_scroll_offset = 0;
    m_needed_cols = 0;
    m_col_amount = 0;
    m_has_label = false;
    m_previous_item_count = 0;
    
    m_max_rows = max_rows;
    m_combo = combo;
    
    m_left_widget = NULL;
    m_right_widget = NULL;
    m_type = WTYPE_RIBBON_GRID;
    
    for (int n=0; n<32; n++)
    {
        m_selected_item[n] = -1;
    }
    m_selected_item[0] = 0; // only player 0 has a selection by default
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::add()
{
    m_has_label = (m_properties[PROP_TEXT] == "bottom");
    m_label_height = m_has_label ? 25 : 0; // FIXME : get height from font, don't hardcode
    
    // ----- add dynamic label at bottom
    if (m_has_label)
    {
        // leave room for many lines, just in case
        rect<s32> label_size = rect<s32>(x, y + h - m_label_height, x+w, y+h+m_label_height*5);
        m_label = GUIEngine::getGUIEnv()->addStaticText(L" ", label_size, false, true /* word wrap */, NULL, -1);
        m_label->setTextAlignment( EGUIA_CENTER, EGUIA_UPPERLEFT );
        m_label->setWordWrap(true);
    }
    
    // ---- add arrow buttons on each side
    // FIXME? these arrow buttons are outside of the widget's boundaries
    if (m_left_widget != NULL)
    {
        // FIXME - do proper memory management, find why it crashes when i try to clean-up
        //delete m_left_widget;
        //delete m_right_widget;
    }
    m_left_widget = new Widget();
    m_right_widget = new Widget();
    
    const int average_y = y + (h-m_label_height)/2;
    m_arrows_w = 30;
    const int button_h = 50;
    
    // right arrow
    rect<s32> right_arrow_location = rect<s32>(x + w - m_arrows_w,
                                               average_y - button_h/2,
                                               x + w,
                                               average_y + button_h/2);
    stringw  rmessage = ">>";
    IGUIButton* right_arrow = GUIEngine::getGUIEnv()->addButton(right_arrow_location, NULL, getNewNoFocusID(), rmessage.c_str(), L"");
    right_arrow->setTabStop(false);
    m_right_widget->m_element = right_arrow;
    m_right_widget->m_event_handler = this;
    m_right_widget->m_properties[PROP_ID] = "right";
    m_right_widget->id = right_arrow->getID();
    m_children.push_back( m_right_widget );
    
    // left arrow
    rect<s32> left_arrow_location = rect<s32>(x,
                                              average_y - button_h/2,
                                              x + m_arrows_w,
                                              average_y + button_h/2);
    stringw  lmessage = "<<";
    IGUIButton* left_arrow = GUIEngine::getGUIEnv()->addButton(left_arrow_location, NULL, getNewNoFocusID(), lmessage.c_str(), L"");
    left_arrow->setTabStop(false);
    m_left_widget->m_element = left_arrow;
    m_left_widget->m_event_handler = this;
    m_left_widget->m_properties[PROP_ID] = "left";
    m_left_widget->id = left_arrow->getID();
    m_children.push_back( m_left_widget );
    
    // ---- Determine number of rows and columns
    
    // Find children size (and ratio)
    m_child_width = atoi(m_properties[PROP_CHILD_WIDTH].c_str());
    m_child_height = atoi(m_properties[PROP_CHILD_HEIGHT].c_str());
    
    if (m_child_width <= 0 || m_child_height <= 0)
    {
        std::cerr << "/!\\ Warning /!\\ : ribbon grid widgets require 'child_width' and 'child_height' arguments" << std::endl;
        m_child_width = 256;
        m_child_height = 256;
    }
    
    // determine row amonunt
    m_row_amount = (int)round((h-m_label_height) / (float)m_child_height);
    if (m_row_amount > m_max_rows) m_row_amount = m_max_rows;
    
    // get and build a list of IDs (by now we may not yet know how many rows we will needs,
    // but we need to get IDs *now* in order for tabbing to work.
    m_ids.resize(m_row_amount);
    for (int i=0; i<m_row_amount; i++)
    {
        m_ids[i] = getNewID();
        std::cout << "ribbon : getNewID returns " <<  m_ids[i] << std::endl;
    }
    
    setSubElements();
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::setSubElements()
{
    // ---- Clean-up what was previously there
    for (int i=0; i<m_children.size(); i++)
    {
        IGUIElement* elem = m_children[i].m_element;
        if (elem != NULL && m_children[i].m_type == WTYPE_RIBBON)
        {
            elem->remove();
            m_children.erase(i);
            i--;
            if (i<0) i = 0;
        }
    }
    m_rows.clearWithoutDeleting(); // rows already deleted above, don't double-delete
    
    // ---- determine column amount
    const float row_height = (float)(h - m_label_height)/(float)m_row_amount;
    float ratio_zoom = (float)row_height / (float)(m_child_height - m_label_height);
    m_col_amount = (int)round( w / ( m_child_width*ratio_zoom ) );
    
    // ajust column amount to not add more items slot than we actually need
    const int item_count = m_items.size();
    std::cout << "item_count=" << item_count << ", row_amount*m_col_amount=" << m_row_amount*m_col_amount << std::endl;
    if (m_row_amount*m_col_amount > item_count)
    {
        m_col_amount = (int)ceil((float)item_count/(float)m_row_amount);
        std::cout << "Adjusting m_col_amount to be " << m_col_amount << std::endl;
    }
    
        
    // Hide arrows when everything is visible
    if (item_count <= m_row_amount*m_col_amount)
    {
        m_left_widget->m_element->setVisible(false);
        m_right_widget->m_element->setVisible(false);
    }
    else
    {
        m_left_widget->m_element->setVisible(true);
        m_right_widget->m_element->setVisible(true);
    }
    
    // ---- add rows
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
        ribbon->m_reserved_id = m_ids[n];
                
        ribbon->x = x + m_arrows_w;
        ribbon->y = y + (int)(n*row_height);
        ribbon->w = w - m_arrows_w*2;
        ribbon->h = (int)(row_height);
        ribbon->m_type = WTYPE_RIBBON;

        std::stringstream name;
        name << this->m_properties[PROP_ID] << "_row" << n;
        ribbon->m_properties[PROP_ID] = name.str();
        ribbon->m_event_handler = this;
        
        // add columns
        for(int i=0; i<m_col_amount; i++)
        {
            IconButtonWidget* icon = new IconButtonWidget();
            icon->m_properties[PROP_ICON]="gui/track_random.png";
            
            // set size to get proper ratio (as most textures are saved scaled down to 256x256)
            icon->m_properties[PROP_WIDTH] = m_properties[PROP_CHILD_WIDTH];
            icon->m_properties[PROP_HEIGHT] = m_properties[PROP_CHILD_HEIGHT];
            if(m_properties[PROP_TEXT] == "all") icon->m_properties[PROP_TEXT] = " ";
            
            // std::cout << "ribbon text = " << m_properties[PROP_TEXT].c_str() << std::endl;
            
            icon->m_type = WTYPE_ICON_BUTTON;
            ribbon->m_children.push_back( icon );
        }
        m_children.push_back( ribbon );
        m_rows.push_back( ribbon );
        ribbon->add();
    }
    
 }
// -----------------------------------------------------------------------------
void RibbonGridWidget::addItem( std::string user_name, std::string code_name, std::string image_file )
{
    ItemDescription desc;
    desc.m_user_name = user_name;
    desc.m_code_name = code_name;
    desc.m_sshot_file = image_file;
    
    m_items.push_back(desc);
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::clearItems()
{
    m_items.clear();
}

#if 0
#pragma mark -
#pragma mark Getters
#endif

const std::string& RibbonGridWidget::getSelectionIDString(const int playerID)
{
    RibbonWidget* row = (RibbonWidget*)(m_rows.size() == 1 ? m_rows.get(0) : getSelectedRibbon(playerID));
    
    if(row != NULL) return row->getSelectionIDString(playerID);
    
    static const std::string nothing = "";
    return nothing;
}
// -----------------------------------------------------------------------------
const std::string& RibbonGridWidget::getSelectionText(const int playerID)
{
    RibbonWidget* row = (RibbonWidget*)(m_rows.size() == 1 ? m_rows.get(0) : getSelectedRibbon(playerID));
    
    if(row != NULL) return row->getSelectionText(playerID);
    
    static const std::string nothing = "";
    return nothing;
}
// -----------------------------------------------------------------------------
RibbonWidget* RibbonGridWidget::getRowContaining(Widget* w) const
{
    const int row_amount = m_rows.size();
    for(int n=0; n<row_amount; n++)
    {
        const RibbonWidget* row = &m_rows[n];
        if(row != NULL)
        {
            if(m_children.contains( w ) ) return (RibbonWidget*)row;
        }
    }
    
    return NULL;
}
// -----------------------------------------------------------------------------
RibbonWidget* RibbonGridWidget::getSelectedRibbon(const int playerID) const
{    
    if (playerID == 0)
    {
        const int row_amount = m_rows.size();
        for(int n=0; n<row_amount; n++)
        {
            const RibbonWidget* row = &m_rows[n];
            if(row != NULL)
            {
                if( GUIEngine::getGUIEnv()->hasFocus(row->m_element) ||
                   m_element->isMyChild( GUIEngine::getGUIEnv()->getFocus() ) ) return (RibbonWidget*)row;
            }
        }
    }
    else
    {
        const int row_amount = m_rows.size();
        for(int n=0; n<row_amount; n++)
        {
            const RibbonWidget* row = &m_rows[n];
            if (row == GUIEngine::g_focus_for_player[playerID])
            {
                return (RibbonWidget*)row;
            }
        }
        
    }
    
    return NULL;
}

#if 0
#pragma mark -
#pragma mark Event Handling
#endif

void RibbonGridWidget::registerHoverListener(RibbonGridHoverListener* listener)
{
    m_hover_listeners.push_back(listener);
}
// -----------------------------------------------------------------------------
bool RibbonGridWidget::rightPressed(const int playerID)
{
    RibbonWidget* w = getSelectedRibbon(playerID);
    if (w != NULL)
    {
        updateLabel();
        
        propagateSelection();
        
        const int listenerAmount = m_hover_listeners.size();
        for (int n=0; n<listenerAmount; n++)
        {
            m_hover_listeners[n].onSelectionChanged(this, getSelectedRibbon(playerID)->getSelectionIDString(playerID), playerID);
        }
    }
    
    if (m_rows[0].m_ribbon_type == RIBBON_TOOLBAR) return false;
    
    return true;
}
// -----------------------------------------------------------------------------
bool RibbonGridWidget::leftPressed(const int playerID)
{
    RibbonWidget* w = getSelectedRibbon(playerID);
    if (w != NULL)
    {
        updateLabel();
        propagateSelection();
        
        const int listenerAmount = m_hover_listeners.size();
        for (int n=0; n<listenerAmount; n++)
        {
            m_hover_listeners[n].onSelectionChanged(this, w->getSelectionIDString(playerID), playerID);
        }
    }
    
    
    if (m_rows[0].m_ribbon_type == RIBBON_TOOLBAR) return false;
    
    return true;
}
// -----------------------------------------------------------------------------
bool RibbonGridWidget::transmitEvent(Widget* w, std::string& originator, const int playerID)
{
    if (originator=="left")
    {
        scroll(-1);
        return false;
    }
    if (originator=="right")
    {
        scroll(1);
        return false;
    }
    
    // find selection in current ribbon
    if (m_combo)
    {
        RibbonWidget* selected_ribbon = (RibbonWidget*)getSelectedRibbon(playerID);
        if (selected_ribbon != NULL)
        {
            m_selected_item[playerID] = selected_ribbon->m_selection[playerID] + m_scroll_offset;
            if (m_selected_item[playerID] >= (int)m_items.size()) m_selected_item[playerID] -= m_items.size();
        }
    }
    
    return true;
}
// -----------------------------------------------------------------------------
bool RibbonGridWidget::mouseHovered(Widget* child)
{
    updateLabel();
    propagateSelection();
    
    // FIXME: don't hardcode player 0
    const int playerID = 0;
    
    if (getSelectedRibbon(playerID) != NULL)
    {
        const int listenerAmount = m_hover_listeners.size();
        for (int n=0; n<listenerAmount; n++)
        {
            m_hover_listeners[n].onSelectionChanged(this, getSelectedRibbon(playerID)->getSelectionIDString(playerID), playerID);
        }
    }
    
    return false;
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::focused()
{
    Widget::focused();
    updateLabel();
    
    const int listenerAmount = m_hover_listeners.size();
    for(int n=0; n<listenerAmount; n++)
    {
        // FIXME: don't hardcode player 0
        const int playerID = 0;
        m_hover_listeners[n].onSelectionChanged(this, getSelectedRibbon(playerID)->getSelectionIDString(playerID), playerID);
    }
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::onRowChange(RibbonWidget* row)
{
    updateLabel(row);
    
    const int listenerAmount = m_hover_listeners.size();
    for (int n=0; n<listenerAmount; n++)
    {
        // FIXME: don't hardcode player 0
        const int playerID = 0;
        m_hover_listeners[n].onSelectionChanged(this, row->getSelectionIDString(playerID), playerID);
    }
}

#if 0
#pragma mark -
#pragma mark Setters / Actions
#endif

void RibbonGridWidget::scroll(const int x_delta)
{
    // Refuse to scroll when everything is visible
    if ((int)m_items.size() <= m_row_amount*m_col_amount) return;
    
    m_scroll_offset += x_delta;
    
    const int max_scroll = std::max(m_col_amount, m_needed_cols) - 1;
    
    if (m_scroll_offset < 0) m_scroll_offset = max_scroll;
    else if (m_scroll_offset > max_scroll) m_scroll_offset = 0;
    
    updateItemDisplay();

    // update selection markers in child ribbon
    if (m_combo)
    {
        for (int n=0; n<32; n++)
        {
            RibbonWidget* ribbon = m_rows.get(0); // there is a single row when we can select items
            int id = m_selected_item[n] - m_scroll_offset;
            if (id < 0) id += m_items.size();
            ribbon->setSelection(id, n);
        }
    }
}
// -----------------------------------------------------------------------------
/** RibbonGridWidget is made of several ribbons; each of them thus has
 its own selection independently of each other. To keep a grid feeling
 (i.e. you remain in the same column when pressing up/down), this method is
 used to ensure that all children ribbons always select the same column */
void RibbonGridWidget::propagateSelection()
{    
    for (int p=0; p<32; p++)
    {
        // find selection in current ribbon
        RibbonWidget* selected_ribbon = (RibbonWidget*)getSelectedRibbon(p);
        if (selected_ribbon == NULL) continue;
        
        const int relative_selection = selected_ribbon->m_selection[p];
        
        if (m_combo)
        {
            m_selected_item[p] = relative_selection + m_scroll_offset;
            if (m_selected_item[p] >= (int)m_items.size()) m_selected_item[p] -= m_items.size();
        }
        
        // set same selection in all ribbons
        const int row_amount = m_rows.size();
        for (int n=0; n<row_amount; n++)
        {
            RibbonWidget* ribbon = m_rows.get(n);
            if (ribbon != selected_ribbon)
            {
                ribbon->m_selection[p] = relative_selection;
                ribbon->updateSelection();
            }
        }
        
    }
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::updateLabel(RibbonWidget* from_this_ribbon)
{
    if (!m_has_label) return;
    
    // FIXME? Don't hardcode player 0 (even though label can only work with a single player)
    const int playerID = 0;
    
    RibbonWidget* row = from_this_ribbon ? from_this_ribbon : (RibbonWidget*)getSelectedRibbon(playerID);
    if (row == NULL) return;
    
    std::string selection_id = row->getSelectionIDString(playerID);
    
    const int amount = m_items.size();
    for (int n=0; n<amount; n++)
    {
        if (m_items[n].m_code_name == selection_id)
        {
            m_label->setText( stringw(m_items[n].m_user_name.c_str()).c_str() );
            return;
        }
    }
    
    m_label->setText( L"Random" );
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::updateItemDisplay()
{
    // Check if we need to update the number of icons in the ribbon
    if ((int)m_items.size() != m_previous_item_count)
    {
        setSubElements();
        m_previous_item_count = m_items.size();
    }
    
    int icon_id = 0;
    
    const int row_amount = m_rows.size();
    const int item_amount = m_items.size();
    
    m_needed_cols = (int)ceil( (float)item_amount / (float)row_amount );
    
    const int max_scroll = std::max(m_col_amount, m_needed_cols) - 1;
    
    for (int n=0; n<row_amount; n++)
    {
        RibbonWidget& row = m_rows[n];
        
        for (int i=0; i<m_col_amount; i++)
        {
            IconButtonWidget* icon = dynamic_cast<IconButtonWidget*>(&row.m_children[i]);
            assert(icon != NULL);
            IGUIButton* button = dynamic_cast<IGUIButton*>(icon->m_element);
            assert(button != NULL);
            
            int col_scroll = i + m_scroll_offset;
            while(col_scroll > max_scroll) col_scroll -= max_scroll+1;
            
            icon_id = (col_scroll)*row_amount + n;
            
            if (icon_id < item_amount)
            {
                std::string track_sshot = m_items[icon_id].m_sshot_file;
                button->setImage( GUIEngine::getDriver()->getTexture(  track_sshot.c_str() ));
                button->setPressedImage( GUIEngine::getDriver()->getTexture( track_sshot.c_str()) );
                
                icon->m_properties[PROP_ID]   = m_items[icon_id].m_code_name;
                icon->m_properties[PROP_TEXT] = m_items[icon_id].m_user_name;
                
                row.setLabel(i, m_items[icon_id].m_user_name);
            }
            else
            {
                button->setImage( GUIEngine::getDriver()->getTexture( (file_manager->getGUIDir() + "/track_random.png").c_str() ) );
                button->setPressedImage( GUIEngine::getDriver()->getTexture( (file_manager->getGUIDir() + "/track_random.png").c_str() ) );
                icon->m_properties[PROP_ID] = "gui/track_random.png";
            }
        } // next column
    } // next row
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::setSelection(int item_id)
{
    if (m_rows.size() > 1)
    {
        std::cout << "\n/!\\ Warning, RibbonGridWidget::setSelection only makes sense on 1-row ribbons " <<
                     "(since there can't logically be a permanent with more than one row)\n\n";
        return;
    }
    
    // FIXME: don't hardcode player 0 ?
    const int PLAYER_ID = 0;
    
    m_selected_item[PLAYER_ID] = item_id;
    if (m_selected_item[PLAYER_ID] >= (int)m_items.size()) m_selected_item[PLAYER_ID] -= m_items.size();
    
    // scroll so selection is visible
    m_scroll_offset = m_selected_item[PLAYER_ID]; // works because in this case there is a single row
    updateItemDisplay();
    
    // set selection
    RibbonWidget* ribbon = m_rows.get(0); // there is a single row when we can select items
    int id = m_selected_item[PLAYER_ID] - m_scroll_offset;
    if (id < 0) id += m_items.size();
    ribbon->setSelection(id, PLAYER_ID);
}

// -----------------------------------------------------------------------------
void RibbonGridWidget::setSelection(int item_id, const int playerID)
{
    m_selected_item[playerID] = item_id;
    

    const std::string& name = m_items[item_id].m_code_name;
    
    int row = -1;
    int id;
    
    for (int r=0; r<m_row_amount; r++)
    {
        id = m_rows[r].findItemNamed(name.c_str());
        if (id > -1)
        {
            row = r;
            break;
        }
    }
    
    if (row == -1)
    {
        std::cerr << "RibbonGridWidget::setSelection cannot find item " << item_id << " (" << name.c_str() << ")\n";
        return;
    }
    
    std::cout << "Player " << playerID << " has kart " << item_id << " (" << name.c_str() << ") in row " << row << std::endl;
    m_rows[row].setSelection(id, playerID);
    m_rows[row].setFocusForPlayer(playerID);

    propagateSelection();
}
