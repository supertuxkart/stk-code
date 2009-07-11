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


#ifndef HEADER_SCREEN_HPP
#define HEADER_SCREEN_HPP

#include <map>
#include <string>

#include "irrlicht.h"

#include "input/input.hpp"
#include "utils/ptr_vector.hpp"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


namespace GUIEngine
{
    class Widget;
   
    void parseScreenFileDiv(irr::io::IrrXMLReader* xml, ptr_vector<Widget>& append_to);
    
    class Screen : public IEventReceiver
    {
        friend class Skin;
        
        bool m_loaded;
        std::string m_filename;
        ptr_vector<Widget, HOLD> m_widgets;
        void loadFromFile();

        static void addWidgetsRecursively(ptr_vector<Widget>& widgets, Widget* parent=NULL);
        void calculateLayout(ptr_vector<Widget>& widgets, Widget* parent=NULL);
        bool onWidgetActivated(Widget* w);
    public:
        // current mouse position, read-only...
        int m_mouse_x, m_mouse_y;
        
        /** this variable is not used by the Screen object itself; it's the routines creating
          * screens that may use it to perform some operations only once. initialized to false.
          */
        bool m_inited;
        
        Screen(const char* filename);
        bool operator ==(const char* filename) const { return m_filename == filename; }
        
        /** returns an object by name, or NULL if not found */
        Widget* getWidget(const char* name);
        
        /** returns an object by name, casted to specified type, or NULL if not found/wrong type */
        template <typename T> T* getWidget(const char* name)
        {
            return dynamic_cast<T*>( getWidget(name) );
        }
        
        Widget* getWidget(const char* name, ptr_vector<Widget>* within_vector);
        Widget* getWidget(const int id, ptr_vector<Widget>* within_vector=NULL);
        
        Widget* getFirstWidget(ptr_vector<Widget>* within_vector=NULL);
        Widget* getLastWidget(ptr_vector<Widget>* within_vector=NULL);
        
        void addWidgets();
        void calculateLayout();
        
        void manualAddWidget(Widget* w);
        void manualRemoveWidget(Widget* w);

        const std::string& getName() const { return m_filename; }
        
        void elementsWereDeleted(ptr_vector<Widget>* within_vector = NULL);
        
        virtual bool OnEvent(const SEvent& event);
        void processAction(const int action, const unsigned int value, Input::InputType type);

    };
    
}

#endif
