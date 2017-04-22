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

#ifndef HEADER_KART_PROPERTIES_MANAGER_HPP
#define HEADER_KART_PROPERTIES_MANAGER_HPP

#include "utils/ptr_vector.hpp"
#include <map>
#include <memory>

#include "network/remote_kart_info.hpp"
#include "utils/no_copy.hpp"

#define ALL_KART_GROUPS_ID  "all"

class AbstractCharacteristic;
class KartProperties;
class XMLNode;

/**
  * \ingroup karts
  */
class KartPropertiesManager: public NoCopy
{
private:
    /** The list of all directories in which to search for karts. */
    static std::vector<std::string>          m_kart_search_path;

    /** All directories from which karts were loaded. Needed by unlock_manager
     *  to load all challenges. */
    std::vector<std::string>                 m_all_kart_dirs;

    /** List of all kart groups. */
    std::vector<std::string>                 m_all_groups;

    /** Mapping of group names to list of kart indices in each group. */
    std::map<std::string, std::vector<int> > m_groups_2_indices;

    /** Vector containing kart numbers that have been selected in multiplayer
     * games.  This it used to ensure the same kart can not be selected more
     * than once. */
    std::vector<int>         m_selected_karts;

    /** Contains a flag for each kart indicating wether it is available on
     *  all clients or not. */
    std::vector<bool>        m_kart_available;

    std::unique_ptr<AbstractCharacteristic>                         m_base_characteristic;
    std::map<std::string, std::unique_ptr<AbstractCharacteristic> > m_difficulty_characteristics;
    std::map<std::string, std::unique_ptr<AbstractCharacteristic> > m_kart_type_characteristics;
    std::map<std::string, std::unique_ptr<AbstractCharacteristic> > m_player_characteristics;

protected:

    typedef PtrVector<KartProperties> KartPropertiesVector;
    /** All available kart configurations */
    KartPropertiesVector m_karts_properties;

public:
                             KartPropertiesManager();
                            ~KartPropertiesManager();
    static void              addKartSearchDir       (const std::string &s);
    const KartProperties*    getKartById            (int i) const;
    const KartProperties*    getKart(const std::string &ident) const;
    const int                getKartId(const std::string &ident) const;
    int                      getKartByGroup(const std::string& group,
                                           int i) const;

    void                     loadCharacteristics    (const XMLNode *root);
    bool                     loadKart               (const std::string &dir);
    void                     loadAllKarts           (bool loading_icon = true);
    void                     unloadAllKarts         ();
    void                     removeKart(const std::string &id);
    const std::vector<int>   getKartsInGroup        (const std::string& g);
    bool                     kartAvailable(int kartid);
    std::vector<std::string> getAllAvailableKarts() const;
    void                     setUnavailableKarts(std::vector<std::string>);
    void                     selectKartName(const std::string &kart_name);
    bool                     testAndSetKart(int kartid);
    void                     getRandomKartList(int count,
                                           RemoteKartInfoList* existing_karts,
                                           std::vector<std::string> *ai_list);
    void                     setHatMeshName(const std::string &hat_name);
    // ------------------------------------------------------------------------
    /** Get the characteristic that holds the base values. */
    const AbstractCharacteristic* getBaseCharacteristic() const { return m_base_characteristic.get(); }
    // ------------------------------------------------------------------------
    /** Get a characteristic that holds the values for a certain difficulty. */
    const AbstractCharacteristic* getDifficultyCharacteristic(const std::string &type) const;
    // ------------------------------------------------------------------------
    /** Get a characteristic that holds the values for a kart type. */
    const AbstractCharacteristic* getKartTypeCharacteristic(const std::string &type) const;
    // ------------------------------------------------------------------------
    /** Get a characteristic that holds the values for a player difficulty. */
    const AbstractCharacteristic* getPlayerCharacteristic(const std::string &type) const;
    // ------------------------------------------------------------------------
    /** Returns a list of all groups. */
    const std::vector<std::string>& getAllGroups() const {return m_all_groups;}
    // ------------------------------------------------------------------------
    /** Clears all selected karts (used in networking only). */
    void clearAllSelectedKarts() { m_selected_karts.clear(); }
    // ------------------------------------------------------------------------
    /** Removed the last selected kart (used in networking only). */
    void removeLastSelectedKart() { m_selected_karts.pop_back(); }
    // ------------------------------------------------------------------------
    /** Returns the number of selected karts (used in networking only). */
    int getNumSelectedKarts() const { return (int) m_selected_karts.size(); }
    // ------------------------------------------------------------------------
    /** Sets a kartid to be selected (used in networking only). */
    void selectKart(int kartid) { m_selected_karts.push_back(kartid); }
    // ------------------------------------------------------------------------
    /** Returns all directories from which karts were loaded. */
    const std::vector<std::string>* getAllKartDirs() const
                                    { return &m_all_kart_dirs; }
    // ------------------------------------------------------------------------
    /** Returns the number of karts. */
    const unsigned int getNumberOfKarts() const {
        return (unsigned int)m_karts_properties.size();
    }   // getNumberOfKarts
};

extern KartPropertiesManager *kart_properties_manager;

#endif

/* EOF */
