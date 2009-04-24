
#include "sdl_manager.hpp"
#include <SDL/SDL.h>
#include "user_config.hpp"
#include "input/input_manager.hpp"
#include "gui/state_manager.hpp"
#include "history.hpp"
/*
#include "items/item_manager.hpp"
#include "items/powerup_manager.hpp"
#include "items/attachment_manager.hpp"
#include "items/projectile_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "material_manager.hpp"
#include "gui/font.hpp"
 */
#include "gui/race_gui.hpp"
#include "gui/engine.hpp"

#include <iostream>
#include <sstream>

namespace SDLManager
{
    long m_flags;
    
    void init()
    {
        if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_TIMER) < 0)
        {
            fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
            exit(1);
        }
        
        m_flags = SDL_OPENGL | SDL_HWSURFACE;
        
        //detect if previous resolution crashed STK
        if (user_config->m_crashed)
        {
            //STK crashed last time
            user_config->m_crashed = false;  //reset flag
            // set window mode as a precaution
            user_config->m_fullscreen = false;
            // blacklist the res if not already done
            std::ostringstream o;
            o << user_config->m_width << "x" << user_config->m_height;
            std::string res = o.str();
            if (std::find(user_config->m_blacklist_res.begin(),
                          user_config->m_blacklist_res.end(),res) == user_config->m_blacklist_res.end())
            {
                user_config->m_blacklist_res.push_back (o.str());
            }
            //use prev screen res settings if available
            if (user_config->m_width != user_config->m_prev_width
                || user_config->m_height != user_config->m_prev_height)
            {
                user_config->m_width = user_config->m_prev_width;
                user_config->m_height = user_config->m_prev_height;
            }
            else //set 'safe' resolution to return to
            {
                user_config->m_width = user_config->m_prev_width = 800;
                user_config->m_height = user_config->m_prev_height = 600;
            }
        }
        
        if(user_config->m_fullscreen)
            m_flags |= SDL_FULLSCREEN;
        
        // setVideoMode(false);
        
        SDL_JoystickEventState(SDL_ENABLE);
    }
    
    //-----------------------------------------------------------------------------
    /** Show cursor.
     */
    void showPointer()
    {
        SDL_ShowCursor(SDL_ENABLE);
    }   // showPointer
    
    //-----------------------------------------------------------------------------
    /** Hide cursor.
     */
    void hidePointer()
    {
        SDL_ShowCursor(SDL_DISABLE);
    }   // hidePointer
    
    //-----------------------------------------------------------------------------
    /** Toggles to fullscreen mode.
     */
    void toggleFullscreen(bool resetTextures)
    {
        user_config->m_fullscreen = !user_config->m_fullscreen;
        
        m_flags = SDL_OPENGL | SDL_HWSURFACE;
        
        if(user_config->m_fullscreen)
        {
            m_flags |= SDL_FULLSCREEN;
            
            if(StateManager::isGameState())
                showPointer();
            
            // Store settings in user config file in case new video mode
            // causes a crash
            user_config->m_crashed = true; //set flag. 
            user_config->saveConfig();
        }
        else if(StateManager::isGameState())
            hidePointer();
        
        // setVideoMode(resetTextures);
    }   // toggleFullscreen
    
    // -----------------------------------------------------------------------------
    /** Sets the video mode. If 8 bit colours are not supported, 5 bits are used;
     *  and if this doesn't work, alpha is disabled, too - before giving up. So
     *  STK should now work with 16 bit windows.
     *  \param resetTextures Forces all textures to be reloaded after a change of 
     *                       resolution. Necessary with windows and Macs OpenGL 
     *                       versions.
     */
    /*
    void setVideoMode(bool resetTextures)
    {
#if defined(WIN32) || defined(__APPLE__)
        if(resetTextures)
        {
            // FIXME: clear texture cache here
            // Windows needs to reload all textures, display lists, ... which means
            // that all models have to be reloaded. So first, free all textures,
            // models, then reload the textures from materials.dat, then reload
            // all models, textures etc.
            
            // startScreen             -> removeTextures();
            attachment_manager      -> removeTextures();
            projectile_manager      -> removeTextures();
            item_manager            -> removeTextures();
            kart_properties_manager -> removeTextures();
            powerup_manager         -> removeTextures();
            
            material_manager->reInit();
            
            
            powerup_manager         -> loadPowerups();
            kart_properties_manager -> loadKartData();
            item_manager            -> loadDefaultItems();
            projectile_manager      -> loadData();
            attachment_manager      -> loadModels();
            
            //        startScreen             -> installMaterial();
            
            //FIXME: the font reinit funcs should be inside the font class
            //Reinit fonts
            delete_fonts();
            init_fonts();
            
            //TODO: this function probably will get deleted in the future; if
            //so, the widget_manager.hpp include has no other reason to be here.
            //widget_manager->reloadFonts();
        }
#endif
    }   // setVideoMode
    */
    // SDL_FreeSurface(m_main_surface); ?
    // the previous sdl driver declared m_main_surface but it was always set to NULL afaik
    
}