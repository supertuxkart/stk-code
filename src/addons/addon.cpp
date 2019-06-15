//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Lucas Baudin, Joerg Henrichs
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
/**
  \page addons Addons
  */

#include "addons/addon.hpp"

#include <fstream>
#include <time.h>

#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

Addon::SortOrder Addon::m_sort_order=Addon::SO_DEFAULT;

Addon::Addon(const XMLNode &xml)
{
    m_name               = "";
    m_id                 = "";
    m_designer           = "";
    m_status             = 0;
    m_installed          = false;
    m_installed_revision = 0;
    m_revision           = 0 ;
    m_zip_file           = "";
    m_description        = "";
    m_icon_url           = "";
    m_icon_basename      = "";
    m_icon_revision      = 0;
    m_size               = 0;
    m_date               = 0;
    m_min_include_ver    = "";
    m_max_include_ver    = "";
    m_rating             = 0.0f;
    m_icon_ready         = false;
    m_still_exists       = false;
    m_type               = xml.getName();

    // FIXME: temporarily till the web page is updated.
    if(m_type=="track")
    {
        int is_arena=0;
        xml.get("arena",          &is_arena            );
        if(is_arena) m_type="arena";
    }

    std::string name;
    std::string description;
    std::string designer;

    xml.get("name",               &name                );
    m_name     = StringUtils::xmlDecode(name);
    m_dir_name = StringUtils::toLowerCase(name);
    xml.get("id",                 &m_dir_name          );
    m_id = createAddonId(m_dir_name);
    xml.get("designer",           &designer            );
    xml.get("status",             &m_status            );

    int64_t tmp;
    xml.get("date",               &tmp                 );
    m_date = tmp;

    xml.get("installed",          &m_installed         );
    xml.get("installed-revision", &m_installed_revision);
    xml.get("revision",           &m_revision          );
    xml.get("file",               &m_zip_file          );
    xml.get("description",        &description         );

    m_description = StringUtils::xmlDecode(description);
    m_designer = StringUtils::xmlDecode(designer);

    // resolve XML entities
    //m_description = StringUtils::replace(m_description, "&#10;", "\n");
    //m_description = StringUtils::replace(m_description, "&#13;", ""); // ignore \r

    if(!xml.get("image", &m_icon_url))
    {
        // If an addon does not exist on the server anymore, it does not
        // have an image. In this case use the icon information which is
        // stored in the addons_installed file
        xml.get("icon-name", &m_icon_basename);
    }
    else
        m_icon_basename = StringUtils::getBasename(m_icon_url);

    xml.get("icon-revision",      &m_icon_revision     );
    xml.get("size",               &m_size              );

    xml.get("min-include-version",&m_min_include_ver   );
    xml.get("max-include-version",&m_max_include_ver   );

    xml.get("rating",             &m_rating            );

};   // Addon(const XML&)

// ----------------------------------------------------------------------------
/** Copies the installation data (like description, revision, icon) from the
 *  downloaded online list to this entry.
*/
void Addon::copyInstallData(const Addon &addon)
{
    m_description   = addon.m_description;
    m_dir_name      = addon.m_dir_name;
    m_revision      = addon.m_revision;
    m_zip_file      = addon.m_zip_file;
    m_icon_url      = addon.m_icon_url;
    m_icon_basename = addon.m_icon_basename;
    m_icon_revision = addon.m_revision;
    m_designer      = addon.m_designer;
    m_status        = addon.m_status;
    m_date          = addon.m_date;
    m_min_include_ver=addon.m_min_include_ver;
    m_max_include_ver=addon.m_max_include_ver;
    m_rating        = addon.m_rating;
    // Support if the type of an addon changes, e.g. this ie necessary
    // when we introduce 'arena' as type (formerly arenas had type 'track').
    m_type          = addon.m_type;
}   // copyInstallData

// ----------------------------------------------------------------------------
/** Writes information about an installed addon (it is only called for
 *  installed addons).
 *  \param out_stream Output stream to write to.
 */
void Addon::writeXML(std::ofstream *out_stream)
{
    // We write m_dir_name as 'id' to stay backwards compatible
    (*out_stream) << "  <"                       << m_type
                  << " name=\""
                  << StringUtils::xmlEncode(m_name)
                  << "\" id=\""                  << m_dir_name
                  << "\" designer=\""
                  << StringUtils::xmlEncode(m_designer)
                  << "\" status=\""              << m_status
                  << "\" date=\""                << m_date
                  << "\" installed=\""
                  << (m_installed ? "true" : "false" )
                  << "\" installed-revision=\""  << m_installed_revision
                  << "\" size=\""                << m_size
                  << "\" icon-revision=\""       << m_icon_revision
                  << "\" icon-name=\"" << m_icon_basename
                  << "\"/>\n";
}   // writeXML

// ----------------------------------------------------------------------------
std::string Addon::getDateAsString() const
{
    return StkTime::toString(m_date);
}   // getDateAsString

// ----------------------------------------------------------------------------
bool Addon::testIncluded(const std::string &min_ver, const std::string &max_ver)
{
    if (min_ver.length() == 0 || max_ver.length() == 0)
        return false;

    int current_version = StringUtils::versionToInt(STK_VERSION);
    int min_version = StringUtils::versionToInt(min_ver);
    int max_version = StringUtils::versionToInt(max_ver);

    return (min_version <= current_version && max_version >= current_version);
}

// ----------------------------------------------------------------------------
/**
 * \brief Filter the add-on with a list of words.
 * \param words A list of words separated by ' '.
 * \return true if the add-on contains one of the words, otherwise false.
 */
bool Addon::filterByWords(const core::stringw words) const
{
    if (words == NULL || words.empty())
        return true;

    std::vector<core::stringw> list = StringUtils::split(words, ' ', false);

    for (unsigned int i = 0; i < list.size(); i++)
    {
        list[i].make_lower();
        
        core::stringw name = core::stringw(m_name).make_lower();
        if (name.find(list[i].c_str()) != -1)
        {
            return true;
        }
        
        core::stringw designer = core::stringw(m_designer).make_lower();
        if (designer.find(list[i].c_str()) != -1)
        {
            return true;
        }
        
        core::stringw description = core::stringw(m_description).make_lower();
        if (description.find(list[i].c_str()) != -1)
        {
            return true;
        }
    }
    
    return false;
} // filterByWords

// ----------------------------------------------------------------------------
/** Deletes the icon file of this addon, and marks it to be re-downloaded (next
 *  time AddonsManager::downloadIcons() is called.
 */
void Addon::deleteInvalidIconFile()
{
    m_icon_ready = false;
    std::string icon = file_manager->getAddonsFile("icons/"+m_icon_basename);
    file_manager->removeFile(icon);
    m_installed = false;
}   // redownloadIcon

// ----------------------------------------------------------------------------
/** A static function that checks if the given ID is an addon. This is
 *  done by testing if the directory name is in the addons directory.
 */
bool Addon::isAddon(const std::string &directory)
{
    return StringUtils::startsWith(directory,file_manager->getAddonsDir());
}   // isAddon

// ----------------------------------------------------------------------------
/** Returns the directory in which this addon is installed. */
std::string Addon::getDataDir() const
{
    return file_manager->getAddonsFile(getTypeDirectory()+m_dir_name);
}   // getDataDir
