//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2019 dumaosen
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

#include "tips/tips_manager.hpp"

#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "utils/log.hpp"

TipsManager* TipsManager::m_tips_manager = NULL;

// ----------------------------------------------------------------------------
/** Constructor, which reads data/tips.xml and stores the information
 *  in objects.
 */
TipsManager::TipsManager()
{
    const std::string file_name = file_manager->getAsset("tips.xml");
    const XMLNode *root = file_manager->createXMLTree(file_name);
    unsigned int num_nodes = root->getNumNodes();
    for (unsigned int i = 0; i < num_nodes; i++)
    {
        const XMLNode *node = root->getNode(i);
        TipSet * tip_set = new TipSet(node);
        m_all_tip_sets[tip_set->getID()] = tip_set;
    }
    if (num_nodes != m_all_tip_sets.size())
        Log::error("TipsManager",
                   "Multiple tipsets with the same id!");

    delete root;
}   // TipsManager

// ----------------------------------------------------------------------------
TipsManager::~TipsManager()
{
    std::map<std::string, TipSet*>::iterator it;
    for ( it = m_all_tip_sets.begin(); it != m_all_tip_sets.end(); ++it ) {
        delete it->second;
    }
    m_all_tip_sets.clear();
}   // ~TipsManager

// ----------------------------------------------------------------------------
TipSet* TipsManager::getTipSet(std::string id) const
{
    std::map<std::string, TipSet*>::const_iterator it =
        m_all_tip_sets.find(id);
    if (it != m_all_tip_sets.end())
        return it->second;
    return NULL;
}
