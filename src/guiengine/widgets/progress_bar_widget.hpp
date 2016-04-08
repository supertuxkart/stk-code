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



#ifndef HEADER_PROGRESS_BAR_HPP
#define HEADER_PROGRESS_BAR_HPP

#include <irrString.h>

#include "guiengine/widget.hpp"
#include "utils/leak_check.hpp"
#include "utils/ptr_vector.hpp"

namespace GUIEngine
{
    /**
      * \brief A progress bar widget.
      * \ingroup widgetsgroup
      */
    class ProgressBarWidget : public Widget
    {
        /** When inferring widget size from its label length, this method will be called to
         * if/how much space must be added to the raw label's size for the widget to be large enough */
        virtual int getWidthNeededAroundLabel()  const { return 35; }

        /** When inferring widget size from its label length, this method will be called to
         * if/how much space must be added to the raw label's size for the widget to be large enough */
        virtual int getHeightNeededAroundLabel() const { return 4; }

        /**  Change the label on the widget */
        void setLabel(const irr::core::stringw label);
        int m_value;
        bool m_show_label;

        /** Values for animation */
        int m_target_value;
        int m_previous_value;

    public:

        LEAK_CHECK()
        ProgressBarWidget(bool show_label = true);
        virtual ~ProgressBarWidget();

        /** Change the value of the widget, it must be a percent. */
        void setValue(int value);

        /** Change the value of the widget smooth, it must be a percent. */
        void moveValue(int value);

        virtual void update(float delta);

        void add();

        /** Get the current value of the widget. */
        int getValue() {return m_value; };

    };
}

#endif
