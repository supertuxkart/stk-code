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
#include "guiengine/engine.hpp"
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

void GroupUserConfigParam::findYourDataInAChildOf(const XMLNode* node)
{
    const XMLNode* child = node->getNode( paramName );
    if (child == NULL)
    {
        //std::cerr << "/!\\ User Config : Couldn't find parameter group " << paramName << std::endl;
        return;
    }

    const int children_amount = m_children.size();
    for (int n=0; n<children_amount; n++)
    {
        m_children[n]->findYourDataInAnAttributeOf(child);
    }

}
void GroupUserConfigParam::findYourDataInAnAttributeOf(const XMLNode* node)
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
    m_value = defaultValue;
    m_default_value = defaultValue;
    
    this->paramName = paramName;
    all_params.push_back(this);
    if(comment != NULL) this->comment = comment;
}

// ---------------------------------------------------------------------------------------

IntUserConfigParam::IntUserConfigParam(int defaultValue, const char* paramName,
                                       GroupUserConfigParam* group, const char* comment)
{
    m_value = defaultValue;
    m_default_value = defaultValue;
    
    this->paramName = paramName;
    group->addChild(this);
    if(comment != NULL) this->comment = comment;
}

void IntUserConfigParam::write(std::ofstream& stream) const
{
    if(comment.size() > 0) stream << "    <!-- " << comment.c_str() << " -->\n";
    stream << "    <" << paramName << " value=\"" << m_value << "\" />\n\n";
}

std::string IntUserConfigParam::toString() const
{
    char buffer[16];
    sprintf(buffer, "%i", m_value);
    return buffer;
}

void IntUserConfigParam::findYourDataInAChildOf(const XMLNode* node)
{
    const XMLNode* child = node->getNode( paramName );
    if(child == NULL)
    {
        //std::cout << "Couldn't find int parameter " << paramName << std::endl;
        return;
    }

    child->get( "value", &m_value );
    //std::cout << "read int " << paramName << ", value=" << value << std::endl;
}
void IntUserConfigParam::findYourDataInAnAttributeOf(const XMLNode* node)
{
    node->get( paramName, &m_value );
}

// ---------------------------------------------------------------------------------------

StringUserConfigParam::StringUserConfigParam(const char* defaultValue, const char* paramName, const char* comment)
{
    
    m_value = defaultValue;
    m_default_value = defaultValue;
    
    this->paramName = paramName;
    all_params.push_back(this);
    if(comment != NULL) this->comment = comment;
}
StringUserConfigParam::StringUserConfigParam(const char* defaultValue, const char* paramName,
                                             GroupUserConfigParam* group, const char* comment)
{
    m_value = defaultValue;
    m_default_value = defaultValue;
    
    this->paramName = paramName;
    group->addChild(this);
    if(comment != NULL) this->comment = comment;
}


void StringUserConfigParam::write(std::ofstream& stream) const
{
    if(comment.size() > 0) stream << "    <!-- " << comment.c_str() << " -->\n";
    stream << "    <" << paramName << " value=\"" << m_value << "\" />\n\n";
}
void StringUserConfigParam::findYourDataInAChildOf(const XMLNode* node)
{
    const XMLNode* child = node->getNode( paramName );
    if(child == NULL) return;

    child->get( "value", &m_value );
}
void StringUserConfigParam::findYourDataInAnAttributeOf(const XMLNode* node)
{
    node->get( paramName, &m_value );
}

std::string StringUserConfigParam::toString() const
{
    return m_value;
}

// ---------------------------------------------------------------------------------------

BoolUserConfigParam::BoolUserConfigParam(bool defaultValue, const char* paramName, const char* comment)
{
    m_value = defaultValue;
    m_default_value = defaultValue;
    
    this->paramName = paramName;
    all_params.push_back(this);
    if(comment != NULL) this->comment = comment;
}
BoolUserConfigParam::BoolUserConfigParam(bool defaultValue, const char* paramName,
                                         GroupUserConfigParam* group, const char* comment)
{
    m_value = defaultValue;
    m_default_value = defaultValue;
    
    this->paramName = paramName;
    group->addChild(this);
    if(comment != NULL) this->comment = comment;
}


void BoolUserConfigParam::write(std::ofstream& stream) const
{
    if(comment.size() > 0) stream << "    <!-- " << comment.c_str() << " -->\n";
    stream << "    <" << paramName << " value=\"" << (m_value ? "true" : "false" ) << "\" />\n\n";
}
void BoolUserConfigParam::findYourDataInAChildOf(const XMLNode* node)
{
    const XMLNode* child = node->getNode( paramName );
    if(child == NULL) return;

    std::string textValue = "";
    child->get( "value", &textValue );

    if(textValue == "true")
    {
        m_value = true;
    }
    else if(textValue == "false")
    {
        m_value = false;
    }
    else
    {
        std::cerr << "Unknown value for " << paramName << "; expected true or false\n";
    }
}

void BoolUserConfigParam::findYourDataInAnAttributeOf(const XMLNode* node)
{
    std::string textValue = "";
    node->get( paramName, &textValue );

    if (textValue == "true")
    {
        m_value = true;
    }
    else if (textValue == "false")
    {
        m_value = false;
    }
    else
    {
        std::cerr << "Unknown value for " << paramName << "; expected true or false\n";
    }
}

std::string BoolUserConfigParam::toString() const
{
    return (m_value ? "true" : "false" );
}


// ---------------------------------------------------------------------------------------

FloatUserConfigParam::FloatUserConfigParam(float defaultValue, const char* paramName, const char* comment)
{
    m_value = defaultValue;
    m_default_value = defaultValue;
    
    this->paramName = paramName;
    all_params.push_back(this);
    if(comment != NULL) this->comment = comment;
}

FloatUserConfigParam::FloatUserConfigParam(float defaultValue, const char* paramName,
                                           GroupUserConfigParam* group, const char* comment)
{
    m_value = defaultValue;
    m_default_value = defaultValue;
    
    this->paramName = paramName;
    group->addChild(this);
    if(comment != NULL) this->comment = comment;
}

void FloatUserConfigParam::write(std::ofstream& stream) const
{
    if(comment.size() > 0) stream << "    <!-- " << comment.c_str() << " -->\n";
    stream << "    <" << paramName << " value=\"" << m_value << "\" />\n\n";
}

void FloatUserConfigParam::findYourDataInAChildOf(const XMLNode* node)
{
    const XMLNode* child = node->getNode( paramName );
    if(child == NULL) return;

    child->get( "value", &m_value );
}

void FloatUserConfigParam::findYourDataInAnAttributeOf(const XMLNode* node)
{
    node->get( paramName, &m_value );
}

std::string FloatUserConfigParam::toString() const
{
    char buffer[16];
    sprintf(buffer, "%f", m_value);
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
    UserConfigParams::m_all_players.clearAndDeleteAll();
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
        GuestPlayerProfile() : PlayerProfile(core::stringc(_("Guest")).c_str())
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

/** comparison function used to sort players */
bool comparePlayers(PlayerProfile* a, PlayerProfile* b)
{
    return a->m_use_frequency > b->m_use_frequency;
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
        if(root) delete root;
        return false;
    }

    // ---- Read config file version
    int configFileVersion = CURRENT_CONFIG_VERSION;
    if(root->get("version", &configFileVersion) < 1)
    {
        GUIEngine::showMessage( _("Your config file was malformed, so it was deleted and a new one will be created."), 10.0f);
        std::cerr << "Warning, malformed user config file! Contains no version\n";
    }
    if (configFileVersion < CURRENT_CONFIG_VERSION)
    {
        // current version (8) is 100% incompatible with other versions (which were lisp)
        // so we just delete the old config. in the future, for smaller updates, we can
        // add back the code previously there that upgraded the config file to the new
        // format instead of overwriting it.
        
        GUIEngine::showMessage( _("Your config file was too old, so it was deleted and a new one will be created."), 10.0f);
        printf("Your config file was too old, so it was deleted and a new one will be created.");
        delete root;
        return false;

    }   // if configFileVersion<SUPPORTED_CONFIG_VERSION

    // ---- Read parameters values (all parameter objects must have been created before this point if
    //      you want them automatically read from the config file)
    const int paramAmount = all_params.size();
    for(int i=0; i<paramAmount; i++)
    {
        all_params[i].findYourDataInAChildOf(root);
    }
    
    
    // ---- Read players
    // we create those AFTER other values are being read simply because we have many Player
    // nodes that all bear the same name, so the generic loading code won't work here
    UserConfigParams::m_all_players.clearAndDeleteAll();
    
    std::vector<XMLNode*> players;
    root->getNodes("Player", players);
    const int amount = players.size();
    for (int i=0; i<amount; i++)
    {
        //std::string name;
        //players[i]->get("name", &name);
        UserConfigParams::m_all_players.push_back( new PlayerProfile(players[i]) );
    }
    
    // sort players by frequency of use
    std::sort (UserConfigParams::m_all_players.contentsVector.begin(),
               UserConfigParams::m_all_players.contentsVector.end(), comparePlayers);
    
    
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
