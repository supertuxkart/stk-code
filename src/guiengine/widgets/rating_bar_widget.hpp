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
        
        float m_rating;
        int m_star_number;
        
    public:
        
        LEAK_CHECK()
        
        RatingBarWidget();
        virtual ~RatingBarWidget() {}
        
        void add();
        
        /** Change the rating value of the widget. */
        void setRating(float rating) { m_rating = rating; };
        
        /** Get the current value of the widget. */
        float getRating() {return m_rating; };
        
        /** Change the number of star of the widget. */
        void setStarNumber(int star_number) { m_star_number = star_number; };
        
        /** Get the current number of star of the widget. */
        int getStarNumber() {return m_star_number; };
        
        int getStepOfStar(int index, int max_step);
    }; 
}

#endif
