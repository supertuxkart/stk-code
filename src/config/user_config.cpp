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

// ---------------------------------------------------------------------------------------

GroupUserConfigParam::GroupUserConfigParam(const char* groupName, const char* comment)
{
    this->paramName = groupName;
    all_params.push_back(this);
    if(comment != NULL) this->comment = comment;
}
void GroupUserConfigParam::write(std::ofstream& stream) const
{
    if(comment.size() > 0) stream << "    <!-- " << comment.c_str() << " -->\n";
    stream << "    <" << paramName << "\n";
    
    const int children_amount = m_children.size();
    for(int n=0; n<children_amount; n++)
    {
        stream << "        " << m_children[n]->paramName << "=\"" << m_children[n]->toString() << "\"\n";
    }
    stream << "        />\n\n";
}

void GroupUserConfigParam::readAsNode(const XMLNode* node)
{
    const XMLNode* child = node->getNode( paramName );
    if(child == NULL)
    {
        //std::cout << "Couldn't find parameter group " << paramName << std::endl;
        return;
    }
    
    const int children_amount = m_children.size();
    for(int n=0; n<children_amount; n++)
    {
        m_children[n]->readAsProperty(child);
    }
    
}
void GroupUserConfigParam::readAsProperty(const XMLNode* node)
{
}
std::string GroupUserConfigParam::toString() const
{
    return "";
}
void GroupUserConfigParam::addChild(UserConfigParam* child)
{
    m_children.push_back(child);
}

// ---------------------------------------------------------------------------------------

IntUserConfigParam::IntUserConfigParam(int defaultValue, const char* paramName, const char* comment)
{
    this->value = defaultValue;
    this->paramName = paramName;
    all_params.push_back(this);
    if(comment != NULL) this->comment = comment;
}
IntUserConfigParam::IntUserConfigParam(int defaultValue, const char* paramName,
                                       GroupUserConfigParam* group, const char* comment)
{
    this->value = defaultValue;
    this->paramName = paramName;
    group->addChild(this);
    if(comment != NULL) this->comment = comment;
}

void IntUserConfigParam::write(std::ofstream& stream) const
{
    if(comment.size() > 0) stream << "    <!-- " << comment.c_str() << " -->\n";
    stream << "    <" << paramName << " value=\"" << value << "\" />\n\n";
}

std::string IntUserConfigParam::toString() const
{
    char buffer[16];
    sprintf(buffer, "%i", value);
    return buffer;
}

void IntUserConfigParam::readAsNode(const XMLNode* node)
{
    const XMLNode* child = node->getNode( paramName );
    if(child == NULL)
    {
        //std::cout << "Couldn't find int parameter " << paramName << std::endl;
        return;
    }
    
    child->get( "value", &value );
    //std::cout << "read int " << paramName << ", value=" << value << std::endl;
}
void IntUserConfigParam::readAsProperty(const XMLNode* node)
{
    node->get( paramName, &value );
}

// ---------------------------------------------------------------------------------------

StringUserConfigParam::StringUserConfigParam(const char* defaultValue, const char* paramName, const char* comment)
{
    this->value = defaultValue;
    this->paramName = paramName;
    all_params.push_back(this);
    if(comment != NULL) this->comment = comment;
}
StringUserConfigParam::StringUserConfigParam(const char* defaultValue, const char* paramName,
                                             GroupUserConfigParam* group, const char* comment)
{
    this->value = defaultValue;
    this->paramName = paramName;
    group->addChild(this);
    if(comment != NULL) this->comment = comment;
}


void StringUserConfigParam::write(std::ofstream& stream) const
{
    if(comment.size() > 0) stream << "    <!-- " << comment.c_str() << " -->\n";
    stream << "    <" << paramName << " value=\"" << value << "\" />\n\n";
}
void StringUserConfigParam::readAsNode(const XMLNode* node)
{
    const XMLNode* child = node->getNode( paramName );
    if(child == NULL) return;
    
    child->get( "value", &value );
}
void StringUserConfigParam::readAsProperty(const XMLNode* node)
{
    node->get( paramName, &value );
}

std::string StringUserConfigParam::toString() const
{
    return value;
}

// ---------------------------------------------------------------------------------------

BoolUserConfigParam::BoolUserConfigParam(bool defaultValue, const char* paramName, const char* comment)
{
    this->value = defaultValue;
    this->paramName = paramName;
    all_params.push_back(this);
    if(comment != NULL) this->comment = comment;
}
BoolUserConfigParam::BoolUserConfigParam(bool defaultValue, const char* paramName,
                                         GroupUserConfigParam* group, const char* comment)
{
    this->value = defaultValue;
    this->paramName = paramName;
    group->addChild(this);
    if(comment != NULL) this->comment = comment;
}


void BoolUserConfigParam::write(std::ofstream& stream) const
{
    if(comment.size() > 0) stream << "    <!-- " << comment.c_str() << " -->\n";
    stream << "    <" << paramName << " value=\"" << (value ? "true" : "false" ) << "\" />\n\n";
}
void BoolUserConfigParam::readAsNode(const XMLNode* node)
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
void BoolUserConfigParam::readAsProperty(const XMLNode* node)
{
    std::string textValue = "";
    node->get( paramName, &textValue );
    
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

std::string BoolUserConfigParam::toString() const
{
    return (value ? "true" : "false" );
}


// ---------------------------------------------------------------------------------------

FloatUserConfigParam::FloatUserConfigParam(float defaultValue, const char* paramName, const char* comment)
{
    this->value = defaultValue;
    this->paramName = paramName;
    all_params.push_back(this);
    if(comment != NULL) this->comment = comment;
}
FloatUserConfigParam::FloatUserConfigParam(float defaultValue, const char* paramName,
                                           GroupUserConfigParam* group, const char* comment)
{
    this->value = defaultValue;
    this->paramName = paramName;
    group->addChild(this);
    if(comment != NULL) this->comment = comment;
}

void FloatUserConfigParam::write(std::ofstream& stream) const
{
    if(comment.size() > 0) stream << "    <!-- " << comment.c_str() << " -->\n";
    stream << "    <" << paramName << " value=\"" << value << "\" />\n\n";
}
void FloatUserConfigParam::readAsNode(const XMLNode* node)
{
    const XMLNode* child = node->getNode( paramName );
    if(child == NULL) return;
    
    child->get( "value", &value );
}
void FloatUserConfigParam::readAsProperty(const XMLNode* node)
{
    node->get( paramName, &value );
}

std::string FloatUserConfigParam::toString() const
{
    char buffer[16];
    sprintf(buffer, "%f", value);
    return buffer;
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
   
}   // CheckAndCreateDir

// -----------------------------------------------------------------------------
/** Load configuration values from file. */
void UserConfig::loadConfig(const std::string& filename)
{
    
    XMLNode* root = file_manager->createXMLTree(filename);
    if(!root)
    {
        std::cerr << "Could not read user config file file " << filename.c_str() << std::endl;
        return;
    }

    // ---- Read config file version
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
    
    
    // ---- Read all other parameters
    const int paramAmount = all_params.size();
    for(int i=0; i<paramAmount; i++)
    {
        all_params[i]->readAsNode(root);
    }
    
    delete root;
    
}   // loadConfig

// -----------------------------------------------------------------------------
/** Write settings to config file. */
void UserConfig::saveConfig(const std::string& filepath)
{
    // TODO : save challenges state
    
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
