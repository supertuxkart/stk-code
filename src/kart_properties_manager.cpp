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

#include <stdexcept>
#include <algorithm>
#include <ctime>
#include "file_manager.hpp"
#include "string_utils.hpp"
#include "kart_properties_manager.hpp"
#include "kart_properties.hpp"
#include "translation.hpp"
#include "user_config.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

KartPropertiesManager *kart_properties_manager=0;

KartPropertiesManager::KartPropertiesManager()
{
    m_all_groups.clear();
}   // KartPropertiesManager

//-----------------------------------------------------------------------------
KartPropertiesManager::~KartPropertiesManager()
{
    for(KartPropertiesVector::iterator i  = m_karts_properties.begin();
        i != m_karts_properties.end(); ++i)
        delete *i;
}   // ~KartPropertiesManager

//-----------------------------------------------------------------------------
void KartPropertiesManager::removeTextures()
{
    for(KartPropertiesVector::iterator i  = m_karts_properties.begin();
        i != m_karts_properties.end(); ++i)
    {
        delete *i;
    }
    m_karts_properties.clear();
    callback_manager->clear(CB_KART);
}   // removeTextures

//-----------------------------------------------------------------------------
void KartPropertiesManager::loadKartData(bool dont_load_models)
{
    m_max_steer_angle = -1.0f;
    std::set<std::string> result;
    file_manager->listFiles(result, file_manager->getKartDir(), 
                            /*is_full_path*/ true);

    // Find out which characters are available and load them
    for(std::set<std::string>::iterator i = result.begin();
            i != result.end(); ++i)
    {
        std::string kart_file;
        try
        {
            kart_file = file_manager->getKartFile((*i)+".kart");
        }
        catch (std::exception& e)
        {
            (void)e;   // remove warning about unused variable
            continue;
        }
        FILE *f=fopen(kart_file.c_str(),"r");
        if(!f) continue;
        fclose(f);
        KartProperties* kp = new KartProperties();
        kp->load(kart_file, "tuxkart-kart", dont_load_models);
        m_karts_properties.push_back(kp);
        if(kp->getMaxSteerAngle(0) > m_max_steer_angle)
        {
            m_max_steer_angle = kp->getMaxSteerAngle(0);
        }
        const std::vector<std::string>& groups=kp->getGroups();
        for(unsigned int g=0; g<groups.size(); g++)
        {
            if(m_groups.find(groups[g])==m_groups.end())
            {
                m_all_groups.push_back(groups[g]);
            }
            m_groups[groups[g]].push_back(m_karts_properties.size()-1);
        }
    }   // for i
}   // loadKartData

//-----------------------------------------------------------------------------
const int KartPropertiesManager::getKartId(const std::string IDENT) const
{
    int j = 0;
    for(KartPropertiesVector::const_iterator i  = m_karts_properties.begin();
        i != m_karts_properties.end(); ++i)
    {
        if ((*i)->getIdent() == IDENT)
            return j;
        ++j;
    }

    char msg[MAX_ERROR_MESSAGE_LENGTH];
    snprintf(msg, sizeof(msg), "KartPropertiesManager: Couldn't find kart: '%s'",
             IDENT.c_str());
    throw std::runtime_error(msg);
}   // getKartId

//-----------------------------------------------------------------------------
const KartProperties* KartPropertiesManager::getKart(const std::string IDENT) const
{
    for(KartPropertiesVector::const_iterator i  = m_karts_properties.begin();
        i != m_karts_properties.end(); ++i)
    {
        if ((*i)->getIdent() == IDENT)
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
}

//-----------------------------------------------------------------------------
/** Returns the (global) index of the n-th kart of a given group. If there is
  * no such kart, -1 is returned 
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
void KartPropertiesManager::fillWithRandomKarts(std::vector<std::string>& vec)
{
    // First: set up flags (based on global kart 
    // index) for which karts are already used
    // -----------------------------------------
    std::vector<bool> used;
    used.resize(getNumberOfKarts(), false);

    int count=vec.size();
    std::vector<std::string> all_karts;
    for(unsigned int i=0; i<vec.size(); i++)
    {
        if(vec[i].empty()) continue;
        int id=getKartId(vec[i]);
        used[id] = true;
        count --;
    }

    // Add karts from the current group
    // --------------------------------
    std::vector<int> karts = getKartsInGroup(user_config->m_kart_group);
    std::vector<int>::iterator k;
    // Remove karts that are already used
    for(unsigned int i=0; i<karts.size();)
    {
        if(used[karts[i]])
            karts.erase(karts.begin()+i);
        else
            i++;
    }
    std::srand((unsigned int)std::time(0));
    std::random_shuffle(karts.begin(), karts.end());

    // Loop over all karts to fill till either all slots are filled, or
    // there are no more karts in the current group
    for(unsigned int i=0; i<vec.size() && count>0 && karts.size()>0; i++)
    {
        if(vec[i].empty())
        {
            used[karts.back()] = true;
            vec[i] = m_karts_properties[karts.back()]->getIdent();
            karts.pop_back();
            count --;
        }
    }
    if(count==0) return;

    // Not enough karts in chosen group, so fill the rest with arbitrary karts
    // -----------------------------------------------------------------------
    // First create an index list with all unused karts.
    karts.clear();
    for(unsigned int i=0; i<getNumberOfKarts(); i++)
    {
        if(!used[i]) karts.push_back(i);
    }
    std::random_shuffle(karts.begin(), karts.end());
    // Then fill up the remaining empty spaces
    for(unsigned int i=0; i<vec.size() && count>0 && karts.size()>0; i++)
    {
        if(vec[i].empty())
        {
            vec[i] = m_karts_properties[karts.back()]->getIdent();
            karts.pop_back();
            count --;
        }
    }
    // There should never be more karts to be selected than there are.
    assert(count==0);
}   // fillWithRandomKarts    

/* EOF */
