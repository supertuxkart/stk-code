// $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Lucas Baudin, Joerg Henrichs
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

#ifndef HEADER_ADDON_HPP
#define HEADER_ADDON_HPP

#include <string>

class XMLNode;

class Addon
{
public:
    /** The name to be displayed. */
    std::string m_name;
    /** Internal id for this addon, which is the name in lower case.
     *  This is used to create a subdirectory for this addon. */
    std::string m_id;
    /** The (highest) revision number available online. */
    int         m_revision;
    /** The currently installed revision. */
    int         m_installed_revision;
    /** The version of the icon that was downloaded. */
    int         m_icon_revision;
    /** A description of this addon. */
    std::string m_description;
    /** The URL of the icon (relative to the server) */
    std::string m_icon_url;
    /** Name of the icon to use. */
    std::string m_icon_basename;
    /** True if the icon is cached/loaded and can be displayed. */
    bool        m_icon_ready;
    /** The name of the zip file on the addon server. */
    std::string m_zip_file;
    /** True if the addon is installed. */
    bool        m_installed;
    /** Type, must be 'kart' or 'track'. */
    std::string m_type;

    Addon() {};
    /** Initialises the object from an XML node. */
    Addon(const XMLNode &xml);
    // ------------------------------------------------------------------------
    void writeXML(std::ofstream *out_stram);
    // ------------------------------------------------------------------------
    void copyInstallData(const Addon &addon);
    // ------------------------------------------------------------------------
    /** Returns the name of the addon. */
    const std::string& getName() const { return m_name; }
    // ------------------------------------------------------------------------
    /** Returns the type of the addon. */
    const std::string& getType() const { return m_type; }
    // ------------------------------------------------------------------------
    /** Returns the filename of the zip file with the addon. */
    const std::string& getZipFileName() const { return m_zip_file; }
    // ------------------------------------------------------------------------
    /** Returns the name of the icon of this addon. */
    const std::string& getIconURL() const { return m_icon_url; }
    // ------------------------------------------------------------------------
    /** Returns the name of the icon (i.e. the basename of the url). */
    const std::string getIconBasename() const { return m_icon_basename; }
    // ------------------------------------------------------------------------
    /** Returns the name of the addon. */
    const std::string& getDescription() const { return m_description; }
    // ------------------------------------------------------------------------
    /** Returns if the addon is installed. */
    bool  isInstalled() const { return m_installed; }
    // ------------------------------------------------------------------------
    /** Returns the installed revision number of an addon. */
    int   getInstalledRevision() const { return m_installed_revision; }
    // ------------------------------------------------------------------------
    /** Returns the latest revision number of this addon. 
    *  m_revision>m_installed_revision if a newer revision is available 
    *  online. */
    int   getRevision() const { return m_revision; }
    // ------------------------------------------------------------------------
    /** Returns the ID of this addon. */
    const std::string& getId() const { return m_id; }
    // ------------------------------------------------------------------------
    /** True if this addon needs to be updated. */
    bool needsUpdate() const
    {
        return m_installed && getInstalledRevision() < getRevision();
    }   // needsUpdate
    // ------------------------------------------------------------------------
    /** Returns true if the (cached) icon needs to be updated. This is the
     *  case if the addon revision number is higher than the revision number
     *  of the icon (this leaves some chance of false positives - i.e. icons
     *  that were not changed will still be downloaded). */
    bool iconNeedsUpdate() const
    {
        return m_revision > m_icon_revision;
    }   // iconNeedsUpdate
    // ------------------------------------------------------------------------
    /** Marks this addon to be installed. If the addon is marked as being
     *  installed, it also updates the installed revision number to be the 
     *  same as currently available revision number. */
    void setInstalled(bool state)
    {
        m_installed         = state;
        if(state)
            m_installed_revision = m_revision;
    }   // setInstalled
    // ------------------------------------------------------------------------
    /** Returns true if the icon of this addon was downloaded and is ready 
     *  to be displayed. */
    bool iconReady() const { return m_icon_ready; }
    // ------------------------------------------------------------------------
    /** Marks that the icon for this addon can be displayed. */
    void setIconReady() 
    { 
        m_icon_revision = m_revision;
        m_icon_ready=true; 
    }   // setIconReady
    // ------------------------------------------------------------------------
};   // Addon


#endif
