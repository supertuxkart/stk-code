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



#ifndef HEADER_SKILL_LEVEL_HPP
#define HEADER_SKILL_LEVEL_HPP

#include <irrString.h>

#include "guiengine/widget.hpp"
#include "utils/leak_check.hpp"
#include "utils/ptr_vector.hpp"

#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/progress_bar_widget.hpp"


namespace GUIEngine
{
    /**
      * \brief A skill level widget.
      * \ingroup widgetsgroup
      */
    class SkillLevelWidget : public Widget
    {
        /** When inferring widget size from its label length, this method will be called to
         * if/how much space must be added to the raw label's size for the widget to be large enough */
        virtual int getWidthNeededAroundLabel()  const { return 35; }
        
        /** When inferring widget size from its label length, this method will be called to
         * if/how much space must be added to the raw label's size for the widget to be large enough */
        virtual int getHeightNeededAroundLabel() const { return 4; }
        
        int m_player_id;
        //shall icon be display left of the skill bar?
        bool m_display_icon;

    public:
        
        LEAK_CHECK()
        
        IconButtonWidget* m_iconbutton;
        ProgressBarWidget* m_bar;

        SkillLevelWidget(const int player_id,
                        bool display_text,
                         const float value = 0);

        virtual ~SkillLevelWidget() {};

        // ------------------------------------------------------------------------
        /** Add the widgets to the current screen */
        virtual void add();

        // -------------------------------------------------------------------------

        virtual void resize();

        // -------------------------------------------------------------------------

        /** Change the value of the widget, it must be a percent. */
        void setValue(const float value);
        
        /** Get the current values of the widget. */
        float getValue() {return m_bar->getValue(); };
        
        /** Change the image for the icon. Expects an absolute file path*/
        void setIcon(const irr::core::stringc& filepath);

        /** Get the current label of the widget. */
        const irr::core::stringw getLabel()
        {
            return m_iconbutton->getText();
        }

        /** If the label should be displayed. */
        void setDisplayIcon(bool display_icon);
    };
}

#endif
