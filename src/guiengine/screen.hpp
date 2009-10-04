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

#include "guiengine/engine.hpp"
#include "guiengine/widget.hpp"
#include "input/input.hpp"
#include "utils/ptr_vector.hpp"

// FIXME : don't use 'using namespace' in header!!!!
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


namespace GUIEngine
{
   
    template<typename SCREEN>
    class ScreenSingleton
    {
    public:

        static SCREEN* getInstance()
        {
            static SCREEN* singleton = NULL;
            
            if (singleton == NULL)
            {
                singleton = new SCREEN();
                GUIEngine::addScreenToList(singleton);
            }
            
            return singleton;
        }
    };
    
    void parseScreenFileDiv(irr::io::IrrXMLReader* xml, ptr_vector<Widget>& append_to);
    
    enum ScreenType
    {
        SCREEN_TYPE_MENU,
        SCREEN_TYPE_CUTSCENE
    };
    
    /**
      * Represents a single screen. Mainly responsible of its children widgets; Screen lays them
      * out, asks them to add themselves, asks them to remove themselves, etc.
      *
      * Also initiates the read of GUI files, even though most of that work is done in "screen_loader.cpp"
      */
    class Screen
    {
        friend class Skin;
        
        bool m_loaded;
        std::string m_filename;
        void loadFromFile();

        static void addWidgetsRecursively(ptr_vector<Widget>& widgets, Widget* parent=NULL);
        void calculateLayout(ptr_vector<Widget>& widgets, Widget* parent=NULL);
        
    public:
        ptr_vector<Widget, HOLD> m_widgets;

        // current mouse position, read-only...
        int m_mouse_x, m_mouse_y;
        
        /** this variable is not used by the Screen object itself; it's the routines creating
          * screens that may use it to perform some operations only once. initialized to false.
          */
        bool m_inited;
        
        Screen(); /**< creates a dummy incomplete object; only use to override behaviour in sub-class */
        Screen(const char* filename);
        virtual ~Screen(){}
        bool operator ==(const char* filename) const { return m_filename == filename; }
        
        /** returns an object by name, or NULL if not found */
        Widget* getWidget(const char* name);
        
        /** returns an object by name, casted to specified type, or NULL if not found/wrong type */
        template <typename T> T* getWidget(const char* name)
        {
            return dynamic_cast<T*>( getWidget(name) );
        }
        
        static Widget* getWidget(const char* name, ptr_vector<Widget>* within_vector);
        static Widget* getWidget(const int id, ptr_vector<Widget>* within_vector);
        
        Widget* getFirstWidget(ptr_vector<Widget>* within_vector=NULL);
        Widget* getLastWidget(ptr_vector<Widget>* within_vector=NULL);
        
        virtual void addWidgets();
        virtual void calculateLayout();
        
        virtual ScreenType getScreenType() { return SCREEN_TYPE_MENU; }
        
        void manualAddWidget(Widget* w);
        void manualRemoveWidget(Widget* w);

        const std::string& getName() const { return m_filename; }
        
        void elementsWereDeleted(ptr_vector<Widget>* within_vector = NULL);
        
        
        virtual void init() = 0;
        virtual void tearDown() = 0;
        
        /**
         * will be called everytime sometimes happens.
         * Events are generally a widget state change. In this case, a pointer to the said widget is passed along its
         * name, so you get its new state and/or act. There are two special events, passed with a NULL widget, and which
         * bear the anmes "init" and "tearDown", called respectively when a screen is being made visible and when it's
         * being left, allowing for setup/clean-up.
         */
        virtual void eventCallback(Widget* widget, const std::string& name) = 0;
        
        virtual void onUpdate(float dt, irr::video::IVideoDriver*) { };
    };
    
}

#endif
