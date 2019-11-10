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

#include "tips/tip_set.hpp"

#include "io/xml_node.hpp"
#include "online/link_helper.hpp"
#include "utils/log.hpp"
#include "utils/random_generator.hpp"

#include <sstream>

/** The constructor reads the dat from the xml information.
 *  \param input XML node for this achievement info.
 */
TipSet::TipSet(const XMLNode * input)
{
    m_id               = "";
    m_type             = TIPSET_NOTYPE;
    m_is_important     = false;
    
    if (!input->get("id", &m_id))
    {
        Log::error("TipSet",
                   "Undefined id for tipset.");
    }

    std::string buffer;
    input->get("type", &buffer);
    if(buffer == "queue")
        m_type = TIPSET_QUEUE;
    else if(buffer == "random")
        m_type = TIPSET_RANDOM;
    else
        Log::warn("TipSet",
                  "Unknown type for tipset \"%s\".", m_id);
    
    if(m_type == TIPSET_QUEUE)
        m_progress = -1; // Initialize progress when it's a queued tip
    
    input->get("important", &m_is_important);

    parseTips(input, m_tipset);
}   // AchievementInfo

// ----------------------------------------------------------------------------
/** Parses recursively the list of goals, to construct the tree of goals */
void TipSet::parseTips(const XMLNode * input, std::vector<tip> &parent)
{
    // Now load the goal nodes
    for (unsigned int n = 0; n < input->getNumNodes(); n++)
    {
        const XMLNode *node = input->getNode(n);
        if (node->getName() != "tip")
            continue; // ignore incorrect node

        std::string text;
        if(!node->get("text", &text))
            continue; // missing text, ignore node

        std::string icon_path;
        if (!node->get("icon", &icon_path))
            icon_path = "gui/icons/main_help.png";

        std::string goto_type;
        if (!node->get("goto", &goto_type))
            goto_type = "no";

        std::string goto_address;
        if (!node->get("address", &goto_address))
            goto_address = "";

        tip child;
        child.text = text;
        child.icon_path = icon_path;
        if (goto_type == "no")
            child.goto_type = GOTO_NO;
        else if (goto_type == "screen")
            child.goto_type = GOTO_SCREEN;
        else if (goto_type == "website")
            child.goto_type = GOTO_WEBSITE;
        else
        {
            Log::warn("TipSet",
                      "Unknown goto type \"%s\".", goto_type);
            child.goto_type = GOTO_NO;
        }

        if(child.goto_type != GOTO_NO)
            child.goto_address = goto_address;

        parent.push_back(child);
    }
    if (parent.size() != input->getNumNodes())
        Log::error("TipSet",
                  "Incorrect tips for the entries of tipset \"%s\".", m_id.c_str());
} // parseGoals

// ----------------------------------------------------------------------------
/** Run a tip's goto */
void TipSet::tip::runGoto()
{
    if(goto_type == GOTO_NO)
    {
        return;
    }
    else if(goto_type == GOTO_SCREEN)
    {
        Log::warn("TipSet",
                  "Goto screen is WIP!");
        return;
    }
    else if(goto_type == GOTO_WEBSITE)
    {
        Online::LinkHelper::openURL(goto_address);
    }
}

// ----------------------------------------------------------------------------
/** Get a tip depend on the tipset's type */
TipSet::tip TipSet::getTip()
{
    if(m_type == TIPSET_NOTYPE)
    {
        Log::error("TipSet",
                   "Unknown type for tipset \"%s\".", m_id);
    }
    else if(m_type == TIPSET_QUEUE)
    {
        ++ m_progress;
        return m_tipset[m_progress];
    }
    else if(m_type == TIPSET_RANDOM)
    {
        RandomGenerator randgen;
        return m_tipset[randgen.get(m_tipset.size())];
    }
}
