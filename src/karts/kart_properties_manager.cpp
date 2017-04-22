//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2004-2015 Ingo Ruhnke <grumbel@gmx.de>
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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

#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/player_profile.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/engine.hpp"
#include "io/file_manager.hpp"
#include "karts/kart_properties.hpp"
#include "karts/xml_characteristic.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include <algorithm>
#include <ctime>
#include <stdio.h>
#include <stdexcept>
#include <iostream>

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
/** Removes all karts from the KartPropertiesManager, so that they can be
 *  reloade. This is necessary after a change of the screen resolution.
 */
void KartPropertiesManager::unloadAllKarts()
{
    m_karts_properties.clearAndDeleteAll();
    m_selected_karts.clear();
    m_kart_available.clear();
    m_groups_2_indices.clear();
    m_all_groups.clear();
}   // unloadAllKarts

//-----------------------------------------------------------------------------
/** Remove a kart from the kart manager.
 *  \param id The kart id (i.e. name of the directory) to remove.
 */
void KartPropertiesManager::removeKart(const std::string &ident)
{
    // Remove the kart properties from the vector of all kart properties
    int index = getKartId(ident);
    const KartProperties *kp = getKart(ident);  // must be done before remove
    m_karts_properties.remove(index);
    m_all_kart_dirs.erase(m_all_kart_dirs.begin()+index);
    m_kart_available.erase(m_kart_available.begin()+index);

    // Remove the just removed kart from the 'group-name to kart property
    // index' mapping. If a group is now empty (i.e. the removed kart was
    // the only member of this group), remove the group
    const std::vector<std::string> &groups = kp->getGroups();

    for (unsigned int i=0; i<groups.size(); i++)
    {
        std::vector<int> ::iterator it;
        it = std::find(m_groups_2_indices[groups[i]].begin(),
                       m_groups_2_indices[groups[i]].end(),   index);
        // Since we are iterating over all groups the kart belongs to,
        // there must be an entry found
        assert(it!=m_groups_2_indices[groups[i]].end());

        m_groups_2_indices[groups[i]].erase(it);

        // Check if the last kart of a group was removed
        if(m_groups_2_indices[groups[i]].size()==0)
        {
            m_groups_2_indices.erase(groups[i]);
            std::vector<std::string>::iterator its;
            its = std::find(m_all_groups.begin(), m_all_groups.end(),
                            groups[i]);
            assert(its!=m_all_groups.end());
            m_all_groups.erase(its);
        }   // if m_groups_2_indices[groups[i]].size()==0)
    }   // for i in all groups the kart belongs to


    // Adjust the indices of all kart properties in the 'group name to
    // kart property index' mapping: all kart properties with an index
    // greater than index were moved one position further to the beginning
    std::map<std::string, std::vector<int> >::iterator it_gr;
    for(it_gr=m_groups_2_indices.begin(); it_gr != m_groups_2_indices.end();
        it_gr++)
    {
        for(unsigned int i=0; i<(*it_gr).second.size(); i++)
        {
            if( (*it_gr).second[i]>index)
                (*it_gr).second[i]--;
        }
    }

    delete kp;

    // Only used for networking and it is safe to just clear it.
    // If a networking game is started it will be initialised properly.
    m_selected_karts.clear();
}   // removeKart

//-----------------------------------------------------------------------------
/** Loads all kart properties and models.
 */
void KartPropertiesManager::loadAllKarts(bool loading_icon)
{
    m_all_kart_dirs.clear();
    std::vector<std::string>::const_iterator dir;
    for(dir = m_kart_search_path.begin(); dir!=m_kart_search_path.end(); dir++)
    {
        // First check if there is a kart in the current directory
        // -------------------------------------------------------
        if(loadKart(*dir)) continue;

        // If not, check each subdir of this directory.
        // --------------------------------------------
        std::set<std::string> result;
        file_manager->listFiles(result, *dir);
        for(std::set<std::string>::const_iterator subdir=result.begin();
            subdir!=result.end(); subdir++)
        {
            const bool loaded = loadKart(*dir+*subdir);

            if (loaded && loading_icon)
            {
                GUIEngine::addLoadingIcon(irr_driver->getTexture(
                    m_karts_properties[m_karts_properties.size()-1]
                            .getAbsoluteIconFile()              )
                                          );
            }
        }   // for all files in the currently handled directory
    }   // for i
}   // loadAllKarts

//-----------------------------------------------------------------------------
/** Loads the characteristics from the characteristics config file.
 *  \param root The xml node where the characteristics are stored.
 */
void KartPropertiesManager::loadCharacteristics(const XMLNode *root)
{
    // Load base characteristics
    std::vector<XMLNode*> nodes;
    root->getNodes("characteristic", nodes);
    bool found = false;
    std::string name;
    for (const XMLNode *baseNode : nodes)
    {
        baseNode->get("name", &name);
        if (name == "base")
        {
            found = true;
            m_base_characteristic.reset(new XmlCharacteristic(baseNode));
            break;
        }
    }
    if (!found)
        Log::fatal("KartPropertiesManager", "Base characteristics not found");

    // Load difficulties
    nodes.clear();
    root->getNode("difficulties")->getNodes("characteristic", nodes);
    for (const XMLNode *type : nodes)
    {
        type->get("name", &name);
        m_difficulty_characteristics.insert(std::pair<const std::string,
            std::unique_ptr<AbstractCharacteristic> >(name,
            std::unique_ptr<AbstractCharacteristic>(new XmlCharacteristic(type))));
    }
    // Load kart type characteristics
    nodes.clear();
    root->getNode("kart-types")->getNodes("characteristic", nodes);
    for (const XMLNode *type : nodes)
    {
        type->get("name", &name);
        m_kart_type_characteristics.insert(std::pair<const std::string,
            std::unique_ptr<AbstractCharacteristic> >(name,
            std::unique_ptr<AbstractCharacteristic>(new XmlCharacteristic(type))));
    }
    // Load player difficulties
    nodes.clear();
    root->getNode("player-characteristics")->getNodes("characteristic", nodes);
    for (const XMLNode *type : nodes)
    {
        type->get("name", &name);
        m_player_characteristics.insert(std::pair<const std::string,
            std::unique_ptr<AbstractCharacteristic> >(name,
            std::unique_ptr<AbstractCharacteristic>(new XmlCharacteristic(type))));
    }
}

//-----------------------------------------------------------------------------
/** Loads a single kart and (if not disabled) the corresponding 3d model.
 *  \param filename Full path to the kart config file.
 */
bool KartPropertiesManager::loadKart(const std::string &dir)
{
    std::string config_filename = dir + "/kart.xml";
    if(!file_manager->fileExists(config_filename))
        return false;

    KartProperties* kart_properties;
    try
    {
        kart_properties = new KartProperties(config_filename);
    }
    catch (std::runtime_error& err)
    {
        Log::error("[KartPropertiesManager]", "Giving up loading '%s': %s",
                    config_filename.c_str(), err.what());
        return false;
    }

    // If the version of the kart file is not supported,
    // ignore this .kart file
    if (kart_properties->getVersion() < stk_config->m_min_kart_version ||
        kart_properties->getVersion() > stk_config->m_max_kart_version)
    {
        Log::warn("[KartPropertiesManager]", "Warning: kart '%s' is not "
                  "supported by this binary, ignored.",
                  kart_properties->getIdent().c_str());
        delete kart_properties;
        return false;
    }

    m_karts_properties.push_back(kart_properties);
    m_kart_available.push_back(true);
    const std::vector<std::string>& groups=kart_properties->getGroups();
    for(unsigned int g=0; g<groups.size(); g++)
    {
        if(m_groups_2_indices.find(groups[g])==m_groups_2_indices.end())
        {
            m_all_groups.push_back(groups[g]);
        }
        m_groups_2_indices[groups[g]].push_back(m_karts_properties.size()-1);
    }
    m_all_kart_dirs.push_back(dir);
    return true;
}   // loadKart

//-----------------------------------------------------------------------------
/** Sets the name of a mesh to use as a hat for all karts.
 *  \param hat_name Name of the hat mash.
  */
void KartPropertiesManager::setHatMeshName(const std::string &hat_name)
{
    for (unsigned int i=0; i<m_karts_properties.size(); i++)
    {
        m_karts_properties[i].setHatMeshName(hat_name);
    }
}   // setHatMeshName

//-----------------------------------------------------------------------------
const AbstractCharacteristic* KartPropertiesManager::getDifficultyCharacteristic(const std::string &type) const
{
    std::map<std::string, std::unique_ptr<AbstractCharacteristic> >::const_iterator
        it = m_difficulty_characteristics.find(type);
    if (it == m_difficulty_characteristics.cend())
        return nullptr;
    return it->second.get();
}   // getDifficultyCharacteristic

//-----------------------------------------------------------------------------
const AbstractCharacteristic* KartPropertiesManager::getKartTypeCharacteristic(const std::string &type) const
{
    std::map<std::string, std::unique_ptr<AbstractCharacteristic> >::const_iterator
        it = m_kart_type_characteristics.find(type);
    if (it == m_kart_type_characteristics.cend())
        return nullptr;
    return it->second.get();
}   // getKartTypeCharacteristic

//-----------------------------------------------------------------------------
const AbstractCharacteristic* KartPropertiesManager::getPlayerCharacteristic(const std::string &type) const
{
    std::map<std::string, std::unique_ptr<AbstractCharacteristic> >::const_iterator
        it = m_player_characteristics.find(type);
    if (it == m_player_characteristics.cend())
        return nullptr;
    return it->second.get();
}   // getPlayerCharacteristic

//-----------------------------------------------------------------------------
/** Returns index of the kart properties with the given ident.
 *  \return Index of kart (between 0 and number of karts - 1).
 */
const int KartPropertiesManager::getKartId(const std::string &ident) const
{
    for (unsigned int i=0; i<m_karts_properties.size(); i++)
    {
        if (m_karts_properties[i].getIdent() == ident)
            return i;
    }

    std::ostringstream msg;
    msg << "KartPropertiesManager: Couldn't find kart: '" << ident << "'";
    throw std::runtime_error(msg.str());
}   // getKartId

//-----------------------------------------------------------------------------
const KartProperties* KartPropertiesManager::getKart(
                                                const std::string &ident) const
{
    for (const KartProperties* kp : m_karts_properties)
    {
        if (kp->getIdent() == ident)
            return kp;
    }

    return NULL;
}   // getKart

//-----------------------------------------------------------------------------
const KartProperties* KartPropertiesManager::getKartById(int i) const
{
    if (i < 0 || i >= int(m_karts_properties.size()))
        return NULL;

    return m_karts_properties.get(i);
}   // getKartById

//-----------------------------------------------------------------------------
/** Returns a list of all available kart identifiers.                        */
std::vector<std::string> KartPropertiesManager::getAllAvailableKarts() const
{
    std::vector<std::string> all;
    for (unsigned int i=0; i<m_karts_properties.size(); i++)
    {
        if (m_kart_available[i])
            all.push_back(m_karts_properties[i].getIdent());
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
    for (unsigned int i=0; i<m_karts_properties.size(); i++)
    {
        if (!m_kart_available[i]) continue;

        if (std::find(karts.begin(), karts.end(),
                      m_karts_properties[i].getIdent())
            == karts.end())
        {
            m_kart_available[i] = false;

            Log::error("[Kart_Properties_Manager]",
                       "Kart '%s' not available on all clients, disabled.",
                       m_karts_properties[i].getIdent().c_str());
        }   // kart not in list
    }   // for i in m_kart_properties

}   // setUnavailableKarts
//-----------------------------------------------------------------------------
/** Returns the (global) index of the n-th kart of a given group. If there is
  * no such kart, -1 is returned.
  */
int KartPropertiesManager::getKartByGroup(const std::string& group,
                                          int n) const
{
    int count=0;
    for (unsigned int i=0; i<m_karts_properties.size(); i++)
    {
        std::vector<std::string> groups = m_karts_properties[i].getGroups();
        if (std::find(groups.begin(), groups.end(), group) == groups.end())
            continue;
        if (count == n) return i;
        count = count + 1;
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
    const KartProperties *kartprop = getKartById(kartid);
    if( PlayerManager::getCurrentPlayer()->isLocked(kartprop->getIdent()) )
        return false;
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
/** Returns a vector with the indices of all karts in the specified group.
 *  \param g The name of the group for which the kart indicies should be
 *           determined
 *  \return A vector of indices with the karts in the given group.
 */
const std::vector<int> KartPropertiesManager::getKartsInGroup(
                                                          const std::string& g)
{
    if (g == ALL_KART_GROUPS_ID)
    {
        std::vector<int> out;
        for (unsigned int n=0; n<m_karts_properties.size(); n++)
        {
            out.push_back(n);
        }
        return out;
    }
    else
    {
        return m_groups_2_indices[g];
    }
}   // getKartsInGroup

//-----------------------------------------------------------------------------
/** Returns a list of randomly selected karts. This list firstly contains
 *  karts in the currently selected group, but which are not in the list
 *  of 'existing karts'. If not enough karts are available in the current
 *  group, karts from all other groups are used to fill up the list.
 *  This is used by the race manager to select the AI karts.
 *  \param count          Number of karts to select randomly.
 *  \param existing_karts List of karts that should not be used. This is the
 *                        list of karts selected by the players.
 *  \param ai_list        List of AI karts already selected (eg through the
 *                        command line). The random AIs will also be added
 *                        to this list.
 */
void KartPropertiesManager::getRandomKartList(int count,
                                            RemoteKartInfoList* existing_karts,
                                            std::vector<std::string> *ai_list)
{
    // First: set up flags (based on global kart
    // index) for which karts are already used
    // -----------------------------------------
    std::vector<bool> used;
    used.resize(getNumberOfKarts(), false);

    std::vector<std::string> random_kart_queue;
    if (existing_karts != NULL)
    {
        for (unsigned int i=0; i<existing_karts->size(); i++)
        {
            try
            {
                int id = getKartId((*existing_karts)[i].getKartName());
                used[id] = true;
            }
            catch (std::runtime_error& ex)
            {
                (void)ex;
                Log::error("[KartPropertiesManager]", "getRandomKartList : "
                    "WARNING, can't find kart '%s'",
                    (*existing_karts)[i].getKartName().c_str());
            }
        }
    }
    for(unsigned int i=0; i<ai_list->size(); i++)
    {
        try
        {
            int id=getKartId((*ai_list)[i]);
            used[id] = true;
        }
        catch (std::runtime_error &ex)
        {
            (void)ex;
            Log::error("[KartPropertiesManager]", "getRandomKartList : WARNING, "
                "can't find kart '%s'",(*ai_list)[i].c_str());
        }
    }

    do
    {
        // if we have no karts left in our queue, re-fill it
        if (count > 0 && random_kart_queue.size() == 0)
        {
            random_kart_queue.clear();
            std::vector<int> karts_in_group =
                getKartsInGroup(UserConfigParams::m_last_used_kart_group);

            assert(karts_in_group.size() > 0);

            // first try not to use a kart already used by a player
            for (unsigned int i=0; i<karts_in_group.size(); i++)
            {
                const KartProperties &kp=m_karts_properties[karts_in_group[i]];
                if (!used[karts_in_group[i]]                 &&
                    m_kart_available[karts_in_group[i]]      &&
                    !PlayerManager::getCurrentPlayer()->isLocked(kp.getIdent())   )
                {
                    random_kart_queue.push_back(kp.getIdent());
                }
            }

            // if we really need to, reuse the same kart as the player
            if (random_kart_queue.size() == 0)
            {
                for (unsigned int i=0; i<karts_in_group.size(); i++)
                {
                    const KartProperties &kp =
                        m_karts_properties[karts_in_group[i]];
                    random_kart_queue.push_back(kp.getIdent());
                }
            }

            assert(random_kart_queue.size() > 0);

            std::random_shuffle(random_kart_queue.begin(),
                                random_kart_queue.end()   );
        }

        while (count > 0 && random_kart_queue.size() > 0)
        {
            ai_list->push_back(random_kart_queue.back());
            random_kart_queue.pop_back();
            count --;
        }

    } while (count > 0);

    // There should always be enough karts
    assert(count==0);
}   // getRandomKartList

/* EOF */
