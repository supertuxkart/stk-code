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

#include "material_manager.hpp"

#include <stdexcept>
#include <sstream>

#include "file_manager.hpp"
#include "material.hpp"
#include "io/xml_node.hpp"
#include "utils/string_utils.hpp"

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
    m_materials.push_back(new Material(m_materials.size()));

    // Use temp material for reading, but then set the shared
    // material index later, so that these materials are not popped
#ifdef HAVE_IRRLICHT
    const std::string fname     = "materials.xml";
#else
    const std::string fname     = "materials.dat";
#endif
    std::string       full_name = file_manager->getTextureFile(fname);
    addSharedMaterial(full_name);
#ifndef HAVE_IRRLICHT
    ssgSetAppStateCallback(getAppState);
    fuzzy_gst        = getMaterial("fuzzy.rgb")->getState();
#endif
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
        std::ostringstream msg;
        msg<<"FATAL: File '"<<filename<<"' not found\n";
        throw std::runtime_error(msg.str());
    }
    if(!pushTempMaterial(filename))
    {
        std::ostringstream msg;
        msg <<"FATAL: Parsing error in '"<<filename<<"'\n";
        throw std::runtime_error(msg.str());
    }
    m_shared_material_index = (int)m_materials.size();
}   // addSharedMaterial

//-----------------------------------------------------------------------------
bool MaterialManager::pushTempMaterial(const std::string& filename)
{
#ifdef HAVE_IRRLICHT
    XMLReader *xml = file_manager->getXMLReader(filename);
    for(unsigned int i=0; i<xml->getNumNodes(); i++)
    {
        const XMLNode *node = xml->getNode(i);
        if(!node)
        {
            // We don't have access to the filename at this stage anymore :(
            fprintf(stderr, "Unknown node in material.dat file\n");
            exit(-1);
        }
        try
        {
            m_materials.push_back(new Material(node, m_materials.size() ));
        }
        catch(std::exception& e)
        {
            // The message contains a '%s' for the filename
            fprintf(stderr, e.what(), filename.c_str());
        }
    }   // for i<xml->getNumNodes)(
    return true;
#else
    FILE *fd = fopen(filename.c_str(), "r" );

    if ( fd == NULL ) return false;

    while ( parseMaterial ( fd ) )
        /* Read file */ ;

    fclose ( fd ) ;
    return true;
#endif
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
#ifndef HAVE_IRRLICHT
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
#endif
//-----------------------------------------------------------------------------
#ifndef HAVE_IRRLICHT
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
            m_materials.push_back(new Material (f, s, m_materials.size() ));
            return true ;
        }
    }

    return false ;
}   // parseMaterial
#endif

//-----------------------------------------------------------------------------
Material *MaterialManager::getMaterial ( ssgLeaf *l )
{
    return m_materials[l -> getExternalPropertyIndex ()] ;
}   // getMaterial

//-----------------------------------------------------------------------------
/** Returns the material of a given name, if it doesn't exist, it is loaded.
 *  Materials that are just loaded are not permanent, and so get deleted after
 *  a race (this is used to load temporary, track specific materials). To make
 *  material permanent, make_permanent must be set to true. This is used for
 *  the powerup_manager, since not all icons for the powerups are listed in the
 *  materials.dat file, causing the missing ones to be temporary only (and
 *  then get deleted after one race, causing the powerup_manager to have 
 *  invalid pointers.
 *  \param fname  Name of the material.
 *  \param is_full_path True if the name includes the path (defaults to false)
 *  \param make_permanent True if this material should be kept in memory 
 *                        (defaults to false)
 */
Material *MaterialManager::getMaterial(const std::string& fname, 
                                       bool is_full_path,
                                       bool make_permanent)
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
#ifdef HAVE_IRRLICHT
    Material* m=new Material(fname, m_materials.size(), is_full_path);
#else
    Material* m=new Material(fname, "", m_materials.size(), is_full_path);
#endif
    m_materials.push_back(m);
    if(make_permanent) m_shared_material_index = (int)m_materials.size();
    return m ;
}   // getMaterial

//=============================================================================
#ifndef HAVE_IRRLICHT
ssgState *getAppState ( char *fname )
{
    Material *m = material_manager->getMaterial ( fname ) ;
    return ( m == NULL ) ? NULL : m -> getState () ;
}   // getAppState
#endif

