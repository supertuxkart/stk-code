//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#ifndef HEADER_FILE_MANAGER_HPP
#define HEADER_FILE_MANAGER_HPP

/**
 * \defgroup io
 */

#include <string>
#include <vector>
#include <set>

#include "irrlicht.h"
using namespace irr;

#include "io/xml_node.hpp"
#include "utils/no_copy.hpp"

/**
  * \brief class handling files and paths
  * \ingroup io
  */
class FileManager : public NoCopy
{
private:
    /** Handle to irrlicht's file systems. */
    io::IFileSystem  *m_file_system;
    /** Pointer to the irrlicht device. This is necessary before reInit is
     *  called to store the NULL device initially created. See Constructor
     *  for details. */
    IrrlichtDevice   *m_device;

    bool              m_is_full_path;
    /** Directory where user config files are stored. */
    std::string       m_config_dir;
    /** Directory where addons are stored. */
    std::string       m_addons_dir;
    /** Root data directory. */
    std::string       m_root_dir;
    std::vector<std::string>    
                      m_texture_search_path,
                      m_model_search_path,
                      m_music_search_path;
    bool              findFile(std::string& full_path,
                               const std::string& fname,
                               const std::vector<std::string>& search_path)
                               const;
    void              makePath(std::string& path, const std::string& dir,
                               const std::string& fname) const;
    bool              checkAndCreateDirectory(const std::string &path);
    io::path          createAbsoluteFilename(const std::string &f);
    void              checkAndCreateConfigDir();
    bool              isDirectory(const std::string &path) const;
#ifdef ADDONS_MANAGER
    void              checkAndCreateAddonsDir();
#endif
public:
                      FileManager(char *argv[]);
                     ~FileManager();
    void              setDevice(IrrlichtDevice *device);
    void              dropFileSystem();
    io::IXMLReader   *createXMLReader(const std::string &filename);
    XMLNode          *createXMLTree(const std::string &filename);

    std::string       getConfigDir() const;
    bool              checkAndCreateDirectoryP(const std::string &path);
#ifdef ADDONS_MANAGER
    const std::string &getAddonsDir() const;
    std::string        getAddonsFile(const std::string &name);
    void checkAndCreateDirForAddons(std::string addons_name,
                                    std::string addons_type);
    bool removeDirectory(const std::string &name);
#endif
    std::string getDataDir       () const;
    std::string getTranslationDir() const;
    std::string getGUIDir        () const;
    std::vector<std::string>getMusicDirs() const;
    std::string getTextureFile   (const std::string& fname) const;
    std::string getDataFile      (const std::string& fname) const;
    std::string getHighscoreFile (const std::string& fname) const;
    std::string getChallengeFile (const std::string& fname) const;
    std::string getLogFile       (const std::string& fname) const;
    std::string getItemFile      (const std::string& fname) const;
    std::string getMusicFile     (const std::string& fname) const;
    std::string getSFXFile       (const std::string& fname) const;
    std::string getFontFile      (const std::string& fname) const;
    std::string getModelFile     (const std::string& fname) const;
    void        listFiles        (std::set<std::string>& result,
                                  const std::string& dir,
                                  bool is_full_path=false,
                                  bool make_full_path=false) const;

    bool       fileExists           (const std::string& path)
        { return m_file_system->existFile(path.c_str()); }

    void       pushTextureSearchPath(const std::string& path);
    void       pushModelSearchPath  (const std::string& path);
    void       pushMusicSearchPath  (const std::string& path)
                                    { m_music_search_path.push_back(path);  }
    void       popTextureSearchPath ();
    void       popModelSearchPath   ();
    void       popMusicSearchPath   () {m_music_search_path.pop_back();     }

};

extern FileManager* file_manager;

#endif
