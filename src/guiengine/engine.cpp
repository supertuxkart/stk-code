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


#include "guiengine/engine.hpp"

#include <iostream>
#include <assert.h>

#include "io/file_manager.hpp"
#include "input/input_manager.hpp"
#include "guiengine/CGUIFont.h"
#include "guiengine/event_handler.hpp"
#include "guiengine/modaldialog.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/skin.hpp"
#include "guiengine/widget.hpp"
#include "modes/world.hpp"

using namespace irr::gui;
using namespace irr::video;

namespace GUIEngine
{

    namespace Private
    {
        IGUIEnvironment* g_env;
        Skin* g_skin = NULL;
        IGUIFont* g_font;
        IGUIFont* g_title_font;

        IrrlichtDevice* g_device;
        IVideoDriver* g_driver;
        Screen* g_current_screen = NULL;
        AbstractStateManager* g_state_manager = NULL;
        Widget* g_focus_for_player[MAX_PLAYER_COUNT];
    }
    using namespace Private;
   
    ptr_vector<Widget, REF> needsUpdate;
    ptr_vector<Screen, REF> g_loaded_screens;

    float dt = 0;
    
    float getLatestDt()
    {
        return dt;
    }
    
    Widget* getFocusForPlayer(const int playerID)
    {
        assert(playerID >= 0);
        assert(playerID < MAX_PLAYER_COUNT);
        
        return g_focus_for_player[playerID];
    }
    void focusNothingForPlayer(const int playerID)
    {
        Widget* focus = getFocusForPlayer(playerID);
        if (focus != NULL) focus->unsetFocusForPlayer(playerID);
        
        g_focus_for_player[playerID] = NULL;
    }
    bool isFocusedForPlayer(const Widget* w, const int playerID)
    {
        assert(w != NULL);
        assert(playerID >= 0);
        assert(playerID < MAX_PLAYER_COUNT);
        
        // If no focus
        if (g_focus_for_player[playerID] == NULL) return false;
        
        // otherwise check if the focus is the given widget
        return g_focus_for_player[playerID]->isSameIrrlichtWidgetAs(w);
    }
       
    int getFontHeight()
    {
        // FIXME: this needs to be reset when changing resolution
        static int fh = g_font->getDimension( L"X" ).Height;
        return fh;
    }
// -----------------------------------------------------------------------------  
void clear()
{
    g_env->clear();
    if (g_current_screen != NULL) g_current_screen->elementsWereDeleted();
    g_current_screen = NULL;
}
// -----------------------------------------------------------------------------  
void cleanForGame()
{
    clear();
    needsUpdate.clearWithoutDeleting();
}
// -----------------------------------------------------------------------------  
void switchToScreen(const char* screen_name)
{    
    needsUpdate.clearWithoutDeleting();
    
    // clean what was left by the previous screen
    g_env->clear();
    if (g_current_screen != NULL) g_current_screen->elementsWereDeleted();
    g_current_screen = NULL;
    Widget::resetIDCounters();
    
    // check if we already loaded this screen
    const int screen_amount = g_loaded_screens.size();
    for(int n=0; n<screen_amount; n++)
    {
        if (g_loaded_screens[n].getName() == screen_name)
        {
            g_current_screen = g_loaded_screens.get(n);
            break;
        }
    }
    
    // screen not found in list of existing ones, so let's create it
    if (g_current_screen == NULL)
    {
        assert(false);
        return;
        //GUIEngine::Screen* new_screen = new GUIEngine::Screen(screen_name);
        //g_loaded_screens.push_back(new_screen);
        //g_current_screen = new_screen;
    }
    
    
    // show screen
    g_current_screen->addWidgets();
}
// -----------------------------------------------------------------------------
void addScreenToList(Screen* cutscene)
{
    g_loaded_screens.push_back(cutscene);
}
// -----------------------------------------------------------------------------
/** to be called after e.g. a resolution switch */
void reshowCurrentScreen()
{
    needsUpdate.clearWithoutDeleting();
    g_state_manager->reshowTopMostMenu();
    //g_current_screen->addWidgets();
}
// -----------------------------------------------------------------------------
void cleanUp()
{
    if (g_skin != NULL) delete g_skin;
    g_skin = NULL;
    for (int i=0; i<g_loaded_screens.size(); i++)
    {
        g_loaded_screens[i].forgetWhatWasLoaded();
    }
    
    g_current_screen = NULL;
    needsUpdate.clearWithoutDeleting();
    
    if (ModalDialog::isADialogActive()) ModalDialog::dismiss();

    delete g_font;
    g_font = NULL;
    delete g_title_font;
    g_title_font = NULL;
        
    // nothing else to delete for now AFAIK, irrlicht will automatically kill everything along the device
}
    
// -----------------------------------------------------------------------------
void init(IrrlichtDevice* device_a, IVideoDriver* driver_a, AbstractStateManager* state_manager )
{
    g_env = device_a->getGUIEnvironment();
    g_device = device_a;
    g_driver = driver_a;
    g_state_manager = state_manager;
    
    for (int n=0; n<MAX_PLAYER_COUNT; n++)
    {
        g_focus_for_player[n] = NULL;
    }
    
    /*
     To make the g_font a little bit nicer, we load an external g_font
     and set it as the new default g_font in the g_skin.
     To keep the standard g_font for tool tip text, we set it to
     the built-in g_font.
     */
    g_skin = new Skin(g_env->getSkin());
    g_env->setSkin(g_skin);
    //g_skin = g_env->getSkin();
    
    // font size is resolution-dependent.
    // normal text will range from 0.8, in 640x* resolutions (won't scale below that) to
    // 1.0, in 1024x* resolutions, and linearly up
    // normal text will range from 0.2, in 640x* resolutions (won't scale below that) to
    // 0.4, in 1024x* resolutions, and linearly up
    const int screen_width = irr_driver->getFrameSize().Width;
    const float normal_text_scale = 0.7f + 0.2f*std::max(0, screen_width - 640)/564.0f;
    const float title_text_scale = 0.2f + 0.2f*std::max(0, screen_width - 640)/564.0f;

    //ScalableFont* sfont = new ScalableFont(g_env, (file_manager->getGUIDir() + "/okolaks.xml").c_str());
    ScalableFont* sfont = new ScalableFont(g_env, file_manager->getFontFile("StkFont.xml").c_str() );
    sfont->setScale(normal_text_scale);
    sfont->setKerningHeight(-5);
    g_font = sfont;
    
    ScalableFont* sfont2 = new ScalableFont(g_env, file_manager->getFontFile("title_font.xml").c_str() );
    sfont2->m_fallback_font = sfont;
    sfont2->m_fallback_font_scale = 4.0f; // because the fallback font is much smaller than the title font
    sfont2->m_fallback_kerning_width = 15;
    sfont2->setScale(title_text_scale);
    sfont2->setKerningWidth(-18);
    sfont2->m_black_border = true;
    g_title_font = sfont2;
    

    if (g_font != NULL) g_skin->setFont(g_font);
    
    //g_skin->setFont(g_env->getBuiltInFont(), EGDF_TOOLTIP);
    
    // set event receiver
    g_device->setEventReceiver(EventHandler::get());
}
// -----------------------------------------------------------------------------
/** transmit event to user event callback (out of encapsulated GUI module) */
void transmitEvent(Widget* widget, std::string& name, const int playerID)
{
    assert(g_state_manager != NULL);
    getCurrentScreen()->eventCallback(widget, name, playerID);
}
    
// -----------------------------------------------------------------------------    
void render(float elapsed_time)
{    
    GUIEngine::dt = elapsed_time;
    
     // ---- menu drawing
    
    // draw background image and sections
    
    const GameState gamestate = g_state_manager->getGameState();
    
    if (gamestate == MENU && !GUIEngine::getCurrentScreen()->needs3D())
    {
        g_skin->drawBgImage();
    }
    else if (gamestate == INGAME_MENU)
    {
        g_skin->drawBGFadeColor();
    }
    
#if (IRRLICHT_VERSION_MAJOR == 1) && (IRRLICHT_VERSION_MINOR >= 7)
    g_driver->enableMaterial2D();
#endif
    
    if (gamestate == MENU || gamestate == INGAME_MENU)
    {
        g_skin->renderSections();
    }
    
    // let irrLicht do the rest (the Skin object will be called for further render)
    g_env->drawAll();
    
    // ---- some menus may need updating
    if (gamestate != GAME)
    {
        if (ModalDialog::isADialogActive()) ModalDialog::getCurrent()->onUpdate(dt);
        else                                getCurrentScreen()->onUpdate(elapsed_time, g_driver);
    }
    else
    {
        RaceManager::getWorld()->getRaceGUI()->renderGlobal(elapsed_time);
    }
    
#if (IRRLICHT_VERSION_MAJOR == 1) && (IRRLICHT_VERSION_MINOR >= 7)
    g_driver->enableMaterial2D(false);
#endif
}   // render

// -----------------------------------------------------------------------------    
Widget* getWidget(const char* name)
{
    // if a modal dialog is shown, search within it too
    if (ModalDialog::isADialogActive())
    {
        Widget* widgetWithinDialog = Screen::getWidget(name, &(ModalDialog::getCurrent()->m_children));
        if (widgetWithinDialog != NULL) return widgetWithinDialog;
    }
    
    Screen* screen = getCurrentScreen();
    
    if (screen == NULL) return NULL;
    
    return Screen::getWidget(name,  &screen->m_widgets);
}
// -----------------------------------------------------------------------------    
Widget* getWidget(const int id)
{
    // if a modal dialog is shown, search within it too
    if (ModalDialog::isADialogActive())
    {        
        Widget* widgetWithinDialog = Screen::getWidget(id, &(ModalDialog::getCurrent()->m_children));
        if (widgetWithinDialog != NULL) return widgetWithinDialog;
    }
    
    Screen* screen = getCurrentScreen();
    
    if (screen == NULL) return NULL;
    
    return Screen::getWidget(id,  &screen->m_widgets);
}

        
}
