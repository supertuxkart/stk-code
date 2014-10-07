//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2013 SuperTuxKart-Team
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


#include "utils/ptr_vector.hpp"

// The order here is important. If all_params is declared later (e.g. after
// the #includes), all elements will be added to all_params, and then
// all_params will be initialised, i.e. cleared!
class UserConfigParam;
static PtrVector<UserConfigParam, REF> all_params;

// X-macros
#define PARAM_PREFIX
#define PARAM_DEFAULT(X) = X
#include "config/user_config.hpp"

#include "config/saved_grand_prix.hpp"
#include "config/stk_config.hpp"
#include "guiengine/engine.hpp"
#include "io/file_manager.hpp"
#include "io/utf_writer.hpp"
#include "io/xml_node.hpp"
#include "race/race_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

const int UserConfig::m_current_config_version = 8;


// ----------------------------------------------------------------------------
UserConfigParam::~UserConfigParam()
{
    all_params.remove(this);
}   // ~UserConfigParam

// ----------------------------------------------------------------------------
/** Writes an inner node.
 *  \param stream the xml writer.
 *  \param level determines indentation level.
 */
void UserConfigParam::writeInner(std::ofstream& stream, int level) const
{
    std::string tab(level * 4,' ');
    stream << "    " << tab.c_str() << m_param_name.c_str() << "=\""
           << toString().c_str() << "\"\n";
}   // writeInner

// ============================================================================
GroupUserConfigParam::GroupUserConfigParam(const char* group_name,
                                           const char* comment)
{
    m_param_name = group_name;
    all_params.push_back(this);
    if(comment != NULL) m_comment = comment;
}   // GroupUserConfigParam

// ============================================================================
GroupUserConfigParam::GroupUserConfigParam(const char* group_name,
                                           GroupUserConfigParam* group,
                                           const char* comment)
{
    m_param_name = group_name;
    group->addChild(this);
    if(comment != NULL) m_comment = comment;
}   // GroupUserConfigParam

// ----------------------------------------------------------------------------
void GroupUserConfigParam::write(std::ofstream& stream) const
{
    const int attr_amount = (int)m_attributes.size();

    // comments
    if(m_comment.size() > 0) stream << "    <!-- " << m_comment.c_str();
    for(int n=0; n<attr_amount; n++)
    {
        if(m_attributes[n]->m_comment.size() > 0)
            stream << "\n             " << m_attributes[n]->m_param_name.c_str()
                   << " : " << m_attributes[n]->m_comment.c_str();
    }

    stream << " -->\n    <" << m_param_name.c_str() << "\n";

    // actual values
    for (int n=0; n<attr_amount; n++)
    {
        m_attributes[n]->writeInner(stream, 1);
    }
    stream << "    >\n";
    const int children_amount = (int)m_children.size();
    for (int n=0; n<children_amount; n++)
    {
        m_children[n]->writeInner(stream, 1);
    }
    stream << "    </" << m_param_name.c_str() << ">\n\n";
}   // write

// ----------------------------------------------------------------------------
void GroupUserConfigParam::writeInner(std::ofstream& stream, int level) const
{
    std::string tab(level * 4,' ');
    for(int i = 0; i < level; i++) tab =+ "    ";
    const int children_amount = m_attributes.size();

    stream << "    " << tab.c_str() << "<" << m_param_name.c_str() << "\n";

    // actual values
    for (int n=0; n<children_amount; n++)
    {
        m_attributes[n]->writeInner(stream, level+1);
    }
    stream << "    " << tab.c_str() << "/>\n";
}   // writeInner

// ----------------------------------------------------------------------------
void GroupUserConfigParam::findYourDataInAChildOf(const XMLNode* node)
{
    const XMLNode* child = node->getNode( m_param_name );
    if (child == NULL)
    {
        //Log::error("User Config", "Couldn't find parameter group %s", m_param_name.c_str());
        return;
    }

    const int attributes_amount = m_attributes.size();
    for (int n=0; n<attributes_amount; n++)
    {
        m_attributes[n]->findYourDataInAnAttributeOf(child);
    }

}   // findYourDataInAChildOf

// ----------------------------------------------------------------------------
void GroupUserConfigParam::findYourDataInAnAttributeOf(const XMLNode* node)
{
}   // findYourDataInAnAttributeOf

// ----------------------------------------------------------------------------
irr::core::stringc GroupUserConfigParam::toString() const
{
    return "";
}   // toString

// ----------------------------------------------------------------------------
void GroupUserConfigParam::clearChildren()
{
    m_children.clear();
}   // clearChildren

// ----------------------------------------------------------------------------
void GroupUserConfigParam::addChild(GroupUserConfigParam* child)
{
    m_children.push_back(child);
}   // addChild

// ----------------------------------------------------------------------------
void GroupUserConfigParam::addChild(UserConfigParam* child)
{
    m_attributes.push_back(child);
}   // addChild


// ============================================================================
template<typename T, typename U>
ListUserConfigParam<T, U>::ListUserConfigParam(const char* param_name,
                                           const char* comment)
{
    m_param_name = param_name;
    all_params.push_back(this);
    if(comment != NULL) m_comment = comment;
}   // ListUserConfigParam

// ============================================================================
template<typename T, typename U>
ListUserConfigParam<T,U>::ListUserConfigParam(const char* param_name,
                                           const char* comment,
                                           int nb_elements,
                                           ...)
{
    m_param_name = param_name;
    all_params.push_back(this);
    if(comment != NULL) m_comment = comment;

    // add the default list
    va_list arguments;
    va_start ( arguments, nb_elements );
    for ( int i = 0; i < nb_elements; i++ )
        m_elements.push_back(T(va_arg ( arguments, U )));
    va_end ( arguments );                  // Cleans up the list
}   // ListUserConfigParam

// ============================================================================
template<typename T, typename U>
ListUserConfigParam<T, U>::ListUserConfigParam(const char* param_name,
                                           GroupUserConfigParam* group,
                                           const char* comment)
{
    m_param_name = param_name;
    group->addChild(this);
    if(comment != NULL) m_comment = comment;
}   // ListUserConfigParam

// ============================================================================
template<typename T, typename U>
ListUserConfigParam<T, U>::ListUserConfigParam(const char* param_name,
                                           GroupUserConfigParam* group,
                                           const char* comment,
                                           int nb_elements,
                                           ...)
{
    m_param_name = param_name;
    group->addChild(this);
    if(comment != NULL) m_comment = comment;

    // add the default list
    va_list arguments;
    va_start ( arguments, nb_elements );
    for ( int i = 0; i < nb_elements; i++ )
        m_elements.push_back(va_arg ( arguments, T ));
    va_end ( arguments );                  // Cleans up the list
}   // ListUserConfigParam

// ----------------------------------------------------------------------------
template<typename T, typename U>
void ListUserConfigParam<T, U>::write(std::ofstream& stream) const
{
    const int elts_amount = m_elements.size();

    // comment
    if(m_comment.size() > 0) stream << "    <!-- " << m_comment.c_str();
    stream << " -->\n    <" << m_param_name.c_str() << "\n";

    stream << "        Size=\"" << elts_amount << "\"\n";
    // actual elements
    for (int n=0; n<elts_amount; n++)
    {
        stream << "        " << n << "=\"" << m_elements[n].c_str() << "\"\n";
    }
    stream << "    >\n";
    stream << "    </" << m_param_name.c_str() << ">\n\n";
}   // write

// ----------------------------------------------------------------------------

template<typename T, typename U>
void ListUserConfigParam<T, U>::findYourDataInAChildOf(const XMLNode* node)
{
    const XMLNode* child = node->getNode( m_param_name );
    if (child == NULL)
    {
        //Log::error("User Config", "Couldn't find parameter group %s", m_param_name.c_str());
        return;
    }

    int attr_count = 0;
    child->get( "Size", &attr_count);
    for (int n=0; n<attr_count; n++)
    {
        T elt;
        std::string str;
        child->get( StringUtils::toString(n), &str);
        StringUtils::fromString<T>(str, elt);
        
        // check if the element is already there :
        bool there = false;
        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            if (elt == m_elements[i])
            {
                there = true;
                break;
            }
        }
        if (!there)
        {
            m_elements.push_back(elt);
        }
    }

}   // findYourDataInAChildOf

// ----------------------------------------------------------------------------
template<typename T, typename U>
void ListUserConfigParam<T, U>::findYourDataInAnAttributeOf(const XMLNode* node)
{
}   // findYourDataInAnAttributeOf

// ----------------------------------------------------------------------------
template<typename T, typename U>
void ListUserConfigParam<T,U>::addElement(T element)
{
    m_elements.push_back(element);
}   // findYourDataInAnAttributeOf

// ----------------------------------------------------------------------------
template<typename T, typename U>
core::stringc ListUserConfigParam<T, U>::toString() const
{
    return "";
}   // toString



// ============================================================================
IntUserConfigParam::IntUserConfigParam(int default_value,
                                       const char* param_name,
                                       const char* comment)
{
    m_value         = default_value;
    m_default_value = default_value;
    m_param_name    = param_name;
    all_params.push_back(this);
    if(comment != NULL) m_comment = comment;
}   // IntUserConfigParam

// ----------------------------------------------------------------------------
IntUserConfigParam::IntUserConfigParam(int default_value,
                                       const char* param_name,
                                       GroupUserConfigParam* group,
                                       const char* comment)
{
    m_value         = default_value;
    m_default_value = default_value;
    m_param_name    = param_name;
    group->addChild(this);
    if(comment != NULL) m_comment = comment;
}   // IntUserConfigParam

// ----------------------------------------------------------------------------
void IntUserConfigParam::write(std::ofstream& stream) const
{
    if(m_comment.size() > 0) stream << "    <!-- " << m_comment.c_str()
                                    << " -->\n";
    stream << "    <" << m_param_name.c_str() << " value=\"" << m_value
           << "\" />\n\n";
}   // write

// ----------------------------------------------------------------------------
irr::core::stringc IntUserConfigParam::toString() const
{
    irr::core::stringc tmp;
    tmp += m_value;

    return tmp;
}   // toString

// ----------------------------------------------------------------------------
void IntUserConfigParam::findYourDataInAChildOf(const XMLNode* node)
{
    const XMLNode* child = node->getNode( m_param_name );
    if(child == NULL)
    {
        //Log::error("UserConfigParam", "Couldn't find int parameter %s", m_param_name.c_str());
        return;
    }

    child->get( "value", &m_value );
    //Log::info("UserConfigParam", "Read int %s ,value = %d", m_param_name.c_str(), value);
}   // findYourDataInAChildOf

// ----------------------------------------------------------------------------
void IntUserConfigParam::findYourDataInAnAttributeOf(const XMLNode* node)
{
    node->get( m_param_name, &m_value );
}   // findYourDataInAnAttributeOf

// ============================================================================
TimeUserConfigParam::TimeUserConfigParam(StkTime::TimeType default_value,
                                         const char* param_name,
                                         const char* comment)
{
    m_value         = default_value;
    m_default_value = default_value;
    m_param_name    = param_name;
    all_params.push_back(this);
    if(comment != NULL) m_comment = comment;
}   // TimeUserConfigParam

// ----------------------------------------------------------------------------
TimeUserConfigParam::TimeUserConfigParam(StkTime::TimeType default_value,
                                         const char* param_name,
                                         GroupUserConfigParam* group,
                                         const char* comment)
{
    m_value         = default_value;
    m_default_value = default_value;
    m_param_name    = param_name;
    group->addChild(this);
    if(comment != NULL) m_comment = comment;
}   // TimeUserConfigParam

// ----------------------------------------------------------------------------
void TimeUserConfigParam::write(std::ofstream& stream) const
{
    if(m_comment.size() > 0) stream << "    <!-- " << m_comment.c_str()
                                    << " -->\n";
    std::ostringstream o;
    o<<m_value;
    stream << "    <" << m_param_name.c_str() << " value=\""
           << o.str().c_str() << "\" />\n\n";
}   // write

// ----------------------------------------------------------------------------
irr::core::stringc TimeUserConfigParam::toString() const
{
    // irrString does not have a += with a 64-bit int type, so
    // we can't use an irrlicht's stringw directly. Since it's only a
    // number, we can use std::string, and convert to stringw

    std::ostringstream o;
    o<<m_value;
    return o.str().c_str();
}   // toString

// ----------------------------------------------------------------------------
void TimeUserConfigParam::findYourDataInAChildOf(const XMLNode* node)
{
    const XMLNode* child = node->getNode( m_param_name );
    if(child == NULL)
    {
        //Log::error("UserConfigParam", "Couldn't find int parameter %s", m_param_name.c_str());
        return;
    }

    int64_t tmp;
    child->get( "value", &tmp );
    m_value = tmp;
}   // findYourDataInAChildOf

// ----------------------------------------------------------------------------
void TimeUserConfigParam::findYourDataInAnAttributeOf(const XMLNode* node)
{
    int64_t tmp;
    node->get( m_param_name, &tmp );
    m_value = tmp;
}   // findYourDataInAnAttributeOf

// ============================================================================
StringUserConfigParam::StringUserConfigParam(const char* default_value,
                                             const char* param_name,
                                             const char* comment)
{
    m_value         = default_value;
    m_default_value = default_value;
    m_param_name    = param_name;
    all_params.push_back(this);
    if(comment != NULL) m_comment = comment;
}   // StringUserConfigParam

// ----------------------------------------------------------------------------
StringUserConfigParam::StringUserConfigParam(const char* default_value,
                                             const char* param_name,
                                             GroupUserConfigParam* group,
                                             const char* comment)
{
    m_value         = default_value;
    m_default_value = default_value;
    m_param_name = param_name;
    group->addChild(this);
    if(comment != NULL) m_comment = comment;
}   // StringUserConfigParam

// ----------------------------------------------------------------------------
void StringUserConfigParam::write(std::ofstream& stream) const
{
    if(m_comment.size() > 0) stream << "    <!-- " << m_comment.c_str()
                                    << " -->\n";
    stream << "    <" << m_param_name.c_str() << " value=\""
           << m_value.c_str() << "\" />\n\n";
}   // write

// ----------------------------------------------------------------------------
void StringUserConfigParam::findYourDataInAChildOf(const XMLNode* node)
{
    const XMLNode* child = node->getNode( m_param_name );
    if(child == NULL) return;

    child->get( "value", &m_value );
}   // findYourDataInAChildOf

// ----------------------------------------------------------------------------
void StringUserConfigParam::findYourDataInAnAttributeOf(const XMLNode* node)
{
    node->get( m_param_name, &m_value );
}   // findYourDataInAnAttributeOf

// ============================================================================
BoolUserConfigParam::BoolUserConfigParam(bool default_value,
                                         const char* param_name,
                                         const char* comment)
{
    m_value         = default_value;
    m_default_value = default_value;

    m_param_name = param_name;
    all_params.push_back(this);
    if(comment != NULL) m_comment = comment;
}   // BoolUserConfigParam

// ----------------------------------------------------------------------------
BoolUserConfigParam::BoolUserConfigParam(bool default_value,
                                         const char* param_name,
                                         GroupUserConfigParam* group,
                                         const char* comment)
{
    m_value         = default_value;
    m_default_value = default_value;

    m_param_name = param_name;
    group->addChild(this);
    if(comment != NULL) m_comment = comment;
}   // BoolUserConfigParam


// ----------------------------------------------------------------------------
void BoolUserConfigParam::write(std::ofstream& stream) const
{
    if(m_comment.size() > 0) stream << "    <!-- " << m_comment.c_str()
                                    << " -->\n";
    stream << "    <" << m_param_name.c_str() << " value=\""
           << (m_value ? "true" : "false" ) << "\" />\n\n";
}   // write

// ----------------------------------------------------------------------------
void BoolUserConfigParam::findYourDataInAChildOf(const XMLNode* node)
{
    const XMLNode* child = node->getNode( m_param_name );
    if(child == NULL) return;

    std::string text_value = "";
    child->get( "value", &text_value );

    if(text_value == "true")
    {
        m_value = true;
    }
    else if(text_value == "false")
    {
        m_value = false;
    }
    else
    {
        Log::error("User Config", "Unknown value for %s; expected true or false", m_param_name.c_str());
    }
}   // findYourDataInAChildOf

// ----------------------------------------------------------------------------
void BoolUserConfigParam::findYourDataInAnAttributeOf(const XMLNode* node)
{
    std::string text_value = "";
    node->get( m_param_name, &text_value );

    if (text_value == "true")
    {
        m_value = true;
    }
    else if (text_value == "false")
    {
        m_value = false;
    }
    else
    {
        Log::error("User Config", "Unknown value for %s; expected true or false", m_param_name.c_str());
    }
}   // findYourDataInAnAttributeOf

// ----------------------------------------------------------------------------
irr::core::stringc BoolUserConfigParam::toString() const
{
    return (m_value ? "true" : "false" );
}   // toString

// ============================================================================
FloatUserConfigParam::FloatUserConfigParam(float default_value,
                                           const char* param_name,
                                           const char* comment)
{
    m_value         = default_value;
    m_default_value = default_value;

    m_param_name = param_name;
    all_params.push_back(this);
    if(comment != NULL) m_comment = comment;
}   // FloatUserConfigParam

// ----------------------------------------------------------------------------
FloatUserConfigParam::FloatUserConfigParam(float default_value,
                                           const char* param_name,
                                           GroupUserConfigParam* group,
                                           const char* comment)
{
    m_value         = default_value;
    m_default_value = default_value;

    m_param_name = param_name;
    group->addChild(this);
    if(comment != NULL) m_comment = comment;
}   // FloatUserConfigParam

// ----------------------------------------------------------------------------
void FloatUserConfigParam::write(std::ofstream& stream) const
{
    if(m_comment.size() > 0) stream << "    <!-- " << m_comment.c_str()
                                    << " -->\n";
    stream << "    <" << m_param_name.c_str() << " value=\"" << m_value
           << "\" />\n\n";
}   // write

// ----------------------------------------------------------------------------
void FloatUserConfigParam::findYourDataInAChildOf(const XMLNode* node)
{
    const XMLNode* child = node->getNode( m_param_name );
    if(child == NULL) return;

    child->get( "value", &m_value );
}   // findYourDataInAChildOf

// ----------------------------------------------------------------------------
void FloatUserConfigParam::findYourDataInAnAttributeOf(const XMLNode* node)
{
    node->get( m_param_name, &m_value );
}   // findYourDataInAnAttributeOf

// ----------------------------------------------------------------------------
core::stringc FloatUserConfigParam::toString() const
{
    irr::core::stringc tmp;
    tmp += m_value;
    return tmp;
}   // toString

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

}   // UserConfig

// -----------------------------------------------------------------------------
UserConfig::~UserConfig()
{
    UserConfigParams::m_saved_grand_prix_list.clearAndDeleteAll();
}   // ~UserConfig

// -----------------------------------------------------------------------------
/** Load configuration values from file. */
bool UserConfig::loadConfig()
{
    const std::string filename = file_manager->getUserConfigFile(m_filename);
    XMLNode* root = file_manager->createXMLTree(filename);
    if(!root || root->getName() != "stkconfig")
    {
        Log::error("UserConfig",
                   "Could not read user config file '%s'.", filename.c_str());
        if(root) delete root;
        // Create a default config file - just in case that stk crashes later
        // there is a config file that can be modified (to e.g. disable
        // shaders)
        saveConfig();
        return false;
    }

    // ---- Read config file version
    int config_file_version = m_current_config_version;
    if(root->get("version", &config_file_version) < 1)
    {
        GUIEngine::showMessage( _("Your config file was malformed, so it was deleted and a new one will be created."), 10.0f);
        Log::error("UserConfig",
                   "Warning, malformed user config file! Contains no version");
    }
    if (config_file_version < m_current_config_version)
    {
        // current version (8) is 100% incompatible with other versions (which were lisp)
        // so we just delete the old config. in the future, for smaller updates, we can
        // add back the code previously there that upgraded the config file to the new
        // format instead of overwriting it.

        GUIEngine::showMessage(_("Your config file was too old, so it was deleted and a new one will be created."), 10.0f);
        Log::info("UserConfig", "Your config file was too old, so it was deleted and a new one will be created.");
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


    // ---- Read Saved GP's
    UserConfigParams::m_saved_grand_prix_list.clearAndDeleteAll();
    std::vector<XMLNode*> saved_gps;
    root->getNodes("SavedGP", saved_gps);
    const int gp_amount = saved_gps.size();
    for (int i=0; i<gp_amount; i++)
    {
        UserConfigParams::m_saved_grand_prix_list.push_back(
                                           new SavedGrandPrix( saved_gps[i]) );
    }
    delete root;

    return true;
}   // loadConfig

// ----------------------------------------------------------------------------
/** Write settings to config file. */
void UserConfig::saveConfig()
{
    const std::string filename = file_manager->getUserConfigFile(m_filename);

    try
    {
        std::ofstream configfile (filename.c_str(), std::ofstream::out);

        configfile << "<?xml version=\"1.0\"?>\n";
        configfile << "<stkconfig version=\"" << m_current_config_version
                   << "\" >\n\n";

        const int paramAmount = all_params.size();
        for(int i=0; i<paramAmount; i++)
        {
            //Log::info("UserConfig", "Saving parameter %d to file", i);
            all_params[i].write(configfile);
        }

        configfile << "</stkconfig>\n";
        configfile.close();
    }
    catch (std::runtime_error& e)
    {
        Log::error("UserConfig::saveConfig", "Failed to write config to %s, because %s",
            filename.c_str(), e.what());
    }

}   // saveConfig

// ----------------------------------------------------------------------------
bool UserConfigParams::logMemory()
     { return (m_verbosity&LOG_MEMORY) == LOG_MEMORY;}

// ----------------------------------------------------------------------------
bool UserConfigParams::logGUI ()
     { return (m_verbosity&LOG_GUI) == LOG_GUI;   }

// ----------------------------------------------------------------------------
bool UserConfigParams::logAddons()
     { return (m_verbosity&LOG_ADDONS) == LOG_ADDONS;}

// ----------------------------------------------------------------------------
bool UserConfigParams::logFlyable()
     { return (m_verbosity&LOG_FLYABLE) == LOG_FLYABLE;  }

// ----------------------------------------------------------------------------
bool UserConfigParams::logMisc()
     { return (m_verbosity&LOG_MISC) == LOG_MISC;  }

