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



#ifndef HEADER_KART_STATS_HPP
#define HEADER_KART_STATS_HPP

#include <irrString.h>

#include "guiengine/widget.hpp"
#include "utils/leak_check.hpp"
#include "utils/ptr_vector.hpp"

#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/progress_bar_widget.hpp"


namespace GUIEngine
{
    /**
      * \brief A progress bar widget.
      * \ingroup widgetsgroup
      */

class KartStatsWidget : public Widget
    {
        /** When inferring widget size from its label length, this method will be called to
         * if/how much space must be added to the raw label's size for the widget to be large enough */
        virtual int getWidthNeededAroundLabel()  const { return 35; }
        
        /** When inferring widget size from its label length, this method will be called to
         * if/how much space must be added to the raw label's size for the widget to be large enough */
        virtual int getHeightNeededAroundLabel() const { return 4; }
        
        /** widget coordinates */
        int m_mass_bar_x, m_mass_bar_y, m_mass_bar_h, m_mass_bar_w;

        /** For animation purposes (see method 'move') */
        int target_x, target_y, target_w, target_h;
        float x_speed, y_speed, w_speed, h_speed;

        int m_mass_value;
        int m_accel_value;
        int m_speed_value;
        
        int m_player_id;

    public:
        
        LEAK_CHECK()
        
        LabelWidget* m_mass_label;
        ProgressBarWidget* m_mass_bar;

        KartStatsWidget(core::recti area, const int player_id,
                        std::string kart_group);
        virtual ~KartStatsWidget() {};

        // ------------------------------------------------------------------------
        /** Add the widgets to the current screen */
        virtual void add();

        // -------------------------------------------------------------------------
        /** Updates the animation (moving/shrinking/etc.) */
        void onUpdate(float delta);

        // -------------------------------------------------------------------------
        /** Event callback */

        // -------------------------------------------------------------------------
        /** Sets the size of the widget as a whole, and placed children widgets
         * inside itself */
        void setSize(const int x, const int y, const int w, const int h);

        /** Change the value of the widget, it must be a percent. */
        void setMass(int value);
        void setAcceleration(int value);
        void setSpeed(int value);
        
        /** Get the current values of the widget. */
        int getMass() {return m_mass_value; };
        int getAcceleration() {return m_accel_value; };
        int getSpeed() {return m_speed_value; };
    };
}

#endif
