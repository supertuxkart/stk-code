//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2008-2013 Steve Baker, Joerg Henrichs
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
 * Contains generic utility classes for file I/O (especially XML handling).
 */

#include <string>
#include <vector>
#include <set>

#include <irrString.h>
#include <IFileSystem.h>
namespace irr { class IrrlichtDevice; }
using namespace irr;

#include "io/xml_node.hpp"
#include "utils/no_copy.hpp"

/**
  * \brief class handling files and paths
  * \ingroup io
  */
class FileManager : public NoCopy
{
public:
    /** The various asset types (and directories) STK might request.
     *  The last entry ASSET_COUNT specifies the number of entries. */
    enum AssetType {ASSET_MIN,
                    CHALLENGE=ASSET_MIN,
                    FONT, GFX, GRANDPRIX, GUI, LIBRARY, MODEL, MUSIC,
                    SCRIPT, SFX, SHADER, SKIN, TEXTURE, 
                    TRANSLATION, ASSET_MAX = TRANSLATION,
                    ASSET_COUNT};
private:

    /** The names of the various subdirectories of the asset types. */
    std::vector< std::string > m_subdir_name;

    /** Handle to irrlicht's file systems. */
    io::IFileSystem  *m_file_system;

    /** Directory where user config files are stored. */
    std::string       m_user_config_dir;

    /** Directory where addons are stored. */
    std::string       m_addons_dir;

    /** The list of all root directories. */
    static std::vector<std::string> m_root_dirs;

    /** Directory to store screenshots in. */
    std::string       m_screenshot_dir;

    /** Directory where resized textures are cached. */
    std::string       m_cached_textures_dir;

    /** Directory where user-defined grand prix are stored. */
    std::string       m_gp_dir;

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
    void              checkAndCreateAddonsDir();
    void              checkAndCreateScreenshotDir();
    void              checkAndCreateCachedTexturesDir();
    void              checkAndCreateGPDir();
#if !defined(WIN32) && !defined(__CYGWIN__) && !defined(__APPLE__)
    std::string       checkAndCreateLinuxDir(const char *env_name,
                                             const char *dir_name,
                                             const char *fallback1,
                                             const char *fallback2=NULL);
#endif

public:
                      FileManager();
                     ~FileManager();
    void              reInit();
    void              dropFileSystem();
    static void       addRootDirs(const std::string &roots);
    io::IXMLReader   *createXMLReader(const std::string &filename);
    XMLNode          *createXMLTree(const std::string &filename);
    XMLNode          *createXMLTreeFromString(const std::string & content);

    std::string       getScreenshotDir() const;
    std::string       getCachedTexturesDir() const;
    std::string       getGPDir() const;
    std::string       getTextureCacheLocation(const std::string& filename);
    bool              checkAndCreateDirectoryP(const std::string &path);
    const std::string &getAddonsDir() const;
    std::string        getAddonsFile(const std::string &name);
    void checkAndCreateDirForAddons(const std::string &dir);
    bool removeFile(const std::string &name) const;
    bool removeDirectory(const std::string &name) const;
    bool copyFile(const std::string &source, const std::string &dest);
    std::vector<std::string>getMusicDirs() const;
    std::string getAssetChecked(AssetType type, const std::string& name,
                                bool abort_on_error=false) const;
    std::string getAsset(AssetType type, const std::string &name) const;
    std::string getAsset(const std::string &name) const;

    std::string searchMusic(const std::string& file_name) const;
    std::string searchTexture(const std::string& fname) const;
    std::string getUserConfigFile(const std::string& fname) const;
    bool        fileExists(const std::string& path) const;
    void        listFiles        (std::set<std::string>& result,
                                  const std::string& dir,
                                  bool make_full_path=false) const;


    void       pushTextureSearchPath(const std::string& path);
    void       pushModelSearchPath(const std::string& path);
    void       popTextureSearchPath();
    void       popModelSearchPath();
    void       popMusicSearchPath();
    void       redirectOutput();

    bool       fileIsNewer(const std::string& f1, const std::string& f2) const;

    // ------------------------------------------------------------------------
    /** Adds a directory to the music search path (or stack).
     */
    void pushMusicSearchPath(const std::string& path)
    {
        m_music_search_path.push_back(path);
    }   // pushMusicSearchPath

};   // FileManager

extern FileManager* file_manager;

#endif
