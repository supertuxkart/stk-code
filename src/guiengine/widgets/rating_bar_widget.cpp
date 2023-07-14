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

#include "guiengine/engine.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/widgets/rating_bar_widget.hpp"
#include "utils/string_utils.hpp"
#include "utils/vs.hpp"

#include <IGUIEnvironment.h>
#include <IGUIElement.h>
#include <IGUIButton.h>
#include <cmath>
#include <string.h>

using namespace GUIEngine;
using namespace irr::core;
using namespace irr;

// -----------------------------------------------------------------------------
RatingBarWidget::RatingBarWidget() : Widget(WTYPE_RATINGBAR)
{
    //m_event_handler = this;
    m_allow_voting = false;
    m_rating = 0.0f;
    m_hover_rating = 0.0f;
    m_stars = 3;
    m_steps = 3;
    m_hovering = false;
    for(int i = 0; i < m_stars; i++)
        m_star_values.push_back(0);
}

// -----------------------------------------------------------------------------
void RatingBarWidget::add()
{
    const irr::core::recti widget_size = rect<s32>(m_x, m_y, m_x + m_w, m_y + m_h);
    m_element = GUIEngine::getGUIEnv()->addButton(widget_size, m_parent, getNewID(), NULL, L"");
    m_id = m_element->getID();
    m_element->setTabStop(false);
    m_element->setTabGroup(false);
}

// -----------------------------------------------------------------------------
/** Get the current step of the star
 *
 * \param index     The index of the star.
 * \return The current step of the star.
 */
int RatingBarWidget::getStepsOfStar(int index)
{
    assert(index >= 0 && index < m_stars); // Index must be between 0 and m_star_number - 1.

    return m_star_values[index];
} // getStepOfStar



void RatingBarWidget::setStepValues(float float_rating)
{
    for (int star = 0; star < m_stars; star++)
    {
        if (float_rating < star)
            m_star_values[star] = 0;
        else if (float_rating > star + 1)
            m_star_values[star] = m_steps-1;
        else
        {
            m_star_values[star] =(int)roundf((float_rating * (m_steps-1)) - (star*(m_steps-1)));
        }
    }
}

// -----------------------------------------------------------------------------

void RatingBarWidget::setRating(float rating)
{
    m_hover_rating = m_rating = rating;
    setStepValues(m_rating);
}

// -----------------------------------------------------------------------------

void RatingBarWidget::setStepValuesByMouse(const core::position2di & mouse_position, const core::recti & stars_rect)
{
    if(m_allow_voting){
        if(stars_rect.isPointInside(mouse_position))
        {
            m_hovering = true;
            float exact_hover = (float)(mouse_position.X - stars_rect.UpperLeftCorner.X) / (float)stars_rect.getWidth() * (float)m_stars;
            m_hover_rating = roundf(exact_hover * (m_steps-1)) / (m_steps-1);
            setStepValues(m_hover_rating);
        }
    }
}

// -----------------------------------------------------------------------------

EventPropagation RatingBarWidget::leftPressed(const int playerID)
{
    if(m_allow_voting && m_hover_rating > 0.0f)
    {
        m_hover_rating -= 1.0f / (m_steps - 1);
        setStepValues(m_hover_rating);
    }
    return EVENT_BLOCK;
}

EventPropagation RatingBarWidget::rightPressed(const int playerID)
{
    if(m_allow_voting && m_hover_rating < m_stars)
    {
        m_hover_rating += 1.0f / (m_steps - 1);
        setStepValues(m_hover_rating);
    }
    return EVENT_BLOCK;
}

bool RatingBarWidget::updateRating()
{
    if(m_allow_voting)
    {
        if (m_hover_rating <= 0.0f || m_hover_rating > m_stars)
        {
            return false;
        }
        else
        {
            m_rating = m_hover_rating;
            return true;
        }
    }
    return false;
}
