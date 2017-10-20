//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 Joerg Henrichs
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

#ifdef ANDROID
#include "main_android.hpp"
#endif

#include <algorithm>
#include <array>

namespace GraphicsRestrictions
{

    class Rule;
    namespace Private
    {
        /** Stores for each grpahics restriction if it's enabled or not. */
        std::vector<bool> m_all_graphics_restriction;

        /** The list of names used in the XML file for the graphics
         *  restriction types. They must be in the same order as the types. */

        std::array<std::string, 31> m_names_of_restrictions =
        {
            {
                "UniformBufferObject",
                "GeometryShader",
                "DrawIndirect",
                "TextureView",
                "TextureStorage",
                "ImageLoadStore",
                "BaseInstance",
                "ComputeShader",
                "ArraysOfArrays",
                "ShaderStorageBufferObject",
                "MultiDrawIndirect",
                "ShaderAtomicCounters",
                "BufferStorage",
                "BindlessTexture",
                "TextureCompressionS3TC",
                "AMDVertexShaderLayer",
                "ExplicitAttribLocation",
                "TextureFilterAnisotropic",
                "TextureFormatBGRA8888",
                "ColorBufferFloat",
                "DriverRecentEnough",
                "HighDefinitionTextures",
                "HighDefinitionTextures256",
                "AdvancedPipeline",
                "FramebufferSRGB",
                "FramebufferSRGBWorkaround1",
                "FramebufferSRGBWorkaround2",
                "GI",
                "ForceLegacyDevice",
                "VertexIdWorking",
                "HardwareSkinning"
            }
        };
    }   // namespace Private
    using namespace Private;

    /** Returns the graphics restrictions type for a string, or
     *  GR_COUNT if the name is not found. */
GraphicsRestrictionsType getTypeForName(const std::string &name)
{
    for (unsigned int i = 0; i < m_names_of_restrictions.size(); i++)
    {
        if (name == m_names_of_restrictions[i])
            return (GraphicsRestrictionsType)i;
    }
    return GR_COUNT;
}   // getTypeForName

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
        while( (p !=s.end()) && ((*p<'0') || (*p>'9')) )
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
        
#ifdef ANDROID
        // Android version should be enough to disable certain features on this
        // platform
        int version = AConfiguration_getSdkVersion(global_android_app->config);
        
        if (version > 0)
        {
            m_version.push_back(version);
            return;
        }
#endif

        // Mesa needs to be tested first, otherwise (if testing for card name
        // further down) it would be detected as a non-mesa driver.
        if (driver_version.find("Mesa") != std::string::npos)
        {
            std::string driver;
            // Try to force the driver name to be in a standard way by removing
            // optional strings
            driver = StringUtils::replace(driver_version, "-devel",         "");
            driver = StringUtils::replace(driver,         "(Core Profile) ", "");
            std::vector<std::string> l = StringUtils::split(driver, ' ');
            if (l.size() > 2)
            {
                // driver can be: "1.4 (3.0 Mesa 10.1.0)" -->
                // we use value next to "Mesa" word.
                for (unsigned int i = 0; i < l.size(); i++)
                {
                    if (l[i] == "Mesa" && i < l.size() - 1)
                    {
                        convertVersionString(l[i+1]);
                        return;
                    }
                }
            }
        }

        // Intel card: driver version = "3.1.0 - Build 9.17.10.3517"
        // ---------------------------------------------------------
        if (StringUtils::startsWith(card_name, "Intel"))
        {
            std::vector<std::string> s = StringUtils::split(driver_version, '-');
            if (s.size() == 2)
            {
                convertVersionString(s[1]);
                return;
            }
        }

        // Nvidia: driver_version = "4.3.0 NVIDIA 340.58"
        // ----------------------------------------------
        if (driver_version.find("NVIDIA") != std::string::npos)
        {
            std::vector<std::string> s = StringUtils::split(driver_version, ' ');
            if (s.size() == 3)
            {
                convertVersionString(s[2]);
                return;
            }

        }

        // ATI: some drivers use e.g.: "4.1 ATI-1.24.38"
        if (driver_version.find("ATI-") != std::string::npos)
        {
            std::string driver;
            // Try to force the driver name to be in a standard way by removing
            // optional strings
            driver = StringUtils::replace(driver_version, "ATI-", "");
            std::vector<std::string> s = StringUtils::split(driver, ' ');
            convertVersionString(s[1]);
            return;
        }

        // AMD: driver_version = "4.3.13283 Core Profile/Debug Context 14.501.1003.0"
        // ----------------------------------------------
        if (card_name.find("AMD") != std::string::npos
            || card_name.find("Radeon") != std::string::npos)
        {
            std::vector<std::string> s = StringUtils::split(driver_version, ' ');
            if (s.size() == 5)
            {
                convertVersionString(s[4]);
                return;
            }

        }

        // ATI: other drivers use "4.0.10188 Core Profile Context"
        if (card_name.find("ATI") != std::string::npos)
        {
            std::vector<std::string> s = StringUtils::split(driver_version, ' ');
            convertVersionString(s[0]);
            return;
        }

        Log::warn("Graphics", "Can not find version for '%s' '%s' - ignored.",
            driver_version.c_str(), card_name.c_str());

    }   // Version

    // ------------------------------------------------------------------------
    /** Compares two version numbers. Equal returns true if the elements are
     *  identical.
     */
    bool operator== (const Version &other) const
    {
        if (m_version.size() != other.m_version.size()) return false;
        for(unsigned int i=0; i<m_version.size(); i++)
            if(other.m_version[i]!=m_version[i]) return false;
        return true;
    }   // operator==
    // ------------------------------------------------------------------------
    /** Compares two version numbers. Equal returns true if the elements are
    *  identical.
    */
    bool operator!= (const Version &other) const
    {
        return !this->operator==(other);
    }   // operator!=
        // ------------------------------------------------------------------------
    /** If *this < other. */
    bool operator< (const Version &other) const
    {
        unsigned int min_n = (unsigned int)std::min(m_version.size(), other.m_version.size());
        for (unsigned int i = 0; i<min_n; i++)
        {
            if (m_version[i] > other.m_version[i]) return false;
            if (m_version[i] < other.m_version[i]) return true;
        }
        if (m_version.size() >= other.m_version.size())
            return false;
        else
            return true;
    }   // operator<
    // ------------------------------------------------------------------------
    /** If *this <= other. */
    bool operator<= (const Version &other) const
    {
        unsigned int min_n = (unsigned int)std::min(m_version.size(), other.m_version.size());
        for (unsigned int i = 0; i<min_n; i++)
        {
            if (m_version[i] > other.m_version[i]) return false;
            if (m_version[i] < other.m_version[i]) return true;
        }
        if (m_version.size() > other.m_version.size())
            return false;
        else
            return true;
    }   // operator<=

};   // class Version
// ============================================================================
class Rule : public NoCopy
{
private:
    /** Operators to test for a card. */
    enum {CARD_IGNORE, CARD_IS, CARD_CONTAINS} m_card_test;

    /** Name of the card for which this rule applies. */
    std::string m_card_name;

    /** Operators to test version numbers with. */
    enum {VERSION_IGNORE, VERSION_EQUAL, VERSION_LESS,
          VERSION_LESS_EQUAL}  m_version_test;

    /** Driver version for which this rule applies. */
    Version m_driver_version;

    /** For which OS this rule applies. */
    std::string m_os;

    /** For which vendor this rule applies. */
    std::string m_vendor;

    /** Which options to disable. */
    std::vector<std::string> m_disable_options;
public:
    Rule(const XMLNode *rule)
    {
        m_version_test = VERSION_IGNORE;
        m_card_test = CARD_IGNORE;

        if(rule->get("is", &m_card_name))
        {
            m_card_test = CARD_IS;
        }
        else if(rule->get("contains", &m_card_name))
        {
            m_card_test = CARD_CONTAINS;
        }

        rule->get("os", &m_os);
        rule->get("vendor", &m_vendor);

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
    bool applies(const std::string &card, const Version &version,
                 const std::string &vendor) const
    {
        // Test for OS
        // -----------
        if(m_os.size()>0)
        {
#if defined(__linux__) && !defined(ANDROID)
            if(m_os!="linux") return false;
#elif defined(WIN32)
            if(m_os!="windows") return false;
#elif defined(__APPLE__)
            if(m_os!="osx") return false;
#elif defined(BSD)
            if(m_os!="bsd") return false;
#elif defined(ANDROID)
            if(m_os!="android") return false;
#else
            return false;
#endif
        }   // m_os.size()>0
        
        // Test for vendor
        // ---------------
        if (m_vendor.size() > 0)
        {
            if (m_vendor != vendor)
                return false;
        }

        // Test for card
        // -------------
        switch(m_card_test)
        {
        case CARD_IGNORE: break;   // always true
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
            if (!(version <= m_driver_version)) return false;
            break;
        case VERSION_LESS:
            if (!(version < m_driver_version )) return false;
            break;
        }   // switch m_version_test
        return true;
        // -----------------------------------------------
    }   // applies

    // ------------------------------------------------------------------------
    /** Returns a list of options to disable. */
    const std::vector<std::string>& getRestrictions() const
    {
        return m_disable_options;
    }   // getRestrictions
    // ------------------------------------------------------------------------
};   // class Rule

// ============================================================================
/** Very rudimentary and brute unit testing. Better than nothing :P
 */
void unitTesting()
{
    assert(Version("1")     == Version("1"));
    assert(Version("1")     != Version("2"));
    assert(Version("1")     <= Version("2"));
    assert(Version("1")     <  Version("2"));
    assert(Version("1.2.3") <  Version("2"));
    assert(Version("1.2.3") <  Version("1.3"));
    assert(Version("1.2.3") <  Version("1.2.4"));
    assert(Version("1.2.3") <  Version("1.2.3.1"));
    assert(Version("1.2.3") <= Version("2"));
    assert(Version("1.2.3") <= Version("1.3"));
    assert(Version("1.2.3") <= Version("1.2.4"));
    assert(Version("1.2.3") <= Version("1.2.3.1"));
    assert(Version("1.2.3") <= Version("1.2.3"));
    assert(Version("1.2.3") == Version("1.2.3"));
    assert(Version("10.3")  <  Version("10.3.2"));
    assert(Version("10.3") <=  Version("10.3.2"));
    assert(!(Version("10.3.2") <  Version("10.3")));
    assert(!(Version("10.3.2") <= Version("10.3")));
    assert(Version("3.3 NVIDIA-10.0.19 310.90.10.05b1",
                   "NVIDIA GeForce GTX 680MX OpenGL Engine")
           == Version("310.90.10.5")                                    );

    assert(Version("4.1 NVIDIA-10.0.43 310.41.05f01",
                    "NVIDIA GeForce GTX 780M OpenGL Engine")
        == Version("310.41.05"));

    assert(Version("3.1 (Core Profile) Mesa 10.3.0",
                  "Mesa DRI Mobile Intel\u00ae GM45 Express Chipset")
           == Version("10.3.0")                                         );
    assert(Version("3.3 (Core Profile) Mesa 10.5.0-devel",
                   "Gallium 0.4 on NVC1")
           == Version("10.5.0")                                         );
    assert(Version("3.3 (Core Profile) Mesa 10.5.0-devel",
                   "Mesa DRI Intel(R) Sandybridge Mobile")
           == Version("10.5.0")                                         );
    assert(Version("2.1 Mesa 10.5.0-devel (git-82e919d)",
                   "Gallium 0.4 on i915 (chipse)")
           == Version("10.5.0")                                         );
    assert(Version("1.4 (3.0 Mesa 10.1.0)",
                   "Mesa DRI Intel(R) Ivybridge Mobile")
           == Version("10.1.0"));
    assert(Version("4.3.13283 Core Profile Context 14.501.1003.0",
                   "AMD Radeon R9 200 Series")
        == Version("14.501.1003.0"));
    assert(Version("4.0.10188 Core Profile Context",
                   "ATI Radeon HD 5400 Series")
        == Version("4.0.10188"));
    assert(Version("4.1 ATI-1.24.38", "AMD Radeon HD 6970M OpenGL Engine")
        == Version("1.24.38"));

}   // unitTesting

// ----------------------------------------------------------------------------
/** Reads in the graphical restriction file.
 *  \param driver_version The GL_VERSION string (i.e. opengl and version
 *         number).
 *  \param card_name The GL_RENDERER string (i.e. graphics card).
 *  \param vendor The GL_VENDOR string
 */
void init(const std::string &driver_version,
          const std::string &card_name,
          const std::string &vendor)
{
    for (unsigned int i = 0; i < GR_COUNT; i++)
        m_all_graphics_restriction.push_back(false);

    std::string filename =
        file_manager->getUserConfigFile("graphical_restrictions.xml");
    if (!file_manager->fileExists(filename))
        filename = file_manager->getAsset("graphical_restrictions.xml");
    const XMLNode *rules = file_manager->createXMLTree(filename);
    if (!rules)
    {
        Log::warn("Graphics", "Could not find graphical_restrictions.xm");
        return;
    }
    if (rules->getName() != "graphical-restrictions")
    {
        delete rules;
        Log::warn("Graphics", "'%s' did not contain graphical-restrictions tag",
            filename.c_str());
        return;
    }

    Version version(driver_version, card_name);
    for (unsigned int i = 0; i<rules->getNumNodes(); i++)
    {
        const XMLNode *xml_rule = rules->getNode(i);
        if (xml_rule->getName() != "card")
        {
            Log::warn("Graphics", "Incorrect node '%s' found in '%s' - ignored.",
                      xml_rule->getName().c_str(), filename.c_str());
            continue;
        }
        Rule rule(xml_rule);
        if (rule.applies(card_name, version, vendor))
        {
            std::vector<std::string> restrictions = rule.getRestrictions();
            std::vector<std::string>::iterator p;
            for (p = restrictions.begin(); p != restrictions.end(); p++)
            {
                GraphicsRestrictionsType t = getTypeForName(*p);
                if (t != GR_COUNT)
                    m_all_graphics_restriction[t] = true;
            }   // for p in rules
        }   // if m_all_rules[i].applies()
    }
    delete rules;
}   // init

// ----------------------------------------------------------------------------
/** Returns if the specified graphics restriction is defined.
 *  \param type The graphical restriction to tes.t
 */
bool isDisabled(GraphicsRestrictionsType type)
{
    return m_all_graphics_restriction[type];
}   // isDisabled

}   // namespace HardwareStats
