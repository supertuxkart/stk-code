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

#include "config/stk_config.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/widget.hpp"
#include "input/input.hpp"
#include "utils/ptr_vector.hpp"

/**
 * \ingroup guiengine
 */
namespace GUIEngine
{
#define DEFINE_SCREEN_SINGLETON( ClassName )  \
    template<> ClassName* GUIEngine::ScreenSingleton< ClassName >::singleton = NULL
    
    /**
     * \brief Declares a class to be a singleton.
     * Normally, all screens will be singletons.
     * Note that you need to use the 'DEFINE_SCREEN_SINGLETON' macro in a .
     * cpp file to actually define the instance (as this can't be done in a .h)
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
    private:
        /** True if the race (if it is running) should be paused when this 
         *  screen is shown. The RaceResultGUI uses this to leave the race
         *  running while it is being shown. */
        bool m_pause_race;

    private:
        friend class Skin;
        
        bool m_loaded;
        std::string m_filename;
        
        static void addWidgetsRecursively(ptr_vector<Widget>& widgets, Widget* parent=NULL);
        
        /** Will be called to determine if the 3D scene must be rendered when at this screen. */
        bool m_render_3d;
        
        /** to catch errors as early as possible, for debugging purposes only */
        unsigned int m_magic_number;
        
        /**
         * \ingroup guiengine
         * \brief Loads a GUI screen from its XML file.
         * Builds a hierarchy of Widget objects whose contents are a direct transcription of the XML file,
         * with little analysis or layout performed on them.
         */
        static void parseScreenFileDiv(irr::io::IrrXMLReader* xml, ptr_vector<Widget>& append_to);
        
    protected:
        bool m_throttle_FPS;

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

        /** the widgets in this screen */
        ptr_vector<Widget, HOLD> m_widgets;
        
    public:
        
        
        /** \brief creates a dummy incomplete object; only use to override behaviour in sub-class */
        Screen(bool pause_race=true);
        
        /** 
          * \brief          creates a screen populated by the widgets described in a STK GUI file
          * \param filename name of the XML file describing the screen. this is NOT a path.
          *                 The passed file name will be searched for in the STK data/gui directory
          */
        Screen(const char* filename, bool pause_race=true);
        
        virtual ~Screen();
        
        bool operator ==(const char* filename) const { return m_filename == filename; }
        
        /** \brief loads this Screen from the file passed to the constructor */
        void loadFromFile();
        
        /** \return whether this screen is currently loaded */
        bool isLoaded() const { return m_loaded; }
        
        /** returns an object by name, or NULL if not found */
        Widget* getWidget(const char* name);
        
        /** returns an object by irrlicht ID, or NULL if not found */
        Widget* getWidget(const int id);

        bool throttleFPS() const { return m_throttle_FPS; }
        
        /** returns an object by name, casted to specified type, or NULL if not found/wrong type */
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
        
        /** \brief adds the irrLicht widgets corresponding to this screen to the IGUIEnvironment */
        void addWidgets();
        
        /** \brief called after all widgets have been added. namely expands layouts into absolute positions */
        void calculateLayout();
        
        /** \brief can be used for custom purposes for which the load-screen-from-XML code won't make it */
        void manualAddWidget(Widget* w);
        
        /** \brief can be used for custom purposes for which the load-screen-from-XML code won't make it */
        void manualRemoveWidget(Widget* w);
        
        /** \return the name of this menu (which is the name of the file) */
        const std::string& getName() const { return m_filename; }
        
        /** 
          * \brief invoked when irrlicht widgets added to the screen have been deleted
          * so that we can drop any pointer to them we had (they are now dangling pointers)
          */
        void elementsWereDeleted(ptr_vector<Widget>* within_vector = NULL);
        
        /** Next time this menu needs to be shown, don't use cached values, re-calculate everything.
         (useful e.g. on reschange, when sizes have changed and must be re-calculated) */
        virtual void unload();
        
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
          * \brief callback invoked when loading this menu
          *
          * \precondition Children widgets of this menu have been created by the time this callback
          *               is invoked.
          * \note this method is not called everytime the screen is shown. Screen::init is.
          *       use this method for persistent setup code (namely, that deals with settupping
          *       children widget objects and needs not be done everytime we visit the screen).
          * \note a Screen object instance may be unloaded then loaded back. This method might thus
          *       be called more than once in the lifetime of a Screen object, however there will always
          *       be an 'unload' event in-between calls to this method.
          */
        virtual void loadedFromFile() = 0;
        
        /**
          * \brief callback invoked when this screen is being unloaded
          * Override this method in children classes if you need to be notified of this.
          * \note a Screen object instance may be unloaded then loaded back at will.
          * \note an unloaded Screen object does not have its children widgets anymore, it only
          *       retains its members (most importantly the path to its GUI file) so that it can be
          *       loaded back later.
          */
        virtual void unloaded() {}
        
        /**
          * \brief Optional callback invoked very early, before widgets have been added (contrast with
          *        init(), which is invoked afer widgets were added)
          */
        virtual void beforeAddingWidget() {}
        
        /** 
          * \brief callback invoked when entering this menu (after the widgets have been added)
          *
          * \note the same instance of your object may be entered/left more than once, so make sure that
          * one instance of your object can be used several times if the same screen is visited several
          * times.
          */
        virtual void init();
        
        /** 
          * \brief callback invoked before leaving this menu
          *
          * @note the same instance of your object may be entered/left more than once, so make sure that
          * one instance of your object can be used several times if the same screen is visited several
          * times.
          */
        virtual void tearDown();
        
        /** 
          * \brief  Called when escape is pressed.
          * \return true if the screen should be closed, false if you handled the press another way
          */
        virtual bool onEscapePressed() { return true; }
        
        /**
         * \brief will be called everytime sometimes happens.
         * Events are generally a widget state change. In this case, a pointer to the said widget is passed along its
         * name, so you get its new state and/or act. There are two special events, passed with a NULL widget, and which
         * bear the anmes "init" and "tearDown", called respectively when a screen is being made visible and when it's
         * being left, allowing for setup/clean-up.
         */
        virtual void eventCallback(Widget* widget, const std::string& name, const int playerID) = 0;
        
        /**
          * \brief optional callback you can override to be notified at every frame.
          */
        virtual void onUpdate(float dt, irr::video::IVideoDriver*) { };
        
        virtual MusicInformation* getMusic() const { return stk_config->m_title_music; }
    };
    
}

#endif
