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
#include "states_screens/dialogs/notification_dialog.hpp"
#include "io/xml_writer.hpp"
#include "utils/log.hpp"
#include "utils/translation.hpp"


#include <sstream>
#include <stdlib.h>
#include <assert.h>

// ============================================================================
Achievement::Achievement(const AchievementInfo * info)
    :m_achievement_info(info)
{
    m_id = info->getID();
    m_achieved = false;
}

// ============================================================================
Achievement::~Achievement()
{

}

// ============================================================================
void Achievement::onRaceEnd()
{
    if(m_achievement_info->needsResetAfterRace())
        this->reset();
}

// ============================================================================
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

// ============================================================================
SingleAchievement::SingleAchievement(const AchievementInfo * info)
    : Achievement(info)
{
    m_progress = 0;
    m_achieved = false;
}

// ============================================================================
void SingleAchievement::load(XMLNode * input)
{
    std::string achieved("");
    input->get("achieved", &achieved);
    if(achieved == "true")
    {
        m_achieved = true;
        return;
    }
    input->get("value", &m_progress);
}
// ============================================================================
void SingleAchievement::save(std::ofstream & out)
{
    out << "        <achievement id=\"" << m_id << "\" "
        << "achieved=\"" << StringUtils::toString(m_achieved) << "\"";
    if(!m_achieved)
    {
        out << " value=\"" << StringUtils::toString(m_progress) << "\"";
    }
    out << "/>\n";
}   // save

// ============================================================================
void SingleAchievement::reset()
{
    m_progress = 0;
}   // reset

// ============================================================================
void SingleAchievement::increase(int increase)
{
    m_progress += increase;
    check();
}

// ============================================================================
irr::core::stringw SingleAchievement::getProgressAsString()
{
    return StringUtils::toWString(m_progress) + "/" + StringUtils::toWString(((SingleAchievementInfo *) m_achievement_info)->getGoalValue());
}

// ============================================================================
MapAchievement::MapAchievement(const AchievementInfo * info)
    : Achievement(info)
{
}

// ============================================================================
void MapAchievement::load(XMLNode * input)
{
    std::string achieved("");
    input->get("achieved", &achieved);
    if(achieved == "true")
    {
        m_achieved = true;
        return;
    }
    std::vector<XMLNode*> xml_entries;
    input->getNodes("entry", xml_entries);
    for (unsigned int n=0; n < xml_entries.size(); n++)
    {
        std::string key("");
        xml_entries[n]->get("key", &key);
        int value(0);
        xml_entries[n]->get("value", &value);
        m_progress_map[key] = value;
    }
}

// ============================================================================
void MapAchievement::save(std::ofstream & out)
{
    out << "        <achievement id=\"" << m_id << "\" achieved=\"" 
        << StringUtils::toString(m_achieved) << "\">\n";
    if(!m_achieved)
    {
        std::map<std::string, int>::iterator i;
        for ( i = m_progress_map.begin(); i != m_progress_map.end(); ++i )
        {
            out << "            <entry key=\"" << i->first.c_str()
                << "\" value=\"" << StringUtils::toString(i->second) 
                << "\"/>\n";
        }
    }
    out << "        </achievement>\n";
}   // save

// ============================================================================

int MapAchievement::getValue(const std::string & key)
{
    if ( m_progress_map.find(key) != m_progress_map.end())
        return m_progress_map[key];
    return 0;
}


// ============================================================================
void MapAchievement::reset()
{
    std::map<std::string, int>::iterator iter;
    for ( iter = m_progress_map.begin(); iter != m_progress_map.end(); ++iter )
    {
        iter->second = 0;
    }
}   // reset

// ============================================================================
void MapAchievement::increase(const std::string & key, int increase)
{
    if ( m_progress_map.find(key) != m_progress_map.end())
    {
        m_progress_map[key] += increase;
        check();
    }
}

// ============================================================================
irr::core::stringw MapAchievement::getProgressAsString()
{
    int progress(0);
    int goal(0);
    const std::map<std::string, int> goal_values = ((MapAchievementInfo *) m_achievement_info)->getGoalValues();
    std::map<std::string, int>::const_iterator iter;
    for ( iter = goal_values.begin(); iter != goal_values.end(); ++iter ) {
        goal += iter->second;
        progress += m_progress_map[iter->first];
    }
    return StringUtils::toWString(progress) + "/" + StringUtils::toWString(goal);
}

