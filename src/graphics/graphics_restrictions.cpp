//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014 Joerg Henrichs
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

#include "graphics/graphics_restrictions.hpp"

#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "utils/log.hpp"
#include "utils/no_copy.hpp"
#include "utils/string_utils.hpp"
#include "utils/types.hpp"

#include <algorithm>

namespace GraphicsRestrictions
{

    class Rule;
    namespace Private
    {
        std::vector<Rule*> m_all_rules;

    }   // namespace Private
    using namespace Private;

// ============================================================================
/** A small utility class to manage and compare version tuples.
 */
class Version
{
private:
    /** The array containing the version number. */
    std::vector<uint32_t> m_version;

    // ----------------------------------------------------------------------------
    /** Searches for the first number in the string, then converts the rest of the
     *  string into a tuple. E.g. "Build 12.34.56" --> [12,34,56].
     */
    void convertVersionString(const std::string &version)
    {
        std::string s = version;
        std::string::iterator p = s.begin();
        while(p !=s.end() && (*p<'0') || (*p>'9') )
            p++;
        s.erase(s.begin(), p);
        m_version = StringUtils::splitToUInt(s, '.');
    }   // convertVersionString

public:
    // ------------------------------------------------------------------------
    /** Dummy default constructor. */
    Version() {}
    // ------------------------------------------------------------------------
    /** Simple constructor which takes a string with "." separated numbers.
     */
    Version(const std::string &version)
    {
        convertVersionString(version);
    }   // Version(std::string)

    // ------------------------------------------------------------------------
    /** Create a version instance from the driver and car name. For example for
     *  an Intel HD3000 card the string is "3.1.0 - Build 9.17.10.3517" it would
     *  create [9,17,10,3517] - i.e. it takes the vendor info into account.
     *  \param driver_version The GL_VERSION string (i.e. opengl and version
     *         number).
     *  \param card_name The GL_RENDERER string (i.e. graphics card).
     */
    Version(const std::string &driver_version, const std::string &card_name)
    {
        m_version.clear();
        // Intel card: driver version = "3.1.0 - Build 9.17.10.3517"
        if (StringUtils::startsWith(card_name, "Intel"))
        {
            std::vector<std::string> s = StringUtils::split(driver_version, '-');
            if (s.size() == 2)
            {
                convertVersionString(s[1]);
                return;
            }
        }

        Log::warn("Graphics", "Can not find version for '%s' '%s' - ignored.",
            driver_version.c_str(), card_name.c_str());

    }   // Version

    // ------------------------------------------------------------------------
    /** Compares two version numbers. Equal returns true if the elements are
     *  identical up to the minimum number of elements, e.g.: [1.2.3] = [1.2].
     *  This allows so e.g. disable a feature for all minor versions by just
     *  specifying the major version.
     */
    bool operator== (const Version &other) const
    {
        int min_n = std::min(m_version.size(), other.m_version.size());
        for(unsigned int i=0; i<min_n; i++)
            if(other.m_version[i]!=m_version[i]) return false;
        return true;
    }   // operator ==
    // ------------------------------------------------------------------------
    bool operator< (const Version &other) const
    {
        int min_n = std::min(m_version.size(), other.m_version.size());
        for(unsigned int i=0; i<min_n; i++)
            if(other.m_version[i]>=m_version[i]) return false;
        return true;
    }   // operator>

};   // class Version
// ============================================================================
class Rule : public NoCopy
{
private:
    /** Operators to test for a card. */
    enum {CARD_IS, CARD_CONTAINS} m_card_test;

    /** Name of the card for which this rule applies. */
    std::string m_card_name;

    /** Operators to test version numbers with. */
    enum {VERSION_IGNORE, VERSION_EQUAL, VERSION_LESS, 
          VERSION_LESS_EQUAL}  m_version_test;

    /** Driver version for which this rule applies. */
    Version m_driver_version;

    /** For which OS this rule applies. */
    std::string m_os;

    /** Which options to disable. */
    std::vector<std::string> m_disable_options;
public:
    Rule(const XMLNode *rule)
    {
        m_version_test = VERSION_IGNORE;
        if(rule->get("is", &m_card_name))
        {
            m_card_test = CARD_IS;
        }
        else if(rule->get("contains", &m_card_name))
        {
            m_card_test = CARD_CONTAINS;
        }

        rule->get("os", &m_os);

        std::string s;
        if(rule->get("version", &s) && s.size()>1)
        {
            if(s.substr(0, 2)=="<=")
            {
                m_version_test = VERSION_LESS_EQUAL;
                s = s.substr(2, s.size());
            }
            else if(s[0]=='<')
            {
                m_version_test = VERSION_LESS;
                s.erase(s.begin());
            }
            else if(s[0]=='=')
            {
                m_version_test = VERSION_EQUAL;
                s.erase(s.begin());
            }
            else
            {
                Log::warn("Graphics", "Invalid verison '%s' found - ignored.",
                          s.c_str());
            }
            m_driver_version = Version(s);
        }   // has version

        if(rule->get("disable", &s))
            m_disable_options = StringUtils::split(s, ' ');
    }   // Rule
    // ------------------------------------------------------------------------
    bool applies(const std::string &card, const Version &version) const
    {
        // Test for OS
        // -----------
        if(m_os.size()>0)
        {
#ifdef __linux__
            if(m_os!="linux") return false;
#endif
#ifdef WIN32
            if(m_os!="windows") return false;
#endif
#ifdef __APPLE__
            if(m_os!="osx") return false;
#endif
#ifdef BSD
            if(m_os!="bsd") return false;
#endif
        }   // m_os.size()>0

        // Test for card
        // -------------
        switch(m_card_test)
        {
        case CARD_IS:
            if(card!=m_card_name) return false;
            break;
        case CARD_CONTAINS: 
            if(card.find(m_card_name)==std::string::npos)
                return false;
            break;
        }   // switch m_card_test

        // Test for driver version
        // -----------------------
        switch(m_version_test)
        {
        case VERSION_IGNORE: break;   // always true
        case VERSION_EQUAL: if(!(version==m_driver_version)) return false;
            break;
        case VERSION_LESS_EQUAL:
            if(m_driver_version < version) return false;
        }   // switch m_version_test
        return true;
        // -----------------------------------------------
    }
    // ------------------------------------------------------------------------
    /** Returns a list of options to disable. */
    const std::vector<std::string>& getRestrictions() const
    {
        return m_disable_options;
    }   // getRestrictions
    // ------------------------------------------------------------------------
};   // class Rule

// ============================================================================

void init()
{
    std::string filename = 
        file_manager->getUserConfigFile("graphical_restrictions.xml");
    if(!file_manager->fileExists(filename))
        filename = file_manager->getAsset("graphical_restrictions.xml");
    const XMLNode *rules = file_manager->createXMLTree(filename);
    if(!rules)
    {
        Log::warn("Graphics", "Could not find graphical_restrictions.xm");
        return;
    }
    if(rules->getName()!="graphical-restrictions")
    {
        delete rules;
        Log::warn("Graphics", "'%s' did not contain graphical-restrictions tag",
                  filename.c_str());
        return;
    }
    for(unsigned int i=0; i<rules->getNumNodes(); i++)
    {
        const XMLNode *rule = rules->getNode(i);
        if(rule->getName()!="card")
        {
            Log::warn("Graphics", "Incorrect node '%s' found in '%s' - ignored.",
                      rule->getName().c_str(), filename.c_str());
            continue;
        }
        m_all_rules.push_back(new Rule(rule));
    }
}   // init

// ----------------------------------------------------------------------------
/** Returns a list of graphical features that need to be disabled for the
 *  specified driver and graphics card (and OS).
 *  \paaram driver_version The GL_VERSION string (i.e. opengl and version
 *          number).
 *  \param card_name The GL_RENDERER string (i.e. graphics card).
 */
std::vector<std::string> getRestrictions(const std::string &driver_version,
                                         const std::string &card_name)
{
    init();

    std::vector<std::string> restrictions;

    Version version(driver_version, card_name);
    for(unsigned int i=0; i<m_all_rules.size(); i++)
    {
        if(m_all_rules[i]->applies(card_name, version))
        {
            std::vector<std::string> r = m_all_rules[i]->getRestrictions();
            restrictions.insert(restrictions.end(), r.begin(), r.end());
        }
    }
    return restrictions;
}   // getRestrictions

}   // namespace HardwareStats
