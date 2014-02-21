//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Glenn De Jonghe
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


#include "achievements/achievement.hpp"

#include "achievements/achievement_info.hpp"
#include "guiengine/dialog_queue.hpp"
#include "io/utf_writer.hpp"
#include "states_screens/dialogs/notification_dialog.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"


#include <assert.h>
#include <fstream>
#include <sstream>
#include <stdlib.h>


Achievement::Achievement(const AchievementInfo * info)
    :m_achievement_info(info)
{
    m_id = info->getID();
    m_achieved = false;
}   // Achievement

// ----------------------------------------------------------------------------
Achievement::~Achievement()
{
}   // ~Achievement

// ----------------------------------------------------------------------------
/** Loads the value from an XML node.
 *  \param input*/
void Achievement::load(const XMLNode *node)
{
    node->get("id",       &m_id      );
    node->get("achieved", &m_achieved);
}   // load

// ----------------------------------------------------------------------------
/** Saves the achievement status to a file.
 *  \param Output stream.
 */
void Achievement::save(UTFWriter &out)
{
    out << L"        <achievement id=\"" << m_id << L"\" "
        << L"achieved=\"" << m_achieved << "\"";
}   // save

// ----------------------------------------------------------------------------
void Achievement::onRaceEnd()
{
    if(m_achievement_info->needsResetAfterRace())
        this->reset();
}

// ----------------------------------------------------------------------------
void Achievement::check()
{
    if(m_achieved)
        return;

    if(m_achievement_info->checkCompletion(this))
    {
        //show achievement
        GUIEngine::DialogQueue::get()->pushDialog(
            new NotificationDialog(NotificationDialog::T_Achievements,
            irr::core::stringw(_("Completed achievement")) + irr::core::stringw(" \"") + m_achievement_info->getTitle() + irr::core::stringw("\".")
        ));
        //send to server
        Online::CurrentUser::get()->onAchieving(m_id);
        m_achieved = true;
    }
}

// ----------------------------------------------------------------------------
/** Constructor for a SingleAchievement.
 */
SingleAchievement::SingleAchievement(const AchievementInfo * info)
    : Achievement(info)
{
    m_progress = 0;
    m_achieved = false;
}   // SingleAchievement

// ----------------------------------------------------------------------------
/** Loads the state from an xml node. 
 *  \param node The XML data.
 */
void SingleAchievement::load(const XMLNode *node)
{
    Achievement::load(node);
    if (!isAchieved())
        node->get("progress", &m_progress);
}   // load

// ----------------------------------------------------------------------------
/** Save the state to a file. The progress is only saved if the achievement 
 *  has not been achieved yet.
 *  \param out The output file.
 */
void SingleAchievement::save(UTFWriter &out)
{
    Achievement::save(out);
    if (!m_achieved)
    {
        out << L" progress=\"" << m_progress << L"\"";
    }
    out << L"/>\n";
}   // save

// ----------------------------------------------------------------------------
/** Resets the challenge. Calls if necessary (i.e. if a challenge needs to
 *  be fulfilled in a single race).
 */
void SingleAchievement::reset()
{
    m_progress = 0;
}   // reset

// ----------------------------------------------------------------------------
/** Adds a value to this counter.
 *  \param increase Value to add.
 */
void SingleAchievement::increase(int increase)
{
    m_progress += increase;
    check();
}   // increase

// ----------------------------------------------------------------------------
/** Returns a "k/n" string indicating how much of an achievement was achieved.
 */
irr::core::stringw SingleAchievement::getProgressAsString()
{
    // The info class returns the goal value
    return StringUtils::toWString(m_progress) + "/"
          +getInfo()->toString();
}   // getProgressAsString

// ============================================================================

/** Constructor for a MapAchievement.
 */
MapAchievement::MapAchievement(const AchievementInfo * info)
    : Achievement(info)
{
}   // MapAchievement

// ----------------------------------------------------------------------------
/** Loads the status from an XML node.
 *  \param node The XML data to load the status from.
 */
void MapAchievement::load(const XMLNode *node)
{
    Achievement::load(node);
    for (unsigned int i = 0; i < node->getNumNodes(); i++)
    {
        const XMLNode *n = node->getNode(i);
        std::string key = n->getName();
        int value = 0;
        n->get("value", &value);
        m_progress_map[key] = value;
    }
}    // load

// ----------------------------------------------------------------------------
/** Saves the status of this achievement to a file.
 *  \param out The file to write the state to.
 */
void MapAchievement::save(UTFWriter &out)
{
    Achievement::save(out);
    if (isAchieved())
    {
        out << "/>\n";
        return;
    }
    out << ">\n";
    std::map<std::string, int>::iterator i;
    for (i = m_progress_map.begin(); i != m_progress_map.end(); ++i)
    {
        out << "          <" << i->first 
            << " value=\""     << i->second << "\"/>\n";
    }
    out << "        </achievement>\n";
}   // save

// ----------------------------------------------------------------------------

int MapAchievement::getValue(const std::string & key)
{
    if ( m_progress_map.find(key) != m_progress_map.end())
        return m_progress_map[key];
    return 0;
}

// ----------------------------------------------------------------------------
void MapAchievement::reset()
{
    std::map<std::string, int>::iterator iter;
    for ( iter = m_progress_map.begin(); iter != m_progress_map.end(); ++iter )
    {
        iter->second = 0;
    }
}   // reset

// ----------------------------------------------------------------------------
void MapAchievement::increase(const std::string & key, int increase)
{
    if (m_progress_map.find(key) != m_progress_map.end())
    {
        m_progress_map[key] += increase;
        check();
    }
    else
        m_progress_map[key] = increase;
}

// ----------------------------------------------------------------------------
irr::core::stringw MapAchievement::getProgressAsString()
{    
    int progress = 0;
    std::map<std::string, int>::const_iterator iter;
    for ( iter = m_progress_map.begin(); iter != m_progress_map.end(); ++iter)
    {
        progress += iter->second;
    }
    return StringUtils::toWString(progress) + "/" + getInfo()->toString();
}

