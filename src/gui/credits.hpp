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


#ifndef HEADER_CREDITS_HPP
#define HEADER_CREDITS_HPP

#include "irrlicht.h"
using namespace irr;

#include "utils/ptr_vector.hpp"

namespace GUIEngine
{
    class CreditsSection;
    
    class Credits
        {
            float m_time_element;
            
            ptr_vector<CreditsSection, HOLD> m_sections;
            CreditsSection* getCurrentSection();
            
            int x, y, w, h;
            core::rect< s32 > m_section_rect;
            
            int m_curr_section;
            int m_curr_element;
            
            float time_before_next_step;
            
        public:
            Credits();
            
            static Credits* getInstance();
            void setArea(const int x, const int y, const int w, const int h);
            
            // start from beginning again
            void reset();
            
            void render(const float elapsed_time);
        };
}
#endif
