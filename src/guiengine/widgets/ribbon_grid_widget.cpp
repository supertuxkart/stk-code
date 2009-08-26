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
    
    m_max_rows = max_rows;
    m_combo = combo;
    
    m_left_widget = NULL;
    m_right_widget = NULL;
    m_type = WTYPE_RIBBON_GRID;
    
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::add()
{
    m_has_label = m_properties[PROP_TEXT] == "bottom";
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
    
    setSubElements();
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::setSubElements()
{
    // Work-around for FIXME below... first clear children to avoid duplicates since we're adding everything again everytime
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
    
    int child_width, child_height;
    child_width = atoi(m_properties[PROP_CHILD_WIDTH].c_str());
    child_height = atoi(m_properties[PROP_CHILD_HEIGHT].c_str());
    
    if (child_width == 0 || child_height == 0)
    {
        std::cerr << "/!\\ Warning /!\\ : ribbon grid widgets require 'child_width' and 'child_height' arguments" << std::endl;
        child_width = 256;
        child_height = 256;
    }
    
    // decide how many rows and column we can show in the available space
    int row_amount = (int)round((h-m_label_height) / (float)child_height);
    //if(row_amount < 2) row_amount = 2;
    if (row_amount > m_max_rows) row_amount = m_max_rows;
    
    const float row_height = (float)(h - m_label_height)/(float)row_amount;
    
    float ratio_zoom = (float)row_height / (float)(child_height - m_label_height);
    m_col_amount = (int)round( w / ( child_width*ratio_zoom ) );
    
    // std::cout << "w=" << w << " child_width=" << child_width << " ratio_zoom="<< ratio_zoom << " m_col_amount=" << m_col_amount << std::endl;
    
    //if(m_col_amount < 5) m_col_amount = 5;
    
    // add rows
    for(int n=0; n<row_amount; n++)
    {
        RibbonWidget* ribbon;
        if (m_combo)
            ribbon = new RibbonWidget(RIBBON_COMBO);
        else
            ribbon = new RibbonWidget(RIBBON_TOOLBAR);
        ribbon->x = x + m_arrows_w;
        ribbon->y = y + (int)(n*row_height);
        ribbon->w = w - m_arrows_w*2;
        ribbon->h = (int)(row_height);
        ribbon->m_type = WTYPE_RIBBON;
        ribbon->m_properties[PROP_ID] = this->m_properties[PROP_ID];
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
void RibbonGridWidget::registerHoverListener(RibbonGridHoverListener* listener)
{
    m_hover_listeners.push_back(listener);
}
// -----------------------------------------------------------------------------
bool RibbonGridWidget::rightPressed()
{
    RibbonWidget* w = getSelectedRibbon();
    if(w != NULL)
    {
        updateLabel();
        propagateSelection();
    }
    
    const int listenerAmount = m_hover_listeners.size();
    for(int n=0; n<listenerAmount; n++)
    {
        m_hover_listeners[n].onSelectionChanged(this, getSelectedRibbon()->getSelectionIDString());
    }
    
    if(m_rows[0].m_ribbon_type == RIBBON_TOOLBAR) return false;
    
    return true;
}
// -----------------------------------------------------------------------------
bool RibbonGridWidget::leftPressed()
{
    RibbonWidget* w = getSelectedRibbon();
    if(w != NULL)
    {
        updateLabel();
        propagateSelection();
    }
    
    const int listenerAmount = m_hover_listeners.size();
    for(int n=0; n<listenerAmount; n++)
    {
        m_hover_listeners[n].onSelectionChanged(this, w->getSelectionIDString());
    }
    
    if(m_rows[0].m_ribbon_type == RIBBON_TOOLBAR) return false;
    
    return true;
}
// -----------------------------------------------------------------------------
bool RibbonGridWidget::transmitEvent(Widget* w, std::string& originator)
{
    if(originator=="left")
    {
        scroll(-1);
        return false;
    }
    if(originator=="right")
    {
        scroll(1);
        return false;
    }
    
    return true;
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::scroll(const int x_delta)
{
    m_scroll_offset += x_delta;
    
    const int max_scroll = std::max(m_col_amount, m_needed_cols) - 1;
    
    if(m_scroll_offset < 0) m_scroll_offset = max_scroll;
    else if(m_scroll_offset > max_scroll) m_scroll_offset = 0;
    
    updateItemDisplay();
}
// -----------------------------------------------------------------------------
bool RibbonGridWidget::mouseHovered(Widget* child)
{
    updateLabel();
    propagateSelection();
    
    if(getSelectedRibbon() != NULL)
    {
        const int listenerAmount = m_hover_listeners.size();
        for(int n=0; n<listenerAmount; n++)
        {
            m_hover_listeners[n].onSelectionChanged(this, getSelectedRibbon()->getSelectionIDString());
        }
    }
    
    return false;
}
// -----------------------------------------------------------------------------
/** RibbonGridWidget is made of several ribbons; each of them thus has
 its own selection independently of each other. To keep a grid feeling
 (i.e. you remain in the same column when pressing up/down), this method is
 used to ensure that all children ribbons always select the same column */
void RibbonGridWidget::propagateSelection()
{
    // find selection in current ribbon
    RibbonWidget* selected_ribbon = (RibbonWidget*)getSelectedRibbon();
    if(selected_ribbon == NULL) return;
    const int i = selected_ribbon->m_selection;
    
    // set same selection in all ribbons
    const int row_amount = m_rows.size();
    for(int n=0; n<row_amount; n++)
    {
        RibbonWidget* ribbon = m_rows.get(n);
        if(ribbon != selected_ribbon)
        {
            ribbon->m_selection = i;
            ribbon->updateSelection();
        }
    }
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::focused()
{
    updateLabel();
    
    const int listenerAmount = m_hover_listeners.size();
    for(int n=0; n<listenerAmount; n++)
    {
        m_hover_listeners[n].onSelectionChanged(this, getSelectedRibbon()->getSelectionIDString());
    }
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::onRowChange(RibbonWidget* row)
{
    updateLabel(row);
    
    const int listenerAmount = m_hover_listeners.size();
    for(int n=0; n<listenerAmount; n++)
    {
        m_hover_listeners[n].onSelectionChanged(this, row->getSelectionIDString());
    }
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::updateLabel(RibbonWidget* from_this_ribbon)
{
    if(!m_has_label) return;
    
    RibbonWidget* row = from_this_ribbon ? from_this_ribbon : (RibbonWidget*)getSelectedRibbon();
    if(row == NULL) return;
    
    
    std::string selection_id = row->getSelectionIDString();
    
    const int amount = m_items.size();
    for(int n=0; n<amount; n++)
    {
        if(m_items[n].m_code_name == selection_id)
        {
            m_label->setText( stringw(m_items[n].m_user_name.c_str()).c_str() );
            return;
        }
    }
    
    m_label->setText( L"Random" );
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
void RibbonGridWidget::setSelection(int item_id)
{
    if(m_rows.size() > 1)
    {
        std::cout << "/!\\ Warning, RibbonGridWidget::setSelection only makes sense on 1-row ribbons (since there can't logically be a selection with more than one row)\n";
    }
    
    
    // scroll so selection is visible
    m_scroll_offset = item_id;
    updateItemDisplay();
    
    // set selection again, because scrolling made it wrong
    RibbonWidget* ribbon = m_rows.get(0);
    ribbon->setSelection(0);
}
void RibbonGridWidget::setSelection(const std::string& code_name)
{
    assert(false);
}
// -----------------------------------------------------------------------------
void RibbonGridWidget::updateItemDisplay()
{
    int icon_id = 0;
    
    const int row_amount = m_rows.size();
    const int item_amount = m_items.size();
    
    m_needed_cols = (int)ceil( (float)item_amount / (float)row_amount );
    
    const int max_scroll = std::max(m_col_amount, m_needed_cols) - 1;
    
    for(int n=0; n<row_amount; n++)
    {
        RibbonWidget& row = m_rows[n];
        
        for(int i=0; i<m_col_amount; i++)
        {
            IconButtonWidget* icon = dynamic_cast<IconButtonWidget*>(&row.m_children[i]);
            assert(icon != NULL);
            IGUIButton* button = dynamic_cast<IGUIButton*>(icon->m_element);
            assert(button != NULL);
            
            int col_scroll = i + m_scroll_offset;
            while(col_scroll > max_scroll) col_scroll -= max_scroll+1;
            
            icon_id = (col_scroll)*row_amount + n;
            
            if( icon_id < item_amount )
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
const std::string& RibbonGridWidget::getSelectionIDString()
{
    RibbonWidget* row = (RibbonWidget*)(m_rows.size() == 1 ? m_rows.get(0) : getSelectedRibbon());
    
    if(row != NULL) return row->getSelectionIDString();
    
    static const std::string nothing = "";
    return nothing;
}
// -----------------------------------------------------------------------------
const std::string& RibbonGridWidget::getSelectionText()
{
    RibbonWidget* row = (RibbonWidget*)(m_rows.size() == 1 ? m_rows.get(0) : getSelectedRibbon());
    
    if(row != NULL) return row->getSelectionText();
    
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
RibbonWidget* RibbonGridWidget::getSelectedRibbon() const
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
    
    return NULL;
}

