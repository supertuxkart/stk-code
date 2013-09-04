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
Achievement::Achievement(AchievementInfo * info)
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
            _("Completed achievement") + '"' + m_achievement_info->getDescription() + '".'
        ));
        //send to server
        Online::CurrentUser::get()->onAchieving(m_id);
        m_achieved = true;
    }
}

// ============================================================================
SingleAchievement::SingleAchievement(AchievementInfo * info)
    : Achievement(info)
{
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
    out << "<achievement id=\"" << m_id << "\""
        << "achieved=\"" << StringUtils::boolstr(m_achieved) << "\""
        << "value=\"" << StringUtils::toString(m_progress) << "\""
        << "/>\n";
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
MapAchievement::MapAchievement(AchievementInfo * info)
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
    out << "<achievement id=\"" << m_id << "\" achieved=\"" << StringUtils::boolstr(m_achieved) << "\">\n";
    std::map<std::string, int>::iterator iter;
    for ( iter = m_progress_map.begin(); iter != m_progress_map.end(); ++iter ) {
        out << "    <entry key=\"" << iter->first.c_str() << "\" value=\"" << StringUtils::toString(iter->second) << "\"/>\n";
    }
    out << "</achievement>\n";
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
    for ( iter = m_progress_map.begin(); iter != m_progress_map.end(); ++iter ) {
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

