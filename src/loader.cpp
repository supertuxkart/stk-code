//  $Id: loader.cpp 1610 2008-03-01 03:18:53Z hikerstk $
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
#endif

#include "loader.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "graphics/moving_texture.hpp"
#include "io/file_manager.hpp"
#include "physics/physical_object.hpp"

Loader* loader = 0;

Loader::Loader()
{
}  // Loader

//-----------------------------------------------------------------------------
Loader::~Loader()
{
}   // ~Loader

//-----------------------------------------------------------------------------
void Loader::makeModelPath(char* path, const char* FNAME) const
{
    if(m_is_full_path)
    {
        strcpy(path, FNAME);
        return;
    }
    
    std::string p=file_manager->getModelFile(FNAME);
    strcpy(path, p.c_str());
    return;
}   // makeModelPath

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
ssgEntity *Loader::load(const std::string& filename,
                        bool optimise, bool is_full_path)
{
    m_is_full_path            = is_full_path;
    // FIXME: for now
    return NULL;
}   // load

//-----------------------------------------------------------------------------
#ifndef HAVE_IRRLICHT
void Loader::preProcessObj ( ssgEntity *n, bool mirror )
{
    if ( n == NULL ) return ;

    n -> dirtyBSphere () ;

    if ( n -> isAKindOf ( ssgTypeLeaf() ) )
    {
        if ( mirror )
            for ( int i = 0 ; i < ((ssgLeaf *)n) -> getNumVertices () ; i++ )
                ((ssgLeaf *)n) -> getVertex ( i ) [ 0 ] *= -1.0f ;

        material_manager->getMaterial((ssgLeaf *) n ) -> applyToLeaf ( (ssgLeaf *) n ) ;
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
#endif
//-----------------------------------------------------------------------------
#ifndef HAVE_IRRLICHT
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

#endif
//-----------------------------------------------------------------------------
#ifndef HAVE_IRRLICHT
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
#endif
