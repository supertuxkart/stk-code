//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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


#include "config/saved_grand_prix.hpp"

#include "io/xml_node.hpp"
#include "karts/kart_properties_manager.hpp"
#include "utils/ptr_vector.hpp"
#include "utils/string_utils.hpp"

#include <sstream>
#include <stdlib.h>


//------------------------------------------------------------------------------
SavedGrandPrix::SavedGPKart::SavedGPKart(GroupUserConfigParam * group,
                         const std::string &ident,
                         int score,
                         int local_player_id,
                         int global_player_id,
                         float overall_time)
           : m_group("Kart", group, "Saved state of a kart") ,
             m_ident(ident.c_str(),"ident", &m_group),
             m_score(score,"score",&m_group),
             m_local_player_id(local_player_id,"local_player_id",&m_group),
             m_global_player_id(global_player_id,"global_player_id",&m_group),
             m_overall_time(overall_time,"overall_time",&m_group)
{

}   // SavedGPKart

//-----------------------------------------------------------------------------
SavedGrandPrix::SavedGPKart::SavedGPKart(GroupUserConfigParam * group,
                                         const XMLNode* node)
              : m_group("Kart", group, "Saved state of a kart"),
                m_ident("-","ident", &m_group),
                m_score(0,"score",&m_group),
                m_local_player_id(0,"local_player_id",&m_group),
                m_global_player_id(0,"global_player_id",&m_group),
                m_overall_time(0.0f,"overall_time",&m_group)
{
    m_ident.findYourDataInAnAttributeOf(node);
    m_score.findYourDataInAnAttributeOf(node);
    m_local_player_id.findYourDataInAnAttributeOf(node);
    m_global_player_id.findYourDataInAnAttributeOf(node);
    m_overall_time.findYourDataInAnAttributeOf(node);
}   // SavedGPKart

// ============================================================================
SavedGrandPrix::SavedGrandPrix(unsigned int player_id,
                               const std::string &gp_id,
                               RaceManager::MinorRaceModeType race_type,
                               RaceManager::Difficulty difficulty,
                               int player_karts,
                               int last_track,
                               int reverse_type,
                               const std::vector<RaceManager::KartStatus> &kart_list)
              : m_savedgp_group("SavedGP",
                                "Represents the saved state of a GP"),
                m_player_id(player_id, "player_id", &m_savedgp_group),
                m_gp_id(gp_id.c_str(), "gp_id", &m_savedgp_group),
                m_race_type((int)race_type,"race_type", &m_savedgp_group),
                m_difficulty((int)difficulty,"difficulty", &m_savedgp_group),
                m_player_karts(player_karts,"player_karts", &m_savedgp_group),
                m_next_track(last_track,"last_track", &m_savedgp_group),
                m_reverse_type(reverse_type,"reverse_type", &m_savedgp_group)
{
    for(unsigned int i =0; i < kart_list.size(); i++)
    {
        SavedGPKart * newKart = new SavedGPKart(&m_savedgp_group,
                                                kart_list[i].m_ident,
                                                kart_list[i].m_score,
                                                kart_list[i].m_local_player_id,
                                                kart_list[i].m_global_player_id,
                                                kart_list[i].m_overall_time
                                                );
        m_karts.push_back(newKart);
    }
}   // SavedGrandPrix

//------------------------------------------------------------------------------
SavedGrandPrix::SavedGrandPrix(const XMLNode* node)
              : m_savedgp_group("SavedGP",
                                "Represents the saved state of a GP"),
                m_player_id   (0, "player_id",   &m_savedgp_group),
                m_gp_id     ("-", "gp_id",       &m_savedgp_group),
                m_race_type  (0,"race_type",     &m_savedgp_group),
                m_difficulty  (0,"difficulty",   &m_savedgp_group),
                m_player_karts(0,"player_karts", &m_savedgp_group),
                m_next_track  (0,"last_track",   &m_savedgp_group),
                m_reverse_type(0,"reverse_type", &m_savedgp_group)
{
    //m_player_group.findYourDataInAChildOf(node);
    m_player_id.   findYourDataInAnAttributeOf(node);
    m_gp_id.       findYourDataInAnAttributeOf(node);
    m_race_type.   findYourDataInAnAttributeOf(node);
    m_difficulty.  findYourDataInAnAttributeOf(node);
    m_player_karts.findYourDataInAnAttributeOf(node);
    m_next_track.  findYourDataInAnAttributeOf(node);
    m_reverse_type.findYourDataInAnAttributeOf(node);

    std::vector<XMLNode*> karts;
    node->getNodes("Kart", karts);
    for(unsigned int i =0; i < karts.size(); i++)
    {
        SavedGPKart * newKart = new SavedGPKart(&m_savedgp_group,karts[i]);
        m_karts.push_back(newKart);
    }
}   // SavedGrandPrix

//------------------------------------------------------------------------------
void SavedGrandPrix::clearKarts()
{
    m_savedgp_group.clearChildren();
    m_karts.clearAndDeleteAll();
}   // clearKarts

//------------------------------------------------------------------------------
void SavedGrandPrix::setKarts(const std::vector<RaceManager::KartStatus> &kart_list)
{
    clearKarts();
    for(unsigned int i =0; i < kart_list.size(); i++)
    {
        SavedGPKart * newKart = new SavedGPKart(&m_savedgp_group,
                                                kart_list[i].m_ident,
                                                kart_list[i].m_score,
                                                kart_list[i].m_local_player_id,
                                                kart_list[i].m_global_player_id,
                                                kart_list[i].m_overall_time
                                                );
        m_karts.push_back(newKart);
    }
}   // setKarts

/* compares two KartStatus-objects for std::sort in the next function */
bool cmp__l(RaceManager::KartStatus first, RaceManager::KartStatus second)
{
    return (first.m_score > second.m_score);
}
//------------------------------------------------------------------------------
void SavedGrandPrix::loadKarts(std::vector<RaceManager::KartStatus> & kart_list)
{

    int aikarts = 0;
    for(unsigned int i = 0; i < m_karts.size(); i++)
    {
        const KartProperties *kp = kart_properties_manager->getKart(m_karts[i].m_ident);

        if(m_karts[i].m_local_player_id == -1) // AI kart
        {
            if(kp) kart_list[aikarts].m_ident = m_karts[i].m_ident;
            kart_list[aikarts].m_score        = m_karts[i].m_score;
            kart_list[aikarts].m_overall_time = m_karts[i].m_overall_time;
            aikarts++;
        }
        else
        {
            // Get correct player
            for(unsigned int x = (unsigned int)kart_list.size()-m_player_karts;
                x < kart_list.size(); x++)
            {
                if(kart_list[x].m_local_player_id == m_karts[i].m_local_player_id)
                {
                    if(kp) kart_list[x].m_ident        = m_karts[i].m_ident;
                    kart_list[x].m_score        = m_karts[i].m_score;
                    kart_list[x].m_overall_time = m_karts[i].m_overall_time;
                }   // if kart_list[x].m_local_player_id == m_karts[i].local
            }   // for x
        }   // if m_local_player_id == -1
    }   // for i

    // The line below causes issues in follow the leader race. The leader
    // is not at first place anymore.
    // std::sort(kart_list.begin(), kart_list.end(), cmp__l);
}   // loadKarts
