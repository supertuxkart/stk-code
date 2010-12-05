//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2006 Ingo Ruhnke <grumbel@gmx.de>
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

#include "karts/kart_properties_manager.hpp"

#include <algorithm>
#include <ctime>
#include <stdio.h>
#include <stdexcept>

#include "challenges/unlock_manager.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties.hpp"
#include "utils/string_utils.hpp"

KartPropertiesManager *kart_properties_manager=0;

std::vector<std::string> KartPropertiesManager::m_kart_search_path;

/** Constructor, only clears internal data structures. */
KartPropertiesManager::KartPropertiesManager()
{
    m_all_groups.clear();
}   // KartPropertiesManager

//-----------------------------------------------------------------------------
/** Destructor. Removes all allocated data.
 */
KartPropertiesManager::~KartPropertiesManager()
{
    for(KartPropertiesVector::iterator i  = m_karts_properties.begin();
        i != m_karts_properties.end(); ++i)
    {
       delete *i;
       // Set the entry in the kart properties vector to 0, otherwise
       // memory errors happen.
       m_karts_properties[i-m_karts_properties.begin()] = NULL;
    }
}   // ~KartPropertiesManager

//-----------------------------------------------------------------------------
/** Adds a directory from which karts are loaded. The kart manager checks if
 *  either this directory itself contains a kart, and if any subdirectory 
 *  contains a kart.
 *  \param dir The directory to add. 
 */
void KartPropertiesManager::addKartSearchDir(const std::string &s)
{
    m_kart_search_path.push_back(s);
}   // addKartSearchDir

//-----------------------------------------------------------------------------
void KartPropertiesManager::unloadAllKarts()
{
    for(KartPropertiesVector::iterator i  = m_karts_properties.begin();
        i != m_karts_properties.end(); ++i)
    {
        delete *i;
    }
    m_karts_properties.clear();
    m_selected_karts.clear();
    m_kart_available.clear();
    m_groups.clear();
    m_all_groups.clear();
    m_kart_search_path.clear();
}   // removeTextures

//-----------------------------------------------------------------------------
void KartPropertiesManager::reLoadAllKarts()
{
    for(KartPropertiesVector::iterator i  = m_karts_properties.begin();
        i != m_karts_properties.end(); ++i)
    {
        delete *i;
    }
    m_karts_properties.clear();
    m_selected_karts.clear();
    m_kart_available.clear();
    m_groups.clear();
    m_all_groups.clear();
    //m_kart_search_path.clear();
	loadAllKarts(false);
}  
//-----------------------------------------------------------------------------
/** Loads all kart properties and models.
 */
void KartPropertiesManager::loadAllKarts(bool loading_icon)
{
    m_all_kart_dirs.clear();
    for(std::vector<std::string>::const_iterator dir=m_kart_search_path.begin();
        dir!=m_kart_search_path.end(); dir++)
    {
        // First check if there is a kart in the current directory
        // -------------------------------------------------------
        if(loadKart(*dir)) continue;

        // If not, check each subdir of this directory.
        // --------------------------------------------
        std::set<std::string> result;
        file_manager->listFiles(result, *dir, /*is_full_path*/ true);
        for(std::set<std::string>::const_iterator subdir=result.begin();
            subdir!=result.end(); subdir++)
        {
            const bool loaded = loadKart(*dir+"/"+*subdir);
            
            if (loaded && loading_icon)
            {
                GUIEngine::addLoadingIcon(irr_driver->getTexture(
                        *dir + "/"+*subdir + "/" +
                        m_karts_properties[m_karts_properties.size()-1]->getIconFile()
                                                                 )
                        );
            }
        }   // for all files in the currently handled directory
    }   // for i
}   // loadAllKarts

//-----------------------------------------------------------------------------
/** Loads a single kart and (if not disabled) the oorresponding 3d model.
 *  \param filename Full path to the kart config file.
 */
bool KartPropertiesManager::loadKart(const std::string &dir)
{
    std::string config_filename=dir+"/kart.xml";
    FILE *f=fopen(config_filename.c_str(), "r");
    if(!f) return false;
    fclose(f);

    KartProperties* kart_properties;
    try
    {
        kart_properties = new KartProperties(config_filename);
    }
    catch (std::runtime_error& err)
    {
        std::cerr << "Giving up loading '" << config_filename.c_str()
                  << "' : " << err.what() << std::endl;
        return false;
    }
    
    // If the version of the kart file is not supported,
    // ignore this .kart file
    if (kart_properties->getVersion() < stk_config->m_min_kart_version ||
        kart_properties->getVersion() > stk_config->m_max_kart_version)
    {
        fprintf(stderr, "[KartPropertiesManager] Warning: kart '%s' is not supported by this binary, ignored.\n",
                kart_properties->getIdent().c_str());
        delete kart_properties;
        return false;
    }

    m_karts_properties.push_back(kart_properties);
    m_kart_available.push_back(true);
    const std::vector<std::string>& groups=kart_properties->getGroups();
    for(unsigned int g=0; g<groups.size(); g++)
    {
        if(m_groups.find(groups[g])==m_groups.end())
        {
            m_all_groups.push_back(groups[g]);
        }
        m_groups[groups[g]].push_back(m_karts_properties.size()-1);
    }
    m_all_kart_dirs.push_back(dir);
    return true;
}   // loadKartData

//-----------------------------------------------------------------------------
const int KartPropertiesManager::getKartId(const std::string &ident) const
{    
    for(KartPropertiesVector::const_iterator i  = m_karts_properties.begin();
        i != m_karts_properties.end(); ++i)
    {
        if(UserConfigParams::m_verbosity>=5)
            std::cout << "[KartPropertiesManager] getKartId : " << (*i)->getIdent().c_str() << std::endl;
        if ((*i)->getIdent() == ident)
            return i-m_karts_properties.begin();
    }

    std::ostringstream msg;
    msg << "KartPropertiesManager: Couldn't find kart: '" << ident << "'";
    throw std::runtime_error(msg.str());
}   // getKartId

//-----------------------------------------------------------------------------
const KartProperties* KartPropertiesManager::getKart(const std::string &ident) const
{
    for(KartPropertiesVector::const_iterator i  = m_karts_properties.begin();
        i != m_karts_properties.end(); ++i)
    {
        if ((*i)->getIdent() == ident)
            return *i;
    }

    return NULL;
}   // getKart

//-----------------------------------------------------------------------------
const KartProperties* KartPropertiesManager::getKartById(int i) const
{
    if (i < 0 || i >= int(m_karts_properties.size()))
        return NULL;

    return m_karts_properties[i];
}   // getKartById

//-----------------------------------------------------------------------------
/** Returns a list of all available kart identifiers.                        */
std::vector<std::string> KartPropertiesManager::getAllAvailableKarts() const
{
    std::vector<std::string> all;
    for(KartPropertiesVector::const_iterator i  = m_karts_properties.begin();
        i != m_karts_properties.end(); ++i)
    {
        if(m_kart_available[i-m_karts_properties.begin()])
            all.push_back((*i)->getIdent());
    }
    return all;
}   // getAllAvailableKarts

//-----------------------------------------------------------------------------
/** Marks all karts except the ones listed in the string vector to be 
 *  unavailable. This function is used on a client when receiving the list of
 *  karts from a client to mark all other karts as unavailable.
 *  \param karts List of karts that are available on a client.
 */
void KartPropertiesManager::setUnavailableKarts(std::vector<std::string> karts)
{
    for(KartPropertiesVector::const_iterator i  = m_karts_properties.begin();
        i != m_karts_properties.end(); ++i)
    {
        if(!m_kart_available[i-m_karts_properties.begin()]) continue;

        if(std::find(karts.begin(), karts.end(), (*i)->getIdent())
            == karts.end())
        {
            m_kart_available[i-m_karts_properties.begin()]=false;
            fprintf(stderr, "Kart '%s' not available on all clients, disabled.\n",
                (*i)->getIdent().c_str());

        }   // kart not in list
    }   // for i in m_kart_properties
    
}   // setUnavailableKarts
//-----------------------------------------------------------------------------
/** Returns the (global) index of the n-th kart of a given group. If there is
  * no such kart, -1 is returned.
  */
int KartPropertiesManager::getKartByGroup(const std::string& group, int n) const
{
    int count=0;
    for(KartPropertiesVector::const_iterator i  = m_karts_properties.begin();
        i != m_karts_properties.end(); ++i)
    {
        std::vector<std::string> groups=(*i)->getGroups();
        if (std::find(groups.begin(), groups.end(), group)==groups.end()) continue;
        if(count==n) return (int)(i-m_karts_properties.begin());
        count=count+1;
    }
    return -1;
}   // getKartByGroup

//-----------------------------------------------------------------------------
bool KartPropertiesManager::testAndSetKart(int kartid)
{
    if(!kartAvailable(kartid)) return false;
    m_selected_karts.push_back(kartid);
    return true;
}   // kartAvailable

//-----------------------------------------------------------------------------
/** Returns true if a kart is available to be selected. A kart is available to
 *  be selected if it is available on all clients (i.e. m_kart_available is 
 *  true), not yet selected, and not locked.
 */
bool KartPropertiesManager::kartAvailable(int kartid)
{
    if(kartid<0 || kartid>=(int)m_kart_available.size()) return false;
    if(!m_kart_available[kartid]) return false;
    std::vector<int>::iterator it;
    for (it = m_selected_karts.begin(); it < m_selected_karts.end(); it++)
    {
        if ( kartid == *it) return false;
    }
    const KartProperties *kartprop=getKartById(kartid);
    if(unlock_manager->isLocked(kartprop->getIdent())) return false;
    return true;
}   // kartAvailable

//-----------------------------------------------------------------------------
/** Sets a kart to be selected by specifying the identifier (name) of the kart.
 *  \param kart_name Name of the kart.
 */
void KartPropertiesManager::selectKartName(const std::string &kart_name)
{
    int kart_id = getKartId(kart_name);
    selectKart(kart_id);
}   // selectKartName

//-----------------------------------------------------------------------------
/** Returns a list of randomly selected karts. This list firstly contains
 *  karts in the currently selected group, but which are not in the list
 *  of 'existing karts'. If not enough karts are available in the current
 *  group, karts from all other groups are used to fill up the list.
 *  This is used by the race manager to select the AI karts.
 *  \param count Number of karts to select randomly.
 *  \param existing_karst List of karts that should not be used. This is the
 *                        list of karts selected by the players.
 */
std::vector<std::string> KartPropertiesManager::getRandomKartList(int count,
                                           RemoteKartInfoList& existing_karts)
{
    std::vector<std::string> random_karts;

    // First: set up flags (based on global kart 
    // index) for which karts are already used
    // -----------------------------------------
    std::vector<bool> used;
    used.resize(getNumberOfKarts(), false);
    
    std::vector<std::string> all_karts;
    for(unsigned int i=0; i<existing_karts.size(); i++)
    {
        try
        {
            int id=getKartId(existing_karts[i].getKartName());
            used[id] = true;
        }
        catch (std::runtime_error& ex)
        {
            std::cerr << "[KartPropertiesManager] getRandomKartList : WARNING, can't find kart '" << existing_karts[i].getKartName() << "'\n";
        }
    }

    // Add karts from the current group
    // --------------------------------
    std::vector<int> karts = getKartsInGroup(UserConfigParams::m_kart_group);
    std::vector<int>::iterator k;
    // Remove karts that are already used or generally not available
    // (i.e. locked or not available on all clients)
    for(unsigned int i=0; i<karts.size();)
    {
        if(used[karts[i]] || !m_kart_available[karts[i]] || 
            unlock_manager->isLocked(m_karts_properties[karts[i]]->getIdent()))
            karts.erase(karts.begin()+i);
        else
            i++;
    }
    std::srand((unsigned int)std::time(0));
    std::random_shuffle(karts.begin(), karts.end());

    // Loop over all karts to fill till either all slots are filled, or
    // there are no more karts in the current group
    while(count>0 && karts.size()>0)
    {
        //used[karts.back()] = true;
        random_karts.push_back(m_karts_properties[karts.back()]->getIdent());
        karts.pop_back();
        count --;
    }
    if(count==0) return random_karts;

    // Not enough karts in chosen group, so fill the rest with arbitrary karts
    // -----------------------------------------------------------------------
    // First create an index list with all unused karts.
    karts.clear();
    for(unsigned int i=0; i<getNumberOfKarts(); i++)
    {
        if(!used[i] && m_kart_available[i] &&
            !unlock_manager->isLocked(m_karts_properties[i]->getIdent()) )
            karts.push_back(i);
    }
    std::random_shuffle(karts.begin(), karts.end());
    
    // Then fill up the remaining empty spaces
    do
    {
        while(count>0 && karts.size()>0)
        {
            random_karts.push_back(m_karts_properties[karts.back()]->getIdent());
            karts.pop_back();
            count --;
        }
        
        // we used all karts but still need more... we'll have no choice but
        // to use the same karts more than once.
        if (count>0 && karts.size() == 0)
        {
            for(unsigned int i=0; i<getNumberOfKarts(); i++)
            {
                if(!used[i] && m_kart_available[i] &&
                   !unlock_manager->isLocked(m_karts_properties[i]->getIdent()) )
                    karts.push_back(i);
            }
            // If there are no unused karts, use used karts again.
            // This means that e.g. if only one kart is availabe, which is
            // used by the player, it will still be used by AI karts (which
            // can be useful for debugging).
            if(karts.size()==0)
            {
                for(unsigned int i=0; i<getNumberOfKarts(); i++)
                {
                    if(m_kart_available[i] &&
                        !unlock_manager->isLocked(m_karts_properties[i]->getIdent()) )
                        karts.push_back(i);
                }
            }
            std::random_shuffle(karts.begin(), karts.end());
        }
        
    } while(count>0);
    
    // There should always be enough karts
    assert(count==0);
    return random_karts;
}   // getRandomKartList    

/* EOF */
