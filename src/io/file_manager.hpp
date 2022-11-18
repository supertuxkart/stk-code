//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2008-2015 Steve Baker, Joerg Henrichs
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
namespace irr
{
    class IrrlichtDevice;
    namespace io { class IFileSystem; }
}
using namespace irr;

#include "io/xml_node.hpp"
#include "utils/no_copy.hpp"

struct TextureSearchPath
{
    std::string m_texture_search_path;
    std::string m_container_id;

    TextureSearchPath(std::string path, std::string container_id) :
        m_texture_search_path(path), m_container_id(container_id)
    {
    }
};

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
                    GFX, GRANDPRIX, GUI_ICON, GUI_SCREEN, GUI_DIALOG,
                    REPLAY, SHADER, SKIN,  TTF, TRANSLATION, BUILTIN_ASSETS=TRANSLATION,
                    LIBRARY, MODEL, MUSIC, SFX, TEXTURE, SCRIPT, ASSET_MAX = SCRIPT,
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

    /** Name of stdout file. */
    static std::string m_stdout_filename;

    /** Directory of stdout file. */
    static std::string m_stdout_dir;

    /** Directory to store screenshots in. */
    std::string       m_screenshot_dir;

    /** Directory to store replays in. */
    std::string       m_replay_dir;

    /** Directory where resized textures are cached. */
    std::string       m_cached_textures_dir;

    /** Directory where user-defined grand prix are stored. */
    std::string       m_gp_dir;

    /** Location of the certificate bundle. */
    std::string       m_cert_bundle_location;

    /** Mobile stk specific to download stk-assets in the first. */
    std::string       m_stk_assets_download_dir;

    std::vector<TextureSearchPath> m_texture_search_path;

    std::vector<std::string>
                      m_model_search_path,
                      m_music_search_path;
    bool              findFile(std::string& full_path,
                               const std::string& fname,
                               const std::vector<std::string>& search_path)
                               const;
    bool              findFile(std::string& full_path,
                               const std::string& fname,
                               const std::vector<TextureSearchPath>& search_path)
                               const;
    void              makePath(std::string& path, const std::string& dir,
                               const std::string& fname) const;
    io::path          createAbsoluteFilename(const std::string &f);
    void              checkAndCreateConfigDir();
    void              checkAndCreateAddonsDir();
    void              checkAndCreateScreenshotDir();
    void              checkAndCreateReplayDir();
    void              checkAndCreateCachedTexturesDir();
    void              checkAndCreateGPDir();
    void              discoverPaths();
    void              addAssetsSearchPath();
    void              resetSubdir();
#if !defined(WIN32) && !defined(__APPLE__)
    std::string       checkAndCreateLinuxDir(const char *env_name,
                                             const char *dir_name,
                                             const char *fallback1,
                                             const char *fallback2=NULL);
#endif

public:
                      FileManager();
                     ~FileManager();
    void              init();
    void              reinitAfterDownloadAssets();
    static void       addRootDirs(const std::string &roots);
    static void       setStdoutName(const std::string &name);
    static void       setStdoutDir(const std::string &dir);
    io::IXMLReader   *createXMLReader(const std::string &filename);
    XMLNode          *createXMLTree(const std::string &filename);
    XMLNode          *createXMLTreeFromString(const std::string & content);

    std::string       getScreenshotDir() const;
    std::string       getReplayDir() const;
    std::string       getCachedTexturesDir() const;
    std::string       getGPDir() const;
    bool              checkAndCreateDirectory(const std::string &path);
    bool              checkAndCreateDirectoryP(const std::string &path);
    const std::string &getAddonsDir() const;
    std::string        getAddonsFile(const std::string &name);
    void checkAndCreateDirForAddons(const std::string &dir);
    static bool isDirectory(const std::string &path);
    bool removeFile(const std::string &name) const;
    bool removeDirectory(const std::string &name) const;
    // ------------------------------------------------------------------------
    bool moveDirectoryInto(std::string source, std::string target);
    // ------------------------------------------------------------------------
    bool copyFile(const std::string &source, const std::string &dest);
    std::vector<std::string>getMusicDirs() const;
    std::string getAssetChecked(AssetType type, const std::string& name,
                                bool abort_on_error=false) const;
    std::string getAsset(AssetType type, const std::string &name) const;
    std::string getAsset(const std::string &name) const;
    // ------------------------------------------------------------------------
    /** Returns the directory of an asset. */
    std::string getAssetDirectory(AssetType type) const
    {
        return m_subdir_name[type];
    }
    // ------------------------------------------------------------------------
    std::string searchMusic(const std::string& file_name) const;
    // ------------------------------------------------------------------------
    std::string searchModel(const std::string& file_name) const;
    std::string searchTexture(const std::string& fname) const;
    std::string getUserConfigFile(const std::string& fname) const;
    bool        fileExists(const std::string& path) const;
    // ------------------------------------------------------------------------
    /** Convenience function to save some typing in the
     *  file manager constructor. */
    bool        fileExists(const char *prefix, const std::string& path) const
    {
        return fileExists(std::string(prefix) + path);
    }
    // ------------------------------------------------------------------------
    bool searchTextureContainerId(std::string& container_id,
        const std::string& file_name) const;
    // ------------------------------------------------------------------------
    /** Returns the name of the stdout file for log messages. */
    static const std::string& getStdoutName() { return m_stdout_filename; }
    // ------------------------------------------------------------------------
    void        listFiles        (std::set<std::string>& result,
                                  const std::string& dir,
                                  bool make_full_path=false) const;


    void       pushTextureSearchPath(const std::string& path, const std::string& container_id);
    void       pushModelSearchPath(const std::string& path);
    void       popTextureSearchPath();
    void       popModelSearchPath();
    void       popMusicSearchPath();
    void       redirectOutput();

    bool       fileIsNewer(const std::string& f1, const std::string& f2) const;
    // ------------------------------------------------------------------------
    const std::string& getUserConfigDir() const   { return m_user_config_dir; }
    // ------------------------------------------------------------------------
    /** Returns the irrlicht file system. */
    irr::io::IFileSystem* getFileSystem() { return m_file_system; }
    // ------------------------------------------------------------------------
    /** Adds a directory to the music search path (or stack).
     */
    void pushMusicSearchPath(const std::string& path)
    {
        m_music_search_path.push_back(path);
    }   // pushMusicSearchPath
    // ------------------------------------------------------------------------
    /** Returns the full path to a shader (this function could be modified
     *  later to allow track-specific shaders).
     *  \param name Name of the shader.
     */
    std::string getShader(const std::string &name) const
    {
        return getAsset(SHADER, name);

    }   // getShader

    std::string getShadersDir() const
    {
        return m_subdir_name[SHADER];
    }
    // ------------------------------------------------------------------------
    const std::string& getSTKAssetsDownloadDir() const
                                          { return m_stk_assets_download_dir; }
    // ------------------------------------------------------------------------
    const std::string& getCertBundleLocation() const
                                             { return m_cert_bundle_location; }

};   // FileManager

extern FileManager* file_manager;

#endif
