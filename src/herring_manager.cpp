//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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
#include <string>
#include <sstream>
#include "user_config.hpp"
#include "herring_manager.hpp"
#include "loader.hpp"
#include "material_manager.hpp"
#include "material.hpp"
#include "kart.hpp"
#include "string_utils.hpp"
#include "translation.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif
/** Simple shadow class, only used here for default herrings. */
class Shadow
{
    ssgBranch *sh ;

public:
    Shadow ( float x1, float x2, float y1, float y2 ) ;
    ssgEntity *getRoot () { return sh ; }
}
;   // Shadow

//-----------------------------------------------------------------------------
Shadow::Shadow ( float x1, float x2, float y1, float y2 )
{
    ssgVertexArray   *va = new ssgVertexArray   () ; sgVec3 v ;
    ssgNormalArray   *na = new ssgNormalArray   () ; sgVec3 n ;
    ssgColourArray   *ca = new ssgColourArray   () ; sgVec4 c ;
    ssgTexCoordArray *ta = new ssgTexCoordArray () ; sgVec2 t ;

    sgSetVec4 ( c, 0.0f, 0.0f, 0.0f, 1.0f ) ; ca->add(c) ;
    sgSetVec3 ( n, 0.0f, 0.0f, 1.0f ) ; na->add(n) ;

    sgSetVec3 ( v, x1, y1, 0.10f ) ; va->add(v) ;
    sgSetVec3 ( v, x2, y1, 0.10f ) ; va->add(v) ;
    sgSetVec3 ( v, x1, y2, 0.10f ) ; va->add(v) ;
    sgSetVec3 ( v, x2, y2, 0.10f ) ; va->add(v) ;

    sgSetVec2 ( t, 0.0f, 0.0f ) ; ta->add(t) ;
    sgSetVec2 ( t, 1.0f, 0.0f ) ; ta->add(t) ;
    sgSetVec2 ( t, 0.0f, 1.0f ) ; ta->add(t) ;
    sgSetVec2 ( t, 1.0f, 1.0f ) ; ta->add(t) ;

    sh = new ssgBranch ;
    sh -> clrTraversalMaskBits ( SSGTRAV_ISECT|SSGTRAV_HOT ) ;

    sh -> setName ( "Shadow" ) ;

    ssgVtxTable *gs = new ssgVtxTable ( GL_TRIANGLE_STRIP, va, na, ta, ca ) ;

    gs -> clrTraversalMaskBits ( SSGTRAV_ISECT|SSGTRAV_HOT ) ;
    gs -> setState ( fuzzy_gst ) ;
    sh -> addKid ( gs ) ;
    sh -> ref () ; /* Make sure it doesn't get deleted by mistake */
}   // Shadow

//=============================================================================
HerringManager* herring_manager;
typedef std::map<std::string,ssgEntity*>::const_iterator CI_type;

HerringManager::HerringManager()
{
    m_all_models.clear();
    // The actual loading is done in loadDefaultHerrings
}   // HerringManager

//-----------------------------------------------------------------------------
void HerringManager::removeTextures()
{
    for(AllHerringType::iterator i =m_all_herrings.begin();
        i!=m_all_herrings.end();  i++)
    {
        delete *i;
    }
    m_all_herrings.clear();

    for(CI_type i=m_all_models.begin(); i!=m_all_models.end(); ++i)
    {
        ssgDeRefDelete(i->second);
    }
    m_all_models.clear();
    callback_manager->clear(CB_HERRING);
}   // removeTextures

//-----------------------------------------------------------------------------
HerringManager::~HerringManager()
{
    for(CI_type i=m_all_models.begin(); i!=m_all_models.end(); ++i)
    {
        ssgDeRefDelete(i->second);
    }
}   // ~HerringManager

//-----------------------------------------------------------------------------
void HerringManager::loadDefaultHerrings()
{
    // Load all models. This can't be done in the constructor, since the loader
    // isn't ready at that stage.

    // Load all models from the models/herrings directory
    // --------------------------------------------------
    std::set<std::string> files;
    loader->listFiles(files, "models/herrings");
    for(std::set<std::string>::iterator i  = files.begin();
            i != files.end();  ++i)
        {
            if(!StringUtils::has_suffix(*i, ".ac")) continue;
            std::string fullName  = "herrings/"+(*i);
            ssgEntity*  h         = loader->load(fullName, CB_HERRING);
            std::string shortName = StringUtils::without_extension(*i);
            h->ref();
            h->setName(shortName.c_str());
            m_all_models[shortName] = h;
        }   // for i


    // Load the old, internal only models
    // ----------------------------------
    sgVec3 yellow = { 1.0f, 1.0f, 0.4f }; CreateDefaultHerring(yellow, "OLD_GOLD"  );
    sgVec3 cyan   = { 0.4f, 1.0f, 1.0f }; CreateDefaultHerring(cyan  , "OLD_SILVER");
    sgVec3 red    = { 0.8f, 0.0f, 0.0f }; CreateDefaultHerring(red   , "OLD_RED"   );
    sgVec3 green  = { 0.0f, 0.8f, 0.0f }; CreateDefaultHerring(green , "OLD_GREEN" );

    setDefaultHerringStyle();
}   // loadDefaultHerrings

//-----------------------------------------------------------------------------
void HerringManager::setDefaultHerringStyle()
{
    // This should go in an internal, system wide configuration file
    const std::string DEFAULT_NAMES[4] = {"bonusblock", "banana",
                                   "goldcoin",   "silvercoin"};

    bool bError=0;
    char msg[MAX_ERROR_MESSAGE_LENGTH];
    for(int i=HE_RED; i<=HE_SILVER; i++)
    {
        m_herring_model[i] = m_all_models[DEFAULT_NAMES[i]];
        if(!m_herring_model[i])
        {
            snprintf(msg, sizeof(msg), 
                     "Herring model '%s' is missing (see herring_manager)!\n",
                     DEFAULT_NAMES[i].c_str());
            bError=1;
            break;
        }   // if !m_herring_model
    }   // for i
    if(bError)
    {
        fprintf(stderr, "The following models are available:\n");
        for(CI_type i=m_all_models.begin(); i!=m_all_models.end(); ++i)
        {
            if(i->second)
            {
                if(i->first.substr(0,3)=="OLD")
                {
                    fprintf(stderr,"   %s internally only.\n",i->first.c_str());
                }
                else
                {
                    fprintf(stderr, "   %s in models/herrings/%s.ac.\n",
                            i->first.c_str(),
                            i->first.c_str());
                }
            }  // if i->second
        }
        throw std::runtime_error(msg);
        exit(-1);
    }   // if bError

}   // setDefaultHerringStyle

//-----------------------------------------------------------------------------
Herring* HerringManager::newHerring(herringType type, sgVec3* xyz)
{
    Herring* h = new Herring(type, xyz, m_herring_model[type]);
    m_all_herrings.push_back(h);
    return h;
}   // newHerring

//-----------------------------------------------------------------------------
void  HerringManager::hitHerring(Kart* kart)
{
    for(AllHerringType::iterator i =m_all_herrings.begin();
        i!=m_all_herrings.end();  i++)
    {
        if((*i)->wasEaten()) continue;
        if((*i)->hitKart(kart))
        {
            (*i)->isEaten();
            kart->collectedHerring(*i);
        }   // if hit
    }   // for m_all_herrings
}   // hitHerring

//-----------------------------------------------------------------------------
/** Remove all herring instances, and the track specific models. This is used
 *  just before a new track is loaded and a race is started.
 */
void HerringManager::cleanup()
{
    for(AllHerringType::iterator i =m_all_herrings.begin();
        i!=m_all_herrings.end();  i++)
    {
        delete *i;
    }
    m_all_herrings.clear();

    setDefaultHerringStyle();

    // Then load the default style from the user_config file
    // -----------------------------------------------------
    // This way if a herring is not defined in the herringstyle-file, the
    // default (i.e. old herring) is used.
    try
    {
        // FIXME: This should go in a system-wide configuration file,
        //        and only one of this and the hard-coded settings in
        //        setDefaultHerringStyle are necessary!!!
        loadHerringStyle(user_config->m_herring_style);
    }
    catch(std::runtime_error)
    {
        fprintf(stderr,_("The herring style '%s' in your configuration file does not exist.\nIt is ignored.\n"),
                user_config->m_herring_style.c_str());
        user_config->m_herring_style="";
    }

    try
    {
        loadHerringStyle(m_user_filename);
    }
    catch(std::runtime_error)
    {
        fprintf(stderr,_("The herring style '%s' specified on the command line does not exist.\nIt is ignored.\n"),
                m_user_filename.c_str());
        m_user_filename="";  // reset to avoid further warnings.
    }

}   // cleanup

//-----------------------------------------------------------------------------
/** Remove all herring instances, and the track specific models. This is used
 * just before a new track is loaded and a race is started
 */
void HerringManager::reset()
{
    for(AllHerringType::iterator i =m_all_herrings.begin();
        i!=m_all_herrings.end();  i++)
    {
        (*i)->reset();
    }  // for i
}   // reset

//-----------------------------------------------------------------------------
void HerringManager::update(float delta)
{
    for(AllHerringType::iterator i =m_all_herrings.begin();
        i!=m_all_herrings.end();  i++)
    {
        (*i)->update(delta);
    }   // for m_all_herrings
}   // delta

//-----------------------------------------------------------------------------
void HerringManager::CreateDefaultHerring(sgVec3 colour, std::string name)
{
    ssgVertexArray   *va = new ssgVertexArray   () ; sgVec3 v ;
    ssgNormalArray   *na = new ssgNormalArray   () ; sgVec3 n ;
    ssgColourArray   *ca = new ssgColourArray   () ; sgVec4 c ;
    ssgTexCoordArray *ta = new ssgTexCoordArray () ; sgVec2 t ;

    sgSetVec3(v, -0.5f, 0.0f, 0.0f ) ; va->add(v) ;
    sgSetVec3(v,  0.5f, 0.0f, 0.0f ) ; va->add(v) ;
    sgSetVec3(v, -0.5f, 0.0f, 0.5f ) ; va->add(v) ;
    sgSetVec3(v,  0.5f, 0.0f, 0.5f ) ; va->add(v) ;
    sgSetVec3(v, -0.5f, 0.0f, 0.0f ) ; va->add(v) ;
    sgSetVec3(v,  0.5f, 0.0f, 0.0f ) ; va->add(v) ;

    sgSetVec3(n,  0.0f,  1.0f,  0.0f ) ; na->add(n) ;

    sgCopyVec3 ( c, colour ) ; c[ 3 ] = 1.0f ; ca->add(c) ;

    sgSetVec2(t, 0.0f, 0.0f ) ; ta->add(t) ;
    sgSetVec2(t, 1.0f, 0.0f ) ; ta->add(t) ;
    sgSetVec2(t, 0.0f, 1.0f ) ; ta->add(t) ;
    sgSetVec2(t, 1.0f, 1.0f ) ; ta->add(t) ;
    sgSetVec2(t, 0.0f, 0.0f ) ; ta->add(t) ;
    sgSetVec2(t, 1.0f, 0.0f ) ; ta->add(t) ;


    ssgLeaf *gset = new ssgVtxTable ( GL_TRIANGLE_STRIP, va, na, ta, ca ) ;

    gset->setState(material_manager->getMaterial("herring.rgb")->getState()) ;

    Shadow* sh = new Shadow ( -0.5f, 0.5f, -0.25f, 0.25f ) ;

    ssgTransform* tr = new ssgTransform () ;

    tr -> addKid ( sh -> getRoot () ) ;
    tr -> addKid ( gset ) ;
    tr -> ref () ; /* Make sure it doesn't get deleted by mistake */
    m_all_models[name] = tr;

}   // CreateDefaultHerring

//-----------------------------------------------------------------------------
void HerringManager::loadHerringStyle(const std::string filename)
{
    if(filename.length()==0) return;
    const lisp::Lisp* root = 0;
    lisp::Parser parser;
    const std::string TMP = "data/" + (std::string)filename + ".herring";
    root = parser.parse(loader->getPath(TMP));

    const lisp::Lisp* herring_node = root->getLisp("herring");
    if(!herring_node)
    {
        char msg[MAX_ERROR_MESSAGE_LENGTH];
        snprintf(msg, sizeof(msg), _("Couldn't load map '%s': no herring node."),
                 filename.c_str());
	delete root;
        throw std::runtime_error(msg);
        delete root;
    }
    setHerring(herring_node, "red",   HE_RED   );
    setHerring(herring_node, "green", HE_GREEN );
    setHerring(herring_node, "gold"  ,HE_GOLD  );
    setHerring(herring_node, "silver",HE_SILVER);
    delete root;
}   // loadHerringStyle

//-----------------------------------------------------------------------------
void HerringManager::setHerring(const lisp::Lisp *herring_node,
                                char *colour, herringType type)
{
    std::string name;
    herring_node->get(colour, name);
    if(name.size()>0)
    {
        m_herring_model[type]=m_all_models[name];
    }
}   // setHerring
