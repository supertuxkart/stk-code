// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
//  Modelled after Supertux's configfile.cpp
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
#include <stdio.h>

#include <stdexcept>
#include <sstream>
#include <string>

// for mkdir:
#if !defined(WIN32) || defined(__CYGWIN__)
#  include <sys/stat.h>
#  include <sys/types.h>
#else
#  include <direct.h>
#endif

#include <SDL/SDL.h>
#include <plib/ul.h>

#include "actionmap.hpp"
#include "user_config.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "lisp/writer.hpp"
#include "translation.hpp"
#include "unlock_manager.hpp"
#include "race_manager.hpp"
#include "file_manager.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

using namespace std;
using namespace lisp;

UserConfig *user_config;

UserConfig::UserConfig()
{
    setDefaults();
    loadConfig();
}   // UserConfig

// -----------------------------------------------------------------------------
UserConfig::UserConfig(const std::string& filename)
{
    setDefaults();
    loadConfig(filename);
}   // UserConfig


// -----------------------------------------------------------------------------
UserConfig::~UserConfig()
{
}

// -----------------------------------------------------------------------------
/**
 * Set the config filename for each platform
 */
void UserConfig::setFilename()
{
#ifdef WIN32
    m_filename = file_manager->getLogFile("supertuxkart.cfg");
#else
    m_filename = file_manager->getLogFile("config");
#endif
}   // setFilename

// -----------------------------------------------------------------------------
/**
 * Load default values for options
 */
void UserConfig::setDefaults()
{
    setFilename();
    m_warning          = "";
    m_keyboard_debug   = false;
    m_track_debug      = 0;
    m_bullet_debug     = false;
    m_fullscreen       = false;
    m_no_start_screen  = false;
    m_sfx              = UC_ENABLE;
    m_music            = UC_ENABLE;
    m_smoke            = false;
    m_display_fps      = false;
    m_herring_style    = "new";
    m_background_music = "";
    m_profile          = 0;
    m_print_kart_sizes = false;
    m_skidding         = false;
    m_max_fps          = 120;
    m_sfx_volume       = 1.0f;
    m_use_kph          = false;
    m_replay_history   = false;
    m_width            = 800;
    m_height           = 600;
    m_prev_width           = m_width;
    m_prev_height          = m_height;
    m_prev_windowed        = false;
    m_crashed              = false;
    m_blacklist_res.clear();
    m_karts            = 4;
    m_log_errors       = false;
    m_kart_group       = "standard";
    m_track_group      = "standard";

    if(getenv("USERNAME")!=NULL)        // for windows
        m_username=getenv("USERNAME");
    else if(getenv("USER")!=NULL)       // Linux, Macs
        m_username=getenv("USER");
    else if(getenv("LOGNAME")!=NULL)    // Linux, Macs
        m_username=getenv("LOGNAME");
    else m_username="nouser";

    // Set the name as the default name for all players.
    for(int i=0; i<4; i++) 
    {
        m_player[i].setName(m_username);
                m_player[i].setLastKartId(0);
    }
        
        // Clear every entry.
        memset(inputMap, 0, sizeof(inputMap));

        /* general game input settings */
        set(GA_ENTER,
                Input(IT_KEYBOARD, SDLK_RETURN),
                Input(IT_KEYBOARD, SDLK_SPACE),
                Input(IT_STICKBUTTON, 0, 0),
                Input(IT_MOUSEBUTTON, 1));
        set(GA_LEAVE,
                Input(IT_KEYBOARD, SDLK_ESCAPE),
                Input(IT_STICKBUTTON, 0, 1),
                Input(IT_MOUSEBUTTON, 2),
                Input(IT_MOUSEBUTTON, 3));
        set(GA_CURSOR_UP,
                Input(IT_KEYBOARD, SDLK_UP),
                Input(IT_MOUSEBUTTON, 4),
                Input(IT_STICKMOTION, 0, 1, AD_NEGATIVE));
        
        set(GA_CURSOR_DOWN,
                Input(IT_KEYBOARD, SDLK_DOWN),
                Input(IT_MOUSEBUTTON, 5),
                Input(IT_STICKMOTION, 0, 1, AD_POSITIVE));

        set(GA_CURSOR_LEFT,
                Input(IT_KEYBOARD, SDLK_LEFT),
                Input(IT_STICKMOTION, 0, 0, AD_NEGATIVE));

        set(GA_CURSOR_RIGHT,
                Input(IT_KEYBOARD, SDLK_RIGHT),
                Input(IT_STICKMOTION, 0, 0, AD_POSITIVE));

        set(GA_CLEAR_MAPPING,
                Input(IT_KEYBOARD, SDLK_BACKSPACE),
                Input(IT_STICKBUTTON, 0, 2));

        set(GA_INC_SCROLL_SPEED,
                Input(IT_KEYBOARD, SDLK_PLUS));
        set(GA_INC_SCROLL_SPEED_FAST,
                Input(IT_KEYBOARD, SDLK_PAGEDOWN));

        set(GA_DEC_SCROLL_SPEED,
                Input(IT_KEYBOARD, SDLK_MINUS));
        set(GA_DEC_SCROLL_SPEED_FAST,
                Input(IT_KEYBOARD, SDLK_PAGEUP));

        set(GA_TOGGLE_FULLSCREEN,
                Input(IT_KEYBOARD, SDLK_F9));
        set(GA_LEAVE_RACE,
                Input(IT_KEYBOARD, SDLK_ESCAPE));
#ifdef DEBUG
        set(GA_DEBUG_ADD_BOWLING,
                Input(IT_KEYBOARD, SDLK_F1));
        set(GA_DEBUG_ADD_MISSILE,
                Input(IT_KEYBOARD, SDLK_F2));
        set(GA_DEBUG_ADD_HOMING,
                Input(IT_KEYBOARD, SDLK_F3));
#endif
        set(GA_DEBUG_TOGGLE_FPS,
                Input(IT_KEYBOARD, SDLK_F12));
        set(GA_DEBUG_TOGGLE_WIREFRAME,
                Input(IT_KEYBOARD, SDLK_F11));
        set(GA_DEBUG_HISTORY,
                Input(IT_KEYBOARD, SDLK_F10));
                
        // TODO: The following should become a static
        // array. This allows:
        // a) resetting to default values
        // b) prevent loading those defaults if config file contains any bindings

    /* Player 1 default input settings */
    set(GA_P1_LEFT,
                Input(IT_KEYBOARD, SDLK_LEFT));
    set(GA_P1_RIGHT,
                Input(IT_KEYBOARD, SDLK_RIGHT));
    set(GA_P1_ACCEL,
                Input(IT_KEYBOARD, SDLK_UP));
    set(GA_P1_BRAKE,
                Input(IT_KEYBOARD, SDLK_DOWN));
    set(GA_P1_WHEELIE,
                Input(IT_KEYBOARD, SDLK_RSHIFT));
    set(GA_P1_JUMP,
                Input(IT_KEYBOARD, SDLK_MINUS));
    set(GA_P1_RESCUE,
                Input(IT_KEYBOARD, SDLK_BACKSPACE));
    set(GA_P1_FIRE,
                Input(IT_KEYBOARD, SDLK_RCTRL));
    set(GA_P1_LOOK_BACK,
                Input(IT_KEYBOARD, SDLK_RALT));

    /* Player 2 default input settings */
    set(GA_P2_LEFT,
                Input(IT_KEYBOARD, SDLK_a));
    set(GA_P2_RIGHT,
                Input(IT_KEYBOARD, SDLK_d));
    set(GA_P2_ACCEL,
                Input(IT_KEYBOARD, SDLK_w));
    set(GA_P2_BRAKE,
                Input(IT_KEYBOARD, SDLK_s));
    set(GA_P2_WHEELIE,
                Input(IT_KEYBOARD, SDLK_LSHIFT));
    set(GA_P2_JUMP,
                Input(IT_KEYBOARD, SDLK_CAPSLOCK));
    set(GA_P2_RESCUE,
                Input(IT_KEYBOARD, SDLK_q));
    set(GA_P2_FIRE,
                Input(IT_KEYBOARD, SDLK_LCTRL));
    set(GA_P2_LOOK_BACK,
                Input(IT_KEYBOARD, SDLK_LALT));

    /* Player 3 default input settings */
    set(GA_P3_LEFT,
                Input(IT_KEYBOARD, SDLK_f));
    set(GA_P3_RIGHT,
                Input(IT_KEYBOARD, SDLK_h));
    set(GA_P3_ACCEL,
                Input(IT_KEYBOARD, SDLK_t));
    set(GA_P3_BRAKE,
                Input(IT_KEYBOARD, SDLK_g));
    set(GA_P3_WHEELIE,
                Input(IT_KEYBOARD, SDLK_c));
    set(GA_P3_JUMP,
                Input(IT_KEYBOARD, SDLK_v));
    set(GA_P3_RESCUE,
                Input(IT_KEYBOARD, SDLK_r));
    set(GA_P3_FIRE,
                Input(IT_KEYBOARD, SDLK_b));
    set(GA_P3_LOOK_BACK,
                Input(IT_KEYBOARD, SDLK_n));

    /* Player 4 default input settings  */
    set(GA_P4_LEFT,
                Input(IT_KEYBOARD, SDLK_j));
    set(GA_P4_RIGHT,
                Input(IT_KEYBOARD, SDLK_l));
    set(GA_P4_ACCEL,
                Input(IT_KEYBOARD, SDLK_i));
    set(GA_P4_BRAKE,
                Input(IT_KEYBOARD, SDLK_k));
    set(GA_P4_WHEELIE,
                Input(IT_KEYBOARD, SDLK_m));
    set(GA_P4_JUMP,
                Input(IT_KEYBOARD, SDLK_COMMA));
    set(GA_P4_RESCUE,
                Input(IT_KEYBOARD, SDLK_u));
    set(GA_P4_FIRE,
                Input(IT_KEYBOARD, SDLK_PERIOD));
    set(GA_P4_LOOK_BACK,
                Input(IT_KEYBOARD, SDLK_SLASH));

}   // setDefaults


// -----------------------------------------------------------------------------
/**
 * load default configuration file for this platform
 */
void UserConfig::loadConfig()
{
    loadConfig(m_filename);
}   // loadConfig

// -----------------------------------------------------------------------------
/**
 * Checks for existance of the tuxkart configuration directory. If the
 * directory does not exist, it will be created. Return values:
 * 1: config dir exists
 * 2: does not exist, but was created
 * 0: does not exist, and could not be created.
 */
int UserConfig::CheckAndCreateDir()
{
    const std::string DIRNAME = file_manager->getHomeDir();
    ulDir*            u       = ulOpenDir(DIRNAME.c_str());
    if(u)
    {  // OK, directory exists
        ulCloseDir(u);
        return 1;
    }
    // The directory does not exist, try to create it
    int bError;
#if defined(WIN32) && !defined(__CYGWIN__)
    bError = _mkdir(DIRNAME.c_str()      ) != 0;
#else
    bError = mkdir(DIRNAME.c_str(), 0755) != 0;
#endif
    if(bError)
    {
        fprintf(stderr, "Couldn't create '%s', config files will not be saved.\n",
                DIRNAME.c_str());
        return 0;
    }
    else
    {
        printf("Config directory '%s' successfully created.\n",DIRNAME.c_str());
        return 2;
    }

}   // CheckAndCreateDir

// -----------------------------------------------------------------------------
/** Load configuration values from file. */
void UserConfig::loadConfig(const std::string& filename)
{
    std::string temp;
    const lisp::Lisp* root = 0;
    int i = 0;
    int dirExist = CheckAndCreateDir();
    // Check if the config directory exists. If not, exit without an error
    // message, an appropriate message was printed by CheckAndCreateDir
    if (dirExist != 1) return;

    try
    {
        lisp::Parser parser;
        root = parser.parse(filename);
    }
    catch(std::exception& e)
    {
        (void)e;  // avoid warning about unreferenced local variable
        printf("Config file '%s' does not exist, it will be created.\n", 
               filename.c_str());
        delete root;
        return;
    }

    try
    {
        const lisp::Lisp* lisp = root->getLisp("tuxkart-config");
        if(!lisp) 
        {
            char msg[MAX_ERROR_MESSAGE_LENGTH];
            snprintf(msg, sizeof(msg), "No tuxkart-config node");
            throw std::runtime_error(msg);
        }
        int configFileVersion = 0;
        lisp->get("configFileVersion", configFileVersion);
        if (configFileVersion < CURRENT_CONFIG_VERSION)
        {
            // Give some feedback to the user about what was changed.
            // Do NOT add a break after the case, so that all changes will be printed
            printf("\nConfig file version '%d' is too old.\n"
                     "The following changes have been applied in the current SuperTuxKart version:\n",
                   configFileVersion);
            int needToAbort=0;
            switch(configFileVersion)
            {
            case 0:  printf("- Single window menu, old status display,new keyboard style settings were removed\n");
                     needToAbort=std::max(needToAbort,0);
            case 1:  printf("- Key bindings were changed, please check the settings. All existing values were discarded.\n");
                     needToAbort=std::max(needToAbort,1);// old keybinds wouldn't make any sense
            case 2:  printf("Added username, using: '%s'.\n", m_username.c_str());
                     needToAbort=std::max(needToAbort,0);
            case 3:  printf("Added username for all players.\n");
                     needToAbort=std::max(needToAbort,0);
            case 4:  printf("Added jumping, which invalidates all key bindings.\n");
                     needToAbort=std::max(needToAbort,0);
            case 99: break;
            default: printf("Config file version '%d' is too old. Discarding your configuration. Sorry. :(\n", configFileVersion);
                     needToAbort=1;
                     break;
            }
            if(needToAbort)
            {
                printf("The old config file is deleted, a new one will be created.\n");
                delete root;
                return;
            }
            printf("This warning can be ignored, the config file will be automatically updated.\n");
            // Keep on reading the config files as far as possible
        }   // if configFileVersion<SUPPORTED_CONFIG_VERSION

        /*get toggles*/
        lisp->get("fullscreen",       m_fullscreen);
        bool doSFX=false;                                // avoid warning
        lisp->get("sfx" ,             doSFX);
        m_sfx = doSFX ? UC_ENABLE : UC_DISABLE;
        lisp->get("nostartscreen",    m_no_start_screen);
        bool doMusic=false;                              // avoid warning
        lisp->get("music",            doMusic);
        m_music = doMusic ? UC_ENABLE : UC_DISABLE;
        lisp->get("smoke",            m_smoke);
        lisp->get("displayFPS",       m_display_fps);
        lisp->get("herringStyle",     m_herring_style);
        lisp->get("background-music", m_background_music);
        lisp->get("max-fps",          m_max_fps);
        lisp->get("sfx-volume",       m_sfx_volume);
        lisp->get("useKPH",           m_use_kph);

        /*get resolution width/height*/
        lisp->get("width",            m_width);
        lisp->get("height",           m_height);
        lisp->get("prev_width",           m_prev_width);
        lisp->get("prev_height",          m_prev_height);
        lisp->get("prev_windowed",        m_prev_windowed);
        //detect if resolution change previously crashed STK
        lisp->get("crash_detected",       m_crashed);
        // blacklisted resolutions
        lisp->getVector("blacklisted_resolutions", 
                                      m_blacklist_res);
        /*get number of karts*/
        lisp->get("karts",            m_karts);

        //get whether to log errors to file
        lisp->get("log-errors",       m_log_errors);
                lisp->get("kart-group",       m_kart_group);
        lisp->get("track-group",      m_track_group);
                // Handle loading the stick config in it own method.
                readStickConfigs(lisp);

        // Unlock information:
        const lisp::Lisp* unlock_info = lisp->getLisp("unlock-info");
        if(unlock_info) unlock_manager->load(unlock_info);

        /*get player configurations*/
        for(i=0; i<PLAYERS; ++i)
        {
            temp = "player-";
            temp += i+'1';

            const lisp::Lisp* reader = lisp->getLisp(temp);
            if(!reader)
            {
                char msg[MAX_ERROR_MESSAGE_LENGTH];
                snprintf(msg, sizeof(msg), "No '%s' node", temp.c_str());
                throw std::runtime_error(msg);
            }
            std::string name;
            reader->get("name", name);
            if (configFileVersion <= 3) 
            {
                // For older config files, replace the default player 
                // names "Player %d" with the user name
                char sDefaultName[10];
                snprintf(sDefaultName, sizeof(sDefaultName),
                         "Player %d",i+1);
                // If the config file does not contain a name or the old
                // default name, set the default username as player name.
                if(name.size()==0 || name==sDefaultName) name=m_username;
            }
            m_player[i].setName(name);

            int lastKartId = 0;
            reader->get("lastKartId", lastKartId);
            m_player[i].setLastKartId(lastKartId);

            // Don't read the key bindings from a config file earlier than 
            // version 5. These config files contain (unused) keybindings for 
            // jumping, so it is possible that the same key is used for 
            // jumping for player 1 and something else for another player.
            // In this case jumping for player one would be disabled (see
            // unsetDuplicates). To be on the safe side, old key bindings 
            // are just discarded.
            if (configFileVersion <= 4) 
            {
                m_warning=_("Old config file found, check your key bindings!");
            }  // configFileVersion <= 4

            // Retrieves a player's INPUT configuration
            readPlayerInput(reader, "left",     KA_LEFT,      i);
            readPlayerInput(reader, "right",    KA_RIGHT,     i);
            readPlayerInput(reader, "accel",    KA_ACCEL,     i);
            readPlayerInput(reader, "brake",    KA_BRAKE,     i);
            readPlayerInput(reader, "wheelie",  KA_WHEELIE,   i);
            readPlayerInput(reader, "jump",     KA_JUMP,      i);
            readPlayerInput(reader, "rescue",   KA_RESCUE,    i);
            readPlayerInput(reader, "fire",     KA_FIRE,      i);
            readPlayerInput(reader, "lookBack", KA_LOOK_BACK, i);
        }   // for i < PLAYERS
    }
    catch(std::exception& e)
    {
        fprintf(stderr, "Error while parsing config '%s':\n", filename.c_str());
        fprintf(stderr,  e.what());
        fprintf(stderr, "\n");
    }


    delete root;
}   // loadConfig

// -----------------------------------------------------------------------------

void
UserConfig::readStickConfigs(const lisp::Lisp *r)
{
        string temp;
        int count = 0;
        
        const lisp::Lisp *scsreader = r->getLisp("stick-configs");
        if (scsreader)
        {
                scsreader->get("count", count);
                        
                for (int i = 0; i < count; i++)
                {
                        temp = "stick-";
                        temp += (i + '1');
                        const lisp::Lisp *screader = scsreader->getLisp(temp);
                        if (screader)
                        {
                                string *id = new string();
                                screader->get("id", *id);
                                
                                StickConfig *sc = new StickConfig(*id);
                                
                                screader->get("preferredIndex", sc->preferredIndex);
                                screader->get("deadzone", sc->deadzone);
                                
                                m_stickconfigs.push_back(sc);
                        }
                }
                
        }

}

// -----------------------------------------------------------------------------
void
UserConfig::readPlayerInput(const lisp::Lisp *r, const char *node,
                                                        KartAction ka, int playerIndex)
{
        readInput(r, node, (GameAction) (playerIndex * KC_COUNT + ka + GA_P1_LEFT));
}
// -----------------------------------------------------------------------------
void
UserConfig::readInput(const lisp::Lisp* r,
                                          const char *node,
                      GameAction action)
{
    string inputTypeName;

    const Lisp* nodeReader = r->getLisp(node);
    if (!nodeReader)
      return;

    // Every unused id variable *must* be set to
    // something different than -1. Otherwise
    // the restored mapping will not be applied.
    Input input = Input(IT_NONE, -1, -1, -1);

    nodeReader->get("type", inputTypeName);
    if (inputTypeName == "keyboard")
    {
        input.type = IT_KEYBOARD;
        nodeReader->get("key", input.id0);
        input.id1 = input.id2 = 0;
    }
    else if (inputTypeName == "stickaxis")
    {
        input.type = IT_STICKMOTION;
        nodeReader->get("stick", input.id0);
        nodeReader->get("axis", input.id1);
        nodeReader->get("direction", input.id2);
    }
    else if (inputTypeName == "stickbutton")
    {
        input.type = IT_STICKBUTTON;
        nodeReader->get("stick", input.id0);
        nodeReader->get("button", input.id1);
        input.id2 = 0;
    }
    else if (inputTypeName == "stickhat")
    {
        input.type = IT_STICKHAT;
        // TODO: Implement me
    }
    else if (inputTypeName == "mouseaxis")
    {
        input.type = IT_MOUSEMOTION;
        nodeReader->get("axis", input.id0);
        nodeReader->get("direction", input.id1);
        input.id2 = 0;
    }
    else if (inputTypeName == "mousebutton")
    {
        input.type = IT_MOUSEBUTTON;
        nodeReader->get("button", input.id0);
        input.id1 = input.id2 = 0;
    }

    if (input.id0 != -1 && input.id1 != -1 && input.id2 != -1)
        {
        setInput(action, input);
        }

}

// -----------------------------------------------------------------------------
/** Call saveConfig with the default filename for this platform. */
void UserConfig::saveConfig()
{
    saveConfig(m_filename);
}   // saveConfig

// -----------------------------------------------------------------------------
/** Write settings to config file. */
void UserConfig::saveConfig(const std::string& filename)
{
    std::string temp;
    int i;

    const int DIR_EXIST = CheckAndCreateDir();
    // Check if the config directory exists (again, since it was already checked
    // when reading the config file - this is done in case that the problem was
    // fixed while tuxkart is running). If the directory does not exist and
    // can not be created, an error message was already printed to stderr,
    // and we can exit here without any further messages.
    if (DIR_EXIST == 0) return;

    Writer *writer = new Writer(filename);
    try
    {
        writer->beginList("tuxkart-config");
        writer->writeComment("If the game's supported config file version is higher than this number the configuration is discarded.");
        writer->write("configFileVersion\t",   CURRENT_CONFIG_VERSION);

        writer->writeComment("the following options can be set to #t or #f:");
        writer->write("sfx\t",   !(m_sfx==UC_DISABLE));
        writer->write("music\t", !(m_music==UC_DISABLE));
        writer->write("smoke\t", m_smoke);
        writer->writeComment("Display frame per seconds");
        writer->write("displayFPS\t", m_display_fps);
        writer->writeComment("Name of the .herring file to use.");
        writer->write("herringStyle\t", m_herring_style);
        writer->writeComment("Background music file to use,");
        writer->write("background-music\t", m_background_music);
        writer->writeComment("Use of kilometers per hours (km/h) instead of mph");
        writer->write("useKPH\t", m_use_kph);
        writer->writeComment("maximum fps, should be at least 60");
        writer->write("max-fps\t", m_max_fps);
        writer->writeComment("Volume for sound effects, see openal AL_GAIN for interpretation");
        writer->write("sfx-volume", m_sfx_volume);

        writer->writeComment("screen resolution and windowing mode");
        writer->write("width\t", m_width);
        writer->write("height\t", m_height);
        writer->write("prev_width\t", m_prev_width);
        writer->write("prev_height\t", m_prev_height);
        writer->write("prev_windowed\t", m_prev_windowed);
        writer->write("crash_detected\t", m_crashed);
        writer->write("blacklisted_resolutions\t", m_blacklist_res);
        writer->write("fullscreen\t", m_fullscreen);

        writer->writeComment("number of karts. -1 means use all");
        writer->write("karts\t", m_karts);

        writer->writeComment("number of karts. -1 means use all");
        writer->write("karts\t", m_karts);
        
        writer->writeComment("error logging to log (true) or stderr (false)");
        writer->write("log-errors\t", m_log_errors);
                
        writer->writeComment("Last selected kart group");
        writer->write("kart-group", m_kart_group);
        writer->writeComment("Last selected track group");
        writer->write("track-group", m_track_group);
                writeStickConfigs(writer);

        // Write unlock information back
        writer->beginList("unlock-info");
        unlock_manager->save(writer);
        writer->endList("unlock-info");

        /* write player configurations */
        for(i=0; i<PLAYERS; ++i)
        {
            temp = "player ";
            temp += i+'1';
            temp += " settings";
            writer->writeComment(temp);
            temp = "player-";
            temp += i+'1';
            writer->beginList(temp);

            writer->write("name\t", m_player[i].getName());

            writer->writeComment("optional");
            writer->write("lastKartId", m_player[i].getLastKartId());

            writePlayerInput(writer, "left\t", KA_LEFT, i);
            writePlayerInput(writer, "right\t", KA_RIGHT, i);
            writePlayerInput(writer, "accel\t", KA_ACCEL, i);
            writePlayerInput(writer, "brake\t", KA_BRAKE, i);
            writePlayerInput(writer, "wheelie\t", KA_WHEELIE, i);
            writePlayerInput(writer, "jump\t", KA_JUMP, i);
            writePlayerInput(writer, "rescue\t", KA_RESCUE, i);
            writePlayerInput(writer, "fire\t", KA_FIRE, i);
            writePlayerInput(writer, "lookBack\t", KA_LOOK_BACK, i);

            writer->endList(temp);
        }   // for i

        writer->endList("tuxkart-config");
    }
    catch(std::exception& e)
    {
        fprintf(stderr, "Couldn't write config: ");
        fprintf(stderr, e.what());
        fprintf(stderr, "\n");
    }
        
        delete writer;
}   // saveConfig

// -----------------------------------------------------------------------------

void
UserConfig::writeStickConfigs(lisp::Writer *writer)
{
        int count = 0;
        string temp;
        
        writer->beginList("stick-configs");
        
        count = (int)m_stickconfigs.size();
        writer->write("count", count);
        
        for (int i = 0; i < count; i++)
        {
                StickConfig *sc = m_stickconfigs[i];
                temp = "stick-";
                temp += i + '1';
                
                writer->beginList(temp);
                                
                writer->write("id", sc->id);
                writer->write("preferredIndex", sc->preferredIndex);
                writer->writeComment("0 means that the default deadzone value is used.");
                writer->write("deadzone", sc->deadzone);
                
                writer->endList(temp);
        }
        
        writer->endList("stick-configs");
}

// -----------------------------------------------------------------------------
void
UserConfig::writePlayerInput(lisp::Writer *writer, const char *node,
                                                         KartAction ka, int playerIndex)
{
        writeInput(writer, node, (GameAction) (playerIndex * KC_COUNT + ka + GA_P1_LEFT));
}
// -----------------------------------------------------------------------------
void
UserConfig::writeInput(lisp::Writer *writer,
                                           const char *node,
                                           GameAction action)
{
    writer->beginList(node);
        
        if (inputMap[action].count)
        {

    const Input input = inputMap[action].inputs[0];

    if (input.type != IT_NONE)
    {
                switch (input.type)
                {
                case IT_NONE:
                        break;
                case IT_KEYBOARD:
                        writer->write("type", "keyboard");
                        writer->write("key", input.id0);
                        break;
                case IT_STICKMOTION:
                        writer->write("type", "stickaxis");
                        writer->write("stick", input.id0);
                        writer->write("axis", input.id1);
                        writer->writeComment("0 is negative/left/up, 1 is positive/right/down");
                        writer->write("direction", input.id2);
                        break;
                case IT_STICKBUTTON:
                        writer->write("type", "stickbutton");
                        writer->write("stick", input.id0);
                        writer->write("button", input.id1);
                        break;
                case IT_STICKHAT:
                        // TODO: Implement me
                        break;
                case IT_MOUSEMOTION:
                        writer->write("type", "mouseaxis");
                        writer->write("axis", input.id0);
                        writer->writeComment("0 is negative/left/up, 1 is positive/right/down");
                        writer->write("direction", input.id1);
                        break;
                case IT_MOUSEBUTTON:
                        writer->write("type", "mousebutton");
                        writer->writeComment("0 is left, 1 is middle, 2 is right, 3 is wheel up, 4 is wheel down");
                        writer->writeComment("other values denote auxillary buttons");
                        writer->write("button", input.id0);
                        break;
                }
        }
        }

    writer->endList(node);
}
// -----------------------------------------------------------------------------
std::string UserConfig::getInputAsString(Input &input)
{
    char msg[MAX_MESSAGE_LENGTH];
    std::ostringstream stm;
    
    switch (input.type)
    {
    case IT_NONE:
        snprintf(msg, sizeof(msg), _("not set"));
        break;
    case IT_KEYBOARD:
        snprintf(msg, sizeof(msg), "%s", SDL_GetKeyName((SDLKey) input.id0));
        break;
    case IT_STICKMOTION:
        snprintf(msg, sizeof(msg), _("joy %d axis %d  %c"),
                 input.id0, input.id1, (input.id2 == AD_NEGATIVE) ? '-' : '+');
        break;
    case IT_STICKBUTTON:
        snprintf(msg, sizeof(msg), _("joy %d btn %d"), input.id0, input.id1);
        break;
    case IT_STICKHAT:
        snprintf(msg, sizeof(msg), _("joy %d hat %d"), input.id0, input.id1);
        break;
    case IT_MOUSEBUTTON:
        snprintf(msg, sizeof(msg), _("mouse btn %d"), input.id0);
        break;
    case IT_MOUSEMOTION:
        snprintf(msg, sizeof(msg), _("mouse axis %d %c"),
                 input.id0, ((input.id1 == AD_NEGATIVE) ? '-' : '+'));
        break;
    default:
        snprintf(msg, sizeof(msg), _("Invalid"));
    }
    
    stm << msg;
    
    return stm.str();
}   // GetKeyAsString

// -----------------------------------------------------------------------------
string
UserConfig::getMappingAsString(GameAction ga)
{
  if (inputMap[ga].count
          && inputMap[ga].inputs[0].type)
  {
    stringstream s;
    s << getInputAsString(inputMap[ga].inputs[0]);

        return s.str();
  }
  else
  {
    return string(_("not set"));
  }
}
// -----------------------------------------------------------------------------
string
UserConfig::getMappingAsString(int playerIndex, KartAction ka)
{
        return getMappingAsString((GameAction) (GA_FIRST_KARTACTION
                                                                                        + playerIndex * KC_COUNT + ka));
}
// -----------------------------------------------------------------------------

void
UserConfig::unsetDuplicates (GameAction ga, Input &i)
{
  for (int cga = GA_FIRST_KARTACTION; cga <= GA_LAST_KARTACTION; cga++)
  {
    if (cga != ga)
    {
      // If the input occurs in any other mapping
      // delete it properly from there.
      
      if (inputMap[cga].count
                  && inputMap[cga].inputs[0].type == i.type
          && inputMap[cga].inputs[0].id0 == i.id0
          && inputMap[cga].inputs[0].id1 == i.id1
          && inputMap[cga].inputs[0].id2 == i.id2)
      {
        // Delete it.
        inputMap[cga].inputs[0].type = IT_NONE;
      }
    }
  }
}
// -----------------------------------------------------------------------------
void
UserConfig::set(GameAction ga, Input i)
{
  inputMap[ga].count = 1;
  inputMap[ga].inputs = new Input[1];
  inputMap[ga].inputs[0] = i;
}
// -----------------------------------------------------------------------------
void
UserConfig::set(GameAction ga, Input i0, Input i1)
{
  inputMap[ga].count = 2;
  inputMap[ga].inputs = new Input[2];
  inputMap[ga].inputs[0] = i0;
  inputMap[ga].inputs[1] = i1;
}
// -----------------------------------------------------------------------------
void
UserConfig::set(GameAction ga, Input i0, Input i1, Input i2)
{
  inputMap[ga].count = 3;
  inputMap[ga].inputs = new Input[3];
  inputMap[ga].inputs[0] = i0;
  inputMap[ga].inputs[1] = i1;
  inputMap[ga].inputs[2] = i2;
}
// -----------------------------------------------------------------------------
void
UserConfig::set(GameAction ga, Input i0, Input i1, Input i2, Input i3)
{
  inputMap[ga].count = 4;
  inputMap[ga].inputs = new Input[4];
  inputMap[ga].inputs[0] = i0;
  inputMap[ga].inputs[1] = i1;
  inputMap[ga].inputs[2] = i2;
  inputMap[ga].inputs[3] = i3;
}

// -----------------------------------------------------------------------------
void
UserConfig::setInput(GameAction ga, Input &input)
{
        // Removes the input from all mappings where it occurs.
        unsetDuplicates(ga, input);

        set(ga, input);
}
// -----------------------------------------------------------------------------
void
UserConfig::setInput(int playerIndex, KartAction ka, Input &input)
{
        setInput((GameAction) (GA_FIRST_KARTACTION
                                                   + playerIndex * KC_COUNT + ka),
                         input);
}
// -----------------------------------------------------------------------------
void
UserConfig::clearInput(int playerIndex, KartAction ka)
{
        inputMap[(GameAction) (GA_FIRST_KARTACTION + playerIndex * KC_COUNT + ka)]
                .count = 0;
}
// -----------------------------------------------------------------------------
ActionMap *
UserConfig::newActionMap(const int from, const int to)
{
        ActionMap *am = new ActionMap();
        
        for (int i = from; i <= to; i++)
        {
                const int count = inputMap[i].count;
                for (int j = 0;j < count; j++)
                        am->putEntry(inputMap[i].inputs[j], (GameAction) i);
        }
        
        return am;
}
// -----------------------------------------------------------------------------
ActionMap *
UserConfig::newMenuActionMap()
{
        return newActionMap(GA_FIRST_MENU, GA_LAST_MENU);
}
// -----------------------------------------------------------------------------
ActionMap *
UserConfig::newIngameActionMap()
{
        // This is rather unfriendly hack but work quite effective:
        // In order to prevent the input driver from handling input mappings
        // for human players which are not in the current game we select a subset
        // of the game actions by looking at the amount of players. The
        // race_manager instance is assumed to be available at this time because
        // this method is called immediately before the RaceGUI instance is created
        // (in MenuManager) and RaceGUI needs race_manager, too.

  // TODO: Reorder ingame GameAction values so that they start with
  // the fixed ones. This would allow simpler looking code here. 

        GameAction gaEnd = GA_NULL;
        
        switch (race_manager->getNumPlayers())
        {
                case 1:
                        gaEnd = GA_P1_LOOK_BACK; break;
                case 2:
                        gaEnd = GA_P2_LOOK_BACK; break;
                case 3:
                        gaEnd = GA_P3_LOOK_BACK; break;
                case 4:
                        gaEnd = GA_P4_LOOK_BACK; break;
        }
        
        ActionMap *am = newActionMap(GA_FIRST_INGAME, gaEnd);

        for (int i = GA_FIRST_INGAME_FIXED; i <= GA_LAST_INGAME_FIXED; i++)
        {
                const int count = inputMap[i].count;
                for (int j = 0;j < count; j++)
                        am->putEntry(inputMap[i].inputs[j], (GameAction) i);
        }

        return am;
}
// -----------------------------------------------------------------------------
/** Determines whether the given Input is used in a mapping where it is marked
  * as fixed. This allows the input driver to discard the mapping and not
  * allow the user to use it.
  */
bool
UserConfig::isFixedInput(InputType type, int id0, int id1, int id2)
{
        for (int i = GA_FIRST_INGAME_FIXED; i <= GA_LAST_INGAME_FIXED; i++)
        {
                const int count = inputMap[i].count;
                for (int j = 0;j < count; j++)
                        if (inputMap[i].inputs[j].type == type
                                && inputMap[i].inputs[j].id0 == id0
                                && inputMap[i].inputs[j].id1 == id1
                                && inputMap[i].inputs[j].id2 == id2)
                                return true;
        }
        
        return false;
}

// -----------------------------------------------------------------------------

void
UserConfig::addStickConfig(StickConfig *sc)
{
        m_stickconfigs.push_back(sc);
}

// -----------------------------------------------------------------------------
const std::vector<UserConfig::StickConfig *> *
UserConfig::getStickConfigs() const
{
        return &m_stickconfigs;
}

UserConfig::StickConfig::StickConfig(string &newId)
: id(newId)
{
        // Nothing else to do.
}

/*EOF*/
