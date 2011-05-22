//  $Id$
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
#if !defined(WIN32) || defined(__CYGWIN__)
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


#include "irrlicht.h"
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

    m_device = createDevice(video::EDT_NULL);

#ifdef __APPLE__
    chdir( buffer );
#endif

    m_file_system  = m_device->getFileSystem();
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
    m_device->drop();
    m_device = NULL;
}   // dropFileSystem

//-----------------------------------------------------------------------------
/** This function is used to re-initialise the file-manager after reading in
 *  the user configuration data.
*/
void FileManager::setDevice(IrrlichtDevice *device)
{
    m_device = device;

    //std::cout << "^^^^^^^^ GRABBING m_device (FileManager) ^^^^^^^^\n";
    m_device->grab();  // To make sure that the device still exists while
                       // file_manager has a pointer to the file system.
    m_file_system  = m_device->getFileSystem();
    TrackManager::addTrackSearchDir(m_root_dir+"/data/tracks");
    KartPropertiesManager::addKartSearchDir(m_root_dir+"/data/karts");
    pushTextureSearchPath(m_root_dir+"/data/textures/");
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
    popMusicSearchPath();
    popModelSearchPath();
    popTextureSearchPath();
    // m_file_system is ref-counted, so no delete/drop necessary.
    m_file_system = NULL;
    //std::cout << "^^^^^^^^ Dropping m_device (FileManager) ^^^^^^^^\n";
    m_device->drop();
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
    // This is not necessary if this is the first member, of ir the
    // addFileArchive call did not add this file systems (this can
    // happen if the file archive has been added prevously, which
    // commonly happens since each kart/track specific path is added
    // twice: once for textures and once for models.
    if(n>0 && (int)m_file_system->getFileArchiveCount()>n)
    {
        // In this case move the just added file archive
        // (which has index n) to position 0 (by -n positions):
        m_file_system->moveFileArchive(n, -n);
    }
}   // pushModelSearchPath

//-----------------------------------------------------------------------------
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
    // This is not necessary if this is the first member, of ir the
    // addFileArchive call did not add this file systems (this can
    // happen if the file archive has been added prevously, which
    // commonly happens since each kart/track specific path is added
    // twice: once for textures and once for models.
    // (which has index n) to position 0 (by -n positions):
    if(n>0 && (int)m_file_system->getFileArchiveCount()>n)
    {
        // In this case move the just added file archive
        // (which has index n) to position 0 (by -n positions):
        m_file_system->moveFileArchive(n, -n);
    }
}   // pushTextureSearchPath

//-----------------------------------------------------------------------------
void FileManager::popTextureSearchPath()
{
    std::string dir = m_texture_search_path.back();
    m_texture_search_path.pop_back();
    m_file_system->removeFileArchive(createAbsoluteFilename(dir));
}   // popTextureSearchPath

//-----------------------------------------------------------------------------
void FileManager::popModelSearchPath()
{
    std::string dir = m_model_search_path.back();
    m_model_search_path.pop_back();
    m_file_system->removeFileArchive(createAbsoluteFilename(dir));
}   // popModelSearchPath

//-----------------------------------------------------------------------------
bool FileManager::findFile(std::string& full_path,
                      const std::string& fname,
                      const std::vector<std::string>& search_path) const
{
    for(std::vector<std::string>::const_reverse_iterator i = search_path.rbegin();
        i != search_path.rend(); ++i)
    {
        full_path = *i + "/" + fname;
        if(m_file_system->existFile(full_path.c_str())) return true;
    }
    full_path="";
    return false;
}   // findFile

//-----------------------------------------------------------------------------
std::string FileManager::getTextureFile(const std::string& FNAME) const
{
    std::string path;
    // FIXME: work around when loading and converting tracks: FNAME
    //        (which is based on an irrlicht return value) contains the
    //        full path
    //if(m_file_system->existFile(FNAME.c_str())) return FNAME;
    findFile(path, FNAME, m_texture_search_path);
    return path;
}   // makeTexturePath

//-----------------------------------------------------------------------------
std::string FileManager::getModelFile(const std::string& FNAME) const
{
    std::string path;
    findFile(path, FNAME, m_model_search_path);
    return path;
}   // getModelFile

//-----------------------------------------------------------------------------
std::string FileManager::getDataDir() const
{
    return m_root_dir+"/data/";
}
//-----------------------------------------------------------------------------
std::string FileManager::getGUIDir() const
{
    return m_root_dir+"/data/gui/";
}

//-----------------------------------------------------------------------------
std::string FileManager::getTranslationDir() const
{
    return m_root_dir+"/data/po";
}   // getTranslationDir

//-----------------------------------------------------------------------------
std::vector<std::string> FileManager::getMusicDirs() const
{
    return m_music_search_path;
}   // getMusicDirs

//-----------------------------------------------------------------------------
std::string FileManager::getDataFile(const std::string& fname) const
{
    return m_root_dir+"/data/"+fname;
}   // getDataFile
//-----------------------------------------------------------------------------
std::string FileManager::getGfxFile(const std::string& fname) const
{
    return m_root_dir+"/data/gfx/"+fname;
}
//-----------------------------------------------------------------------------
/** If the directory specified in path does not exist, it is created.
 * \params path Directory to test.
 * \return  True if the directory exists or could be created, false otherwise.
 */
bool FileManager::checkAndCreateDirectory(const std::string &path)
{
    // irrlicht apparently returns true for files and directory
    // (using access/_access internally):
    if(m_file_system->existFile(io::path(path.c_str())))
        return true;

    std::cout << "creating directory <" << path << ">" << std::endl;
    
    // Otherwise try to create the directory:
#if defined(WIN32) && !defined(__CYGWIN__)
    bool error = _mkdir(path.c_str()) != 0;
#else
    bool error = mkdir(path.c_str(), 0755) != 0;
#endif
    return !error;
}   // checkAndCreateDirectory

//-----------------------------------------------------------------------------
bool FileManager::checkAndCreateDirectoryP(const std::string &path)
{
    // irrlicht apparently returns true for files and directory
    // (using access/_access internally):
    if(m_file_system->existFile(io::path(path.c_str())))
        return true;
    
    std::cout << "creating directory(ies) <" << path << "> ..." << std::endl;
    
    std::vector<std::string> split = StringUtils::split(path,'/');
    std::string current_path = "";
    for (unsigned int i=0; i<split.size(); i++)
    {
        current_path += split[i] + "/";
        std::cout << "   Checking for: " << current_path << std::endl;
        if (m_file_system->existFile(io::path(current_path.c_str())))
        {
            //std::cout << "The directory exist." << std::endl;
        }
        else
        {
            if (!checkAndCreateDirectory(current_path))
            {
                fprintf(stderr, "Can't create dir '%s'",
                        current_path.c_str());
                break;
            }
        }
    }
    bool error = checkAndCreateDirectory(path);

    return error;
}   // checkAndCreateDirectory
//-----------------------------------------------------------------------------
/** Checks if the config directory exists, and it not, tries to create it. */
void FileManager::checkAndCreateConfigDir()
{
    if(getenv("SUPERTUXKART_SAVEDIR") && 
        checkAndCreateDirectory(getenv("SUPERTUXKART_SAVEDIR")) )
    {
        m_config_dir = getenv("SUPERTUXKART_SAVEDIR");
    }
    else
    {
#if defined(WIN32)
        // Try to use the APPDATA directory to store config files and highscore
        // lists. If not defined, used the current directory.
        if(getenv("APPDATA")!=NULL)
        {
            m_config_dir  = getenv("APPDATA");
            if(!checkAndCreateDirectory(m_config_dir))
            {
                fprintf(stderr, 
                    "Can't create config dir '%s', falling back to '.'.\n",
                    m_config_dir.c_str());
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
            std::cerr << "No home directory, this should NOT happen!\n";
            // Fall back to system-wide app data (rather than user-specific data),
            // but should not happen anyway.
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
            std::cerr << "No home directory, this should NOT happen "
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
                fprintf(stderr, 
                    "Can't create dir '%s', falling back to use '%s'.\n",
                    m_config_dir.c_str(), getenv("HOME"));
                m_config_dir = getenv("HOME");
                m_config_dir += ".";
            }
        }
        const std::string CONFIGDIR("supertuxkart");

        m_config_dir += "/";
        m_config_dir += CONFIGDIR;
#endif
    }   // if(getenv("SUPERTUXKART_SAVEDIR") && checkAndCreateDirectory(...))


    if(!checkAndCreateDirectory(m_config_dir))
    {
        fprintf(stderr, 
                "Can not  create config dir '%s', falling back to '.'.\n",
                m_config_dir.c_str());
        m_config_dir = ".";
    }
    return;
}   // checkAndCreateConfigDir

// ----------------------------------------------------------------------------
void FileManager::checkAndCreateAddonsDir()
{
#if defined(WIN32)
    m_addons_dir  = m_config_dir+"/addons";
#elif defined(__APPLE__)
    m_addons_dir  = getenv("HOME");
    m_addons_dir += "/Library/Application Support/SuperTuxKart/Addons";
#else
    // Remaining unix variants. Use the new standards for config directory
    // i.e. either XDG_CONFIG_HOME or $HOME/.config
    if (getenv("XDG_DATA_HOME")!=NULL){
        m_addons_dir = getenv("XDG_DATA_HOME");
    }
    else if (!getenv("HOME"))
    {
        std::cerr << "No home directory, this should NOT happen "
                  << "- trying '.addons' for addons files!\n";
        m_addons_dir = "stkaddons";
    }
    else
    {
        m_addons_dir  = getenv("HOME");
        m_addons_dir += "/.local/share";
        if(!checkAndCreateDirectory(m_config_dir))
        {
            // If $HOME/.config can not be created:
            fprintf(stderr, 
                    "Can't create dir '%s', falling back to use '%s'.\n",
                    m_config_dir.c_str(), getenv("HOME"));
            m_addons_dir = getenv("HOME");
            m_addons_dir += ".";
        }
    }

    const std::string CONFIGDIR("supertuxkart");

    m_addons_dir += "/";
    m_addons_dir += CONFIGDIR;
#endif

    if(!checkAndCreateDirectory(m_addons_dir))
    {
        fprintf(stderr, 
                "Can not create add-ons dir '%s', falling back to '.'.\n", 
                m_addons_dir.c_str());
        m_config_dir = ".";
    }

    if (!checkAndCreateDirectory(m_addons_dir + "/icons/"))
    {
        fprintf(stderr, "Failed to create add-ons icon dir at '%s'\n",
            (m_addons_dir + "/icons/").c_str());
    }
    if (!checkAndCreateDirectory(m_addons_dir + "/tmp/"))
    {
        fprintf(stderr, "Failed to create add-ons tmp dir at '%s'\n",
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
/** Returns a filename in the addons directory.
 *  \param name Name of the file.
 */
std::string FileManager::getAddonsFile(const std::string &name)
{
    return getAddonsDir()+"/"+name;
}

//-----------------------------------------------------------------------------
std::string FileManager::getConfigDir() const
{
    return m_config_dir;
}   // getConfigDir

//-----------------------------------------------------------------------------
std::string FileManager::getLogFile(const std::string& fname) const
{
    return getConfigDir()+"/"+fname;
}   // getLogFile

//-----------------------------------------------------------------------------
std::string FileManager::getMusicFile(const std::string& fname) const
{
    std::string path;
    const bool success = findFile(path, fname, m_music_search_path);
    if (!success)
    {
        throw std::runtime_error(
            "[FileManager::getMusicFile] Cannot find music file <"+fname+">");
    }
    return path;
}   // getMusicFile

//-----------------------------------------------------------------------------
std::string FileManager::getSFXFile(const std::string& fname) const
{
    return m_root_dir+"/data/sfx/"+fname;
}   // getSFXFile
//-----------------------------------------------------------------------------
std::string FileManager::getFontFile(const std::string& fname) const
{
    return m_root_dir+"/data/fonts/"+fname;
}   // getFontFile
//-----------------------------------------------------------------------------
std::string FileManager::getHighscoreFile(const std::string& fname) const
{
    return getConfigDir()+"/"+fname;
}   // getHighscoreFile

//-----------------------------------------------------------------------------
/** Returns the full path of the challenge file. */
std::string FileManager::getChallengeFile(const std::string &fname) const
{
    return getConfigDir()+"/"+fname;
}   // getChallengeFile

//-----------------------------------------------------------------------------
/** Returns the full path of the tutorial file. */
std::string FileManager::getTutorialFile(const std::string &fname) const
{
    return getConfigDir()+"/"+fname;
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
void FileManager::listFiles(std::set<std::string>& result, 
                            const std::string& dir, bool is_full_path, 
                            bool make_full_path) const
{
    result.clear();

#ifdef WIN32
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
        if(UserConfigParams::m_verbosity>=3)
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
#ifdef WIN32
        return RemoveDirectory(name.c_str())==TRUE;
#else
    return remove(name.c_str())==0;
#endif
}   // remove directory

