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



#ifndef HEADER_SKILL_LEVEL_HPP
#define HEADER_SKILL_LEVEL_HPP

#include <irrString.h>

#include "guiengine/widget.hpp"
#include "utils/leak_check.hpp"
#include "utils/ptr_vector.hpp"

#include "guiengine/widgets/label_widget.hpp"
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
        
        /** widget coordinates */
        int m_bar_x, m_bar_y, m_bar_h, m_bar_w;
        int m_label_x, m_label_y, m_label_h, m_label_w;

        std::string m_label_name;
        
        int m_player_id;
        bool m_display_text;

    public:
        
        LEAK_CHECK()
        
        LabelWidget* m_label;
        ProgressBarWidget* m_bar;

        SkillLevelWidget(core::recti area, const int player_id, bool multiplayer, bool display_text,
                         const int value = 0, const irr::core::stringw& label = "default");

        virtual ~SkillLevelWidget() {};

        // ------------------------------------------------------------------------
        /** Add the widgets to the current screen */
        virtual void add();

        // -------------------------------------------------------------------------

        virtual void move(const int x, const int y, const int w, const int h);

        // -------------------------------------------------------------------------

        /** Sets the size of the widget as a whole, and placed children widgets
         * inside itself */
        void setSize(const int x, const int y, const int w, const int h);

        /** Change the value of the widget, it must be a percent. */
        void setValue(const int value);
        
        /** Get the current values of the widget. */
        int getValue() {return m_bar->getValue(); };

        /** Change the label of the widget */
        void setLabel(const irr::core::stringw& label);

        /** Get the current label of the widget. */
        const irr::core::stringw getLabel()
        {
            return m_label->getText();
        }

        /** If the label should be displayed. */
        void setDisplayText(bool display_text);
    };
}

#endif
