// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 SuperTuxKart-Team
//  Modelled after Supertux's configfile.cpp
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

// for mkdir:
#if !defined(WIN32) || defined(__CYGWIN__)
#  include <sys/stat.h>
#  include <sys/types.h>
#else
#  include <direct.h>
#endif

#include <SDL/SDL.h>
#include <plib/ul.h>

#include "user_config.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "lisp/writer.hpp"
#include "translation.hpp"

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
{}

// -----------------------------------------------------------------------------
std::string UserConfig::getConfigDir()
{
    std::string DIRNAME;
#ifdef WIN32
    // For now the old windows config way is used: store a config file
    // in the current directory (in other OS a special subdirectory is created)
    DIRNAME = ".";
#else
    if(getenv("HOME")!=NULL)
    {
        DIRNAME = getenv("HOME");
    }
    else
    {
        DIRNAME = ".";
    }
    DIRNAME += "/";
    DIRNAME += CONFIGDIR;
#endif
    return DIRNAME;
}  // getConfigDir

// -----------------------------------------------------------------------------
/**
 * Set the config filename for each platform
 */
void UserConfig::setFilename()
{
    filename = getConfigDir();
    filename += "/";
#ifdef WIN32
    filename += "supertuxkart.cfg";
#else
    filename += "config";
#endif
}   // setFilename

// -----------------------------------------------------------------------------
/**
 * Load default values for options
 */
void UserConfig::setDefaults()
{
    setFilename();
    m_fullscreen       = false;
    m_no_start_screen  = false;
    m_sfx              = true;
    m_music            = true;
    m_smoke            = false;
    m_display_fps      = false;
    m_herring_style    = "new";
    m_disable_magnet   = true;
    m_profile          = 0;
    m_use_kph          = false;
    m_improved_physics = false;
    m_replay_history   = false;
    m_width            = 800;
    m_height           = 600;
    m_karts            = 4;

    
    for(int i=0; i<4; i++) 
    {
        char pl[255];
        snprintf(pl, sizeof(pl), _("Player %d"), i+1);
        m_player[i].setName(pl);
    }


    /*player 1 default keyboard settings*/
    m_player[0].setInput(KC_LEFT,   IT_KEYBOARD, SDLK_LEFT,      0, 0);
    m_player[0].setInput(KC_RIGHT,  IT_KEYBOARD, SDLK_RIGHT,     0, 0);
    m_player[0].setInput(KC_ACCEL,  IT_KEYBOARD, SDLK_UP,        0, 0);
    m_player[0].setInput(KC_BRAKE,  IT_KEYBOARD, SDLK_DOWN,      0, 0);
    m_player[0].setInput(KC_WHEELIE,IT_KEYBOARD, SDLK_RSHIFT,    0, 0);
    m_player[0].setInput(KC_JUMP,   IT_KEYBOARD, SDLK_MINUS,     0, 0);
    m_player[0].setInput(KC_RESCUE, IT_KEYBOARD, SDLK_BACKSPACE, 0, 0);
    m_player[0].setInput(KC_FIRE,   IT_KEYBOARD, SDLK_RCTRL,     0, 0);

    /*player 2 default keyboard settings*/
    m_player[1].setInput(KC_LEFT,   IT_KEYBOARD, SDLK_a,         0, 0);
    m_player[1].setInput(KC_RIGHT,  IT_KEYBOARD, SDLK_d,         0, 0);
    m_player[1].setInput(KC_ACCEL,  IT_KEYBOARD, SDLK_w,         0, 0);
    m_player[1].setInput(KC_BRAKE,  IT_KEYBOARD, SDLK_s,         0, 0);
    m_player[1].setInput(KC_WHEELIE,IT_KEYBOARD, SDLK_LSHIFT,    0, 0);
    m_player[1].setInput(KC_JUMP,   IT_KEYBOARD, SDLK_CAPSLOCK,  0, 0);
    m_player[1].setInput(KC_RESCUE, IT_KEYBOARD, SDLK_LALT,      0, 0);
    m_player[1].setInput(KC_FIRE,   IT_KEYBOARD, SDLK_LCTRL,     0, 0);

    /*player 3 default keyboard settings*/
    m_player[2].setInput(KC_LEFT,   IT_KEYBOARD, SDLK_f,         0, 0);
    m_player[2].setInput(KC_RIGHT,  IT_KEYBOARD, SDLK_h,         0, 0);
    m_player[2].setInput(KC_ACCEL,  IT_KEYBOARD, SDLK_t,         0, 0);
    m_player[2].setInput(KC_BRAKE,  IT_KEYBOARD, SDLK_g,         0, 0);
    m_player[2].setInput(KC_WHEELIE,IT_KEYBOARD, SDLK_c,         0, 0);
    m_player[2].setInput(KC_JUMP,   IT_KEYBOARD, SDLK_v,         0, 0);
    m_player[2].setInput(KC_RESCUE, IT_KEYBOARD, SDLK_b,         0, 0);
    m_player[2].setInput(KC_FIRE,   IT_KEYBOARD, SDLK_n,         0, 0);

    /*player 4 default keyboard settings*/
    m_player[3].setInput(KC_LEFT,   IT_KEYBOARD, SDLK_j,         0, 0);
    m_player[3].setInput(KC_RIGHT,  IT_KEYBOARD, SDLK_l,         0, 0);
    m_player[3].setInput(KC_ACCEL,  IT_KEYBOARD, SDLK_i,         0, 0);
    m_player[3].setInput(KC_BRAKE,  IT_KEYBOARD, SDLK_k,         0, 0);
    m_player[3].setInput(KC_WHEELIE,IT_KEYBOARD, SDLK_m,         0, 0);
    m_player[3].setInput(KC_JUMP,   IT_KEYBOARD, SDLK_COMMA,     0, 0);
    m_player[3].setInput(KC_RESCUE, IT_KEYBOARD, SDLK_PERIOD,    0, 0);
    m_player[3].setInput(KC_FIRE,   IT_KEYBOARD, SDLK_SLASH,     0, 0);
}   // setDefaults


// -----------------------------------------------------------------------------
/**
 * load default configuration file for this platform
 */
void UserConfig::loadConfig()
{
    loadConfig(filename);
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
    const std::string DIRNAME = getConfigDir();
    ulDir*      u       = ulOpenDir(DIRNAME.c_str());
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
        fprintf(stderr, _("Couldn't create '%s', config files will not be saved.\n"),
                DIRNAME.c_str());
        return 0;
    }
    else
    {
        printf(_("Config directory '%s' successfully created.\n"),DIRNAME.c_str());
        return 2;
    }

}   // CheckAndCreateDir

// -----------------------------------------------------------------------------
/** Load configuration values from file. */
void UserConfig::loadConfig(const std::string& filename)
{
    std::string temp;
    const lisp::Lisp* root = 0;
    int i;
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
        printf(_("Config file '%s' does not exist, it will be created.\n"), 
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
            snprintf(msg, sizeof(msg), _("No tuxkart-config node"));
            throw std::runtime_error(msg);
        }
        int configFileVersion = 0;
        lisp->get("configFileVersion", configFileVersion);
        if (configFileVersion < SUPPORTED_CONFIG_VERSION)
        {
            // Give some feedback to the user about what was changed.
            // Do NOT add a break after the case, so that all changes will be printed
            printf(_("\nConfig file version '%d' is too old.\n"
                     "The following changes have been applied in the current SuperTuxKart version:\n"),
                   configFileVersion);
            int needToAbort=0;
            switch(configFileVersion)
            {
            case 0:  printf(_("- Single window menu, old status display,new keyboard style settings were removed\n"));
                needToAbort=0;
            case 1:  printf(_("- Key bindings were changed, please check the settings. All existing values were discarded.\n"));
                needToAbort=1;  // if the old keybinds were read, they wouldn't make any sense
            case 99: break;
            default: printf(_("Config file version '%d' is too old. Discarding your configuration. Sorry. :(\n"), configFileVersion);
                     break;
            }
            if(needToAbort)
            {
                delete root;
                return;
            }
            printf(_("This warning can be ignored.\n"));
            // Keep on reading the config files as far as possible
        }   // if configFileVersion<SUPPORTED_CONFIG_VERSION

        /*get toggles*/
        lisp->get("fullscreen",       m_fullscreen);
        lisp->get("sfx" ,             m_sfx);
        lisp->get("nostartscreen",    m_no_start_screen);
        lisp->get("music",            m_music);
        lisp->get("smoke",            m_smoke);
        lisp->get("displayFPS",       m_display_fps);
        lisp->get("herringStyle",     m_herring_style);
        lisp->get("disableMagnet",    m_disable_magnet);
        lisp->get("useKPH",           m_use_kph);
        lisp->get("improvedPhysics",  m_improved_physics);

        /*get resolution width/height*/
        lisp->get("width",            m_width);
        lisp->get("height",           m_height);

        /*get number of karts*/
        lisp->get("karts", m_karts);

        /*get player configurations*/
        for(i=0; i<PLAYERS; ++i)
        {
            temp = "player-";
            temp += i+'1';

            const lisp::Lisp* reader = lisp->getLisp(temp);
            if(!reader)
            {
                char msg[MAX_ERROR_MESSAGE_LENGTH];
                snprintf(msg, sizeof(msg), _("No '%s' node"), temp.c_str());
                throw std::runtime_error(msg);
            }
            std::string name;
            reader->get("name", name);
            m_player[i].setName(name);

            int lastKartId = 0;
            reader->get("lastKartId", lastKartId);
            m_player[i].setLastKartId(lastKartId);

            // Retrieves a player's INPUT configuration
            readInput(reader, "left", KC_LEFT, m_player[i]);
            readInput(reader, "right", KC_RIGHT, m_player[i]);
            readInput(reader, "accel", KC_ACCEL, m_player[i]);
            readInput(reader, "brake", KC_BRAKE, m_player[i]);

            readInput(reader, "wheelie", KC_WHEELIE, m_player[i]);
            readInput(reader, "jump", KC_JUMP, m_player[i]);
            readInput(reader, "rescue", KC_RESCUE, m_player[i]);
            readInput(reader, "fire", KC_FIRE, m_player[i]);
        }
    }
    catch(std::exception& e)
    {
        fprintf(stderr, _("Error while parsing config '%s':\n"), filename.c_str());
        fprintf(stderr,  e.what());
        fprintf(stderr, "\n");
    }
    delete root;
}   // loadConfig

// -----------------------------------------------------------------------------
void UserConfig::readInput(const lisp::Lisp* &r,
                       const char *node,
                       KartActions action,
                       Player& player)
{
    std::string inputTypeName;
    const lisp::Lisp* subReader = r->getLisp(node);
    InputType it=IT_KEYBOARD;
    // Every unused id variable *must* be set to
    // something different than -1. Otherwise
    // the restored mapping will not be applied
    // to the player.
    int id0 = -1, id1 = -1, id2 = -1;

    subReader->get("type", inputTypeName);
    if (inputTypeName == "keyboard")
    {
        it = IT_KEYBOARD;
        subReader->get("key", id0);
        id1 = id2 = 0;
    }
    else if (inputTypeName == "stickaxis")
    {
        it = IT_STICKMOTION;
        subReader->get("stick", id0);
        subReader->get("axis", id1);
        subReader->get("direction", id2);
    }
    else if (inputTypeName == "stickbutton")
    {
        it = IT_STICKBUTTON;
        subReader->get("stick", id0);
        subReader->get("button", id1);
        id2 = 0;
    }
    else if (inputTypeName == "stickhat")
    {
        it = IT_STICKHAT;
        // TODO: Implement me
    }
    else if (inputTypeName == "mouseaxis")
    {
        it = IT_MOUSEMOTION;
        subReader->get("axis", id0);
        subReader->get("direction", id1);
        id2 = 0;
    }
    else if (inputTypeName == "mousebutton")
    {
        it = IT_MOUSEBUTTON;
        subReader->get("button", id0);
        id1 = id2 = 0;
    }

    if (id0 != -1 && id1 != -1 && id2 != -1)
        player.setInput(action, it, id0, id1, id2);
}

// -----------------------------------------------------------------------------
/** Call saveConfig with the default filename for this platform. */
void UserConfig::saveConfig()
{
    saveConfig(filename);
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

    try
    {
        lisp::Writer writer(filename);

        writer.beginList("tuxkart-config");
        writer.writeComment("If the game's supported config file version is higher than this number the configuration is discarded.");
        writer.write("configFileVersion\t",   CURRENT_CONFIG_VERSION);

        writer.writeComment("the following options can be set to #t or #f:");
        writer.write("sfx\t",   m_sfx);
        writer.write("music\t", m_music);
        writer.write("smoke\t", m_smoke);
        writer.writeComment("Display frame per seconds");
        writer.write("displayFPS\t", m_display_fps);
        writer.writeComment("Name of the .herring file to use.");
        writer.write("herringStyle\t", m_herring_style);
        writer.writeComment("Allow players to disable a magnet");
        writer.write("disableMagnet\t", m_disable_magnet);
        writer.writeComment("Use of kilometers per hours (km/h) instead of mph");
        writer.write("useKPH\t", m_use_kph);
        writer.writeComment("With improved physics the gravity on a non-horizontal");
        writer.writeComment("plane will add an accelerating force on the kart");
        writer.write("improvedPhysics\t", m_improved_physics);

        writer.writeComment("screen resolution and windowing mode");
        writer.write("width\t", m_width);
        writer.write("height\t", m_height);
        writer.write("fullscreen\t", m_fullscreen);

        writer.writeComment("number of karts. -1 means use all");
        writer.write("karts\t", m_karts);

        /* write player configurations */
        for(i=0; i<PLAYERS; ++i)
        {
            temp = "player ";
            temp += i+'1';
            temp += " settings";
            writer.writeComment(temp);
            temp = "player-";
            temp += i+'1';
            writer.beginList(temp);

            writer.write("name\t", m_player[i].getName());

            writer.writeComment("optional");
            writer.write("lastKartId", m_player[i].getLastKartId());

            writeInput(writer, "left\t", KC_LEFT, m_player[i]);
            writeInput(writer, "right\t", KC_RIGHT, m_player[i]);
            writeInput(writer, "accel\t", KC_ACCEL, m_player[i]);
            writeInput(writer, "brake\t", KC_BRAKE, m_player[i]);
            writeInput(writer, "wheelie\t", KC_WHEELIE, m_player[i]);
            writeInput(writer, "jump\t", KC_JUMP, m_player[i]);
            writeInput(writer, "rescue\t", KC_RESCUE, m_player[i]);
            writeInput(writer, "fire\t", KC_FIRE, m_player[i]);

            writer.endList(temp);
        }   // for i

        writer.endList("tuxkart-config");
    }
    catch(std::exception& e)
    {
        fprintf(stderr, _("Couldn't write config: "));
        fprintf(stderr, e.what());
        fprintf(stderr, "\n");
    }
}   // saveConfig

// -----------------------------------------------------------------------------
void UserConfig::writeInput(lisp::Writer &writer, const char *node, KartActions action, Player& player)
{
    const Input *INPUT = player.getInput(action);

    writer.beginList(node);

    switch (INPUT->type)
    {
    case IT_KEYBOARD:
        writer.write("type", "keyboard");
        writer.write("key", INPUT->id0);
        break;
    case IT_STICKMOTION:
        writer.write("type", "stickaxis");
        writer.write("stick", INPUT->id0);
        writer.write("axis", INPUT->id1);
        writer.writeComment("0 is negative/left/up, 1 is positive/right/down");
        writer.write("direction", INPUT->id2);
        break;
    case IT_STICKBUTTON:
        writer.write("type", "stickbutton");
        writer.write("stick", INPUT->id0);
        writer.write("button", INPUT->id1);
        break;
    case IT_STICKHAT:
        // TODO: Implement me
        break;
    case IT_MOUSEMOTION:
        writer.write("type", "mouseaxis");
        writer.write("axis", INPUT->id0);
        writer.writeComment("0 is negative/left/up, 1 is positive/right/down");
        writer.write("direction", INPUT->id1);
        break;
    case IT_MOUSEBUTTON:
        writer.write("type", "mousebutton");
        writer.write("button", INPUT->id0);
        break;
    }

    writer.endList(node);
}

// -----------------------------------------------------------------------------
std::string UserConfig::getInputAsString(int player_index, KartActions control)
{
    const Input *INPUT         = m_player[player_index].getInput(control);
    char msg[MAX_MESSAGE_LENGTH];
    std::ostringstream stm;
    
    switch (INPUT->type)
    {
    case IT_KEYBOARD:
        snprintf(msg, sizeof(msg), _("%s"), SDL_GetKeyName((SDLKey) INPUT->id0));
        break;
    case IT_STICKMOTION:
        snprintf(msg, sizeof(msg), _("joy %d axis %d  %c"),
                 INPUT->id0, INPUT->id1, (INPUT->id2 == AD_NEGATIVE) ? '-' : '+');
        break;
    case IT_STICKBUTTON:
        snprintf(msg, sizeof(msg), _("joy %d btn %d"), INPUT->id0, INPUT->id1);
        break;
    case IT_STICKHAT:
        snprintf(msg, sizeof(msg), _("joy %d hat %d"), INPUT->id0, INPUT->id1);
        break;
    case IT_MOUSEBUTTON:
        snprintf(msg, sizeof(msg), _("mouse btn %d"), INPUT->id0);
        break;
    case IT_MOUSEMOTION:
        snprintf(msg, sizeof(msg), _("mouse axis %d %c"),
                 INPUT->id0, ((INPUT->id1 == AD_NEGATIVE) ? '-' : '+'));
        break;
    default:
        snprintf(msg, sizeof(msg), _("Invalid"));
    }
    
    stm << msg;
    
    return stm.str();
}   // GetKeyAsString

// -----------------------------------------------------------------------------

/*EOF*/
