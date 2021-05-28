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

#ifndef SERVER_ONLY

#include "tips/tips_manager.hpp"

#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "utils/log.hpp"
#include "utils/random_generator.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

TipsManager* TipsManager::m_tips_manager = NULL;

// ----------------------------------------------------------------------------
/** Constructor, which reads data/tips.xml and stores the information
 *  in objects.
 */
TipsManager::TipsManager()
{
    const std::string file_name = file_manager->getAsset("tips.xml");
    if (file_name.empty())
        return;
    const XMLNode *root = file_manager->createXMLTree(file_name);
    if (!root)
        return;
    unsigned int num_nodes = root->getNumNodes();

    for (unsigned int i = 0; i < num_nodes; i++)
    {
        const XMLNode *node = root->getNode(i);
        addTipSet(node);
    }

    if (num_nodes != m_all_tip_sets.size())
    {
        Log::error("TipsManager",
                   "Multiple tipsets with the same id!");
    }

    delete root;
} // TipsManager

// ----------------------------------------------------------------------------
void TipsManager::addTipSet(const XMLNode *input)
{
    std::string id;

    if (!input->get("id", &id))
    {
        Log::error("TipSet",
                   "Undefined id for tipset.");
    }

    for (unsigned int n = 0; n < input->getNumNodes(); n++)
    {
        const XMLNode *node = input->getNode(n);
        if (node->getName() != "tip")
            continue; // ignore incorrect node

        std::string text;
        if(!node->get("text", &text))
            continue; // missing text, ignore node

        // Gettext is used here
        m_all_tip_sets[id].push_back(_(text.c_str()));
    }
    if (m_all_tip_sets[id].size() != input->getNumNodes())
    {
        Log::error("TipSet",
            "Incorrect tips for the entries of tipset \"%s\".", id.c_str());
    }
}

// ----------------------------------------------------------------------------
const irr::core::stringw& TipsManager::getTip(const std::string& id) const
{
    auto ret = m_all_tip_sets.find(id);
    if (ret == m_all_tip_sets.end())
    {
        // Should not happen
        static core::stringw empty;
        return empty;
    }
    RandomGenerator randgen;
    unsigned pos = randgen.get(ret->second.size());
    return ret->second.at(pos);
} // getTipSet

#endif
