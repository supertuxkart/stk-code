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


#include "io/file_manager.hpp"

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/skin.hpp"
#include "karts/kart_properties_manager.hpp"
#include "tracks/track_manager.hpp"
#include "utils/command_line.hpp"
#include "utils/extract_mobile_assets.hpp"
#include "utils/file_utils.hpp"
#include "utils/log.hpp"
#include "utils/mem_utils.hpp"
#include "utils/string_utils.hpp"

#ifdef ANDROID
#include "io/assets_android.hpp"
#endif

#ifdef __HAIKU__
#include <Path.h>
#include <FindDirectory.h>
#endif

#include <stdio.h>
#include <stdexcept>
#include <sstream>
#include <sys/stat.h>
#include <iostream>
#include <string>

#include <IFileSystem.h>

namespace irr {
    namespace io
    {
        IFileSystem* createFileSystem();
    }
}

// For mkdir
#if !defined(WIN32)
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <dirent.h>
#  include <unistd.h>
#else
#  define WIN32_LEAN_AND_MEAN
#  include <direct.h>
#  include <windows.h>
#  include <stdio.h>
#  if !defined(__MINGW32__)
     /*Needed by the remove directory function */
#    define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#    define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#  endif
#endif


std::vector<std::string> FileManager::m_root_dirs;
std::string              FileManager::m_stdout_filename = "stdout.log";
std::string              FileManager::m_stdout_dir;

#ifdef __APPLE__
// dynamic data path detection onmac
#  include <CoreFoundation/CoreFoundation.h>

bool macSetBundlePathIfRelevant(std::string& data_dir)
{
    Log::debug("[FileManager]", "Checking whether we are using an app bundle... ");
    // the following code will enable STK to find its data when placed in an
    // app bundle on mac OS X.
    // returns true if path is set, returns false if path was not set
    char path[1024];
    CFBundleRef main_bundle = CFBundleGetMainBundle(); assert(main_bundle);
    CFURLRef main_bundle_URL = CFBundleCopyBundleURL(main_bundle);
    assert(main_bundle_URL);
    CFStringRef cf_string_ref = CFURLCopyFileSystemPath(main_bundle_URL,
                                                        kCFURLPOSIXPathStyle);
    assert(cf_string_ref);
    CFStringGetCString(cf_string_ref, path, 1024, kCFStringEncodingUTF8);
    CFRelease(main_bundle_URL);
    CFRelease(cf_string_ref);

    // IOS version of stk store data folder directly inside supertuxkart.app
    std::string contents = std::string(path) + "/";
#ifndef IOS_STK
    contents += std::string("Contents");
#endif
    if(contents.find(".app") != std::string::npos)
    {
#ifdef IOS_STK
        data_dir = contents;
        return true;
#else
        Log::debug("[FileManager]", "yes");
        // executable is inside an app bundle, use app bundle-relative paths
        data_dir = contents + std::string("/Resources/");
        return true;
#endif
    }
    else
    {
        Log::debug("[FileManager]", "no");
        return false;
    }
}
#endif

// ============================================================================
FileManager* file_manager = 0;

/** The constructor of the file manager creates an irrlicht file system and
 *  detects paths for the user config file and assets base directory (data).
 *  A second initialisation is done later once (see init()), once the user
 *  config file is read. This is necessary since part of discoverPaths
 *  depend on artist debug mode.
 */
FileManager::FileManager()
{
    m_root_dirs.clear();
    resetSubdir();
#ifdef __APPLE__
    // irrLicht's createDevice method has a nasty habit of messing the CWD.
    // since the code above may rely on it, save it to be able to restore
    // it after.
    char buffer[256];
    getcwd(buffer, 256);
#endif

#ifdef __APPLE__
    chdir( buffer );
#endif

    m_file_system = irr::io::createFileSystem();

#ifdef ANDROID
    AssetsAndroid android_assets(this);
    android_assets.init();
#endif

    std::string exe_path;

    // Search for the root directory
    // =============================

    // Also check for data dirs relative to the path of the executable.
    // This is esp. useful for Visual Studio, since it's not necessary
    // to define the working directory when debugging, it works automatically.
    std::string root_dir;
    const std::string version = std::string("supertuxkart.") + STK_VERSION;
    if (fileExists(CommandLine::getExecName()))
    {
        exe_path = StringUtils::getPath(CommandLine::getExecName());
    }
    if(exe_path.size()==0 || exe_path[exe_path.size()-1]!='/')
        exe_path += "/";
    if ( getenv ( "SUPERTUXKART_DATADIR" ) != NULL )
        root_dir = std::string(getenv("SUPERTUXKART_DATADIR"))+"/data/" ;
#ifdef __APPLE__
    else if( macSetBundlePathIfRelevant( root_dir ) ) { root_dir = root_dir + "data/"; }
#endif
#ifdef __SWITCH__
    else if(fileExists("sdmc:/stk-data/", version))
        root_dir = "sdmc:/stk-data/";
    else if(fileExists("romfs:/data/", version))
        root_dir = "romfs:/data/";
#endif
    else if(fileExists("./data/", version))
        root_dir = "./data/" ;
    else if(fileExists("../data/", version))
        root_dir = "../data/" ;
    else if(fileExists("../../data/", version))
        root_dir = "../../data/" ;
    // Test for old style build environment, with executable in root of stk
    else if(fileExists(exe_path+"data/"+version))
        root_dir = (exe_path+"data/").c_str();
    // Check for windows cmake style: bld/Debug/bin/supertuxkart.exe
    else if (fileExists(exe_path + "../../../data/"+version))
        root_dir = exe_path + "../../../data/";
    else if (fileExists(exe_path + "../data/"+version))
    {
        root_dir = exe_path.c_str();
        root_dir += "../data/";
    }
    else
    {
#ifdef SUPERTUXKART_DATADIR
        root_dir = SUPERTUXKART_DATADIR"/data/";
#else
        root_dir = "/usr/local/share/games/supertuxkart/";
#endif
    }

    if (!m_file_system->existFile((root_dir + version).c_str()))
    {
        Log::error("FileManager", "Could not find file '%s'in any "
                   "standard location (esp. ../data).", version.c_str());
        Log::error("FileManager",
                   "Last location checked '%s'.", root_dir.c_str());
        Log::fatal("FileManager",
                   "Set $SUPERTUXKART_DATADIR to point to the data directory.");
        // fatal will exit the application
    }

    addRootDirs(root_dir);

    std::string assets_dir;
#ifdef MOBILE_STK
    m_stk_assets_download_dir = getenv("HOME");
#ifdef IOS_STK
    m_stk_assets_download_dir += "/Library/Application Support/SuperTuxKart/stk-assets/";
#elif defined (ANDROID)
    m_stk_assets_download_dir += "/stk-assets/";
#else
#error You must set m_stk_assets_download_dir to appropriate place for your platform
#endif

#else
    if (getenv("SUPERTUXKART_ASSETS_DIR") != NULL)
    {
        assets_dir = std::string(getenv("SUPERTUXKART_ASSETS_DIR"));
    }
    else if (fileExists(root_dir + "../../stk-assets"))
    {
        assets_dir = root_dir + "../../stk-assets";
    }
    else if (fileExists(root_dir + "../../supertuxkart-assets"))
    {
        assets_dir = root_dir + "../../supertuxkart-assets";
    }
    else if (getenv("SUPERTUXKART_ROOT_PATH") != NULL)
    {
        //is this needed?
        assets_dir = std::string(getenv("SUPERTUXKART_ROOT_PATH"));
    }
#endif
    if (!assets_dir.empty() && assets_dir != root_dir)
    {
        addRootDirs(assets_dir);
    }

    checkAndCreateConfigDir();
    checkAndCreateAddonsDir();
    checkAndCreateScreenshotDir();
    checkAndCreateReplayDir();
    checkAndCreateCachedTexturesDir();
    checkAndCreateGPDir();

    redirectOutput();
}   // FileManager

// ----------------------------------------------------------------------------
/** Reset subdirectories to initial state, for example after download assets
 */
void FileManager::resetSubdir()
{
    m_subdir_name.clear();
    m_subdir_name.resize(ASSET_COUNT);
    m_subdir_name[CHALLENGE  ] = "challenges";
    m_subdir_name[GFX        ] = "gfx";
    m_subdir_name[GRANDPRIX  ] = "grandprix";
    m_subdir_name[GUI_ICON   ] = "gui/icons";
    m_subdir_name[GUI_SCREEN ] = "gui/screens";
    m_subdir_name[GUI_DIALOG ] = "gui/dialogs";
    m_subdir_name[LIBRARY    ] = "library";
    m_subdir_name[MODEL      ] = "models";
    m_subdir_name[MUSIC      ] = "music";
    m_subdir_name[REPLAY     ] = "replay";
    m_subdir_name[SCRIPT     ] = "tracks";
    m_subdir_name[SFX        ] = "sfx";
    m_subdir_name[SKIN       ] = "skins";
    m_subdir_name[SHADER     ] = "shaders";
    m_subdir_name[TEXTURE    ] = "textures";
    m_subdir_name[TTF        ] = "ttf";
    m_subdir_name[TRANSLATION] = "po";
}   // resetSubdir

// ----------------------------------------------------------------------------
/** Detects where the assets are stored.
 */
void FileManager::discoverPaths()
{
    resetSubdir();
    // We can't use _() here, since translations will only be initalised
    // after the filemanager (to get the path to the tranlsations from it)
    for(unsigned int i=0; i<m_root_dirs.size(); i++)
        Log::info("[FileManager]", "Data files will be fetched from: '%s'",
                   m_root_dirs[i].c_str());
    Log::info("[FileManager]", "User directory is '%s'.",
              m_user_config_dir.c_str());
    Log::info("[FileManager]", "Addons files will be stored in '%s'.",
               m_addons_dir.c_str());
    Log::info("[FileManager]", "Screenshots will be stored in '%s'.",
               m_screenshot_dir.c_str());
    Log::info("[FileManager]", "User-defined grand prix will be stored in '%s'.",
               m_gp_dir.c_str());

    // Reset for re-downloading assets if needed
    TrackManager::removeTrackSearchDirs();
    KartPropertiesManager::removeKartSearchDirs();

    /** Now search for the path to all needed subdirectories. */
    // ==========================================================
    // This must be done here since otherwise translations will not be found.
    std::vector<bool> dir_found;
    dir_found.resize(ASSET_COUNT, false);
#ifdef MOBILE_STK
    assert(!m_root_dirs.empty());
    for (unsigned j = ASSET_MIN; j <= BUILTIN_ASSETS; j++)
    {
        if (!dir_found[j] && fileExists(m_root_dirs[0] + m_subdir_name[j]))
        {
            dir_found[j] = true;
            m_subdir_name[j] = m_root_dirs[0] + m_subdir_name[j] + "/";
        }
    }

    bool has_full_assets = ExtractMobileAssets::hasFullAssets();
    // Clear previous assets version to free space
    if (!has_full_assets && fileExists(m_stk_assets_download_dir))
        removeDirectory(m_stk_assets_download_dir);

    // Use stk-assets-full for karts, tracks, textures..., otherwise in data/
    std::string assets_root = has_full_assets ?
        m_stk_assets_download_dir : m_root_dirs[0];
    for (unsigned j = LIBRARY; j <= ASSET_MAX; j++)
    {
        if (!dir_found[j] && fileExists(assets_root + m_subdir_name[j]))
        {
            dir_found[j] = true;
            m_subdir_name[j] = assets_root + m_subdir_name[j] + "/";
        }
    }
    if (fileExists(assets_root + "tracks/"))
        TrackManager::addTrackSearchDir(assets_root + "tracks/");
    if (fileExists(assets_root + "karts/"))
        KartPropertiesManager::addKartSearchDir(assets_root + "karts/");

    if (UserConfigParams::m_artist_debug_mode)
    {
        if (fileExists(assets_root + "wip-tracks/"))
            TrackManager::addTrackSearchDir(assets_root + "wip-tracks/");
        if (fileExists(assets_root + "wip-karts/"))
            KartPropertiesManager::addKartSearchDir(assets_root + "wip-karts/");
    }
#else
    for(unsigned int i=0; i<m_root_dirs.size(); i++)
    {
        if(fileExists(m_root_dirs[i]+"tracks/"))
            TrackManager::addTrackSearchDir(m_root_dirs[i]+"tracks/");
        if(fileExists(m_root_dirs[i]+"karts/"))
            KartPropertiesManager::addKartSearchDir(m_root_dirs[i]+"karts/");

        // If artist debug mode is enabled, add
        // work-in-progress tracks and karts
        if (UserConfigParams::m_artist_debug_mode)
        {
            if(fileExists(m_root_dirs[i] + "wip-tracks/"))
                TrackManager::addTrackSearchDir(m_root_dirs[i] + "wip-tracks/");
            if(fileExists(m_root_dirs[i] + "wip-karts/"))
                KartPropertiesManager::addKartSearchDir(m_root_dirs[i] + "wip-karts/");
        }
        for(unsigned int j=ASSET_MIN; j<=ASSET_MAX; j++)
        {
            if(!dir_found[j] && fileExists(m_root_dirs[i]+m_subdir_name[j]))
            {
                dir_found[j] = true;
                m_subdir_name[j] = m_root_dirs[i]+m_subdir_name[j]+"/";
            }   // !dir_found && file_exist
        }   // for j=ASSET_MIN; j<=ASSET_MAX
    }   // for i<m_root_dirs
#endif

    bool was_error = false;
    for(unsigned int i=ASSET_MIN; i<=ASSET_MAX; i++)
    {
        if(!dir_found[i])
        {
            Log::warn("[FileManager]", "Directory '%s' not found, aborting.",
                      m_subdir_name[i].c_str());
            was_error = true;
        }
        else
            Log::info("[FileManager]", "Asset %d will be loaded from '%s'.",
                      i, m_subdir_name[i].c_str());
    }
    if(was_error)
        Log::fatal("[FileManager]", "Not all assets found - aborting.");

}  // discoverPaths

//-----------------------------------------------------------------------------
/** This function is used to initialise the file-manager after reading in
 *  the user configuration data. Esp. discovering the paths of all assets
 *  depends on the user config file (artist debug mode).
 */
void FileManager::init()
{
    discoverPaths();
    addAssetsSearchPath();
    m_cert_bundle_location = FileUtils::getPortableWritingPath(
        m_file_system->getAbsolutePath(getAsset("cacert.pem").c_str()).c_str());

    // Clean up left-over files in addons/tmp that are older than 24h
    // ==============================================================
    // (The 24h delay is useful when debugging a problem with a zip file)
    // We do when starting STK because for mobile STK destructor of file
    // manager may never be called if only home button is pressed
    std::set<std::string> allfiles;
    std::string tmp=getAddonsFile("tmp");
    listFiles(allfiles, tmp);
    for(std::set<std::string>::iterator i=allfiles.begin();
        i!=allfiles.end(); i++)
    {
        if((*i)=="." || (*i)=="..") continue;
        // For now there should be only zip files or .part files
        // (not fully downloaded files) in tmp. Warn about any
        // other files.
        std::string full_path=tmp+"/"+*i;
        if(StringUtils::getExtension(*i)!="zip" &&
           StringUtils::getExtension(*i)!="part"    )
        {
            Log::warn("[FileManager]", "Unexpected tmp file '%s' found.",
                       full_path.c_str());
            continue;
        }
        if(isDirectory(full_path))
        {
            // Gee, a .zip file which is a directory - stay away from it
            Log::warn("[FileManager]", "'%s' is a directory and will not be deleted.",
                      full_path.c_str());
            continue;
        }
        struct stat mystat;
        FileUtils::statU8Path(full_path, &mystat);
        StkTime::TimeType current = StkTime::getTimeSinceEpoch();
        if(current - mystat.st_ctime <24*3600)
        {
            if(UserConfigParams::logAddons())
                Log::verbose("[FileManager]", "'%s' is less than 24h old "
                             "and will not be deleted.",
                             full_path.c_str());
            continue;
        }
        if(UserConfigParams::logAddons())
            Log::verbose("[FileManager]", "Deleting tmp file'%s'.",full_path.c_str());
        removeFile(full_path);

    }   // for i in all files in tmp
}   // init

//-----------------------------------------------------------------------------
void FileManager::addAssetsSearchPath()
{
    // Note that we can't push the texture search path in the constructor
    // since this also adds a file archive to the file system - and
    // m_file_system is deleted (in irr_driver)
    pushTextureSearchPath(m_subdir_name[TEXTURE], "textures");
    pushTextureSearchPath(m_subdir_name[TEXTURE]+"skybox/", "skybox");

    if (fileExists(m_subdir_name[TEXTURE]+"deprecated/"))
        pushTextureSearchPath(m_subdir_name[TEXTURE]+"deprecated/", "deprecatedtex");

    pushTextureSearchPath(m_subdir_name[GUI_ICON], "gui/icons");

    pushModelSearchPath  (m_subdir_name[MODEL]);
    pushMusicSearchPath  (m_subdir_name[MUSIC]);

    // Add more paths from the STK_MUSIC_PATH environment variable
    if(getenv("SUPERTUXKART_MUSIC_PATH")!=NULL)
    {
        std::string path=getenv("SUPERTUXKART_MUSIC_PATH");
        std::vector<std::string> dirs = StringUtils::splitPath(path);
        for(int i=0;i<(int)dirs.size(); i++)
            pushMusicSearchPath(dirs[i]);
    }
}   // addAssetsSearchPath

//-----------------------------------------------------------------------------
void FileManager::reinitAfterDownloadAssets()
{
    m_file_system->removeAllFileArchives();
    m_texture_search_path.clear();
    m_model_search_path.clear();
    m_music_search_path.clear();
    discoverPaths();
    addAssetsSearchPath();
    // Add back addons search path
    KartPropertiesManager::addKartSearchDir(
                 file_manager->getAddonsFile("karts/"));
    track_manager->addTrackSearchDir(
                 file_manager->getAddonsFile("tracks/"));
}   // reinitAfterDownloadAssets

//-----------------------------------------------------------------------------
FileManager::~FileManager()
{
    // Clean up rest of file manager
    // =============================
    popMusicSearchPath();
    popModelSearchPath();
    popTextureSearchPath();
    popTextureSearchPath();
    m_file_system->drop();
    m_file_system = NULL;
}   // ~FileManager

// ----------------------------------------------------------------------------
/** Returns true if the specified file exists.
 */
bool FileManager::fileExists(const std::string& path) const
{
#ifdef DEBUG
    bool exists = m_file_system->existFile(path.c_str());
    if(exists) return true;
    // Now the original file was not found. Test if replacing \ with / helps:
    std::string s = StringUtils::replace(path, "\\", "/");
    exists = m_file_system->existFile(s.c_str());
    if(exists)
        Log::warn("FileManager", "File '%s' does not exists, but '%s' does!",
        path.c_str(), s.c_str());
    return exists;
#else
    return m_file_system->existFile(path.c_str());
#endif
}   // fileExists
//-----------------------------------------------------------------------------
/** Adds paths to the list of stk root directories.
 *  \param roots A ":" separated string of directories to add.
 */
void FileManager::addRootDirs(const std::string &roots)
{
    std::vector<std::string> all = StringUtils::splitPath(roots);
    for(unsigned int i=0; i<all.size(); i++)
    {
        if(all[i].size()==0 || all[i][all[i].size()-1]!='/')
            all[i] += "/";
        m_root_dirs.push_back(all[i]);
    }
}   // addRootDirs

//-----------------------------------------------------------------------------
io::IXMLReader *FileManager::createXMLReader(const std::string &filename)
{
    return m_file_system->createXMLReader(filename.c_str());
}   // getXMLReader
//-----------------------------------------------------------------------------
/** Reads in a XML file and converts it into a XMLNode tree.
 *  \param filename Name of the XML file to read.
 */
XMLNode *FileManager::createXMLTree(const std::string &filename)
{
    try
    {
        XMLNode* node = new XMLNode(filename);
        return node;
    }
    catch (std::runtime_error& e)
    {
        if (UserConfigParams::logMisc())
        {
            Log::error("[FileManager]", "createXMLTree: %s", e.what());
        }
        return NULL;
    }
}   // createXMLTree

//-----------------------------------------------------------------------------
/** Reads in XML from a string and converts it into a XMLNode tree.
 *  \param content the string containing the XML content.
 */
XMLNode *FileManager::createXMLTreeFromString(const std::string & content)
{
    try
    {
        char *b = new char[content.size()];
        assert(b);
        memcpy(b, content.c_str(), content.size());
        io::IReadFile * ireadfile =
            m_file_system->createMemoryReadFile(b, (int)content.size(),
                                                "tempfile", true);
        io::IXMLReader * reader = m_file_system->createXMLReader(ireadfile);
        XMLNode* node = new XMLNode(reader);
        reader->drop();
        ireadfile->drop();
        return node;
    }
    catch (std::runtime_error& e)
    {
        if (UserConfigParams::logMisc())
        {
            Log::error("[FileManager]", "createXMLTreeFromString: %s", e.what());
        }
        return NULL;
    }
}   // createXMLTreeFromString

//-----------------------------------------------------------------------------
/** In order to add and later remove paths we have to specify the absolute
 *  filename (and replace '\' with '/' on windows).
 */
io::path FileManager::createAbsoluteFilename(const std::string &f)
{
    io::path abs_path=m_file_system->getAbsolutePath(f.c_str());
    abs_path=m_file_system->flattenFilename(abs_path);
    return abs_path;
}   // createAbsoluteFilename

//-----------------------------------------------------------------------------
/** Adds a model search path to the list of model search paths.
 *  This path will be searched before any other existing paths.
 */
void FileManager::pushModelSearchPath(const std::string& path)
{
    m_model_search_path.push_back(path);
    std::unique_lock<std::recursive_mutex> ul = m_file_system->acquireFileArchivesMutex();

    const int n=m_file_system->getFileArchiveCount();
    m_file_system->addFileArchive(createAbsoluteFilename(path),
                                  /*ignoreCase*/false,
                                  /*ignorePaths*/false,
                                  io::EFAT_FOLDER);
    // A later added file archive should be searched first (so that
    // track specific models are found before models in data/models).
    // This is not necessary if this is the first member, or if the
    // addFileArchive call did not add this file systems (this can
    // happen if the file archive has been added prevously, which
    // commonly happens since each kart/track specific path is added
    // twice: once for textures and once for models).
    if(n>0 && (int)m_file_system->getFileArchiveCount()>n)
    {
        // In this case move the just added file archive
        // (which has index n) to position 0 (by -n positions):
        m_file_system->moveFileArchive(n, -n);
    }
}   // pushModelSearchPath

//-----------------------------------------------------------------------------
/** Adds a texture search path to the list of texture search paths.
 *  This path will be searched before any other existing paths.
 */
void FileManager::pushTextureSearchPath(const std::string& path, const std::string& container_id)
{
    m_texture_search_path.push_back(TextureSearchPath(path, container_id));
    std::unique_lock<std::recursive_mutex> ul = m_file_system->acquireFileArchivesMutex();

    const int n=m_file_system->getFileArchiveCount();
    m_file_system->addFileArchive(createAbsoluteFilename(path),
                                  /*ignoreCase*/false,
                                  /*ignorePaths*/false,
                                  io::EFAT_FOLDER);
    // A later added file archive should be searched first (so that
    // e.g. track specific textures are found before textures in
    // data/textures).
    // This is not necessary if this is the first member, or if the
    // addFileArchive call did not add this file systems (this can
    // happen if the file archive has been added previously, which
    // commonly happens since each kart/track specific path is added
    // twice: once for textures and once for models).
    if(n>0 && (int)m_file_system->getFileArchiveCount()>n)
    {
        // In this case move the just added file archive
        // (which has index n) to position 0 (by -n positions):
        m_file_system->moveFileArchive(n, -n);
    }
}   // pushTextureSearchPath

//-----------------------------------------------------------------------------
/** Removes the last added texture search path from the list of paths.
 */
void FileManager::popTextureSearchPath()
{
    if (!m_texture_search_path.empty())
    {
        TextureSearchPath dir = m_texture_search_path.back();
        m_texture_search_path.pop_back();
        m_file_system->removeFileArchive(createAbsoluteFilename(dir.m_texture_search_path));
    }
}   // popTextureSearchPath

//-----------------------------------------------------------------------------
/** Removes the last added model search path from the list of paths.
 */
void FileManager::popModelSearchPath()
{
    if (!m_model_search_path.empty())
    {
        std::string dir = m_model_search_path.back();
        m_model_search_path.pop_back();
        m_file_system->removeFileArchive(createAbsoluteFilename(dir));
    }
}   // popModelSearchPath

// ------------------------------------------------------------------------
/** Removes the last added directory from the music search path.
 */
void FileManager::popMusicSearchPath()
{
    if(!m_music_search_path.empty())
    {
        m_music_search_path.pop_back();
    }
}

//-----------------------------------------------------------------------------
/** Tries to find the specified file in any of the given search paths.
 *  \param full_path On return contains the full path of the file, or
 *         "" if the file is not found.
 *  \param file_name The name of the file to look for.
 *  \param search_path The list of paths to search for the file.
 *  \return True if the file is found, false otherwise.
 */
bool FileManager::findFile(std::string& full_path,
                      const std::string& file_name,
                      const std::vector<std::string>& search_path) const
{
    for(std::vector<std::string>::const_reverse_iterator
        i = search_path.rbegin();
        i != search_path.rend(); ++i)
    {
        full_path = *i + file_name;
        if(m_file_system->existFile(full_path.c_str())) return true;
    }
    full_path="";
    return false;
}   // findFile

//-----------------------------------------------------------------------------
/** Tries to find the specified file in any of the given search paths.
*  \param full_path On return contains the full path of the file, or
*         "" if the file is not found.
*  \param file_name The name of the file to look for.
*  \param search_path The list of paths to search for the file.
*  \return True if the file is found, false otherwise.
*/
bool FileManager::findFile(std::string& full_path,
    const std::string& file_name,
    const std::vector<TextureSearchPath>& search_path) const
{
    for (std::vector<TextureSearchPath>::const_reverse_iterator
        i = search_path.rbegin();
        i != search_path.rend(); ++i)
    {
        full_path = i->m_texture_search_path + file_name;
        if (m_file_system->existFile(full_path.c_str())) return true;
    }
    full_path = "";
    return false;
}   // findFile

//-----------------------------------------------------------------------------
std::string FileManager::getAssetChecked(FileManager::AssetType type,
                                         const std::string& name,
                                         bool abort_on_error) const
{
    std::string path = m_subdir_name[type]+name;
    if(fileExists(path))
        return path;

    if(abort_on_error)
    {
        Log::fatal("[FileManager]", "Can not find file '%s' in '%s'",
                   name.c_str(), m_subdir_name[type].c_str());
    }
    return "";
}   // getAssetChecked

//-----------------------------------------------------------------------------
/** Returns the full path of a file of the given asset class. It is not
 *  checked if the file actually exists (use getAssetChecked() instead if
 *  checking is needed).
 *  \param type Type of the asset class.
 *  \param name Name of the file to search.
 *  \return Full path to the file.
 */
std::string FileManager::getAsset(FileManager::AssetType type,
                                  const std::string &name) const
{
    if (type == GUI_ICON && GUIEngine::getSkin()->hasIconTheme())
    {
        return GUIEngine::getSkin()->getThemedIcon("gui/icons/" + name);
    }
    return m_subdir_name[type] + name;
}   // getAsset

//-----------------------------------------------------------------------------
/** Searches in all root directories for the specified file.
 *  \param name Name of the file to find.
 *  \return Full path of the file, or "" if not found.
 */
std::string FileManager::getAsset(const std::string &name) const
{
    std::string path;
    findFile(path, name, m_root_dirs);
    return path;
}   // getAsset

//-----------------------------------------------------------------------------
/** Returns the directory in which screenshots should be stored.
 */
std::string FileManager::getScreenshotDir() const
{
    return m_screenshot_dir;
}   // getScreenshotDir

//-----------------------------------------------------------------------------
/** Returns the directory in which replay file should be stored.
 */
std::string FileManager::getReplayDir() const
{
    return m_replay_dir;
}   // getReplayDir

//-----------------------------------------------------------------------------
/** Returns the directory in which resized textures should be cached.
*/
std::string FileManager::getCachedTexturesDir() const
{
    return m_cached_textures_dir;
}   // getCachedTexturesDir

//-----------------------------------------------------------------------------
/** Returns the directory in which user-defined grand prix should be stored.
 */
std::string FileManager::getGPDir() const
{
    return m_gp_dir;
}   // getGPDir

//-----------------------------------------------------------------------------
/** Returns the full path of a texture file name by searching in all
 *  directories currently in the texture search path. The difference to
 *  a call getAsset(TEXTURE,...) is that the latter will only return
 *  textures from .../textures, while the searchTexture will also
 *  search e.g. in kart or track directories (depending on what is currently
 *  being loaded).
 *  \param file_name Name of the texture file to search.
 *  \return The full path for the texture, or "" if the texture was not found.
 */
std::string FileManager::searchTexture(const std::string& file_name) const
{
    std::string path;
    findFile(path, file_name, m_texture_search_path);
    return path;
}   // searchTexture

//-----------------------------------------------------------------------------

bool FileManager::searchTextureContainerId(std::string& container_id,
    const std::string& file_name) const
{
    std::string full_path;
    for (std::vector<TextureSearchPath>::const_reverse_iterator
        i = m_texture_search_path.rbegin();
        i != m_texture_search_path.rend(); ++i)
    {
        full_path = i->m_texture_search_path + file_name;
        if (m_file_system->existFile(full_path.c_str()))
        {
            container_id = i->m_container_id;
            return true;
        }
    }
    full_path = "";
    return false;
}   // findFile

//-----------------------------------------------------------------------------
/** Returns the list of all directories in which music files are searched.
 */
std::vector<std::string> FileManager::getMusicDirs() const
{
    return m_music_search_path;
}   // getMusicDirs

//-----------------------------------------------------------------------------
/** If the directory specified in path does not exist, it is created. This
 *  function does not support recursive operations, so if a directory "a/b"
 *  is tested, and "a" does not exist, this function will fail.
 *  \params path Directory to test.
 *  \return  True if the directory exists or could be created,
 *           false otherwise.
 */
bool FileManager::checkAndCreateDirectory(const std::string &path)
{
    // irrlicht apparently returns true for files and directory
    // (using access/_access internally):
    if(m_file_system->existFile(io::path(path.c_str())))
        return true;

    Log::info("FileManager", "Creating directory '%s'.", path.c_str());

    // Otherwise try to create the directory:
#if defined(WIN32)
    bool error = _wmkdir(StringUtils::utf8ToWide(path).c_str()) != 0;
#else
    bool error = mkdir(path.c_str(), 0777) != 0;
#endif
    return !error;
}   // checkAndCreateDirectory

//-----------------------------------------------------------------------------
/** If the directory specified in path does not exist, it is created
 *  recursively (mkdir -p style).
 *  \params path Directory to test.
 *  \return  True if the directory exists or could be created, false otherwise.
 */
bool FileManager::checkAndCreateDirectoryP(const std::string &path)
{
    // irrlicht apparently returns true for files and directory
    // (using access/_access internally):
    if(m_file_system->existFile(io::path(path.c_str())))
        return true;

    Log::info("[FileManager]", "Creating directory(ies) '%s'", path.c_str());

    std::vector<std::string> split = StringUtils::split(path,'/');
    std::string current_path = "";
    for (unsigned int i=0; i<split.size(); i++)
    {
        current_path += split[i] + "/";
        //Log::verbose("[FileManager]", "Checking for: '%s",
        //            current_path.c_str());

        if (!checkAndCreateDirectory(current_path))
        {
            Log::error("[FileManager]", "Can't create dir '%s'",
                    current_path.c_str());
            break;
        }
    }
    bool error = checkAndCreateDirectory(path);

    return error;
}   // checkAndCreateDirectory

//-----------------------------------------------------------------------------
/** Checks if the config directory exists, and it not, tries to create it.
 *  It will set m_user_config_dir to the path to which user-specific config
 *  files are stored.
 */
void FileManager::checkAndCreateConfigDir()
{
    if(getenv("SUPERTUXKART_SAVEDIR") &&
        checkAndCreateDirectory(getenv("SUPERTUXKART_SAVEDIR")) )
    {
        m_user_config_dir = getenv("SUPERTUXKART_SAVEDIR");
    }
    else
    {

#if defined(WIN32)

        // Try to use the APPDATA directory to store config files and highscore
        // lists. If not defined, used the current directory.
        std::vector<wchar_t> env;
        // An environment variable has a maximum size limit of 32,767 characters
        env.resize(32767, 0);
        DWORD length = GetEnvironmentVariable(L"APPDATA", env.data(), 32767);
        if (length != 0)
        {
            m_user_config_dir = StringUtils::wideToUtf8(env.data());
            if (!checkAndCreateDirectory(m_user_config_dir))
            {
                Log::error("[FileManager]", "Can't create config dir '%s"
                            ", falling back to '.'.", m_user_config_dir.c_str());
                m_user_config_dir = ".";
            }
        }
        else
            m_user_config_dir = ".";

        m_user_config_dir += "/supertuxkart";

#elif defined(__APPLE__)

        if (getenv("HOME") != NULL)
        {
            m_user_config_dir = getenv("HOME");
        }
        else
        {
            Log::error("[FileManager]",
                        "No home directory, this should NOT happen!");
            // Fall back to system-wide app data (rather than
            // user-specific data), but should not happen anyway.
            m_user_config_dir = "";
        }
        m_user_config_dir += "/Library/Application Support/";
        const std::string CONFIGDIR("SuperTuxKart");
        m_user_config_dir += CONFIGDIR;

#elif defined(__HAIKU__)

        BPath settings_dir;
        if (find_directory(B_USER_SETTINGS_DIRECTORY, &settings_dir, true, NULL) == B_OK)
        {
            m_user_config_dir = std::string(settings_dir.Path());
        }
        else if (getenv("HOME") != NULL)
        {
            m_user_config_dir = getenv("HOME");
            m_user_config_dir += "/config/settings";
        }
        m_user_config_dir += "/SuperTuxKart";

#else

        // Remaining unix variants. Use the new standards for config directory
        // i.e. either XDG_CONFIG_HOME or $HOME/.config
        if (getenv("XDG_CONFIG_HOME") !=NULL)
        {
            m_user_config_dir = getenv("XDG_CONFIG_HOME");
            checkAndCreateDirectory(m_user_config_dir);
        }
        else if (!getenv("HOME"))
        {
            Log::error("[FileManager]",
                        "No home directory, this should NOT happen "
                        "- trying '.' for config files!");
            m_user_config_dir = ".";
        }
        else
        {
            m_user_config_dir  = getenv("HOME");
            checkAndCreateDirectory(m_user_config_dir);

            m_user_config_dir += "/.config";
            if(!checkAndCreateDirectory(m_user_config_dir))
            {
                // If $HOME/.config can not be created:
                Log::error("[FileManager]",
                            "Cannot create directory '%s', falling back to use '%s'",
                            m_user_config_dir.c_str(), getenv("HOME"));
                m_user_config_dir = getenv("HOME");
            }
        }
        m_user_config_dir += "/supertuxkart";

#endif

    }   // if(getenv("SUPERTUXKART_SAVEDIR") && checkAndCreateDirectory(...))

    if(m_user_config_dir.size()>0 && *m_user_config_dir.rbegin()!='/')
        m_user_config_dir += "/";

    m_user_config_dir += "config-0.10/";
    if(!checkAndCreateDirectoryP(m_user_config_dir))
    {
        Log::warn("FileManager", "Can not  create config dir '%s', "
                  "falling back to '.'.", m_user_config_dir.c_str());
        m_user_config_dir = "./";
    }

    if (m_stdout_dir.empty())
    {
        m_stdout_dir = m_user_config_dir;
    }

    return;
}   // checkAndCreateConfigDir

// ----------------------------------------------------------------------------
/** Creates the directories for the addons data. This will set m_addons_dir
 *  with the appropriate path, and also create the subdirectories in this
 *  directory.
 */
void FileManager::checkAndCreateAddonsDir()
{
#if defined(WIN32)
    m_addons_dir  = m_user_config_dir+"../addons/";
#elif defined(__HAIKU__)
    m_addons_dir  = m_user_config_dir+"addons/";
#elif defined(__APPLE__)
    m_addons_dir  = getenv("HOME");
    m_addons_dir += "/Library/Application Support/SuperTuxKart/Addons/";
#elif defined(__HAIKU__)
    m_addons_dir  = m_user_config_dir+"addons/";
#else
    m_addons_dir = checkAndCreateLinuxDir("XDG_DATA_HOME", "supertuxkart",
                                          ".local/share", ".stkaddons");
    m_addons_dir += "addons/";
#endif

    if(!checkAndCreateDirectory(m_addons_dir))
    {
        Log::error("FileManager", "Can not create add-ons dir '%s', "
                   "falling back to '.'.", m_addons_dir.c_str());
        m_addons_dir = "./";
    }

    if (!checkAndCreateDirectory(m_addons_dir + "icons/"))
    {
        Log::error("FileManager", "Failed to create add-ons icon dir at '%s'.",
                   (m_addons_dir + "icons/").c_str());
    }
    if (!checkAndCreateDirectory(m_addons_dir + "tmp/"))
    {
        Log::error("FileManager", "Failed to create add-ons tmp dir at '%s'.",
                   (m_addons_dir + "tmp/").c_str());
    }

}   // checkAndCreateAddonsDir

// ----------------------------------------------------------------------------
/** Creates the directories for screenshots. This will set m_screenshot_dir
 *  with the appropriate path.
 */
void FileManager::checkAndCreateScreenshotDir()
{
#if defined(WIN32) || defined(__HAIKU__)
    m_screenshot_dir  = m_user_config_dir+"screenshots/";
#elif defined(__APPLE__)
    m_screenshot_dir  = getenv("HOME");
    m_screenshot_dir += "/Library/Application Support/SuperTuxKart/Screenshots/";
#else
    m_screenshot_dir  = checkAndCreateLinuxDir("XDG_DATA_HOME", "supertuxkart",
                                          ".local/share", ".stkscreenshots");
    m_screenshot_dir += "screenshots/";
#endif

    if(!checkAndCreateDirectory(m_screenshot_dir))
    {
        Log::error("FileManager", "Can not create screenshot directory '%s', "
                   "falling back to '.'.", m_screenshot_dir.c_str());
        m_screenshot_dir = ".";
    }

}   // checkAndCreateScreenshotDir

// ----------------------------------------------------------------------------
/** Creates the directories for replay recorded. This will set m_replay_dir
 *  with the appropriate path.
 */
void FileManager::checkAndCreateReplayDir()
{
#if defined(WIN32) || defined(__HAIKU__)
    m_replay_dir = m_user_config_dir + "replay/";
#elif defined(__APPLE__)
    m_replay_dir  = getenv("HOME");
    m_replay_dir += "/Library/Application Support/SuperTuxKart/replay/";
#else
    m_replay_dir = checkAndCreateLinuxDir("XDG_DATA_HOME", "supertuxkart",
                                          ".local/share", ".supertuxkart");
    m_replay_dir += "replay/";
#endif

    if(!checkAndCreateDirectory(m_replay_dir))
    {
        Log::error("FileManager", "Can not create replay directory '%s', "
                   "falling back to '.'.", m_replay_dir.c_str());
        m_replay_dir = ".";
    }

}   // checkAndCreateReplayDir

// ----------------------------------------------------------------------------
/** Creates the directories for cached textures. This will set
*  m_cached_textures_dir with the appropriate path.
*/
void FileManager::checkAndCreateCachedTexturesDir()
{
#if defined(WIN32) || defined(__HAIKU__)
    m_cached_textures_dir = m_user_config_dir + "cached-textures/";
#elif defined(__APPLE__)
    m_cached_textures_dir = getenv("HOME");
    m_cached_textures_dir += "/Library/Application Support/SuperTuxKart/CachedTextures/";
#else
    m_cached_textures_dir = checkAndCreateLinuxDir("XDG_CACHE_HOME", "supertuxkart", ".cache/", ".");
    m_cached_textures_dir += "cached-textures/";
#endif

    if (!checkAndCreateDirectory(m_cached_textures_dir))
    {
        Log::error("FileManager", "Can not create cached textures directory '%s', "
            "falling back to '.'.", m_cached_textures_dir.c_str());
        m_cached_textures_dir = ".";
    }

}   // checkAndCreateCachedTexturesDir

// ----------------------------------------------------------------------------
/** Creates the directories for user-defined grand prix. This will set m_gp_dir
 *  with the appropriate path.
 */
void FileManager::checkAndCreateGPDir()
{
#if defined(WIN32) || defined(__HAIKU__)
    m_gp_dir = m_user_config_dir + "grandprix/";
#elif defined(__APPLE__)
    m_gp_dir  = getenv("HOME");
    m_gp_dir += "/Library/Application Support/SuperTuxKart/grandprix/";
#else
    m_gp_dir = checkAndCreateLinuxDir("XDG_DATA_HOME", "supertuxkart",
                                          ".local/share", ".supertuxkart");
    m_gp_dir += "grandprix/";
#endif

    if(!checkAndCreateDirectory(m_gp_dir))
    {
        Log::error("FileManager", "Can not create user-defined grand prix directory '%s', "
                   "falling back to '.'.", m_gp_dir.c_str());
        m_gp_dir = ".";
    }

}   // checkAndCreateGPDir

// ----------------------------------------------------------------------------
#if !defined(WIN32) && !defined(__APPLE__)

/** Find a directory to use for remaining unix variants. Use the new standards
 *  for config directory based on XDG_* environment variables, or a
 *  subdirectory under $HOME, trying two different fallbacks. It will also
 *  check if the directory 'dirname' can be created (to avoid problems that
 *  e.g. $env_name is '/', which exists, but can not be written to.
 *  \param env_name  Name of the environment variable to test first.
 *  \param dir_name  Name of the directory to create
 *  \param fallback1 Subdirectory under $HOME to use if the environment
 *         variable is not defined or can not be created.
 *  \param fallback2 Subdirectory under $HOME to use if the environment
 *         variable and fallback1 are not defined or can not be created.
 */
std::string FileManager::checkAndCreateLinuxDir(const char *env_name,
                                                const char *dir_name,
                                                const char *fallback1,
                                                const char *fallback2)
{
    bool dir_ok = false;
    std::string dir;

    if (getenv(env_name)!=NULL)
    {
        dir = getenv(env_name);
        dir_ok = checkAndCreateDirectory(dir);
        if(!dir_ok)
            Log::warn("FileManager", "Cannot create $%s.", env_name);

        if(dir[dir.size()-1]!='/') dir += "/";
        // Do an additional test here, e.g. in case that XDG_DATA_HOME is '/'
        // and since dir_ok is set, it would not test any of the other options
        // like $HOME/.local/share
        dir_ok = checkAndCreateDirectory(dir+dir_name);
        if(!dir_ok)
            Log::warn("FileManager", "Cannot create $%s/%s.", dir.c_str(),
                      dir_name);
    }

    if(!dir_ok && getenv("HOME"))
    {
        // Use ~/.local/share :
        dir  = getenv("HOME");
        if(dir.size()>0 && dir[dir.size()-1]!='/') dir += "/";
        dir += fallback1;
        // This will create each individual subdirectory if
        // dir_name contains "/".
        dir_ok = checkAndCreateDirectoryP(dir);
        if(!dir_ok)
            Log::warn("FileManager", "Cannot create $HOME/%s.",
                      fallback1);
    }
    if(!dir_ok && fallback2 && getenv("HOME"))
    {
        dir  = getenv("HOME");
        if(dir.size()>0 && dir[dir.size()-1]!='/') dir += "/";
        dir += fallback2;
        dir_ok = checkAndCreateDirectory(dir);
        if(!dir_ok)
            Log::warn("FileManager", "Cannot create $HOME/%s.",
                      fallback2);
    }

    if(!dir_ok)
    {
        Log::warn("FileManager", "Falling back to use '.'.");
        dir = "./";
    }

    if(dir.size()>0 && dir[dir.size()-1]!='/') dir += "/";
    dir += dir_name;
    dir_ok = checkAndCreateDirectory(dir);
    if(!dir_ok)
    {
        // If the directory can not be created
        Log::error("FileManager", "Cannot create directory '%s', "
                   "falling back to use '.'.", dir.c_str());
        dir="./";
    }
    if(dir.size()>0 && dir[dir.size()-1]!='/') dir += "/";
    return dir;
}   // checkAndCreateLinuxDir
#endif

//-----------------------------------------------------------------------------
/** Sets the name for the stdout log file.
 *  \param filename Filename to use (relative to the user config dir).
 */
void FileManager::setStdoutName(const std::string& filename)
{
    m_stdout_filename = filename;
}   // setStdoutName

//-----------------------------------------------------------------------------
/** Sets the directory for the stdout log file.
 *  \param dir Directory to use
 */
void FileManager::setStdoutDir(const std::string& dir)
{
    m_stdout_dir = dir;

    if (!m_stdout_dir.empty() && m_stdout_dir[m_stdout_dir.size() - 1] != '/')
    {
        m_stdout_dir += "/";
    }
}   // setStdoutDir

//-----------------------------------------------------------------------------
/** Redirects output to go into files in the user's config directory
 *  instead of to the console. It keeps backup copies of previous stdout files
 *  (3 atm), which can help to diagnose problems caused by a previous crash.
 */
void FileManager::redirectOutput()
{
    // Do a simple log rotate: stdout.log.2 becomes stdout.log.3 etc
    const int NUM_BACKUPS=3;
    std::string logoutfile = m_stdout_dir + m_stdout_filename;
    for(int i=NUM_BACKUPS; i>1; i--)
    {
        std::ostringstream out_old;
        out_old << logoutfile << "." << i;
        removeFile(out_old.str());
        std::ostringstream out_new;
        out_new << logoutfile << "." << i-1;
        if(fileExists(out_new.str()))
        {
            FileUtils::renameU8Path(out_new.str(), out_old.str());
        }
    }   // for i in NUM_BACKUPS

    if(fileExists(logoutfile))
    {
        std::ostringstream out;
        out << logoutfile<<".1";
        // No good place to log error messages when log is not yet initialised
        FileUtils::renameU8Path(logoutfile, out.str());
    }

    //Enable logging of stdout and stderr to logfile
    Log::verbose("main", "Error messages and other text output will "
                         "be logged to %s.", logoutfile.c_str());
    Log::openOutputFiles(logoutfile);
}   // redirectOutput

//-----------------------------------------------------------------------------
/** Returns the directory for addon files. */
const std::string &FileManager::getAddonsDir() const
{
    return m_addons_dir;
}   // getAddonsDir

//-----------------------------------------------------------------------------
/** Returns the full path of a file in the addons directory.
 *  \param name Name of the file.
 */
std::string FileManager::getAddonsFile(const std::string &name)
{
    return getAddonsDir()+name;
}   // getAddonsFile

//-----------------------------------------------------------------------------
/** Returns the full path of the config directory.
 */
std::string FileManager::getUserConfigFile(const std::string &fname) const
{
    return m_user_config_dir+fname;
}   // getUserConfigFile

//-----------------------------------------------------------------------------
/** Returns the full path of a music file by searching all music search paths.
 *  It throws an exception if the file is not found.
 *  \param file_name File name to search for.
 */
std::string FileManager::searchMusic(const std::string& file_name) const
{
    std::string path;
    bool success = findFile(path, file_name, m_music_search_path);
    if(!success)
    {
        // If a music file is not found in any of the music search paths
        // check all root dirs. This is used by stk_config to load the
        // title music before any music search path is defined)
        path = getAsset(MUSIC, file_name);
        success = fileExists(path);
    }
    if (!success)
    {
        throw std::runtime_error(
            "[FileManager::getMusicFile] Cannot find music file '"
            +file_name+"'.");
    }
    return path;
}   // searchMusic

//-----------------------------------------------------------------------------
/** Returns the full path of a model file by searching all model search paths.
 *  It throws an exception if the file is not found.
 *  \param file_name File name to search for.
 */
std::string FileManager::searchModel(const std::string& file_name) const
{
    std::string path;
    bool success = findFile(path, file_name, m_model_search_path);
    if (!success)
    {
        throw std::runtime_error(
            "[FileManager::searchModel] Cannot find model file '"
            +file_name+"'.");
    }
    return path;
}   // searchModel

//-----------------------------------------------------------------------------
/** Returns true if the given name is a directory.
 *  \param path File name to test.
 */
bool FileManager::isDirectory(const std::string &path)
{
    struct stat mystat;
    std::string s(path);
    // At least on windows stat returns an error if there is
    // a '/' at the end of the path.
    if(s[s.size()-1]=='/')
        s.erase(s.end()-1, s.end());
    if(FileUtils::statU8Path(s, &mystat) < 0) return false;
    return S_ISDIR(mystat.st_mode);
}   // isDirectory

//-----------------------------------------------------------------------------
/** Returns a list of files in a given directory.
 *  \param result A reference to a std::vector<std::string> which will
 *         hold all files in a directory. The vector will be cleared.
 *  \param dir The director for which to get the directory listing.
 *  \param make_full_path If set to true, all listed files will be full paths.
 */
void FileManager::listFiles(std::set<std::string>& result,
                            const std::string& dir,
                            bool make_full_path) const
{
    result.clear();

    if (!isDirectory(dir))
        return;

    irr::io::IFileList* files = m_file_system->createFileList(dir.c_str());

    for (int n = 0; n < (int)files->getFileCount(); n++)
    {
        result.insert(make_full_path ? dir + "/" + files->getFileName(n).c_str()
                                     : files->getFileName(n).c_str());
    }

    files->drop();
}   // listFiles

//-----------------------------------------------------------------------------
/** Creates a directory for an addon.
 *  \param addons_name Name of the directory to create.
 *  \param addons_type The type, which is used as a subdirectory. E.g.:
 *         'karts' (m_addons_dir/karts/name will be created).
 */
void FileManager::checkAndCreateDirForAddons(const std::string &dir)
{
    // Tries to create directory recursively
    bool success = checkAndCreateDirectoryP(dir);
    if(!success)
    {
        Log::warn("FileManager", "There is a problem with the addons dir.");
        return;
    }
}   // checkAndCreateDirForAddons

// ----------------------------------------------------------------------------
/** Removes the specified file.
 *  \return True if successful, or false if the file is not a regular file or
 *           can not be removed.
 */
bool FileManager::removeFile(const std::string &name) const
{
    // If the file does not exists, everything is fine
    if(!fileExists(name))
       return true;

    struct stat mystat;
    if(FileUtils::statU8Path(name, &mystat) < 0) return false;
    if( S_ISREG(mystat.st_mode))
    {
#if defined(WIN32)
        return _wremove(StringUtils::utf8ToWide(name).c_str()) == 0;
#else
        return remove(name.c_str()) == 0;
#endif
    }
    return false;
}   // removeFile

// ----------------------------------------------------------------------------
/** Removes a directory (including all files contained). The function could
 *  easily recursively delete further subdirectories, but this is commented
 *  out atm (to limit the amount of damage in case of a bug).
 *  \param name Directory name to remove.
 *  \param return True if removal was successful.
 */
bool FileManager::removeDirectory(const std::string &name) const
{
    std::set<std::string> files;
    listFiles(files, name, /*is full path*/ true);

    for (std::string file : files)
    {
        if (file == "." || file == ".." || file == name + "/." ||
            file == name + "/..")
            continue;

        if (UserConfigParams::logMisc())
            Log::verbose("FileManager", "Deleting directory '%s'.",
                         file.c_str());

        if (isDirectory(file))
        {
            // This should not be necessary (since this function is only
            // used to remove addons), and it limits the damage in case
            // of any bugs - i.e. if name should be "/" or so.
            // We need to remove whole data directory on Android though, i.e.
            // when we install newer STK version and new assets are extracted.
            // So enable it only for Android for now.
            #ifdef MOBILE_STK
            removeDirectory(file);
            #else
            if (file.find(m_addons_dir) != std::string::npos)
                removeDirectory(file);
            #endif
        }
        else
        {
            removeFile(file);
        }
    }

#if defined(WIN32)
    return RemoveDirectory(StringUtils::utf8ToWide(name).c_str())==TRUE;
#else
    return remove(name.c_str())==0;
#endif
}   // remove directory

// ----------------------------------------------------------------------------
/** Copies the file source to dest.
 *  \param source The file to read.
 *  \param dest The new filename.
 *  \return True if the copy was successful, false otherwise.
 */
bool FileManager::copyFile(const std::string &source, const std::string &dest)
{
    FILE *f_source = FileUtils::fopenU8Path(source, "rb");
    FILE *f_dest = FileUtils::fopenU8Path(dest, "wb");
    constexpr int BUFFER_SIZE=32768;
    char *buffer = new char[BUFFER_SIZE];
    auto scoped = [&]() {
        if (f_source) fclose(f_source);
        if (f_dest) fclose(f_dest);
        if (buffer) delete [] buffer;
    };
    MemUtils::deref<decltype(scoped)> cls(scoped); 
    if(!f_source || !f_dest || !buffer) return false;
    size_t n;
    while((n=fread(buffer, 1, BUFFER_SIZE, f_source))>0)
    {
        if(fwrite(buffer, 1, n, f_dest)!=n)
        {
            Log::error("FileManager", "Write error copying '%s' to '%s",
                        source.c_str(), dest.c_str());
            return false;

        }   // if fwrite()!=n
    }   // while

    return true;
}   // copyFile

// ----------------------------------------------------------------------------
/** Returns true if the first file is newer than the second. The comparison is
*   based on the modification time of the two files.
*/
bool FileManager::fileIsNewer(const std::string& f1, const std::string& f2) const
{
    struct stat stat1;
    struct stat stat2;
    FileUtils::statU8Path(f1, &stat1);
    FileUtils::statU8Path(f2, &stat2);
    return stat1.st_mtime > stat2.st_mtime;
}   // fileIsNewer

// ----------------------------------------------------------------------------
/** Move the source directory into the target directory location.
*   The target directory must be on the same drive as the source.
*/
bool FileManager::moveDirectoryInto(std::string source, std::string target)
{
    if (!isDirectory(source) || !isDirectory(target))
        return false;

    // Remove the last '/'
    if (source[source.size() - 1] == '/')
        source.erase(source.end() - 1, source.end());
    std::string folder = StringUtils::getBasename(source);
    if (target[target.size() - 1] != '/')
        target += "/";
    target += folder;

    // The result target directory must not already exist
    if (isDirectory(target))
        return false;

#if defined(WIN32)
    return MoveFileExW(StringUtils::utf8ToWide(source).c_str(),
        StringUtils::utf8ToWide(target).c_str(),
        MOVEFILE_WRITE_THROUGH) != 0;
#else
    return rename(source.c_str(), target.c_str()) != -1;
#endif
}   // moveDirectoryInto
