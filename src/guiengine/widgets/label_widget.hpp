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



#ifndef HEADER_LABEL_HPP
#define HEADER_LABEL_HPP

#include <irrlicht.h>

#include "guiengine/widget.hpp"
#include "utils/ptr_vector.hpp"

namespace GUIEngine
{
    /** \brief A simple label widget.
      * \ingroup widgets
      */
    class LabelWidget : public Widget
    {
        bool               m_has_color;
        irr::video::SColor m_color;

        /** Scroll speed in characters/seconds (0 if no scrolling). */
        float              m_scroll_speed;
        /** Current scroll offset. */
        float              m_scroll_offset;
    public:
                 LabelWidget(bool title=false, bool bright=false);
        virtual ~LabelWidget() {}
        
        virtual void add();
                
        /** Sets the color of the widget. 
         *  \param color The color to use for this widget. */
        void     setColor(const irr::video::SColor& color)
        {
            m_color     = color;
            m_has_color = true;
        }   // setColor
        

        virtual void setText(const wchar_t *text);
        virtual void update(float dt);
        
        // --------------------------------------------------------------------
        /** Overloaded function which takes a stringw. */
        virtual void setText(const irr::core::stringw &s) 
        {
            setText(s.c_str()); 
        }
        
        // --------------------------------------------------------------------
        
        /** Sets horizontal scroll speed. */
        void    setScrollSpeed(float speed);
        
        // --------------------------------------------------------------------
        bool scrolledOff() const;

    };
}

#endif
