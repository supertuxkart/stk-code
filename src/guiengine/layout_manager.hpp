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

#ifndef __LAYOUT_MANAGER_HPP__
#define __LAYOUT_MANAGER_HPP__

#include <cstring> // for NULL
#include "utils/ptr_vector.hpp"

#include "guiengine/widget.hpp"

namespace GUIEngine
{
    class Widget;
    
    class ITopLevelWidgetContainer
    {
    protected:
        /** the widgets in this screen */
        ptr_vector<Widget, HOLD> m_widgets;
        
        /**
         * Screen is generally able to determine its first widget just fine, but in highly complex screens
         * (e.g. multiplayer kart selection) you can help it by providing the first widget manually.
         */
        Widget* m_first_widget;
        
        /**
         * Screen is generally able to determine its last widget just fine, but in highly complex screens
         * (e.g. multiplayer kart selection) you can help it by providing the first widget manually.
         */
        Widget* m_last_widget;
        
        void addWidgetsRecursively(ptr_vector<Widget>& widgets, Widget* parent=NULL);

    
    public:
        virtual ~ITopLevelWidgetContainer() {}
        
        virtual int getWidth() = 0;
        virtual int getHeight() = 0;
        
        /** \return an object by name, or NULL if not found */
        Widget* getWidget(const char* name);
        
        /** \return an object by irrlicht ID, or NULL if not found */
        Widget* getWidget(const int id);
        
        /** \return an object by name, casted to specified type, or NULL if not found/wrong type */
        template <typename T> T* getWidget(const char* name)
        {
            Widget* out = getWidget(name);
            T* outCasted = dynamic_cast<T*>( out );
            if (out != NULL && outCasted == NULL)
            {
                fprintf(stderr, "Screen::getWidget : Widget '%s' of type '%s' cannot be casted to "
                        "requested type '%s'!\n", name, typeid(*out).name(), typeid(T).name()); 
                abort();
            }
            return outCasted;
        }
        
        static Widget* getWidget(const char* name, ptr_vector<Widget>* within_vector);
        static Widget* getWidget(const int id, ptr_vector<Widget>* within_vector);
      
        Widget* getFirstWidget(ptr_vector<Widget>* within_vector=NULL);
        Widget* getLastWidget(ptr_vector<Widget>* within_vector=NULL);
        
        bool isMyChild(Widget* widget) const;
    };
    
    class LayoutManager
    {
    
        /**
         * \brief Receives as string the raw property value retrieved from XML file.
         * Will try to make sense of it, as an absolute value or a percentage.
         *
         * Return values :
         *     Will write to either absolute or percentage, depending on the case.
         *     Returns false if couldn't convert to either
         */
        static bool convertToCoord(std::string& x, int* absolute /* out */, int* percentage /* out */);
        
    public:
        /**
         * \brief Find a widget's x, y, w and h coords from what is specified in the XML properties.
         * Most notably, expands coords relative to parent and percentages.
         */
        static void readCoords(Widget* self, ITopLevelWidgetContainer* topLevelContainer, Widget* parent);
        
        /**
         * \brief Recursive call that lays out children widget within parent (or screen if none).
         *
         * Manages 'horizontal-row' and 'vertical-row' layouts, along with the proportions
         * of the remaining children, as well as absolute sizes and locations.
         */
        static void calculateLayout(ptr_vector<Widget>& widgets, ITopLevelWidgetContainer* topLevelContainer,
                                    Widget* parent=NULL);
        
    };
}

#endif
