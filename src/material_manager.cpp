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

#include <stdexcept>
#include "file_manager.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "translation.hpp"
#include "string_utils.hpp"

#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#  define strdup _strdup
#endif

ssgState *fuzzy_gst;

MaterialManager *material_manager=0;

MaterialManager::MaterialManager()
{
    /* Create list - and default material zero */

    m_materials.reserve(256);
    // We can't call init/loadMaterial here, since the global variable
    // material_manager has not yet been initialised, and
    // material_manager is used in the Material constructor.
    // Therefore, the code for loading the material had to
    // be moved into a separate function.
}

//-----------------------------------------------------------------------------
int MaterialManager::addEntity(Material *m)
{
    m_materials.push_back(m);
    return (int)m_materials.size()-1;
}

//-----------------------------------------------------------------------------
void MaterialManager::reInit()
{
    for(std::vector<Material*>::const_iterator i=m_materials.begin();
        i!=m_materials.end(); i++)
    {
        delete *i;
    }
    m_materials.clear();
    loadMaterial();
}   // reInit

//-----------------------------------------------------------------------------
void MaterialManager::loadMaterial()
{
    // Create the default/empty material.
    m_materials.push_back(new Material((int)m_materials.size()));

    // Use temp material for reading, but then set the shared
    // material index later, so that these materials are not popped
    const std::string fname     = "materials.dat";
    std::string       full_name = file_manager->getTextureFile(fname);
    addSharedMaterial(full_name);

    ssgSetAppStateCallback(getAppState);
    fuzzy_gst        = getMaterial("fuzzy.rgb")->getState();
    // Save index of shared textures
    m_shared_material_index = (int)m_materials.size();
}   // MaterialManager

//-----------------------------------------------------------------------------
void MaterialManager::addSharedMaterial(const std::string& filename)
{
    // Use temp material for reading, but then set the shared
    // material index later, so that these materials are not popped
    if(filename=="")
    {
        char msg[MAX_ERROR_MESSAGE_LENGTH];
        snprintf(msg, sizeof(msg), "FATAL: File '%s' not found\n", filename.c_str());
        throw std::runtime_error(msg);
    }
    if(!pushTempMaterial(filename))
    {
        char msg[MAX_ERROR_MESSAGE_LENGTH];
        snprintf(msg, sizeof(msg), "FATAL: Parsing error in '%s'\n", filename.c_str());
        throw std::runtime_error(msg);
    }
    m_shared_material_index = (int)m_materials.size();
}   // addSharedMaterial

//-----------------------------------------------------------------------------
bool MaterialManager::pushTempMaterial(const std::string& filename)
{
    FILE *fd = fopen(filename.c_str(), "r" );

    if ( fd == NULL ) return false;

    while ( parseMaterial ( fd ) )
        /* Read file */ ;

    fclose ( fd ) ;
    return true;
}   // pushTempMaterial

//-----------------------------------------------------------------------------
void MaterialManager::popTempMaterial()
{
    for(int i=(int)m_materials.size()-1; i>=this->m_shared_material_index; i--)
    {
        delete m_materials[i];
        m_materials.pop_back();
    }   // for i
}   // popTempMaterial

//-----------------------------------------------------------------------------
char* MaterialManager::parseFileName(char **str)
{
    char *p = *str ;

    /* Skip leading spaces */
    while ( *p <= ' ' && *p != '\0' ) p++ ;

    /* Skip blank lines and comments */
    if ( *p == '#' || *p == '\0' )
        return NULL ;

    if ( *p != '"' )
    {
        fprintf(stderr, "ERROR: Material file entries must start with '\"'\n"
                "ERROR: Offending line is '%s'\n", *str);
        return NULL ;
    }

    /* Filename? */
    char *f = ++p ;
    while ( *p != '"' && *p != '\0' ) p++ ;

    if ( *p != '"' )
    {
        fprintf(stderr,
                "ERROR: Unterminated string constant '%s' in materials file.\n", *str ) ;
        return NULL ;
    }

    *p = '\0' ;
    *str = ++p ;

    return f ;
}   // parseFilename

//-----------------------------------------------------------------------------
int MaterialManager::parseMaterial ( FILE *fd )
{
    char str [ 1024 ] ;

    while ( ! feof ( fd ) )
    {
        char *s = str ;

        if ( fgets ( s, 1024, fd ) == NULL )
            return false ;

        s [ strlen(s) - 1 ] = '\0' ;

        char *f = parseFileName ( & s ) ;

        if ( f != NULL )
        {
            m_materials.push_back(new Material (f, s, (int)m_materials.size() ));
            return true ;
        }
    }

    return false ;
}   // parseMaterial

//-----------------------------------------------------------------------------
Material *MaterialManager::getMaterial ( ssgLeaf *l )
{
    return m_materials[l -> getExternalPropertyIndex ()] ;
}   // getMaterial

//-----------------------------------------------------------------------------
Material *MaterialManager::getMaterial(const std::string& fname, 
                                       bool is_full_path )
{
    if(fname=="")
    {
        // This happens while reading the stk_config file, which contains
        // kart_properties information (but no icon file): since at this 
        // stage loadMaterial() hasn't been called, an exception can be
        // triggered here (as it happened with visual c++), when
        // m_materials[0] is accessed.
        if(m_materials.size()>=1) return m_materials[0];
        return NULL;
    }

    std::string basename=StringUtils::basename(fname);

    // Search backward so that temporary (track) textures are found first
    for(int i = (int)m_materials.size()-1; i>=0; i-- )
    {
        if(m_materials[i]->getTexFname()==basename) return m_materials[i];
    }

    // Add the new material
    Material* m=new Material(fname,"", (int)m_materials.size(), is_full_path);
    m_materials.push_back(m);

    return m ;
}   // getMaterial

//=============================================================================
ssgState *getAppState ( char *fname )
{
    Material *m = material_manager->getMaterial ( fname ) ;
    return ( m == NULL ) ? NULL : m -> getState () ;
}   // getAppState

