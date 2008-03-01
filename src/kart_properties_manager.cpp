//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2006 Ingo Ruhnke <grumbel@gmx.de>
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
#include <algorithm>
#include <ctime>
#include "file_manager.hpp"
#include "string_utils.hpp"
#include "kart_properties_manager.hpp"
#include "kart_properties.hpp"
#include "translation.hpp"
#if defined(WIN32) && !defined(__CYGWIN__)
#  define snprintf _snprintf
#endif

KartPropertiesManager *kart_properties_manager=0;

KartPropertiesManager::KartPropertiesManager()
{}

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
    file_manager->listFiles(result, "data");

    // Find out which characters are available and load them
    for(std::set<std::string>::iterator i = result.begin();
            i != result.end(); ++i)
        {
            if (StringUtils::has_suffix(*i, ".tkkf"))
            {
                KartProperties* kp = new KartProperties();
                std::string filename=file_manager->getKartFile(*i);
                kp->load(filename.c_str(), "tuxkart-kart", dont_load_models);
                m_karts_properties.push_back(kp);
                if(kp->getMaxSteerAngle() > m_max_steer_angle)
                {
                    m_max_steer_angle = kp->getMaxSteerAngle();
                }
            }   // if
        }   // for i
}   // loadKartData

//-----------------------------------------------------------------------------
const int KartPropertiesManager::getKartId(const std::string IDENT)
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
    snprintf(msg, sizeof(msg), _("KartPropertiesManager: Couldn't find kart: '%s'"),
             IDENT.c_str());
    throw std::runtime_error(msg);
}   // getKartId

//-----------------------------------------------------------------------------
const KartProperties* KartPropertiesManager::getKart(const std::string IDENT)
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
const KartProperties* KartPropertiesManager::getKartById(int i)
{
    if (i < 0 || i >= int(m_karts_properties.size()))
        return NULL;

    return m_karts_properties[i];
}

/*FIXME: the next function is unused, if it is not useful, it should be
  deleted.*/
//-----------------------------------------------------------------------------
std::vector<std::string> KartPropertiesManager::getRandomKarts(int len)
{
    std::vector<std::string> all_karts;

    for(KartPropertiesVector::const_iterator i  = m_karts_properties.begin();
        i != m_karts_properties.end(); ++i)
    {
        all_karts.push_back((*i)->getIdent());
    }

    std::random_shuffle(all_karts.begin(), all_karts.end());

    all_karts.resize(len);

    return all_karts;
}   // getRandomKart

//-----------------------------------------------------------------------------
void KartPropertiesManager::fillWithRandomKarts(std::vector<std::string>& vec)
{
    std::vector<std::string> all_karts;

    for(KartPropertiesVector::const_iterator i = m_karts_properties.begin();
        i != m_karts_properties.end(); ++i)
        all_karts.push_back((*i)->getIdent());

    std::srand((unsigned int)std::time(0));

    std::random_shuffle(all_karts.begin(), all_karts.end());

    int new_kart = 0;
    for(int i = 0; i < int(vec.size()); ++i)
    {
        while(vec[i].empty())
        {
            if (std::find(vec.begin(), vec.end(), all_karts[new_kart]) == vec.end())
            { // Found a new kart, so use it
                vec[i] = all_karts[new_kart];
            }
            else if (!(all_karts.size() >= vec.size()))
            { // We need to fill more karts than we have available, so don't care about dups
                vec[i] = all_karts[new_kart];
            }

            new_kart += 1;
        }   // while
    }   // for i
}

/* EOF */
