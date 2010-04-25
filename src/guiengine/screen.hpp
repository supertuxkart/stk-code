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
#include <typeinfo>

#include "irrlicht.h"

#include "guiengine/engine.hpp"
#include "guiengine/widget.hpp"
#include "input/input.hpp"
#include "utils/ptr_vector.hpp"

/**
 * \ingroup guiengine
 */
namespace GUIEngine
{
#define DEFINE_SCREEN_SINGLETON( ClassName )  template<> ClassName* GUIEngine::ScreenSingleton< ClassName >::singleton = NULL
    
    /**
     * \brief Declares a class to be a singleton.
     * Normally, all screens will be singletons.
     * Note that you need to use the 'DEFINE_SCREEN_SINGLETON' macro in a .cpp file to
     * actually define the instance (as this can't be done in a .h)
     * \ingroup guiengine
     */
    template<typename SCREEN>
    class ScreenSingleton
    {
        static SCREEN* singleton;
        
    public:
        
        ~ScreenSingleton()
        {
            singleton = NULL;
        }
        
        static SCREEN* getInstance()
        {
            if (singleton == NULL)
            {
                singleton = new SCREEN();
                GUIEngine::addScreenToList(singleton);
            }
            
            return singleton;
        }
        
    };
    
    /**
     * \ingroup guiengine
     * \brief Loads a GUI screen from its XML file.
     * Builds a hierarchy of Widget objects whose contents are a direct transcription of the XML file,
     * with little analysis or layout performed on them.
     */
    void parseScreenFileDiv(irr::io::IrrXMLReader* xml, ptr_vector<Widget>& append_to);
    
    /**
     * \brief Represents a single GUI screen.
     * Mainly responsible of its children widgets; Screen lays them
     * out, asks them to add themselves, asks them to remove themselves, etc.
     *
     * Also initiates the read of GUI files, even though most of that work is done in "screen_loader.cpp"
     *
     * \ingroup guiengine
     */
    class Screen
    {
        friend class Skin;
        
        bool m_loaded;
        std::string m_filename;
        void loadFromFile();
        
        static void addWidgetsRecursively(ptr_vector<Widget>& widgets, Widget* parent=NULL);
        void calculateLayout(ptr_vector<Widget>& widgets, Widget* parent=NULL);
        
        /** Will be called to determine if the 3D scene must be rendered when at this screen. */
        bool m_render_3d;
        
        
        unsigned int m_magic_number;
    public:
        bool throttleFPS;
        
        ptr_vector<Widget, HOLD> m_widgets;
        
        // current mouse position, read-only...
        int m_mouse_x, m_mouse_y;
        
        /** this variable is not used by the Screen object itself; it's the routines creating
         * screens that may use it to perform some operations only once. initialized to false.
         */
        bool m_inited;
        
        /** Next time this menu needs to be shown, don't use cached values, re-calculate everything.
         (useful e.g. on reschange, when sizes have changed and must be re-calculated) */
        virtual void forgetWhatWasLoaded();
        
        /** \brief creates a dummy incomplete object; only use to override behaviour in sub-class */
        Screen();
        
        /** 
          * \brief          creates a screen populated by the widgets described in a STK GUI file
          * \param filename name of the XML file describing the screen. this is NOT a path.
          *                 The passed file name will be searched for in the STK data/gui directory
          */
        Screen(const char* filename);
        
        virtual ~Screen();
        
        bool operator ==(const char* filename) const { return m_filename == filename; }
        
        /** returns an object by name, or NULL if not found */
        Widget* getWidget(const char* name);
        
        /** returns an object by name, casted to specified type, or NULL if not found/wrong type */
        template <typename T> T* getWidget(const char* name)
        {
            Widget* out = getWidget(name);
            T* outCasted = dynamic_cast<T*>( out );
            if (out != NULL && outCasted == NULL)
            {
                fprintf(stderr, "Screen::getWidget : Widget '%s' of type '%s' cannot be casted to "
                        "requested type '%s'!\n", name, typeid(*out).name(), typeid(T).name()); 
            }
            return outCasted;
        }
        
        static Widget* getWidget(const char* name, ptr_vector<Widget>* within_vector);
        static Widget* getWidget(const int id, ptr_vector<Widget>* within_vector);
        
        Widget* getFirstWidget(ptr_vector<Widget>* within_vector=NULL);
        Widget* getLastWidget(ptr_vector<Widget>* within_vector=NULL);
        
        /** \brief adds the irrLicht widgets corresponding to this screen to the IGUIEnvironment */
        void addWidgets();
        
        /** called after all widgets have been added. namely expands layouts into absolute positions */
        void calculateLayout();
        
        /** \brief can be used for custom purposes for which the load-screen-from-XML code won't make it */
        void manualAddWidget(Widget* w);
        
        /** \brief can be used for custom purposes for which the load-screen-from-XML code won't make it */
        void manualRemoveWidget(Widget* w);
        
        /** \return the name of this menu (which is the name of the file) */
        const std::string& getName() const { return m_filename; }
        
        void elementsWereDeleted(ptr_vector<Widget>* within_vector = NULL);
        
        /** Will be called to determine if the 3D scene must be rendered when at this screen */
        bool needs3D() { return m_render_3d; }
        
        /** \brief invoke this method for screens that use a 3D scene as background
          *
          * (if this method is not invoked with 'true' as parameter, the menu background will
          *  be rendered instead).
          *
          * \note to create the 3D background, use the facilities provided by the irrLicht scene
          *       manager, this class will not set up any 3D scene.
          */
        void setNeeds3D(bool needs3D) { m_render_3d = needs3D; }
        
        /** 
          * \brief callback invoked when entering this menu
          *
          * @note the same instance of your object may be entered/left more than once, so make sure that
          * one instance of your object can be used several times if the same screen is visited several
          * times.
          */
        virtual void init() = 0;
        
        /** 
          * \brief callback invoked before leaving this menu
          *
          * @note the same instance of your object may be entered/left more than once, so make sure that
          * one instance of your object can be used several times if the same screen is visited several
          * times.
          */
        virtual void tearDown() = 0;
        
        /** 
          * \brief  Called when escape is pressed.
          * \return true if the screen should be closed, false if you handled the press another way
          */
        virtual bool onEscapePressed() { return true; }
        
        /**
         * will be called everytime sometimes happens.
         * Events are generally a widget state change. In this case, a pointer to the said widget is passed along its
         * name, so you get its new state and/or act. There are two special events, passed with a NULL widget, and which
         * bear the anmes "init" and "tearDown", called respectively when a screen is being made visible and when it's
         * being left, allowing for setup/clean-up.
         */
        virtual void eventCallback(Widget* widget, const std::string& name, const int playerID) = 0;
        
        virtual void onUpdate(float dt, irr::video::IVideoDriver*) { };
    };
    
}

#endif
