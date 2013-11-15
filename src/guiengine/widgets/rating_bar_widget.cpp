//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2013 Marianne Gagnon
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
#include "guiengine/widgets/rating_bar_widget.hpp"
#include "utils/string_utils.hpp"
#include <string.h>

#include <IGUIEnvironment.h>
#include <IGUIElement.h>
#include <IGUIButton.h>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr;

// -----------------------------------------------------------------------------
RatingBarWidget::RatingBarWidget() : Widget(WTYPE_RATINGBAR)
{
    m_rating = 0;
    m_star_number = 0;
}

// -----------------------------------------------------------------------------
void RatingBarWidget::add()
{
    rect<s32> widget_size = rect<s32>(m_x, m_y, m_x + m_w, m_y + m_h);
    m_element = GUIEngine::getGUIEnv()->addButton(widget_size, m_parent, getNewNoFocusID(), NULL, L"");

    m_id = m_element->getID();
    m_element->setTabStop(false);
    m_element->setTabGroup(false);
}

// -----------------------------------------------------------------------------
/** Get the current step of the star
 * 
 * \param index     The index of the star.
 * \param max_step  The number of different steps that a star can display. Two 
 *                   step are obligatory: full and empty.
 * \return The current step of the star.
 */
int RatingBarWidget::getStepOfStar(int index, int max_step)
{
    assert(index >= 0 && index < m_star_number); // Index must be between 0 and m_star_number - 1.
    assert(max_step >= 2); // The maximun number of step must be superior or equals to 2.

    if (m_rating < index)
    {
        return 0;
    }
    else if (m_rating > index + 1)
    {
        return max_step - 1;
    }
    else
    {
        float step_size = 1 / (float)(max_step - 1);
        
        for (int i = 0; i < max_step; i++)
        {
            if (m_rating > index + step_size * (i - 0.5)
                && m_rating < index + step_size * (i + 0.5))
                return i;
        }
    }
    
    return 0;
    // TODO: Assert or throws a exception, what type?
} // getStepOfStar

