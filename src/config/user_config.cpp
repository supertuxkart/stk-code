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
#include <stdlib.h>
#include <fstream>

// for mkdir:
#if !defined(WIN32) || defined(__CYGWIN__)
#  include <sys/stat.h>
#  include <sys/types.h>
#else
#  include <direct.h>
#endif

#include "challenges/unlock_manager.hpp"
#include "config/stk_config.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "lisp/writer.hpp"
#include "race/race_manager.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"


class UserConfigParam;
static std::vector<UserConfigParam*> all_params;


// X-macros
#define PARAM_PREFIX
#define PARAM_DEFAULT(X) = X
#include "config/user_config.hpp"

IntUserConfigParam::IntUserConfigParam(int defaultValue, const char* paramName)
{
    this->value = defaultValue;
    this->paramName = paramName;
    all_params.push_back(this);
}

void IntUserConfigParam::write(std::ofstream& stream) const
{
    stream << "    <" << paramName << " value=\"" << value << "\" />\n";
}

void IntUserConfigParam::read(const XMLNode* node)
{
    const XMLNode* child = node->getNode( paramName );
    if(child == NULL) return;
    
    child->get( "value", &value );
}

StringUserConfigParam::StringUserConfigParam(const char* defaultValue, const char* paramName)
{
    this->value = defaultValue;
    this->paramName = paramName;
    all_params.push_back(this);
}
void StringUserConfigParam::write(std::ofstream& stream) const
{
    stream << "    <" << paramName << " value=\"" << value << "\" />\n";
}
void StringUserConfigParam::read(const XMLNode* node)
{
    const XMLNode* child = node->getNode( paramName );
    if(child == NULL) return;
    
    child->get( "value", &value );
}

BoolUserConfigParam::BoolUserConfigParam(bool defaultValue, const char* paramName)
{
    this->value = defaultValue;
    this->paramName = paramName;
    all_params.push_back(this);

}
void BoolUserConfigParam::write(std::ofstream& stream) const
{
    stream << "    <" << paramName << " value=\"" << (value ? "true" : "false" )<< "\" />\n";
}
void BoolUserConfigParam::read(const XMLNode* node)
{
    const XMLNode* child = node->getNode( paramName );
    if(child == NULL) return;
    
    std::string textValue = "";
    child->get( "value", &textValue );
    
    if(textValue == "true")
    {
        value = true;
    }
    else if(textValue == "false")
    {
        value = false;
    }
    else
        std::cerr << "Unknown value for " << paramName << "; expected true or false\n";
}


FloatUserConfigParam::FloatUserConfigParam(bool defaultValue, const char* paramName)
{
    this->value = defaultValue;
    this->paramName = paramName;
    all_params.push_back(this);

}
void FloatUserConfigParam::write(std::ofstream& stream) const
{
    stream << "    <" << paramName << " value=\"" << value << "\" />\n";
}
void FloatUserConfigParam::read(const XMLNode* node)
{
    const XMLNode* child = node->getNode( paramName );
    if(child == NULL) return;
    
    child->get( "value", &value );
}

// =====================================================================================
// =====================================================================================

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
    //m_blacklist_res.clear();

    std::string username = "unnamed player";
    
    if(getenv("USERNAME")!=NULL)        // for windows
        username = getenv("USERNAME");
    else if(getenv("USER")!=NULL)       // Linux, Macs
        username = getenv("USER");
    else if(getenv("LOGNAME")!=NULL)    // Linux, Macs
        username = getenv("LOGNAME");


    std::cout << "creating Players with name " << username.c_str() << std::endl;
    
    // Set the name as the default name for all players.
    // TODO : create only one by default and let users add more?
    UserConfigParams::m_player.push_back( Player(username + " 1") );
    UserConfigParams::m_player.push_back( Player(username + " 2") );
    UserConfigParams::m_player.push_back( Player(username + " 3") );
    UserConfigParams::m_player.push_back( Player(username + " 4") );
    

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
 * Checks for existance of the STK configuration directory. If the
 * directory does not exist, it will be created. Return values:
 * 1: config dir exists
 * 2: does not exist, but was created
 * 0: does not exist, and could not be created.
 */
int UserConfig::CheckAndCreateDir()
{
    // the standard C/C++ libraries don't include anything allowing to check
    // for directory existance. I work around this by checking if trying to 
    // check for the config file (first reading, then writing)
    
    const std::string filename = file_manager->getHomeDir() + "/config";
    
    std::ofstream test(filename.c_str(), std::ios::in);
    
    if(test.fail() || !test.is_open())
    {
        std::ofstream test2(filename.c_str(), std::ios::out);
        
        if(test2.fail() || !test2.is_open())
        {
            int bError;
#if defined(WIN32) && !defined(__CYGWIN__)
            bError = _mkdir(file_manager->getHomeDir().c_str()      ) != 0;
#else
            bError = mkdir(file_manager->getHomeDir().c_str(), 0755) != 0;
#endif
            if(bError)
            {
                fprintf(stderr, "Couldn't create '%s', config files will not be saved.\n",
                        file_manager->getHomeDir().c_str());
                return 0;
            }
            else
            {
                printf("Config directory '%s' successfully created.\n", file_manager->getHomeDir().c_str());
                return 2;
            }
        }
        if(test2.is_open()) test2.close();
        
    }
    if(test.is_open()) test.close();
    return 1;
    
    /*
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
     */

}   // CheckAndCreateDir

// -----------------------------------------------------------------------------
/** Load configuration values from file. */
// TODO : implement
void UserConfig::loadConfig(const std::string& filename)
{
    
    XMLNode* root = file_manager->createXMLTree(filename);
    if(!root)
    {
        std::cerr << "Could not read user config file file " << filename.c_str() << std::endl;
        return;
    }
    
    const XMLNode* node = root;
    //const XMLNode* node = root->getNode("stkconfig");
    //if(node == NULL)
    //{
    //    std::cerr << "Error, malformed user config file! Contains no <stkconfig> tag!\n";
    //    return;
    //}
    
    
    int configFileVersion = CURRENT_CONFIG_VERSION;
    if(!root->get("version", &configFileVersion) < 1)
    {
        std::cerr << "Warning, malformed user config file! Contains no version\n";
    }
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
            case 2:  printf("Added username.\n");
                needToAbort=std::max(needToAbort,0);
            case 3:  printf("Added username for all players.\n");
                needToAbort=std::max(needToAbort,0);
            case 4:  printf("Added jumping, which invalidates all key bindings.\n");
                needToAbort=std::max(needToAbort,0);
            case 6:  printf("Added nitro and drifting, removed jumping and wheelie.\n");
                //nitro_name="wheelie";
                //drift_name="jump";
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
    
    
    const int paramAmount = all_params.size();
    for(int i=0; i<paramAmount; i++)
    {
        all_params[i]->read(node);
    }
    
    
#if 0
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
        // TODO - input I/O
        // readLastInputConfigurations(NULL);

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
            case 2:  printf("Added username, using: '%s'.\n", UserConfigParams::m_username.c_str());
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
        lisp->get("music-volume",     m_music_volume);
        
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
        //readStickConfigs(lisp);

        // Address of server
        lisp->get("server-address",   m_server_address);
        lisp->get("server-port",      m_server_port);

        // Unlock information:
        const lisp::Lisp* unlock_info = lisp->getLisp("unlock-info");
        if(unlock_info) unlock_manager->load(unlock_info);

        /*get player configurations*/
        // TODO : save/load players to/from file
        /*
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
            // TODO - input config I/O
            //for(int ka=PA_FIRST+1; ka<PA_COUNT; ka++)
            //{
            //   readPlayerInput(reader, KartActionStrings[ka],
            //        (KartAction)ka, i);
            //}

            // Leave those in for backwards compatibility (i.e. config files
            // with jump/wheelie). If jump/wheelie are not defined, nothing
            // happens (the same input is read again).
            //readPlayerInput(reader, nitro_name, KA_NITRO, i);
            //readPlayerInput(reader, drift_name, KA_DRIFT, i);
        }   // for i < PLAYERS
         */

        // Read the last input device configurations. It is important that this
        // happens after reading the player config, since if no last config
        // is given, the last config is initialised with the current player
        // config.
        // TODO - input I/O
        // readLastInputConfigurations(lisp);
    }
    catch(std::exception& e)
    {
        fprintf(stderr, "Error while parsing config '%s':\n", filename.c_str());
        fprintf(stderr, "%s", e.what());
        fprintf(stderr, "\n");
    }

    delete root;
#endif
}   // loadConfig

// -----------------------------------------------------------------------------
/** Write settings to config file. */
void UserConfig::saveConfig(const std::string& filepath)
{
    const int DIR_EXIST = CheckAndCreateDir();
    // Check if the config directory exists (again, since it was already checked
    // when reading the config file - this is done in case that the problem was
    // fixed while tuxkart is running). If the directory does not exist and
    // can not be created, an error message was already printed to stderr,
    // and we can exit here without any further messages.
    if (DIR_EXIST == 0)
    {
        std::cerr << "User config firectory does not exist, cannot save config file!\n";
        return;
    }

    std::ofstream configfile;
    configfile.open (filepath.c_str());
    
    if(!configfile.is_open())
    {
        std::cerr << "Failed to open " << filepath.c_str() << " for writing, user config won't be saved\n";
        return;
    }
    
    configfile << "<stkconfig version=\"" << CURRENT_CONFIG_VERSION << "\" >\n";

    const int paramAmount = all_params.size();
    for(int i=0; i<paramAmount; i++)
    {
        //std::cout << "saving parameter " << i << " to file\n";
        all_params[i]->write(configfile);
    }
    configfile << "</stkconfig>\n";
    configfile.close();  
    
}   // saveConfig
