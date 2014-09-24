//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2013 Marianne Gagnon
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
#include "utils/cpp2011.hpp"

#include <irrString.h>
#include <IXMLReader.h>

namespace irr
{
    namespace gui { class IGUIElement; }
}
using namespace irr;

#include "config/stk_config.hpp"
#include "guiengine/abstract_top_level_container.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/event_handler.hpp"
#include "guiengine/widget.hpp"
#include "input/input.hpp"
#include "utils/ptr_vector.hpp"

#include "utils/leak_check.hpp"

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
    protected:
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
     * Also initiates the read of GUI files, even though most of that work is
     * done in "screen_loader.cpp"
     *
     * \ingroup guiengine
     */
    class Screen : public AbstractTopLevelContainer
    {
    private:
        /** True if the race (if it is running) should be paused when this
         *  screen is shown. The RaceResultGUI uses this to leave the race
         *  running while it is being shown. */
        bool m_pause_race;

        friend class Skin;

        bool m_loaded;

        std::string m_filename;

        /** Will be called to determine if the 3D scene must be rendered when
         *  at this screen.
         */
        bool m_render_3d;

        /** to catch errors as early as possible, for debugging purposes only */
        unsigned int m_magic_number;

    protected:
        bool m_throttle_FPS;

    public:

        LEAK_CHECK()

        /**
         * \ingroup guiengine
         * \brief Loads a GUI screen from its XML file.
         *
         * Builds a hierarchy of Widget objects whose contents are a direct
         * transcription of the XML file, with little analysis or layout
         * performed on them.
         */
        static void parseScreenFileDiv(irr::io::IXMLReader* xml,
                                       PtrVector<Widget>& append_to,
                                       irr::gui::IGUIElement* parent = NULL);


        Screen(bool pause_race=true);

        Screen(const char* filename, bool pause_race=true);

        virtual ~Screen();

        bool operator ==(const char* filename) const { return m_filename == filename; }

        void loadFromFile();

        /** \return whether this screen is currently loaded */
        bool isLoaded() const { return m_loaded; }

        bool throttleFPS() const { return m_throttle_FPS; }

        void addWidgets();

        void calculateLayout();

        void manualAddWidget(Widget* w);

        void manualRemoveWidget(Widget* w);

        /** \return the name of this menu (which is the name of the file) */
        const std::string& getName() const { return m_filename; }

        virtual void unload();

        /** Will be called to determine if the 3D scene must be rendered when
         *  at this screen
         */
        bool needs3D() { return m_render_3d; }

        /** \brief Invoke this method for screens that use a 3D scene as
         *         background.
         *
         *  (if this method is not invoked with 'true' as parameter, the menu
         *  background will be rendered instead).
         *
         *  \note To create the 3D background, use the facilities provided by
         *        the irrLicht scene manager, this class will not set up any
         *        3D scene.
         */
        void setNeeds3D(bool needs3D) { m_render_3d = needs3D; }

        /**
         * \brief Callback invoked when loading this menu.
         *
         * \pre   Children widgets of this menu have been created by the time
         *        this callback is invoked.
         * \note  This method is not called everytime the screen is shown.
         *        Screen::init is.
         *        Use this method for persistent setup code (namely, that
         *        deals with setting up children widget objects and needs not
         *        be done everytime we visit the screen).
         * \note  A Screen object instance may be unloaded then loaded back.
         *        This method might thus be called more than once in the
         *        lifetime of a Screen object, however there will always
         *        be an 'unload' event in-between calls to this method.
         */
        virtual void loadedFromFile() = 0;

        /**
          * \brief Callback invoked when this screen is being unloaded.
          *        Override this method in children classes if you need to be
          *        notified of this.
          * \note  A Screen object instance may be unloaded then loaded back
          *        at will.
          * \note  An unloaded Screen object does not have its children widgets
          *        anymore, it only retains its members (most importantly the
          *        path to its GUI file) so that it can be loaded back later.
          */
        virtual void unloaded() {}

        /**
          * \brief Optional callback invoked very early, before widgets have
          *        been added (contrast with init(), which is invoked afer
          *        widgets were added)
          */
        virtual void beforeAddingWidget() {}

        /**
          * \brief Callback invoked when entering this menu (after the
          *        widgets have been added).
          *
          * \note  The same instance of your object may be entered/left more
          *        than once, so make sure that one instance of your object
          *        can be used several times if the same screen is visited
          *        several times.
          */
        virtual void init();

        /** Displays this screen bu pushing it onto the stack of screen 
         *  in the state manager. */
        void push();

        /**
          * \brief Callback invoked before leaving this menu.
          *
          * \note  The same instance of your object may be entered/left more
          *        than once, so make sure that one instance of your object can
          *        be used several times if the same screen is visited several
          *        times.
          */
        virtual void tearDown();

        /**
          * \brief  Called when escape is pressed.
          * \return true if the screen should be closed,
          *         false if you handled the press another way
          */
        virtual bool onEscapePressed() { return true; }

        /**
         * \brief will be called everytime something happens.
         *
         * Events are generally a widget state change. In this case, a pointer
         * to the said widget is passed along its name, so you get its new
         * state and/or act. There are two special events, passed with a NULL
         * widget, and which bear the names "init" and "tearDown", called
         * respectively when a screen is being made visible and when it's being
         * left, allowing for setup/clean-up.
         */
        virtual void eventCallback(Widget* widget, const std::string& name, const int playerID) = 0;

        /**
         * \brief optional callback you can override to be notified at every frame.
         */
        virtual void onUpdate(float dt) { };

        /**
         * \return which music to play at this screen
         */
        virtual MusicInformation* getMusic() const { return stk_config->m_title_music; }

        /**
         * \return which music to play at this screen, if accessed in "in-game-menu" mode
         */
        virtual MusicInformation* getInGameMenuMusic() const { return NULL; }

        virtual int getWidth();

        virtual int getHeight();

        /**
         * \brief Override this if you need to be notified of player actions
         *        in subclasses.
         */
        virtual EventPropagation filterActions(PlayerAction action,
                                               int deviceID,
                                               const unsigned int value,
                                               Input::InputType type,
                                               int playerId)
            { return EVENT_LET; }

        /** Callback you can use if you want to know when the user pressed
         *  on a disabled ribbon item.
         *  (the main use I see for this is to give feedback)
         */
        virtual void onDisabledItemClicked(const std::string& item) {}

        /**
         * \brief Override this if you need to be notified of raw input in
         *        subclasses.
         */
        virtual void filterInput(Input::InputType type,
                                 int deviceID,
                                 int btnID,
                                 int axisDir,
                                 int value) {}

        /** Callback that gets called when a dialog is closed.
         *  Can be used to set focus for instance.
         */
        virtual void onDialogClose() {}
    };

    class CutsceneScreen : public Screen
    {
    public:
        CutsceneScreen(const char* name) : Screen(name, false)
        {
            setNeeds3D(true);
            m_throttle_FPS = false;
        }

        virtual void onCutsceneEnd() = 0;
    };
}

#endif
