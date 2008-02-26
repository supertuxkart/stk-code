//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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
#    define snprintf       _snprintf
#  endif
#endif
#include "plib/ul.h"
#include "loader.hpp"
#include "world.hpp"
#include "btBulletDynamicsCommon.h"
#include "moving_physics.hpp"
#include "moving_texture.hpp"
#include "translation.hpp"
#include "material_manager.hpp"

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

Loader* loader = 0;

Loader::Loader()
{
    std::string datadir;
    m_current_callback_type = CB_COLLECTABLE;

    if ( getenv ( "SUPERTUXKART_DATADIR" ) != NULL )
        datadir = getenv ( "SUPERTUXKART_DATADIR" ) ;
    else
#ifdef __APPLE__
        if( macSetBundlePathIfRelevant( datadir ) ) { /* nothing to do */ }
    else
#endif
#ifdef _MSC_VER
        if ( _access ( "data\\tuxtrack.track", 04 ) == 0 )
#else
        if ( access ( "data/tuxtrack.track", F_OK ) == 0 )
#endif
            datadir = "." ;
        else
#ifdef _MSC_VER
            if ( _access ( "..\\data\\tuxtrack.track", 04 ) == 0 )
#else
            if ( access ( "../data/tuxtrack.track", F_OK ) == 0 )
#endif
                datadir = ".." ;
            else
#ifdef SUPERTUXKART_DATADIR
                datadir = SUPERTUXKART_DATADIR ;
#else
                datadir = "/usr/local/share/games/supertuxkart" ;
#endif
    fprintf(stderr, _("Data files will be fetched from: '%s'\n"), 
            datadir.c_str() ) ;
    addSearchPath(datadir);
}  // Loader

//-----------------------------------------------------------------------------
Loader::~Loader()
{}   // ~Loader

//-----------------------------------------------------------------------------
void Loader::makePath(std::string& path, const std::string& dir, const std::string& fname) const
{

    struct stat mystat;

    for(std::vector<std::string>::const_iterator i = m_search_path.begin();
        i != m_search_path.end(); ++i)
    {
        path=(*i)+DIR_SEPARATOR+dir+DIR_SEPARATOR+fname;
        if(stat(path.c_str(), &mystat) >= 0) return;
    }

    // error case...
    char msg[MAX_ERROR_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg), "Could not find path for '%s'.", fname.c_str());

    throw std::runtime_error(msg);

}   // makePath

//-----------------------------------------------------------------------------
void Loader::makeModelPath(char* path, const char* FNAME) const
{
    std::string p(path);
    makePath(p, std::string(getModelDir()), FNAME);
    strcpy(path, p.c_str());
}   // makeModelPath

//-----------------------------------------------------------------------------
std::string Loader::getTextureFile(const std::string& FNAME) const
{
    std::string path;
    makePath(path, getTextureDir(), FNAME.c_str());
    return path;
}   // makeTexturePath

//-----------------------------------------------------------------------------
std::string Loader::getKartFile(const std::string& fname) const
{
    std::string path;
    makePath(path, "data", fname.c_str());
    return path;
}   // getKartFile

//-----------------------------------------------------------------------------
std::string Loader::getTrackFile(const std::string& fname) const
{
    std::string path;
    makePath(path, "data", fname.c_str());
    return path;
}   // getTrackFile

//-----------------------------------------------------------------------------
std::string Loader::getConfigFile(const std::string& fname) const
{
    std::string path;
    makePath(path, "data", fname.c_str());
    return path;
}   // getConfigFile

//-----------------------------------------------------------------------------
std::string Loader::getHomeDir() const
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
    DIRNAME += DIR_SEPARATOR+CONFIGDIR;
#endif
    return DIRNAME;
}   // getHomeDir

//-----------------------------------------------------------------------------
std::string Loader::getLogFile(const std::string& fname) const
{
    return getHomeDir()+DIR_SEPARATOR+fname;
}   // getConfigFile

//-----------------------------------------------------------------------------
std::string Loader::getHighscoreFile(const std::string& fname) const
{
    return getHomeDir()+DIR_SEPARATOR+fname;
}   // getHighscoreFile

//-----------------------------------------------------------------------------
#ifdef HAVE_GHOST_REPLAY
std::string Loader::getReplayFile(const std::string& fname) const
{
    std::string path;
    makePath(path, "replay", fname.c_str());
    return path;

}   // getReplayFile
#endif
//-----------------------------------------------------------------------------
void Loader::addSearchPath(const std::string& PATH)
{
    m_search_path.push_back(PATH);
}   // addSearchPath

//-----------------------------------------------------------------------------
void Loader::initConfigDir()
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
std::string Loader::getPath(const char* FNAME) const
{
    struct stat mystat;
    std::string result;

    std::string native_fname=FNAME;
    const size_t LEN = strlen(FNAME);
    for(size_t i = 0; i < LEN; ++i)
        if(native_fname[i] == '\\' || native_fname[i] == '/')
            native_fname[i] = DIR_SEPARATOR;

    for(std::vector<std::string>::const_iterator i = m_search_path.begin();
        i != m_search_path.end(); ++i)
    {
        result = *i;
        result += DIR_SEPARATOR;
        result += native_fname;

        if(stat(result.c_str(), &mystat) < 0)
            continue;

        return result;
    }

    char msg[MAX_ERROR_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg), _("Couldn't find file '%s'."), FNAME);
    throw std::runtime_error(msg);
}   // getPath

//-----------------------------------------------------------------------------
void Loader::listFiles(std::set<std::string>& result, const std::string& dir)
    const
    {
        struct stat mystat;

        // don't list directories with a slash on the end, it'll fail on win32
        assert(dir[dir.size()-1] != '/');

        result.clear();

        for(std::vector<std::string>::const_iterator i = m_search_path.begin();
            i != m_search_path.end(); ++i)
        {
            std::string path = *i+DIR_SEPARATOR+dir;

            if(stat(path.c_str(), &mystat) < 0)
                continue;
            if(! S_ISDIR(mystat.st_mode))
                continue;

            ulDir* mydir = ulOpenDir(path.c_str());
            if(!mydir) continue;

            ulDirEnt* mydirent;
            while( (mydirent = ulReadDir(mydir)) != 0)
            {
                result.insert(mydirent->d_name);
            }
            ulCloseDir(mydir);
        }
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
ssgEntity *Loader::load(const std::string& filename, CallbackType t,
                        bool optimise)
{
    m_current_callback_type=t;
    ssgEntity *obj = optimise ? ssgLoad  (filename.c_str(), this) 
                              : ssgLoadAC(filename.c_str(), this);
    preProcessObj(obj, false);
    return obj;
}   // load

//-----------------------------------------------------------------------------
void Loader::preProcessObj ( ssgEntity *n, bool mirror )
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
ssgBranch *Loader::animInit (char *data ) const
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

ssgBranch *Loader::createBranch(char *data) const
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

