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



//#include <stdio.h>
//#include <stdexcept>
//#include <sstream>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include "utils/ptr_vector.hpp"

class UserConfigParam;
static ptr_vector<UserConfigParam, REF> all_params;


// X-macros
#define PARAM_PREFIX
#define PARAM_DEFAULT(X) = X
#include "config/user_config.hpp"

#include "config/player.hpp"
#include "config/stk_config.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "race/race_manager.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

// ---------------------------------------------------------------------------------------

UserConfigParam::~UserConfigParam()
{
    all_params.remove(this);
}

GroupUserConfigParam::GroupUserConfigParam(const char* groupName, const char* comment)
{
    this->paramName = groupName;
    all_params.push_back(this);
    if(comment != NULL) this->comment = comment;
}
void GroupUserConfigParam::write(std::ofstream& stream) const
{
    const int children_amount = m_children.size();

    // comments
    if(comment.size() > 0) stream << "    <!-- " << comment.c_str();
    for(int n=0; n<children_amount; n++)
    {
        if(m_children[n]->comment.size() > 0)
            stream << "\n             " << m_children[n]->paramName << " : " << m_children[n]->comment.c_str();
    }


    stream << " -->\n    <" << paramName << "\n";

    // actual values
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

#if 0
#pragma mark -
#pragma mark UserConfig
#endif

UserConfig *user_config;

UserConfig::UserConfig()
{
    m_filename = "config.xml";
    m_warning  = "";
    //m_blacklist_res.clear();
    if(!loadConfig() || UserConfigParams::m_all_players.size() == 0)
    {
        addDefaultPlayer();
        saveConfig();
    }
}   // UserConfig

// -----------------------------------------------------------------------------
UserConfig::~UserConfig()
{
}   // ~UserConfig

// -----------------------------------------------------------------------------
void UserConfig::addDefaultPlayer()
{

    std::string username = "unnamed player";

    if(getenv("USERNAME")!=NULL)        // for windows
        username = getenv("USERNAME");
    else if(getenv("USER")!=NULL)       // Linux, Macs
        username = getenv("USER");
    else if(getenv("LOGNAME")!=NULL)    // Linux, Macs
        username = getenv("LOGNAME");

    
    class GuestPlayerProfile : public PlayerProfile
    {
    public:
        GuestPlayerProfile() : PlayerProfile("Guest")
        {
            m_is_guest_account = true;
        }
    };
    
    // add default guest player
    UserConfigParams::m_all_players.push_back( new GuestPlayerProfile() );

    // Set the name as the default name for all players.
    UserConfigParams::m_all_players.push_back( new PlayerProfile(username.c_str()) );

}

// -----------------------------------------------------------------------------
/** Load configuration values from file. */
bool UserConfig::loadConfig()
{
    const std::string filename = file_manager->getConfigDir()+"/"+m_filename;
    XMLNode* root = file_manager->createXMLTree(filename);
    if(!root || root->getName() != "stkconfig")
    {
        std::cerr << "Could not read user config file file " << filename.c_str() << std::endl;
        return false;
    }

    // ---- Read config file version
    int configFileVersion = CURRENT_CONFIG_VERSION;
    if(root->get("version", &configFileVersion) < 1)
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
            return false;
        }
        printf("This warning can be ignored, the config file will be automatically updated.\n");
        // Keep on reading the config files as far as possible
    }   // if configFileVersion<SUPPORTED_CONFIG_VERSION

    // ---- Read all other parameters
    const int paramAmount = all_params.size();
    for(int i=0; i<paramAmount; i++)
    {
        all_params[i].readAsNode(root);
    }

    // ---- Read players
    UserConfigParams::m_all_players.clearAndDeleteAll();

    std::vector<XMLNode*> players;
    root->getNodes("Player", players);
    const int amount = players.size();
    for(int i=0; i<amount; i++)
    {
        std::string name;
        players[i]->get("name", &name);
        UserConfigParams::m_all_players.push_back( new PlayerProfile(name.c_str()) );
    }    
    delete root;
    return true;
}   // loadConfig

// -----------------------------------------------------------------------------
/** Write settings to config file. */
void UserConfig::saveConfig()
{
    const std::string dir = file_manager->getConfigDir();
    if(dir=="")
    {
        std::cerr << "User config firectory does not exist, cannot save config file!\n";
        return;
    }

    const std::string filename = dir + "/" + m_filename;

    std::ofstream configfile;
    configfile.open (filename.c_str());

    if(!configfile.is_open())
    {
        std::cerr << "Failed to open " << filename.c_str() << " for writing, user config won't be saved\n";
        return;
    }

    configfile << "<?xml version=\"1.0\"?>\n";
    configfile << "<stkconfig version=\"" << CURRENT_CONFIG_VERSION << "\" >\n\n";

    const int paramAmount = all_params.size();
    for(int i=0; i<paramAmount; i++)
    {
        //std::cout << "saving parameter " << i << " to file\n";
        all_params[i].write(configfile);
    }
        
    configfile << "</stkconfig>\n";
    configfile.close();

}   // saveConfig
