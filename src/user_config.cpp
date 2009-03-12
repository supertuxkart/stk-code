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

#include "user_config.hpp"

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
#define _WINSOCKAPI_
#include <plib/ul.h>

#include "stk_config.hpp"
#include "actionmap.hpp"
#include "race_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "io/file_manager.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "lisp/writer.hpp"
#include "utils/translation.hpp"
#include "utils/string_utils.hpp"

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
}   // ~UserConfig

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
    m_warning           = "";
    m_gamepad_debug     = false;
    m_track_debug       = 0;
    m_bullet_debug      = false;
    m_fullscreen        = false;
    m_no_start_screen   = false;
    m_sfx               = UC_ENABLE;
    m_music             = UC_ENABLE;
    m_graphical_effects = true;
    m_display_fps       = false;
    m_item_style        = "items";
    m_background_music  = "";
    m_profile           = 0;
    m_print_kart_sizes  = false;
    m_max_fps           = 120;
    m_sfx_volume        = 1.0f;
    m_use_kph           = false;
    m_width             = 800;
    m_height            = 600;
    m_prev_width        = m_width;
    m_prev_height       = m_height;
    m_prev_windowed     = false;
    m_crashed           = false;
    m_blacklist_res.clear();
    m_num_karts         = 4;
    m_num_laps          = 4;
    m_difficulty        = 0;
    m_background_index  = 0;
    m_log_errors        = false;
    m_kart_group        = "standard";
    m_track_group       = "standard";
    m_last_track        = "jungle";
    m_server_address    = "localhost";
    m_server_port       = 2305;

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
    memset(m_input_map, 0, sizeof(m_input_map));

    /* general game input settings */
    set(GA_ENTER,
        Input(Input::IT_KEYBOARD, SDLK_RETURN),
        Input(Input::IT_KEYBOARD, SDLK_SPACE),
        Input(Input::IT_STICKBUTTON, 0, 0),
        Input(Input::IT_MOUSEBUTTON, 1));
    set(GA_LEAVE,
        Input(Input::IT_KEYBOARD, SDLK_ESCAPE),
        Input(Input::IT_STICKBUTTON, 0, 1),
        Input(Input::IT_MOUSEBUTTON, 2),
        Input(Input::IT_MOUSEBUTTON, 3));
    set(GA_CURSOR_UP,
        Input(Input::IT_KEYBOARD, SDLK_UP),
        Input(Input::IT_MOUSEBUTTON, 4),
        Input(Input::IT_STICKMOTION, 0, 1, Input::AD_NEGATIVE));
    set(GA_CURSOR_DOWN,
        Input(Input::IT_KEYBOARD, SDLK_DOWN),
        Input(Input::IT_MOUSEBUTTON, 5),
        Input(Input::IT_STICKMOTION, 0, 1, Input::AD_POSITIVE));
    set(GA_CURSOR_LEFT,
        Input(Input::IT_KEYBOARD, SDLK_LEFT),
        Input(Input::IT_STICKMOTION, 0, 0, Input::AD_NEGATIVE));
    set(GA_CURSOR_RIGHT,
        Input(Input::IT_KEYBOARD, SDLK_RIGHT),
        Input(Input::IT_STICKMOTION, 0, 0, Input::AD_POSITIVE));
    set(GA_CLEAR_MAPPING,
        Input(Input::IT_KEYBOARD, SDLK_BACKSPACE),
        Input(Input::IT_STICKBUTTON, 0, 2));
    set(GA_INC_SCROLL_SPEED,
        Input(Input::IT_KEYBOARD, SDLK_PLUS));
    set(GA_INC_SCROLL_SPEED_FAST,
        Input(Input::IT_KEYBOARD, SDLK_PAGEDOWN));
    set(GA_DEC_SCROLL_SPEED,
        Input(Input::IT_KEYBOARD, SDLK_MINUS));
    set(GA_DEC_SCROLL_SPEED_FAST,
        Input(Input::IT_KEYBOARD, SDLK_PAGEUP));
    set(GA_TOGGLE_FULLSCREEN,
        Input(Input::IT_KEYBOARD, SDLK_F9));
    set(GA_LEAVE_RACE,
        Input(Input::IT_KEYBOARD, SDLK_ESCAPE));
#ifdef DEBUG
    set(GA_DEBUG_ADD_BOWLING,
        Input(Input::IT_KEYBOARD, SDLK_F1));
    set(GA_DEBUG_ADD_MISSILE,
        Input(Input::IT_KEYBOARD, SDLK_F2));
    set(GA_DEBUG_ADD_HOMING,
        Input(Input::IT_KEYBOARD, SDLK_F3));
#endif
    set(GA_DEBUG_TOGGLE_FPS,
        Input(Input::IT_KEYBOARD, SDLK_F12));
    set(GA_DEBUG_TOGGLE_WIREFRAME,
        Input(Input::IT_KEYBOARD, SDLK_F11));
    set(GA_DEBUG_HISTORY,
        Input(Input::IT_KEYBOARD, SDLK_F10));

    // TODO: The following should become a static
    // array. This allows:
    // a) resetting to default values
    // b) prevent loading those defaults if config file contains any bindings

    /* Player 1 default input settings */
    set(GA_P1_LEFT,      Input(Input::IT_KEYBOARD, SDLK_LEFT));
    set(GA_P1_RIGHT,     Input(Input::IT_KEYBOARD, SDLK_RIGHT));
    set(GA_P1_ACCEL,     Input(Input::IT_KEYBOARD, SDLK_UP));
    set(GA_P1_BRAKE,     Input(Input::IT_KEYBOARD, SDLK_DOWN));
    set(GA_P1_NITRO,     Input(Input::IT_KEYBOARD, SDLK_l));
    set(GA_P1_DRIFT,     Input(Input::IT_KEYBOARD, SDLK_k));
    set(GA_P1_RESCUE,    Input(Input::IT_KEYBOARD, SDLK_h));
    set(GA_P1_FIRE,      Input(Input::IT_KEYBOARD, SDLK_SPACE));
    set(GA_P1_LOOK_BACK, Input(Input::IT_KEYBOARD, SDLK_j));

    /* Player 2 default input settings */
    set(GA_P2_LEFT,      Input(Input::IT_KEYBOARD, SDLK_a));
    set(GA_P2_RIGHT,     Input(Input::IT_KEYBOARD, SDLK_d));
    set(GA_P2_ACCEL,     Input(Input::IT_KEYBOARD, SDLK_w));
    set(GA_P2_BRAKE,     Input(Input::IT_KEYBOARD, SDLK_s));
    set(GA_P2_NITRO,     Input(Input::IT_KEYBOARD, SDLK_LSHIFT));
    set(GA_P2_DRIFT,     Input(Input::IT_KEYBOARD, SDLK_CAPSLOCK));
    set(GA_P2_RESCUE,    Input(Input::IT_KEYBOARD, SDLK_q));
    set(GA_P2_FIRE,      Input(Input::IT_KEYBOARD, SDLK_LCTRL));
    set(GA_P2_LOOK_BACK, Input(Input::IT_KEYBOARD, SDLK_LALT));

    // If the same key is used for more than one player, the setting 
    // is overwritten when reading back the file. To allow us to have 
    // good settings for player 1 (and considering that 3 or 4 people 
    // on a single keyboard is crazy anyway), we disable the defaults 
    // for player 3 and 4.
#undef DEFAULTS_FOR_PLAYER34
#ifdef DEFAULTS_FOR_PLAYER34
    /* Player 3 default input settings */
    set(GA_P3_LEFT,      Input(Input::IT_KEYBOARD, SDLK_f));
    set(GA_P3_RIGHT,     Input(Input::IT_KEYBOARD, SDLK_h));
    set(GA_P3_ACCEL,     Input(Input::IT_KEYBOARD, SDLK_t));
    set(GA_P3_BRAKE,     Input(Input::IT_KEYBOARD, SDLK_g));
    set(GA_P3_NITRO,     Input(Input::IT_KEYBOARD, SDLK_c));
    set(GA_P3_DRIFT,     Input(Input::IT_KEYBOARD, SDLK_v));
    set(GA_P3_RESCUE,    Input(Input::IT_KEYBOARD, SDLK_r));
    set(GA_P3_FIRE,      Input(Input::IT_KEYBOARD, SDLK_b));
    set(GA_P3_LOOK_BACK, Input(Input::IT_KEYBOARD, SDLK_n));

    /* Player 4 default input settings  */
    set(GA_P4_LEFT,      Input(Input::IT_KEYBOARD, SDLK_j));
    set(GA_P4_RIGHT,     Input(Input::IT_KEYBOARD, SDLK_l));
    set(GA_P4_ACCEL,     Input(Input::IT_KEYBOARD, SDLK_i));
    set(GA_P4_BRAKE,     Input(Input::IT_KEYBOARD, SDLK_k));
    set(GA_P4_NITRO,     Input(Input::IT_KEYBOARD, SDLK_m));
    set(GA_P4_DRIFT,     Input(Input::IT_KEYBOARD, SDLK_COMMA));
    set(GA_P4_RESCUE,    Input(Input::IT_KEYBOARD, SDLK_u));
    set(GA_P4_FIRE,      Input(Input::IT_KEYBOARD, SDLK_PERIOD));
    set(GA_P4_LOOK_BACK, Input(Input::IT_KEYBOARD, SDLK_SLASH));
#endif

}   // setDefaults

// -----------------------------------------------------------------------------
/** Sets the next background image index. */
void UserConfig::nextBackgroundIndex()
{
    m_background_index++;
    if(m_background_index>=(int)stk_config->m_mainmenu_background.size())
        m_background_index = 0;
}
 
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
        // This will initialise the last input configuration with the
        // default values from the current (=default) player input
        // device configuration.
        readLastInputConfigurations(NULL);

        delete root;
        return;
    }

    // In older config files, nitro is still named 'wheelie', and drift is jump
    std::string nitro_name="nitro";
    std::string drift_name="drift";
    try
    {
        const lisp::Lisp* lisp = root->getLisp("tuxkart-config");
        if(!lisp) 
        {
            throw std::runtime_error("No tuxkart-config node");
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
            case 6:  printf("Added nitro and drifting, removed jumping and wheelie.\n");
                     nitro_name="wheelie";
                     drift_name="jump";
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
        lisp->get("graphical-effects",m_graphical_effects);
        lisp->get("displayFPS",       m_display_fps);
        lisp->get("itemStyle",        m_item_style);
        lisp->get("background-music", m_background_music);
        lisp->get("max-fps",          m_max_fps);
        lisp->get("sfx-volume",       m_sfx_volume);
        lisp->get("useKPH",           m_use_kph);

        /*get resolution width/height*/
        lisp->get("width",            m_width);
        lisp->get("height",           m_height);
        lisp->get("prev_width",       m_prev_width);
        lisp->get("prev_height",      m_prev_height);
        lisp->get("prev_windowed",    m_prev_windowed);
        //detect if resolution change previously crashed STK
        lisp->get("crash_detected",   m_crashed);
        // blacklisted resolutions
        lisp->getVector("blacklisted_resolutions",
                                      m_blacklist_res);
        /*Get default number of karts, number of laps, and difficulty. */
        lisp->get("karts",            m_num_karts);
        lisp->get("laps",             m_num_laps);
        lisp->get("difficulty",       m_difficulty);

        lisp->get("kart-group",       m_kart_group);
        lisp->get("track-group",      m_track_group);
        lisp->get("last-track",       m_last_track);

        // Get background image index.
        lisp->get("background",       m_background_index);

        // Handle loading the stick config in it own method.
        readStickConfigs(lisp);

        // Address of server
        lisp->get("server-address",   m_server_address);
        lisp->get("server-port",      m_server_port);

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
                std::ostringstream msg;
                msg << "No '" << temp << "' node";
                throw std::runtime_error(msg.str());
            }
            std::string name;
            reader->get("name", name);
            if (configFileVersion <= 3)
            {
                // For older config files, replace the default player
                // names "Player %d" with the user name
                std::ostringstream sDefaultName;

                sDefaultName << "Player " << i+1;
                // If the config file does not contain a name or the old
                // default name, set the default username as player name.
                if(name.size()==0 || name==sDefaultName.str()) name=m_username;
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
            for(int ka=KA_FIRST; ka<=KA_LAST; ka++)
            {
                readPlayerInput(reader, KartActionStrings[ka],
                    (KartAction)ka, i);
            }
            // Leave those in for backwards compatibility (i.e. config files
            // with jump/wheelie). If jump/wheelie are not defined, nothing
            // happens (the same input is read again).
            readPlayerInput(reader, nitro_name, KA_NITRO, i);
            readPlayerInput(reader, drift_name, KA_DRIFT, i);
        }   // for i < PLAYERS

        // Read the last input device configurations. It is important that this
        // happens after reading the player config, since if no last config
        // is given, the last config is initialised with the current player
        // config.
        readLastInputConfigurations(lisp);
    }
    catch(std::exception& e)
    {
        fprintf(stderr, "Error while parsing config '%s':\n", filename.c_str());
        fprintf(stderr, "%s", e.what());
        fprintf(stderr, "\n");
    }

    delete root;
}   // loadConfig

// -----------------------------------------------------------------------------
void UserConfig::readStickConfigs(const lisp::Lisp *r)
{
    std::string temp;
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
                std::string id;
                screader->get("id", id);

                StickConfig *sc = new StickConfig(id);

                screader->get("preferredIndex", sc->m_preferredIndex);
                screader->get("deadzone", sc->m_deadzone);

                m_stickconfigs.push_back(sc);
            }
        }
    }
}   // readStickConfigs

// -----------------------------------------------------------------------------
/** Reads the last used configuration of an input device. If reader - NULL or
 *  if no last-input-configuration node is contained in the config file, the
 *  last input configuration will be initialised with the current player
 *  configuration.
 *  \param reader The lisp reader.
 */
void UserConfig::readLastInputConfigurations(const lisp::Lisp *reader)
{
    const lisp::Lisp* nodeReader = reader
                                 ? reader->getLisp("last-input-configurations")
                                 : NULL;
    // No last input configuration specified. Use the current player mappings
    // to initialise this mapping.
    if (!nodeReader)
    {
        for(unsigned int player_index=0; player_index<PLAYERS; player_index++)
        {
            for(int ka=KA_FIRST; ka<=KA_LAST; ka++)
            {
                const Input &inp = getInput(player_index, (KartAction)ka);
                std::string device = getInputDeviceName(player_index);
                if(device.size()>0)
                    m_last_input_configuration[device].m_input[ka] = inp;
            }   // for ka in KA_FIRST, ..., KA_LAST
        }   // for player_index<PLAYERS
        return;
    }

    int count=0;
    nodeReader->get("count", count);
    for(int i=0; i<count; i++)
    {
        std::ostringstream s;
        s<<"index-"<<i;
        const lisp::Lisp *node = nodeReader->getLisp(s.str());
        if(!node) continue;
        std::string device_name;
        node->get("id", device_name);
        for(int ka=KA_FIRST; ka<=KA_LAST; ka++)
        {
            const lisp::Lisp *action = node->getLisp(KartActionStrings[ka]);
            if(!action) continue;
            Input input = readInput(action);
            m_last_input_configuration[device_name].m_input[ka] = input;
        }   // for ka=KA_FIRST, KA_LAST
    }   // for i<count
}   // readLastInputConfigurations

// -----------------------------------------------------------------------------
void UserConfig::readPlayerInput(const lisp::Lisp *r, const std::string &node,
                                 KartAction ka, int playerIndex)
{
    readInputNode(r, node, (GameAction) (playerIndex * KC_COUNT + ka + GA_P1_LEFT));
}   // readPlayerInput

// -----------------------------------------------------------------------------
void UserConfig::readInputNode(const lisp::Lisp* r, const std::string &node,
                               GameAction action)
{
    std::string inputTypeName;

    const lisp::Lisp* nodeReader = r->getLisp(node);
    if (!nodeReader)
        return;

    // Every unused id variable *must* be set to
    // something different than -1. Otherwise
    // the restored mapping will not be applied.
    Input input = readInput(nodeReader);

    if (input.id0 != -1 && input.id1 != -1 && input.id2 != -1)
    {
        setInput(action, input);
    }
}   // readInputNode

// -----------------------------------------------------------------------------
Input UserConfig::readInput(const lisp::Lisp* nodeReader)
{
    // Every unused id variable *must* be set to
    // something different than -1. Otherwise
    // the restored mapping will not be applied.
    Input input = Input(Input::IT_NONE, -1, -1, -1);
    std::string inputTypeName;

    nodeReader->get("type", inputTypeName);
    if (inputTypeName == "keyboard")
    {
        input.type = Input::IT_KEYBOARD;
        nodeReader->get("key", input.id0);
        input.id1 = input.id2 = 0;
    }
    else if (inputTypeName == "stickaxis")
    {
        input.type = Input::IT_STICKMOTION;
        nodeReader->get("stick", input.id0);
        nodeReader->get("axis", input.id1);
        nodeReader->get("direction", input.id2);
    }
    else if (inputTypeName == "stickbutton")
    {
        input.type = Input::IT_STICKBUTTON;
        nodeReader->get("stick", input.id0);
        nodeReader->get("button", input.id1);
        input.id2 = 0;
    }
    else if (inputTypeName == "stickhat")
    {
        input.type = Input::IT_STICKHAT;
        // TODO: Implement me
    }
    else if (inputTypeName == "mouseaxis")
    {
        input.type = Input::IT_MOUSEMOTION;
        nodeReader->get("axis", input.id0);
        nodeReader->get("direction", input.id1);
        input.id2 = 0;
    }
    else if (inputTypeName == "mousebutton")
    {
        input.type = Input::IT_MOUSEBUTTON;
        nodeReader->get("button", input.id0);
        input.id1 = input.id2 = 0;
    }
    return input;
}   // readInput

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

    lisp::Writer *writer = new lisp::Writer(filename);
    try
    {
        writer->beginList("tuxkart-config");
        writer->writeComment("If the game's supported config file version is higher than this number the configuration is discarded.");
        writer->write("configFileVersion\t",   CURRENT_CONFIG_VERSION);

        writer->writeComment("the following options can be set to #t or #f:");
        writer->write("sfx\t",   !(m_sfx==UC_DISABLE));
        writer->write("music\t", !(m_music==UC_DISABLE));
        writer->write("graphical-effects\t", m_graphical_effects);
        writer->writeComment("Display frame per seconds");
        writer->write("displayFPS\t", m_display_fps);
        writer->writeComment("Name of the .items file to use.");
        writer->write("itemStyle\t", m_item_style);
        writer->writeComment("Background music file to use,");
        writer->write("background-music\t", m_background_music);
        writer->writeComment("Use of kilometers per hours (km/h) instead of mph");
        writer->write("useKPH\t", m_use_kph);
        writer->writeComment("maximum fps, should be at least 60");
        writer->write("max-fps\t", m_max_fps);
        writer->writeComment("Volume for sound effects, see openal AL_GAIN for interpretation");
        writer->write("sfx-volume", m_sfx_volume);

        writer->writeComment("screen resolution and windowing mode");
        writer->write("width\t",          m_width);
        writer->write("height\t",         m_height);
        writer->write("prev_width\t",     m_prev_width);
        writer->write("prev_height\t",    m_prev_height);
        writer->write("prev_windowed\t",  m_prev_windowed);
        writer->write("crash_detected\t", m_crashed);
        writer->write("blacklisted_resolutions\t", 
                                          m_blacklist_res);
        writer->write("fullscreen\t",     m_fullscreen);

        writer->writeComment("Number of karts. -1 means use all");
        writer->write("karts\t",          m_num_karts);
        writer->writeComment("Number of laps.");
        writer->write("laps\t",            m_num_laps);
        writer->writeComment("Difficulty: 0=easy, 1=medium, 2=hard");
        writer->write("difficulty\t",     m_difficulty);
        writer->writeComment("Last selected kart group");
        writer->write("kart-group",       m_kart_group);
        writer->writeComment("Last selected track group");
        writer->write("track-group",      m_track_group);
        writer->writeComment("Last track played");
        writer->write("last-track",       m_last_track);
        writer->writeComment("Menu background image to use");
        writer->write("background",       m_background_index);
        writer->writeComment("Information about last server used");
        writer->write("server-address",   m_server_address);
        writer->write("server-port",      m_server_port);

        writeStickConfigs(writer);
        writeLastInputConfigurations(writer);

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

            for(int ka=KA_FIRST; ka<=KA_LAST; ka++)
            {
                writePlayerInput(writer, KartActionStrings[ka]+"\t", 
                                 (KartAction)ka, i);
            }
            writer->endList(temp);
        }   // for i

        writer->endList("tuxkart-config");
    }
    catch(std::exception& e)
    {
        fprintf(stderr, "Couldn't write config: ");
        fprintf(stderr, "%s",e.what());
        fprintf(stderr, "\n");
    }

    delete writer;
}   // saveConfig

// -----------------------------------------------------------------------------
void UserConfig::writeStickConfigs(lisp::Writer *writer)
{
    int count = 0;
    std::string temp;

    writer->beginList("stick-configs");

    count = (int)m_stickconfigs.size();
    writer->write("count", count);

    for (int i = 0; i < count; i++)
    {
        StickConfig *sc = m_stickconfigs[i];
        temp = "stick-";
        temp += i + '1';

        writer->beginList(temp);

        writer->write("id",             sc->m_id);
        writer->write("preferredIndex", sc->m_preferredIndex);
        writer->writeComment("0 means that the default deadzone value is used.");
        writer->write("deadzone",       sc->m_deadzone);

        writer->endList(temp);
    }

    writer->endList("stick-configs");
}   // writeStickConfigs

// -----------------------------------------------------------------------------
/** Saves the last configuration for each used input device to the player 
 *  config file.
 *  \param writer The config file writer object.
 */
void UserConfig::writeLastInputConfigurations(lisp::Writer *writer)
{
    writer->beginList("last-input-configurations");
    writer->write("count\t", (int)m_last_input_configuration.size());
    std::map<std::string, InputConfiguration>::iterator i;
    int index =0;
    for(i =m_last_input_configuration.begin();
        i!=m_last_input_configuration.end();
        i++)
    {
        std::ostringstream s;
        s<<"index-"<<index;
        index++;
        writer->beginList(s.str());
        writer->write("id", i->first);
        for(int ka=KA_FIRST; ka<=KA_LAST; ka++)
        {
            writer->beginList(KartActionStrings[ka]);
            writeInput(writer, i->second.m_input[ka]);
            writer->endList(KartActionStrings[ka]);
        }   // for i
        writer->endList(s.str());
    }   // for i
    writer->endList("last-input-configurations");
}   // writeLastInputConfigurations

// -----------------------------------------------------------------------------
void UserConfig::writePlayerInput(lisp::Writer *writer, const std::string &node,
                                  KartAction ka, int playerIndex)
{
    writeInputNode(writer, node, (GameAction) (playerIndex * KC_COUNT + ka + GA_P1_LEFT));
}   // writePlayerInput

// -----------------------------------------------------------------------------
void UserConfig::writeInputNode(lisp::Writer *writer, const std::string &node,
                                GameAction action)
{
    writer->beginList(node);

    if (m_input_map[action].count)
    {
        writeInput(writer, m_input_map[action].inputs[0]);
    }

    writer->endList(node);
}   // writeInputNode

// -----------------------------------------------------------------------------
void UserConfig::writeInput(lisp::Writer *writer, const Input &input)
{
    if (input.type == Input::IT_NONE) return;
    switch (input.type)
    {
    case Input::IT_NONE:
        break;
    case Input::IT_KEYBOARD:
        writer->write("type", "keyboard");
        writer->write("key", input.id0);
        break;
    case Input::IT_STICKMOTION:
        writer->write("type", "stickaxis");
        writer->write("stick", input.id0);
        writer->write("axis", input.id1);
        writer->writeComment("0 is negative/left/up, 1 is positive/right/down");
        writer->write("direction", input.id2);
        break;
    case Input::IT_STICKBUTTON:
        writer->write("type", "stickbutton");
        writer->write("stick", input.id0);
        writer->write("button", input.id1);
        break;
    case Input::IT_STICKHAT:
        // TODO: Implement me
        break;
    case Input::IT_MOUSEMOTION:
        writer->write("type", "mouseaxis");
        writer->write("axis", input.id0);
        writer->writeComment("0 is negative/left/up, 1 is positive/right/down");
        writer->write("direction", input.id1);
        break;
    case Input::IT_MOUSEBUTTON:
        writer->write("type", "mousebutton");
        writer->writeComment("0 is left, 1 is middle, 2 is right, 3 is wheel up, 4 is wheel down");
        writer->writeComment("other values denote auxillary buttons");
        writer->write("button", input.id0);
        break;
    }
}   // writeInput

// -----------------------------------------------------------------------------
std::string UserConfig::getInputAsString(const Input &input)
{
    std::string s;

    switch (input.type)
    {
    case Input::IT_NONE:
        s = _("not set");
        break;
    case Input::IT_KEYBOARD:
        s = SDL_GetKeyName((SDLKey) input.id0);
        break;
    case Input::IT_STICKMOTION:
        s = StringUtils::insert_values( _("joy %d axis %d  %s"), input.id0, 
                                         input.id1, 
                                         (input.id2 == Input::AD_NEGATIVE) 
                                         ? '-' : '+'                        );
        break;
    case Input::IT_STICKBUTTON:
        s = StringUtils::insert_values( _("joy %d btn %d"), 
                                        input.id0, input.id1);
        break;
    case Input::IT_STICKHAT:
        s = StringUtils::insert_values( _("joy %d hat %d"),
                                        input.id0, input.id1);
        break;
    case Input::IT_MOUSEBUTTON:
        s = StringUtils::insert_values( _("mouse btn %d"), input.id0);
        break;
    case Input::IT_MOUSEMOTION:
        s = StringUtils::insert_values( _("mouse axis %d %s"),
                                        input.id0, 
                                        (input.id1 == Input::AD_NEGATIVE) 
                                        ? '-': '+'                        );
        break;
    default:
        s = _("Invalid");
    }

    return s;
}   // GetKeyAsString

// -----------------------------------------------------------------------------
std::string UserConfig::getMappingAsString(GameAction ga)
{
    if (m_input_map[ga].count &&
        m_input_map[ga].inputs[0].type)
    {
        std::stringstream s;
        s << getInputAsString(m_input_map[ga].inputs[0]);

        return s.str();
    }
    else
    {
        return std::string(_("not set"));
    }
}   // getMappingAsString

// -----------------------------------------------------------------------------
std::string UserConfig::getMappingAsString(int playerIndex, KartAction ka)
{
    return getMappingAsString((GameAction) (GA_FIRST_KARTACTION
                                            + playerIndex * KC_COUNT + ka) );
}   // getMappingAsString

// -----------------------------------------------------------------------------
void UserConfig::unsetDuplicates(GameAction ga, const Input &i)
{
    for (int cga = GA_FIRST_KARTACTION; cga <= GA_LAST_KARTACTION; cga++)
    {
        if (cga != ga)
        {
            // If the input occurs in any other mapping
            // delete it properly from there.

            if (m_input_map[cga].count
                && m_input_map[cga].inputs[0].type == i.type
                && m_input_map[cga].inputs[0].id0 == i.id0
                && m_input_map[cga].inputs[0].id1 == i.id1
                && m_input_map[cga].inputs[0].id2 == i.id2)
            {
                // Delete it.
                m_input_map[cga].inputs[0].type = Input::IT_NONE;
            }
        }
    }
}   // unsetDuplicates

// -----------------------------------------------------------------------------
void UserConfig::set(GameAction ga, const Input &i)
{
    m_input_map[ga].count = 1;
    m_input_map[ga].inputs[0] = i;
}   // set(1 input)
// -----------------------------------------------------------------------------
void UserConfig::set(GameAction ga, const Input &i0, const Input &i1)
{
    m_input_map[ga].count = 2;
    m_input_map[ga].inputs[0] = i0;
    m_input_map[ga].inputs[1] = i1;
}   // set(2 inputs)
// -----------------------------------------------------------------------------
void UserConfig::set(GameAction ga, const Input &i0, const Input &i1, const Input &i2)
{
    m_input_map[ga].count = 3;
    m_input_map[ga].inputs[0] = i0;
    m_input_map[ga].inputs[1] = i1;
    m_input_map[ga].inputs[2] = i2;
}   //set(3 inputs)
// -----------------------------------------------------------------------------
void UserConfig::set(GameAction ga, const Input &i0, const Input &i1, 
                     const Input &i2, const Input &i3)
{
    m_input_map[ga].count = 4;
    m_input_map[ga].inputs[0] = i0;
    m_input_map[ga].inputs[1] = i1;
    m_input_map[ga].inputs[2] = i2;
    m_input_map[ga].inputs[3] = i3;
}   // set(4 inputs)

// -----------------------------------------------------------------------------
void UserConfig::setInput(GameAction ga, const Input &input)
{
    // Removes the input from all mappings where it occurs.
    unsetDuplicates(ga, input);

    set(ga, input);
}   // setInput

// -----------------------------------------------------------------------------
/** Determines a name for a 'input configuration' (i.e. which input action 
 *  triggers what kart action like left, right, ...). The result is the
 *  name of the gamepad, "keyboard-"+keycode for left, or "mouse" (or "" if 
 *  something else).
 *  \param player_index Player index 0, ..., max
 */
std::string UserConfig::getInputDeviceName(int player_index) const
{
    std::string config_name;
    const Input &left_input = getInput(player_index, KA_LEFT);
    switch(left_input.type)
    {
    case Input::IT_KEYBOARD    : {   // config name: keyboard+player_index
                                     std::ostringstream s;
                                     s<<"keyboard-"<<left_input.id0;
                                     config_name = s.str();
                                     break;
                                 }
    case Input::IT_STICKBUTTON :
    case Input::IT_STICKHAT    :
    case Input::IT_STICKMOTION : config_name=m_stickconfigs[left_input.id0]->m_id;
                                 break;
    case Input::IT_MOUSEBUTTON :
    case Input::IT_MOUSEMOTION : config_name="mouse"; break;
    default                    : config_name="";      break;
    }   // switch left_input.type
    return config_name;
}   // getInputDeviceName

// -----------------------------------------------------------------------------
void UserConfig::setInput(int player_index, KartAction ka, const Input &input)
{
    setInput((GameAction) (GA_FIRST_KARTACTION
                            + player_index * KC_COUNT + ka),
             input);

    // Now save the (complete) current configuration as the last used
    // configuration for the input device that is defined for the left
    // action. First determine the name of the device, and then
    // copy the configuration.
    std::string device_name = getInputDeviceName(player_index);
    if(device_name.size()==0) return;

    // If left is set, grab the corresponding last used configuration:
    if(ka==KA_LEFT)
    {
        if(m_last_input_configuration.find(device_name)!=m_last_input_configuration.end())
        {
            for(int i=KA_FIRST; i<=KA_LAST; i++)
            {
                // Don't use the stored value for KA_LEFT, otherwise it's not possible
                // to change the left axis for a gamepad anymore.
                if(i==KA_LEFT) continue;
                Input last_inp = m_last_input_configuration[device_name].m_input[(KartAction)i];
                if(last_inp.type==Input::IT_STICKBUTTON ||
                   last_inp.type==Input::IT_STICKHAT    ||
                   last_inp.type==Input::IT_STICKMOTION    )
                {   // in case of joystick, adjust the joystick index - in case
                    // that there are two joystickts with the same name
                    last_inp.id0 = input.id0;
                }
                setInput((GameAction) (GA_FIRST_KARTACTION
                            + player_index * KC_COUNT + i),
                          last_inp                           );
            }   // for i=KA_FIRST, KA_LAST
        }   // if device_name found in last_input_configuration
    }
    else
    {
        // We change an entry (but not left) --> save as new 'last configuration'
        for(int i=KA_FIRST; i<=KA_LAST; i++)
        {
            m_last_input_configuration[device_name].m_input[(KartAction)i] =
                getInput(player_index, (KartAction)i);
        }   // for i=KA_FIRST, KA_LAST
    }   // ka!=KA_LEFT
}   // setInput

// -----------------------------------------------------------------------------
/** Returns a reference to the first entry in the user's input mapping.
 *  \param player_index Index of player (starting with zero)
 *  \param ka           Kart action for which the input is requested.
 */
const Input &UserConfig::getInput(int player_index, KartAction ka) const
{
    return m_input_map[(GameAction) (GA_FIRST_KARTACTION
                        + player_index * KC_COUNT + ka)].inputs[0];
}   // getInput

// -----------------------------------------------------------------------------
void UserConfig::clearInput(int playerIndex, KartAction ka)
{
    m_input_map[(GameAction) (GA_FIRST_KARTACTION + playerIndex * KC_COUNT + ka)]
    .count = 0;
}   // clearInput

// -----------------------------------------------------------------------------
ActionMap *UserConfig::newActionMap(const int from, const int to)
{
    ActionMap *am = new ActionMap();

    for (int i = from; i <= to; i++)
    {
        const int count = m_input_map[i].count;
        for (int j = 0;j < count; j++)
            am->putEntry(m_input_map[i].inputs[j], (GameAction) i);
    }

    return am;
}   // newActionMap

// -----------------------------------------------------------------------------
ActionMap *UserConfig::newMenuActionMap()
{
    return newActionMap(GA_FIRST_MENU, GA_LAST_MENU);
}   // newMenuActionMap

// -----------------------------------------------------------------------------
ActionMap *UserConfig::newIngameActionMap()
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

    switch (race_manager->getNumLocalPlayers())
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
        const int count = m_input_map[i].count;
        for (int j = 0;j < count; j++)
            am->putEntry(m_input_map[i].inputs[j], (GameAction) i);
    }

    return am;
}   // newIngameActionMap

// -----------------------------------------------------------------------------
/** Determines whether the given Input is used in a mapping where it is marked
  * as fixed. This allows the input driver to discard the mapping and not
  * allow the user to use it.
  */
bool UserConfig::isFixedInput(Input::InputType type, int id0, int id1, int id2)
{
    for (int i = GA_FIRST_INGAME_FIXED; i <= GA_LAST_INGAME_FIXED; i++)
    {
        const int count = m_input_map[i].count;
        for (int j = 0;j < count; j++)
            if (m_input_map[i].inputs[j].type == type
                && m_input_map[i].inputs[j].id0 == id0
                && m_input_map[i].inputs[j].id1 == id1
                && m_input_map[i].inputs[j].id2 == id2)
                return true;
    }

    return false;
}   // isFixedInput
