//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//            (C) 2008 Steve Baker, Joerg Henrichs
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

#include <irrlicht.h>

#include "io/file_manager.hpp"

#include <stdio.h>
#include <stdexcept>
#include <sstream>
#include <sys/stat.h>
#include <iostream>
#include <string>

// For mkdir
#if !defined(WIN32)
#  include <sys/stat.h>
#  include <sys/types.h>
// For RemoveDirectory
#else
#  include <direct.h>
#  include <Windows.h>
#endif

/*Needed by the remove directory function */

#ifndef WIN32
#  include <dirent.h>
#endif

#ifdef WIN32
#  include <io.h>
#  include <stdio.h>
#  ifndef __CYGWIN__
#    define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#    define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#  endif
#else
#  include <unistd.h>
#endif

#include "btBulletDynamicsCommon.h"

#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "karts/kart_properties_manager.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"


#ifdef __APPLE__
// dynamic data path detection onmac
#  include <CoreFoundation/CoreFoundation.h>

bool macSetBundlePathIfRelevant(std::string& data_dir)
{
    printf("[FileManager] checking whether we are using an app bundle... ");
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
    CFStringGetCString(cf_string_ref, path, 1024, kCFStringEncodingASCII);
    CFRelease(main_bundle_URL);
    CFRelease(cf_string_ref);

    std::string contents = std::string(path) + std::string("/Contents");
    if(contents.find(".app") != std::string::npos)
    {
        printf("yes\n");
        // executable is inside an app bundle, use app bundle-relative paths
        data_dir = contents + std::string("/Resources/");
        return true;
    }
    else
    {
        printf("no\n");
        return false;
    }
}
#endif

// ============================================================================
FileManager* file_manager = 0;

/** With irrlicht the constructor creates a NULL device. This is necessary to
 *  handle the Chicken/egg problem with irrlicht: access to the file system
 *  is given from the device, but we can't create the device before reading
 *  the user_config file (for resolution, fullscreen). So we create a dummy
 *  device here to begin with, which is then later (once the real device
 *  exists) changed in reInit().
 *
 */
FileManager::FileManager(char *argv[])
{
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

    m_file_system  = irr_driver->getDevice()->getFileSystem();
    m_file_system->grab();
    m_is_full_path = false;

    irr::io::path exe_path;

    // Also check for data dirs relative to the path of the executable.
    // This is esp. useful for Visual Studio, since it's not necessary
    // to define the working directory when debugging, it works automatically.
    if(m_file_system->existFile(argv[0]))
        exe_path = m_file_system->getFileDir(argv[0]);

    if ( getenv ( "SUPERTUXKART_DATADIR" ) != NULL )
        m_root_dir= getenv ( "SUPERTUXKART_DATADIR" ) ;
#ifdef __APPLE__
    else if( macSetBundlePathIfRelevant( m_root_dir ) ) { /* nothing to do */ }
#endif
    else if(m_file_system->existFile("data/stk_config.xml"))
        m_root_dir = "." ;
    else if(m_file_system->existFile("../data/stk_config.xml"))
        m_root_dir = ".." ;
    else if(m_file_system->existFile(exe_path+"/data/stk_config.xml"))
        m_root_dir = exe_path.c_str();
    else if(m_file_system->existFile(exe_path+"/../data/stk_config.xml"))
    {
        m_root_dir = exe_path.c_str();
        m_root_dir += "/..";
    }
    else
#ifdef SUPERTUXKART_DATADIR
        m_root_dir = SUPERTUXKART_DATADIR ;
#else
        m_root_dir = "/usr/local/share/games/supertuxkart" ;
#endif
    // We can't use _() here, since translations will only be initalised
    // after the filemanager (to get the path to the tranlsations from it)
    fprintf(stderr, "[FileManager] Data files will be fetched from: '%s'\n",
            m_root_dir.c_str() );
    checkAndCreateConfigDir();
    checkAndCreateAddonsDir();
}  // FileManager

 //-----------------------------------------------------------------------------
/** Remove the dummy file system (which is called from IrrDriver before
 *  creating the actual device.
 */
void FileManager::dropFileSystem()
{
    m_file_system->drop();
}   // dropFileSystem

//-----------------------------------------------------------------------------
/** This function is used to re-initialise the file-manager after reading in
 *  the user configuration data.
*/
void FileManager::reInit()
{
    m_file_system  = irr_driver->getDevice()->getFileSystem();
    m_file_system->grab();
    TrackManager::addTrackSearchDir(m_root_dir+"/data/tracks");
    KartPropertiesManager::addKartSearchDir(m_root_dir+"/data/karts");
    pushTextureSearchPath(getTextureDir());
    pushTextureSearchPath(getTextureDir()+"/deprecated");
    pushModelSearchPath  (m_root_dir+"/data/models/"  );
    pushMusicSearchPath  (m_root_dir+"/data/music/"   );

    // Add more paths from the STK_MUSIC_PATH environment variable
    if(getenv("SUPERTUXKART_MUSIC_PATH")!=NULL)
    {
        std::string path=getenv("SUPERTUXKART_MUSIC_PATH");
        std::vector<std::string> dirs = StringUtils::splitPath(path);
        for(int i=0;i<(int)dirs.size(); i++)
            pushMusicSearchPath(dirs[i]);
    }
}   // reInit

//-----------------------------------------------------------------------------
FileManager::~FileManager()
{
    // Clean up left-over files in addons/tmp that are older than 24h
    // ==============================================================
    // (The 24h delay is useful when debugging a problem with a zip file)
    std::set<std::string> allfiles;
    std::string tmp=getAddonsFile("tmp");
    listFiles(allfiles, tmp, /*fullpath*/true);
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
            printf("[addons] Warning: unexpected tmp file '%s' found.\n",
                   full_path.c_str());
            continue;
        }
        if(isDirectory(full_path))
        {
            // Gee, a .zip file which is a directory - stay away from it
            printf("[addons] '%s' is a directory and will not be deleted.\n",
                    full_path.c_str());
            continue;
        }
        struct stat mystat;
        stat(full_path.c_str(), &mystat);
        Time::TimeType current = Time::getTimeSinceEpoch();
        if(current - mystat.st_ctime <24*3600)
        {
            if(UserConfigParams::logAddons())
                printf("[addons] '%s' is less than 24h old "
                       "and will not be deleted.\n",
                        full_path.c_str());
            continue;
        }
        if(UserConfigParams::logAddons())
            printf("[addons] Deleting tmp file'%s'.\n",full_path.c_str());
        removeFile(full_path);

    }   // for i in all files in tmp

    // Clean up rest of file manager
    // =============================
    popMusicSearchPath();
    popModelSearchPath();
    popTextureSearchPath();
    popTextureSearchPath();
    m_file_system->drop();
    m_file_system = NULL;
}   // ~FileManager

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
            fprintf(stderr, "[FileManager::createXMLTree] %s\n", e.what());
        }
        return NULL;
    }
}   // getXMLTree

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
void FileManager::pushTextureSearchPath(const std::string& path)
{
    m_texture_search_path.push_back(path);
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
    std::string dir = m_texture_search_path.back();
    m_texture_search_path.pop_back();
    m_file_system->removeFileArchive(createAbsoluteFilename(dir));
}   // popTextureSearchPath

//-----------------------------------------------------------------------------
/** Removes the last added model search path from the list of paths.
 */
void FileManager::popModelSearchPath()
{
    std::string dir = m_model_search_path.back();
    m_model_search_path.pop_back();
    m_file_system->removeFileArchive(createAbsoluteFilename(dir));
}   // popModelSearchPath

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
        full_path = *i + "/" + file_name;
        if(m_file_system->existFile(full_path.c_str())) return true;
    }
    full_path="";
    return false;
}   // findFile

//-----------------------------------------------------------------------------
/** Returns the full path of a texture file name by searching for this 
 *  file in all texture search paths.
 *  \param file_name Name of the texture file to search.
 *  \return The full path for the texture, or "" if the texture was not found.
 */
std::string FileManager::getTextureFile(const std::string& file_name) const
{
    std::string path;
    findFile(path, file_name, m_texture_search_path);
    return path;
}   // getTextureFile

//-----------------------------------------------------------------------------
/** Returns the full path of a model file name by searching for this 
 *  file in all model search paths.
 *  \param file_name Name of the model file to search.
 *  \return The full path for the model, or "" if the model was not found.
 */
std::string FileManager::getModelFile(const std::string& file_name) const
{
    std::string path;
    findFile(path, file_name, m_model_search_path);
    return path;
}   // getModelFile

//-----------------------------------------------------------------------------
/** Returns the data directory.
 */
std::string FileManager::getDataDir() const
{
    return m_root_dir+"/data/";
}   // getDataDir

//-----------------------------------------------------------------------------
/** Returns the GUI directory.
 */
std::string FileManager::getGUIDir() const
{
    return m_root_dir+"/data/gui/";
}   // getGUIDir

//-----------------------------------------------------------------------------
/** Returns the base directory for all textures.
 */
std::string FileManager::getTextureDir() const
{
    return m_root_dir+"/data/textures/";
}   // getTextureDir

//-----------------------------------------------------------------------------
/** Returns the translation directory.
 */
std::string FileManager::getTranslationDir() const
{
    return m_root_dir+"/data/po";
}   // getTranslationDir

//-----------------------------------------------------------------------------
/** Returns the list of all directories in which music files are searched.
 */
std::vector<std::string> FileManager::getMusicDirs() const
{
    return m_music_search_path;
}   // getMusicDirs

//-----------------------------------------------------------------------------
/** Returns the full path of a file in the data directory.
 *  \param file_name The file name (potentially including a path) to
 *         be used in the data directory.
 */
std::string FileManager::getDataFile(const std::string& file_name) const
{
    return m_root_dir+"/data/"+file_name;
}   // getDataFile
//-----------------------------------------------------------------------------
/** Returns the full path of graphical effect file
 *  \param file_name Name of the graphical effect file.
 */
std::string FileManager::getGfxFile(const std::string& file_name) const
{
    return m_root_dir+"/data/gfx/"+file_name;
}
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

    std::cout << "[FileManager] Creating directory \"" << path << "\"\n";
    
    // Otherwise try to create the directory:
#if defined(WIN32) && !defined(__CYGWIN__)
    bool error = _mkdir(path.c_str()) != 0;
#else
    bool error = mkdir(path.c_str(), 0755) != 0;
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
    
    std::cout << "[FileManager] Creating directory(ies) '" << path << "'.\n";
    
    std::vector<std::string> split = StringUtils::split(path,'/');
    std::string current_path = "";
    for (unsigned int i=0; i<split.size(); i++)
    {
        current_path += split[i] + "/";
        std::cout << "[FileManager]   Checking for: '"
                  << current_path << "'.\n";
        if (!m_file_system->existFile(io::path(current_path.c_str())))
        {
            if (!checkAndCreateDirectory(current_path))
            {
                fprintf(stderr, "[FileManager] Can't create dir '%s'",
                        current_path.c_str());
                break;
            }
        }
    }
    bool error = checkAndCreateDirectory(path);

    return error;
}   // checkAndCreateDirectory

//-----------------------------------------------------------------------------
/** Checks if the config directory exists, and it not, tries to create it. 
 *  It will set m_config_dir to the path to which user-specific config files
 *  are stored.
 */
void FileManager::checkAndCreateConfigDir()
{
    if(getenv("SUPERTUXKART_SAVEDIR") && 
        checkAndCreateDirectory(getenv("SUPERTUXKART_SAVEDIR")) )
    {
        m_config_dir = getenv("SUPERTUXKART_SAVEDIR");
    }
    else
    {
#if defined(WIN32) || defined(__CYGWIN__)
        // Try to use the APPDATA directory to store config files and highscore
        // lists. If not defined, used the current directory.
        if(getenv("APPDATA")!=NULL)
        {
            m_config_dir  = getenv("APPDATA");
            if(!checkAndCreateDirectory(m_config_dir))
            {
                std::cerr << "[FileManager] Can't create config dir '"
                          << m_config_dir << "', falling back to '.'.\n";
                m_config_dir = ".";
            }
        }
        else
            m_config_dir = ".";
        const std::string CONFIGDIR("supertuxkart");

        m_config_dir += "/";
        m_config_dir += CONFIGDIR;
#elif defined(__APPLE__)
        if (getenv("HOME")!=NULL)
        {
            m_config_dir = getenv("HOME");
        }
        else
        {
            std::cerr << 
                "[FileManager] No home directory, this should NOT happen!\n";
            // Fall back to system-wide app data (rather than 
            // user-specific data), but should not happen anyway.
            m_config_dir = "";
        }
        m_config_dir += "/Library/Application Support/";
        const std::string CONFIGDIR("SuperTuxKart");
        m_config_dir += CONFIGDIR;
#  else
        // Remaining unix variants. Use the new standards for config directory
        // i.e. either XDG_CONFIG_HOME or $HOME/.config
        if (getenv("XDG_CONFIG_HOME")!=NULL){
            m_config_dir = getenv("XDG_CONFIG_HOME");
        }
        else if (!getenv("HOME"))
        {
            std::cerr 
                << "[FileManager] No home directory, this should NOT happen "
                << "- trying '.' for config files!\n";
            m_config_dir = ".";
        }
        else
        {
            m_config_dir  = getenv("HOME");
            m_config_dir += "/.config";
            if(!checkAndCreateDirectory(m_config_dir))
            {
                // If $HOME/.config can not be created:
                std::cerr << "[FileManager] Cannot create directory '" 
                          << m_config_dir <<"', falling back to use '" 
                          << getenv("HOME")<< "'.\n";
                m_config_dir = getenv("HOME");
            }
        }
        m_config_dir += "/supertuxkart";
#endif
    }   // if(getenv("SUPERTUXKART_SAVEDIR") && checkAndCreateDirectory(...))


    if(!checkAndCreateDirectory(m_config_dir))
    {
        std::cerr << "[FileManager] Can not  create config dir '" 
                  << m_config_dir << "', falling back to '.'.\n";
        m_config_dir = ".";
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
#if defined(WIN32) || defined(__CYGWIN__)
    m_addons_dir  = m_config_dir+"/addons";
#elif defined(__APPLE__)
    m_addons_dir  = getenv("HOME");
    m_addons_dir += "/Library/Application Support/SuperTuxKart/Addons";
#else
    // Remaining unix variants. Use the new standards for config directory
    // i.e. either XDG_CONFIG_HOME or $HOME/.local/share

    bool dir_ok = false;

    if (getenv("XDG_DATA_HOME")!=NULL)
    {
        m_addons_dir = getenv("XDG_DATA_HOME");
        dir_ok = checkAndCreateDirectory(m_addons_dir);
        if(!dir_ok)
            std::cerr << "[FileManager] Cannot create $XDG_DATA_HOME.\n";

        // Do an additional test here, e.g. in case that XDG_DATA_HOME is '/' 
        // and since dir_ok is set, it would not test any of the other options
        // like $HOME/.local/share
        dir_ok = checkAndCreateDirectory(m_addons_dir+"/supertuxkart");
        if(!dir_ok)
            std::cerr 
               << "[FileManager] Cannot create $XDG_DATA_HOME/supertuxkart.\n";
    }

    if(!dir_ok && getenv("HOME"))
    {
        // Use ~/.local/share :
        m_addons_dir  = getenv("HOME");
        m_addons_dir += "/.local/share";
        // This tests for ".local" and then for ".local/share"
        dir_ok = checkAndCreateDirectoryP(m_addons_dir);
        if(!dir_ok)
            std::cerr << "[FileManager] Cannot create $HOME/.local/share.\n";
    }
    if(!dir_ok && getenv("HOME"))
    {
        // Use ~/.stkaddons
        m_addons_dir  = getenv("HOME");
        m_addons_dir += "/.stkaddons";
        dir_ok = checkAndCreateDirectory(m_addons_dir);
        if(!dir_ok)
            std::cerr << "[FileManager] Cannot create $HOME/.stkaddons.\n";
    }

    if(!dir_ok)
    {
        std::cerr << "[FileManager] Falling back to use '.'.";
        m_addons_dir = ".";
    }

    m_addons_dir += "/supertuxkart";
    dir_ok = checkAndCreateDirectory(m_addons_dir);
    if(!dir_ok)
    {
        // If the directory can not be created, abort
        std::cerr << " [FileManager] Cannot create directory '"
                  << m_addons_dir<<"', falling back to use '.'.\n";
        m_addons_dir=".";

    }
    m_addons_dir += "/addons";
    
#endif

    if(!checkAndCreateDirectory(m_addons_dir))
    {
        fprintf(stderr, 
                "[FileManager] Can not create add-ons dir '%s', "
                "falling back to '.'.\n", m_addons_dir.c_str());
        m_addons_dir = ".";
    }
    std::cout << "[FileManager] Addons files will be stored in '"
              << m_addons_dir << "'.\n";

    if (!checkAndCreateDirectory(m_addons_dir + "/icons/"))
    {
        fprintf(stderr, 
                "[FileManager] Failed to create add-ons icon dir at '%s'\n",
                (m_addons_dir + "/icons/").c_str());
    }
    if (!checkAndCreateDirectory(m_addons_dir + "/tmp/"))
    {
        fprintf(stderr, 
                "[FileManager] Failed to create add-ons tmp dir at '%s'\n",
                (m_addons_dir + "/tmp/").c_str());
    }
}   // checkAndCreateAddonsDir

//-----------------------------------------------------------------------------
/** Returns the directory for addon files. */
const std::string &FileManager::getAddonsDir() const
{
    return m_addons_dir;
}   // getADdonsDir

//-----------------------------------------------------------------------------
/** Returns the full path of a file in the addons directory.
 *  \param name Name of the file.
 */
std::string FileManager::getAddonsFile(const std::string &name)
{
    return getAddonsDir()+"/"+name;
}   // getAddonsFile

//-----------------------------------------------------------------------------
/** Returns the full path of the config directory.
 */
std::string FileManager::getConfigDir() const
{
    return m_config_dir;
}   // getConfigDir

//-----------------------------------------------------------------------------
/** Returns the full path of a file in the user config directory which is
 *  used to store stdout/stderr if it is redirected.
 *  \param name Name of the file.
 */
std::string FileManager::getLogFile(const std::string& file_name) const
{
    return getConfigDir()+"/"+file_name;
}   // getLogFile

//-----------------------------------------------------------------------------
/** Returns the full path of a music file by searching all music search paths.
 *  It throws an exception if the file is not found.
 *  \param file_name File name to search for.
 */
std::string FileManager::getMusicFile(const std::string& file_name) const
{
    std::string path;
    const bool success = findFile(path, file_name, m_music_search_path);
    if (!success)
    {
        throw std::runtime_error(
            "[FileManager::getMusicFile] Cannot find music file '"
            +file_name+"'.");
    }
    return path;
}   // getMusicFile

//-----------------------------------------------------------------------------
/** Returns the full path of a sound effect file.
 *  \param file_name Name of the sound effect file.
 */
std::string FileManager::getSFXFile(const std::string& file_name) const
{
    return m_root_dir+"/data/sfx/"+file_name;
}   // getSFXFile

//-----------------------------------------------------------------------------
/** Returns the full path of a font file.
 *  \param file_name Name of the font file.
 */
std::string FileManager::getFontFile(const std::string& file_name) const
{
    return m_root_dir+"/data/fonts/"+file_name;
}   // getFontFile

//-----------------------------------------------------------------------------
/** Returns the full path of a highscore file (which is stored in the user
 *  specific config directory).
 *  \param file_name Name of the sound effect file.
 */
std::string FileManager::getHighscoreFile(const std::string& file_name) const
{
    return getConfigDir()+"/"+file_name;
}   // getHighscoreFile

//-----------------------------------------------------------------------------
/** Returns the full path of the challenge file (which is stored in a user
 *  specific config area).
 *  \param file_name Name of the file.
 */
std::string FileManager::getChallengeFile(const std::string &file_name) const
{
    return getConfigDir()+"/"+file_name;
}   // getChallengeFile

//-----------------------------------------------------------------------------
/** Returns the full path of the tutorial file. 
 *  \param file_name Name of the tutorial file to return.
 */
std::string FileManager::getTutorialFile(const std::string &file_name) const
{
    return getConfigDir()+"/"+file_name;
}   // getTutorialFile

//-----------------------------------------------------------------------------
/** Returns true if the given name is a directory.
 *  \param path File name to test.
 */
bool FileManager::isDirectory(const std::string &path) const
{
    struct stat mystat;

    if(stat(path.c_str(), &mystat) < 0) return false;
    return S_ISDIR(mystat.st_mode);
}   // isDirectory

//-----------------------------------------------------------------------------
/** Returns a list of files in a given directory.
 *  \param result A reference to a std::vector<std::string> which will
 *         hold all files in a directory. The vector will be cleared.
 *  \param dir The director for which to get the directory listing.
 *  \param is_full_path True if directory is already a full path,
 *         otherwise m_root_dir is used.
 *  \param make_full_path If set to true, all listed files will be full paths.
 */
void FileManager::listFiles(std::set<std::string>& result, 
                            const std::string& dir, bool is_full_path, 
                            bool make_full_path) const
{
    result.clear();

#if defined(WIN32)
    std::string path = is_full_path ? dir : m_root_dir+"/"+dir;
#else
    std::string path = is_full_path ? dir + "/" : m_root_dir+"/"+dir + "/";
#endif
    //printf("******* Path : %s \n", path.c_str());

    if(!isDirectory(path)) return;

    io::path previous_cwd = m_file_system->getWorkingDirectory();

    if(!m_file_system->changeWorkingDirectoryTo( path.c_str() ))
    {
        printf("FileManager::listFiles : Could not change CWD!\n");
        return;
    }
    irr::io::IFileList* files = m_file_system->createFileList();

    for(int n=0; n<(int)files->getFileCount(); n++)
    {
        result.insert(make_full_path ? path+"/"+ files->getFileName(n).c_str()
                                     : files->getFileName(n).c_str()         );
    }

    m_file_system->changeWorkingDirectoryTo( previous_cwd );
    files->drop();
}   // listFiles

//-----------------------------------------------------------------------------
/** Creates a directory for an addon.
 *  \param addons_name Name of the directory to create.
 *  \param addons_type The type, which is used as a subdirectory. E.g.:
 *         'karts' (m_addons_dir/karts/name will be created).
 */
void FileManager::checkAndCreateDirForAddons(std::string addons_name, 
                                             std::string addons_type)
{
    std::string path = getAddonsFile(addons_type);
    bool success = checkAndCreateDirectory(path);
    if(!success)
    {
        std::cout << "There is a problem with the addons dir." << std::endl;
        return;
    }
    checkAndCreateDirectory(path+"/"+addons_name);
}   // checkAndCreateDirForAddons

// ----------------------------------------------------------------------------
/** Removes the specified file, returns true if successful, or false
 *  if the file is not a regular file or can not be removed.
 */
bool FileManager::removeFile(const std::string &name) const
{
    struct stat mystat;
    if(stat(name.c_str(), &mystat) < 0) return false;
    if( S_ISREG(mystat.st_mode))
        return remove(name.c_str())==0;
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
    for(std::set<std::string>::iterator i=files.begin(); i!=files.end(); i++)
    {
        if((*i)=="." || (*i)=="..") continue;
        if(UserConfigParams::logMisc())
            printf("Deleting directory '%s'.\n", (*i).c_str());
        std::string full_path=name+"/"+*i;
        if(isDirectory(full_path))
        {
            // This should not be necessary (since this function is only
            // used to remove addons), and it limits the damage in case
            // of any bugs - i.e. if name should be "/" or so.
            // removeDirectory(full_path);
        }
        else
        {
            removeFile(full_path);
        }
    }
#if defined(WIN32)
        return RemoveDirectory(name.c_str())==TRUE;
#else
    return remove(name.c_str())==0;
#endif
}   // remove directory

