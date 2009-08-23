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

#include "io/file_manager.hpp"

#include <stdexcept>
#include <sstream>
#include <sys/stat.h>
#include <string>
#ifdef WIN32
#  include <io.h>
#  include <stdio.h>
#  ifndef __CYGWIN__
#    define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
     //   Some portabilty defines
#  endif
#  define CONFIGDIR       "."
#else
#  include <unistd.h>
#  define CONFIGDIR       ".supertuxkart"
#endif

#include "irrlicht.h"
#include "btBulletDynamicsCommon.h"

#include "graphics/irr_driver.hpp"
#include "graphics/material_manager.hpp"
#include "utils/string_utils.hpp"

#ifdef __APPLE__
// dynamic data path detection onmac
#  include <CoreFoundation/CoreFoundation.h>

bool macSetBundlePathIfRelevant(std::string& data_dir)
{
    printf("checking whether we are using an app bundle... ");
    // the following code will enable STK to find its data when placed in an app bundle on mac OS X.
    // returns true if path is set, returns false if path was not set
    char path[1024];
    CFBundleRef main_bundle = CFBundleGetMainBundle(); assert(main_bundle);
    CFURLRef main_bundle_URL = CFBundleCopyBundleURL(main_bundle); assert(main_bundle_URL);
    CFStringRef cf_string_ref = CFURLCopyFileSystemPath( main_bundle_URL, kCFURLPOSIXPathStyle); assert(cf_string_ref);
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
FileManager::FileManager()
{
#ifdef __APPLE__
    // irrLicht's createDevice method has a nasty habit of messing the CWD.
    // since the code above may rely on it, save it to be able to restore it after.
    char buffer[256];
    getcwd(buffer, 256);
#endif

    m_device = createDevice(video::EDT_NULL);

#ifdef __APPLE__
    chdir( buffer );
#endif

    m_file_system  = m_device->getFileSystem();
    m_is_full_path = false;

    if ( getenv ( "SUPERTUXKART_DATADIR" ) != NULL )
        m_root_dir= getenv ( "SUPERTUXKART_DATADIR" ) ;
#ifdef __APPLE__
    else if( macSetBundlePathIfRelevant( m_root_dir ) ) { /* nothing to do */ }
#endif
   // else if(m_file_system->existFile("/Developer/games/supertuxkart/data/stk_config.xml"))
    //    m_root_dir = "/Developer/games/supertuxkart" ;
    // FIXME - existFile() fails to detect the file, even though it exists, on my computer
    else if(m_file_system->existFile("data/stk_config.xml"))
        m_root_dir = "." ;
    else if(m_file_system->existFile("../data/stk_config.xml"))
        m_root_dir = ".." ;
    else
#ifdef SUPERTUXKART_DATADIR
        m_root_dir = SUPERTUXKART_DATADIR ;
#else
        m_root_dir = "/usr/local/share/games/supertuxkart" ;
#endif
    // We can't use _() here, since translations will only be initalised
    // after the filemanager (to get the path to the tranlsations from it)
    fprintf(stderr, "Data files will be fetched from: '%s'\n",
            m_root_dir.c_str() );

    pushTextureSearchPath(m_root_dir+"/data/textures");
    pushModelSearchPath  (m_root_dir+"/data/models"  );
    pushMusicSearchPath  (m_root_dir+"/data/music"   );
    m_file_system->addFolderFileArchive("data/models");
    // Add more paths from the STK_MUSIC_PATH environment variable
    if(getenv("SUPERTUXKART_MUSIC_PATH")!=NULL)
    {
        std::string path=getenv("SUPERTUXKART_MUSIC_PATH");
        std::vector<std::string> dirs=StringUtils::split(path,':');
        for(int i=(int)dirs.size()-1; i>=0; i--)
        {
            // Remove '/' at the end of paths, since this can cause
            // problems with windows when using stat()
            while(dirs[i].size()>=1 && dirs[i][dirs[i].size()-1]=='/')
            {
                dirs[i]=dirs[i].substr(0, dirs[i].size()-1);
            }
            // remove empty entries
            if(dirs[i].size()==0)
            {
                dirs.erase(dirs.begin()+i);
                continue;
            }
        }
#ifdef WIN32
        // Handle filenames like d:/dir, which becomes ["d","/dir"]
        for(int i=(int)dirs.size()-1; i>=0; i--)
        {
            if(dirs[i].size()>1) continue;
            if(i==dirs.size()-1)    // last element
            {
                dirs[i]+=":";      // turn "c" back into "c:"
            }
            else
            {
                dirs[i]+=":"+dirs[i+1]; // restore "d:/dir" back
                dirs.erase(dirs.begin()+i+1);
            }
        }
#endif
        for(int i=0;i<(int)dirs.size(); i++)
            pushMusicSearchPath(dirs[i]);
    }
}  // FileManager

//-----------------------------------------------------------------------------
/** Remove the dummy file system (which is called from IrrDriver before
 *  creating the actual device.
 */
void FileManager::dropFileSystem()
{
    m_device->drop();
}   // dropFileSystem

//-----------------------------------------------------------------------------
/** This function is used to re-initialise the file-manager after reading in
 *  the user configuration data.
*/
void FileManager::setDevice(IrrlichtDevice *device)
{
    m_device = device;
    m_device->grab();  // To make sure that the device still exists while
                       // file_manager has a pointer to the file system.
    m_file_system  = m_device->getFileSystem();
}   // reInit

//-----------------------------------------------------------------------------
FileManager::~FileManager()
{
    popMusicSearchPath();
    popModelSearchPath();
    popTextureSearchPath();
    // m_file_system is ref-counted, so no delete/drop necessary.
    m_file_system = NULL;
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
    io::IXMLReader *xml_reader = createXMLReader(filename);
    if(!xml_reader) return NULL;
    return new XMLNode(xml_reader);
}   // getXMLTree

//-----------------------------------------------------------------------------
void FileManager::pushModelSearchPath(const std::string& path)
{
    m_model_search_path.push_back(path);
    m_file_system->addFolderFileArchive(path.c_str());
}   // pushModelSearchPath

//-----------------------------------------------------------------------------
void FileManager::pushTextureSearchPath(const std::string& path)
{
    m_texture_search_path.push_back(path);
    m_file_system->addFolderFileArchive(path.c_str());
}   // pushTextureSearchPath

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
std::string FileManager::getTrackDir() const
{
    return m_root_dir+"/data/tracks";
}   // getTrackDir
//-----------------------------------------------------------------------------
std::string FileManager::getDataDir() const
{
    return m_root_dir+"/data/";
}
//-----------------------------------------------------------------------------
std::string FileManager::getGUIDir() const
{
    return m_root_dir+"/data/gui";
}
//-----------------------------------------------------------------------------
std::string FileManager::getKartDir() const
{
    return m_root_dir+"/data/karts";
}   // getKartDir

//-----------------------------------------------------------------------------
std::string FileManager::getItemsDir() const
{
    return m_root_dir+"/data/items";
}   // getItemsDir
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
std::string FileManager::getTrackFile(const std::string& fname,
                                      const std::string& track_name) const
{
    // tracks file are in data/tracks/TRACKNAME/TRACKNAME.ext
    // but if a track name is supplied use it (which is necessary
    // e.g. to load a model from a track directory
    std::string basename = (track_name!="") ? track_name
                           : StringUtils::without_extension(fname);
    return getTrackDir()+"/"+basename+"/"+fname;
}   // getTrackFile

//-----------------------------------------------------------------------------
std::string FileManager::getKartFile(const std::string& fname,
                                     const std::string& kart_name) const
{
    // kart file are in data/karts/KARTNAME/KARTNAME.ext
    // but if a kart name is supplied use it (which is necessary
    // e.g. to load a model from a kart directory
    std::string basename = (kart_name!="") ? kart_name
                           : StringUtils::without_extension(fname);
    return getKartDir()+"/"+basename+"/"+fname;
}   // getKartFile

//-----------------------------------------------------------------------------
std::string FileManager::getConfigFile(const std::string& fname) const
{
    return m_root_dir+"/data/"+fname;
}   // getConfigFile

//-----------------------------------------------------------------------------
std::string FileManager::getItemFile(const std::string& fname) const
{
    return getItemsDir()+"/"+fname;
}   // getConfigFile

//-----------------------------------------------------------------------------
std::string FileManager::getHomeDir() const
{
    std::string DIRNAME;
#ifdef WIN32
    // Try to use the APPDATA directory to store config files and highscore
    // lists. If not defined, used the current directory.
    std::ostringstream s;
    if(getenv("APPDATA")!=NULL)
    {
        s<<getenv("APPDATA")<<"/supertuxkart/";
        DIRNAME=s.str();
    }
    else DIRNAME=".";

#else
    if(getenv("HOME")!=NULL)
    {
        DIRNAME = getenv("HOME");
    }
    else
    {
        DIRNAME = ".";
    }
    DIRNAME += "/";
    DIRNAME += CONFIGDIR;
#endif
    return DIRNAME;
}   // getHomeDir

//-----------------------------------------------------------------------------
std::string FileManager::getLogFile(const std::string& fname) const
{
    return getHomeDir()+"/"+fname;
}   // getLogFile

//-----------------------------------------------------------------------------
std::string FileManager::getMusicFile(const std::string& fname) const
{
    std::string path;
    findFile(path, fname, m_music_search_path);
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
    return getHomeDir()+"/"+fname;
}   // getHighscoreFile

//-----------------------------------------------------------------------------
void FileManager::listFiles(std::set<std::string>& result, const std::string& dir,
                            bool is_full_path, bool make_full_path) const
{
    result.clear();

#ifdef IRR_SVN
    std::string previous_cwd1 = std::string(m_file_system->getWorkingDirectory().c_str());
#else
    std::string previous_cwd1 = m_file_system->getWorkingDirectory();
#endif
#ifdef WIN32
    std::string path = is_full_path ? dir : m_root_dir+"/"+dir;
#else
    std::string path = is_full_path ? dir + "/" : m_root_dir+"/"+dir + "/";
#endif
    //printf("******* Path : %s \n", path.c_str());

    struct stat mystat;

    if(stat(path.c_str(), &mystat) < 0) return;
    if(! S_ISDIR(mystat.st_mode))       return;

#ifdef IRR_SVN
    std::string previous_cwd = std::string(m_file_system->getWorkingDirectory().c_str());
#else
    std::string previous_cwd = m_file_system->getWorkingDirectory();
#endif

    if(!m_file_system->changeWorkingDirectoryTo( path.c_str() ))
    {
        printf("FileManager::listFiles : Could not change CWD!\n");
        return;
    }
    irr::io::IFileList* files = m_file_system->createFileList();

    for(int n=0; n<(int)files->getFileCount(); n++)
    {
        //printf("---- Entry : %s \n", (make_full_path ? path+"/"+ files->getFileName(n) : files->getFileName(n)).c_str());
#ifdef IRR_SVN
        result.insert(make_full_path ? path+"/"+ files->getFileName(n).c_str() : files->getFileName(n).c_str());
#else
        result.insert(make_full_path ? path+"/"+ files->getFileName(n) : files->getFileName(n));
#endif
    }

    m_file_system->changeWorkingDirectoryTo( previous_cwd.c_str() );
}   // listFiles

//-----------------------------------------------------------------------------
