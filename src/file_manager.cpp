//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
//            (C) 2008 Steve Baker, Joerg Henrichs
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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
#    define snprintf       _snprintf
#    define access         _access
#    define                F_OK  04
#  endif
#  define CONFIGDIR       "."
#else
#  define CONFIGDIR       ".supertuxkart"
#endif
#include "plib/ul.h"
#include "file_manager.hpp"
#include "world.hpp"
#include "btBulletDynamicsCommon.h"
#include "moving_physics.hpp"
#include "moving_texture.hpp"
#include "translation.hpp"
#include "material_manager.hpp"
#include "string_utils.hpp"

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
        data_dir = contents + std::string("/Resources/data");
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

FileManager::FileManager()
{
    m_is_full_path = false;
    
    m_current_callback_type = CB_COLLECTABLE;

    if ( getenv ( "SUPERTUXKART_DATADIR" ) != NULL )
        m_root_dir= getenv ( "SUPERTUXKART_DATADIR" ) ;
#ifdef __APPLE__
    else if( macSetBundlePathIfRelevant( m_root_dir ) ) { /* nothing to do */ }
#endif
    else if ( access ( "data/stk_config.data", F_OK ) == 0 )
        m_root_dir = "." ;
    else if ( access ( "../data/stk_config.data", F_OK ) == 0 )
        m_root_dir = ".." ;
    else
#ifdef SUPERTUXKART_DATADIR
        m_root_dir = SUPERTUXKART_DATADIR ;
#else
        m_root_dir = "/usr/local/share/games/supertuxkart" ;
#endif
    fprintf(stderr, _("Data files will be fetched from: '%s'\n"), 
            m_root_dir.c_str() );

    pushTextureSearchPath(m_root_dir+"/data/textures");
    pushModelSearchPath  (m_root_dir+"/models"       );
    pushMusicSearchPath  (m_root_dir+"/ogg"          );
}  // FileManager

//-----------------------------------------------------------------------------
FileManager::~FileManager()
{
    popMusicSearchPath();
    popModelSearchPath();
    popTextureSearchPath();
}   // ~FileManager

//-----------------------------------------------------------------------------
bool FileManager::findFile(std::string& full_path,
                      const std::string& fname, 
                      const std::vector<std::string>& search_path) const
{
    struct stat mystat;
    
    for(std::vector<std::string>::const_iterator i = search_path.begin();
        i != search_path.end(); ++i)
    {
        //full_path=m_root_dir + "/" + *i + "/" + fname;
        full_path = *i + "/" + fname;
        if(stat(full_path.c_str(), &mystat) >= 0) return true;
    }
    full_path="";
    return false;
}   // findFile

//-----------------------------------------------------------------------------
void FileManager::makeModelPath(char* path, const char* FNAME) const
{
    if(m_is_full_path)
    {
        strcpy(path, FNAME);
        return;
    }
    
    std::string p;
    if(findFile(p, FNAME, m_model_search_path))
    {
        strcpy(path, p.c_str());
        return;
    }
}   // makeModelPath

//-----------------------------------------------------------------------------
std::string FileManager::getTextureFile(const std::string& FNAME) const
{
    std::string path;
    findFile(path, FNAME, m_texture_search_path);
    return path;
}   // makeTexturePath

//-----------------------------------------------------------------------------
std::string FileManager::getModelFile(const std::string& FNAME) const
{
    std::string path;
    findFile(path, FNAME, m_model_search_path);
    return path;
}   // makeTexturePath

//-----------------------------------------------------------------------------
std::string FileManager::getKartFile(const std::string& fname) const
{
    return m_root_dir+"/data/"+fname;
}   // getKartFile

//-----------------------------------------------------------------------------
std::string FileManager::getTrackDir() const
{
    return m_root_dir+"/data/tracks";
}   // getTrackDir
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
std::string FileManager::getConfigFile(const std::string& fname) const
{
    return m_root_dir+"/data/"+fname;
}   // getConfigFile

//-----------------------------------------------------------------------------
std::string FileManager::getHomeDir() const
{
    std::string DIRNAME;
#ifdef WIN32
    // For now the old windows config way is used: store a config file
    // in the current directory (in other OS a special subdirectory is created)
    DIRNAME=".";
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
    return m_root_dir+"/oggs/"+fname;
}   // getMusicFile
//-----------------------------------------------------------------------------
std::string FileManager::getSFXFile(const std::string& fname) const
{
    return m_root_dir+"/wavs/"+fname;
}   // getSFXFile
//-----------------------------------------------------------------------------
std::string FileManager::getFontFile(const std::string& fname) const
{
    return m_root_dir+"/fonts/"+fname;
}   // getFontFile
//-----------------------------------------------------------------------------
std::string FileManager::getHighscoreFile(const std::string& fname) const
{
    return getHomeDir()+"/"+fname;
}   // getHighscoreFile

//-----------------------------------------------------------------------------
#ifdef HAVE_GHOST_REPLAY
std::string FileManager::getReplayFile(const std::string& fname) const
{
    return m_root_dir+"/replay/"+fname;
}   // getReplayFile
#endif
//-----------------------------------------------------------------------------
void FileManager::initConfigDir()
{
#ifdef WIN32
    /*nothing*/
#else
    /*if HOME environment variable exists
    create directory $HOME/.supertuxkart*/
    if(getenv("HOME")!=NULL)
    {
        std::string pathname;
        pathname = getenv("HOME");
        pathname += "/.supertuxkart";
        mkdir(pathname.c_str(), 0755);
    }
#endif
}   // initConfigDir

//-----------------------------------------------------------------------------
void FileManager::listFiles(std::set<std::string>& result, const std::string& dir,
                       bool is_full_path) const
{
        struct stat mystat;

        // don't list directories with a slash on the end, it'll fail on win32
        assert(dir[dir.size()-1] != '/');

        result.clear();

        std::string path = is_full_path ? dir : m_root_dir+"/"+dir;

        if(stat(path.c_str(), &mystat) < 0) return;
        if(! S_ISDIR(mystat.st_mode))       return; 

        ulDir* mydir = ulOpenDir(path.c_str());
        if(!mydir) return;

        ulDirEnt* mydirent;
        while( (mydirent = ulReadDir(mydir)) != 0)
        {
            result.insert(mydirent->d_name);
        }
        ulCloseDir(mydir);
}   // listFiles

//-----------------------------------------------------------------------------
/** Loads a .ac model
 *
 *  Loads the .ac model 'filename'. Callbacks contained in this file
 *  are stored in the callback class t. If optimise is set to false,
 *  the file will not be flattened, which is necessary for the kart
 *  models - flattening them will remove the wheel nodes, withouth
 *  which the wheels do not rotate.
 *
 *  \param filename File to load
 *
 *  \param t        Callback category for callbacks included in this
 *                  file (see callback_manager.hpp)
 *
 *  \param optimise Default is true. If set to false, the model will not
 *                  be flattened.
 */
ssgEntity *FileManager::load(const std::string& filename, CallbackType t,
                        bool optimise, bool is_full_path)
{
    m_current_callback_type   = t;
    m_is_full_path            = is_full_path;
    ssgEntity *obj            = optimise ? ssgLoad  (filename.c_str(), this) 
                                         : ssgLoadAC(filename.c_str(), this);
    preProcessObj(obj, false);
    return obj;
}   // load

//-----------------------------------------------------------------------------
void FileManager::preProcessObj ( ssgEntity *n, bool mirror )
{
    if ( n == NULL ) return ;

    n -> dirtyBSphere () ;

    if ( n -> isAKindOf ( ssgTypeLeaf() ) )
    {
        if ( mirror )
            for ( int i = 0 ; i < ((ssgLeaf *)n) -> getNumVertices () ; i++ )
                ((ssgLeaf *)n) -> getVertex ( i ) [ 0 ] *= -1.0f ;

        material_manager->getMaterial ( (ssgLeaf *) n ) -> applyToLeaf ( (ssgLeaf *) n ) ;
        return ;
    }

    if ( mirror && n -> isAKindOf ( ssgTypeTransform () ) )
    {
        sgMat4 xform ;

        ((ssgTransform *)n) -> getTransform ( xform ) ;
        xform [ 0 ][ 0 ] *= -1.0f ;
        xform [ 1 ][ 0 ] *= -1.0f ;
        xform [ 2 ][ 0 ] *= -1.0f ;
        xform [ 3 ][ 0 ] *= -1.0f ;
        ((ssgTransform *)n) -> setTransform ( xform ) ;
    }

    ssgBranch *b = (ssgBranch *) n ;

    for ( int i = 0 ; i < b -> getNumKids () ; i++ )
        preProcessObj ( b -> getKid ( i ), mirror ) ;
}

//-----------------------------------------------------------------------------
ssgBranch *FileManager::animInit (char *data ) const
{
    while ( ! isdigit ( *data ) && *data != '\0' )
        data++ ;

    const int   START_LIM =        strtol(data, &data, 0 );
    const int   END_LIM   =        strtol(data, &data, 0 );
    const float TIME_LIM  = (float)strtod(data, &data    );

    while ( *data <= ' ' && *data != '\0' )
        data++ ;

    char mode = toupper ( *data ) ;

    ssgTimedSelector *br = new ssgTimedSelector;

    br->setLimits  (START_LIM+1, END_LIM+1 ) ;
    br->setDuration(TIME_LIM ) ;
    br->setMode    ((mode=='O') ?  SSG_ANIM_ONESHOT
                    :  (mode=='S') ?  SSG_ANIM_SWING
                    : SSG_ANIM_SHUTTLE ) ;
    br->control    (SSG_ANIM_START ) ;

    return br;
}   // animInit


//-----------------------------------------------------------------------------
/** Handle userdata that is stored in the model files. Mostly the userdata
 *  indicates that a special branch is to be created (e.g. a ssgCutout instead
 * of the standard branch). But some userdata indicate that callbacks need
 * to be created, which are then handled by the callback manager.
 */

ssgBranch *FileManager::createBranch(char *data) const
{

    if ( data == NULL || data[0] != '@' ) return NULL;

    data++ ;   /* Skip the '@' */

    if ( strncmp("billboard", data, strlen("billboard") ) == 0 )
        return  new ssgCutout();

    if ( strncmp("DONT_DELETE", data, strlen("DONT_DELETE") ) == 0 )
    {
        printf("DONT\n");
        ssgBranch *br = new ssgTransform();
        br->setUserData(new ssgBase());
        return br;
    }

    if ( strncmp("invisible", data, strlen("invisible") ) == 0 )
        return new ssgInvisible();

    if ( strncmp ( "switch", data, strlen ( "switch" ) ) == 0 )
    {
        ssgSelector *sel = new ssgSelector();
        sel->select(0);
        return sel;
    }

    if ( strncmp ( "animate", data, strlen ( "animate" ) ) == 0 )
        return animInit(data);


    if ( strncmp ( "autodcs", data, strlen ( "autodcs" ) ) == 0 )
    {
        ssgTransform *br = new ssgTransform();
        Callback     *c  = new MovingTexture(data, br);
        br->setUserData(new ssgBase());
        callback_manager->addCallback(c, m_current_callback_type);
        return br;
    }

    if ( strncmp ( "autotex", data, strlen ( "autotex" ) ) == 0 )
    {
        ssgTexTrans *br = new ssgTexTrans();
        Callback    *c  = new MovingTexture(data, br);
        callback_manager->addCallback(c, m_current_callback_type);
        return br;
    }
    if(strncmp("physics", data, strlen("physics")) == 0)
    {
        MovingPhysics *mp = new MovingPhysics(std::string(data));
        callback_manager->addCallback(mp, m_current_callback_type);
        return mp;
    }
    fprintf(stderr, "Warning: Ignoring userdata '%s'\n", data);
    return NULL ;
}   // createBranch

