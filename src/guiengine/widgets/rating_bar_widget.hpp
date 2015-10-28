//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
//                2013 Glenn De Jonghe
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



#ifndef HEADER_RATING_BAR_HPP
#define HEADER_RATING_BAR_HPP

#include <irrString.h>

#include "guiengine/widget.hpp"
#include "utils/leak_check.hpp"
#include "utils/ptr_vector.hpp"

namespace GUIEngine
{
    /**
      * \brief A rating bar widget.
      * \ingroup widgetsgroup
      */
    class RatingBarWidget : public Widget
    {
    private:
        float                   m_rating;
        float                   m_hover_rating;
        int                     m_stars;
        int                     m_steps;
        std::vector<int>        m_star_values;
        bool                    m_hovering;
        bool                    m_allow_voting;

        void setStepValues(float rating);

    public:
        
        LEAK_CHECK()
        
        RatingBarWidget();
        virtual ~RatingBarWidget() {}
        


        void add();
        
        /** Change the rating value of the widget. */
        void setRating(float rating);
        
        /** Get the current value of the widget. */
        float getRating() {return m_rating; };
        
        /** Change the number of stars of the widget. */
        void setStarNumber(int star_number) { m_stars = star_number; };
        
        /** Get the current number of stars of the widget. */
        int getStarNumber() {return m_stars; };
        
        int getStepsOfStar(int index);

        void setStepValuesByMouse(const core::position2di & mouse_position, const core::recti & stars_rect);

        virtual void onClick();

        void allowVoting() { m_allow_voting = true; }
    };
}

#endif
