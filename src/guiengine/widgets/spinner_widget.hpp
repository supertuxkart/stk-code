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



#ifndef HEADER_SPINNER_HPP
#define HEADER_SPINNER_HPP

#include <irrString.h>
namespace irr
{
    namespace video { class ITexture; }
}

#include "guiengine/widget.hpp"
#include "utils/leak_check.hpp"
#include "utils/ptr_vector.hpp"

namespace GUIEngine
{
    
    /** \brief A spinner or gauge widget (to select numbers / percentages).
      * \ingroup widgetsgroup
      */
    class SpinnerWidget : public Widget
    {
    public:
        class ISpinnerConfirmListener
        {
        public:
            virtual ~ISpinnerConfirmListener() {}
            
            /**
              * \brief Invoked when the spinner is selected and "fire" is pressed
              * \return whether to block the event from further processing
              */
            virtual EventPropagation onSpinnerConfirmed() = 0;
        };
        
    protected:
        
        ISpinnerConfirmListener* m_listener;
        
        int m_value, m_min, m_max;
        
        int m_spinner_widget_player_id;
        bool m_use_background_color;
        
        /** If each value the spinner can take has an associated text, this vector will be non-empty */
        std::vector<irr::core::stringw> m_labels;
        
        /** Whether the value of this spinner is displayed using an icon rather than with a plain label */
        bool m_graphical;
        
        /** \brief Whether this widget is a gauge
          * the behaviour is the same but the look is a bit different, instead of displaying a number,
          * it displays how close the value is to the maximum by filling a line
          */
        bool m_gauge;
    
    
        /** \brief Whether to wrap back to the first value when going "beyond" the last value */
        bool m_wrap_around;

        /** \brief Keeps track of the custom text in spinner (a text which isn't related to a value)
        *   to remember it and set it back (example : when we deactivate the widget)
        */
        core::stringw m_customText;
        
        /** \brief implementing method from base class Widget */
        virtual EventPropagation transmitEvent(Widget* w,
                                               const std::string& originator,
                                               const int playerID);
        
        /** \brief implementing method from base class Widget */
        virtual EventPropagation rightPressed(const int playerID);
        
        /** \brief implementing method from base class Widget */
        virtual EventPropagation leftPressed(const int playerID);

        /** \brief implementing method from base class Widget */
        virtual void onClick();
        
        /** When inferring widget size from its label length, this method will be called to
         * if/how much space must be added to the raw label's size for the widget to be large enough */
        virtual int getWidthNeededAroundLabel()  const { return 25; }
        
        /** When inferring widget size from its label length, this method will be called to
         * if/how much space must be added to the raw label's size for the widget to be large enough */
        virtual int getHeightNeededAroundLabel() const { return 8; }
        
        /** Call only if this spinner is graphical. Returns the current texture to display */
        irr::video::ITexture* getTexture();
       
    public:

        LEAK_CHECK()
        
        SpinnerWidget(const bool gauge=false);
        virtual ~SpinnerWidget() {}
        virtual void move(const int x, const int y, const int w, const int h);
                
        void addLabel(irr::core::stringw label);
        void clearLabels();

    // next four functions are for background colour behind playername in multikart screen selection
        void setUseBackgroundColor()                {m_use_background_color=true;        }
        bool getUseBackgroundColor()                {return m_use_background_color;      }
        void setSpinnerWidgetPlayerID(int playerID) {m_spinner_widget_player_id=playerID;}
        int getSpinnerWidgetPlayerID()              {return m_spinner_widget_player_id;  }
        void unsetUseBackgroundColor()              {m_use_background_color=false;       }



        void setListener(ISpinnerConfirmListener* listener) { m_listener = listener; }

        /** \brief implement method from base class Widget */
        virtual void add();

        /**
         * \brief            sets the current value of the spinner
         * \param new_value  the new value that will be become the current value of this spinner.
         */
        void setValue(const int new_value);
        
        /**
          * \brief        sets the current value of the spinner
          * \pre the 'new_value' string passed must be the name of an item
          *               (added through SpinnerWidget::addLabel)in the spinner
          */
        void setValue(irr::core::stringw new_value);
        
        /**
          * \return whether this spinner is of "gauge" type
          */
        bool isGauge()  const { return m_gauge; }
        
        /**
         * \brief retrieve the current value of the spinner
         * \return the current value of the spinner, in a int form
         */
        int  getValue() const { return m_value; }
        
        /**
          * \brief retrieve the current value of the spinner
          * \return the current value of the spinner, in a string form
          */
        irr::core::stringw getStringValue() const;

        /**
          * \return the maximum value the spinner can take
          */
        // --------------------------------------------------------------------
        /** Returns the maximum value. */
        int  getMax()  const { return m_max;   }
        // --------------------------------------------------------------------
        /** \brief Sets the maximum value for a spinner.
         *  If the current value is larger than the new maximum, the current
         *  value is set to the new maximum. */
        void setMax(int n)
        {
            m_max = n;
            if(getValue()>m_max) setValue(m_max);
        }   // setMax
        // --------------------------------------------------------------------
        /**
          * \return the minimum value the spinner can take
          */
        int  getMin()   const { return m_min;   }
        // --------------------------------------------------------------------
        /** \brief Sets the minimum value for a spinner.
         *  If the current value is smaller than the new minimum, the current
         *  value is set to the new minimum. */
        void setMin(int n)
        {
            m_min = n;
            if(getValue()<m_min) setValue(m_min);
        }   // setMin
        
        // --------------------------------------------------------------------
        /** Override method from base class Widget */
        virtual void setActive(bool active = true);

        /** Display custom text in spinner */
        void setCustomText(const core::stringw& text);
    };

}

#endif
