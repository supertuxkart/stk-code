//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2014 Glenn De Jonghe
//                     2014 Joerg Henrichs
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

#include <stdlib.h>

/** Constructur, initialises this object with the data from the
 *  corresponding AchievementInfo.
 */
Achievement::Achievement(const AchievementInfo * info)
           : m_achievement_info(info)
{
    m_id       = info->getID();
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

    for (unsigned int i = 0; i < node->getNumNodes(); i++)
    {
        const XMLNode *n = node->getNode(i);
        std::string key = n->getName();
        int value = 0;
        n->get("value", &value);
        m_progress_map[key] = value;
    }
}   // load

// ----------------------------------------------------------------------------
/** Saves the achievement status to a file.
 *  \param Output stream.
 */
void Achievement::save(UTFWriter &out)
{
    out << L"        <achievement id=\"" << m_id << L"\" "
        << L"achieved=\"" << m_achieved << "\"";
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
            << " value=\"" << i->second << "\"/>\n";
    }
    out << "        </achievement>\n";
}   // save

// ----------------------------------------------------------------------------
/** Returns the value for a key.
 */
int Achievement::getValue(const std::string & key)
{
    if (m_progress_map.find(key) != m_progress_map.end())
        return m_progress_map[key];
    return 0;
}
// ----------------------------------------------------------------------------
/** Resets all currently key values to 0. Called if the reset-after-race flag
 *  is set for the corresponding AchievementInfo.
 */
void Achievement::reset()
{
    std::map<std::string, int>::iterator iter;
    for (iter = m_progress_map.begin(); iter != m_progress_map.end(); ++iter)
    {
        iter->second = 0;
    }
}   // reset

// ----------------------------------------------------------------------------
/** Returns how much of an achievement has been achieved in the form n/m.
 *  The AchievementInfo adds up all goal values to get 'm', and this
 *  this class end up all current key values for 'n'.
 */
irr::core::stringw Achievement::getProgressAsString()
{
    int progress = 0;
    std::map<std::string, int>::const_iterator iter;
    switch (m_achievement_info->getCheckType())
    {
    case AchievementInfo::AC_ALL_AT_LEAST:
        for (iter = m_progress_map.begin(); iter != m_progress_map.end(); ++iter)
        {
            progress += iter->second;
        }
        break;
    case AchievementInfo::AC_ONE_AT_LEAST:
        for (iter = m_progress_map.begin(); iter != m_progress_map.end(); ++iter)
        {
            if(iter->second>progress) progress = iter->second;
        }
        break;
    default:
        Log::fatal("Achievement", "Missing getProgressAsString for type %d.",
                   m_achievement_info->getCheckType());
    }
    return StringUtils::toWString(progress) + "/" + getInfo()->toString();
}   // getProgressAsString

// ----------------------------------------------------------------------------
/** Increases the value of a key by a specified amount.
 *  \param key The key whose value is increased.
 *  \param increase Amount to add to the value of this key.
 */
void Achievement::increase(const std::string & key, int increase)
{
    if (m_progress_map.find(key) != m_progress_map.end())
        m_progress_map[key] += increase;
    else
        m_progress_map[key] = increase;
    check();
}   // increase

// ----------------------------------------------------------------------------
/** Called at the end of a race to potentially reset values.
 */
void Achievement::onRaceEnd()
{
    if(m_achievement_info->needsResetAfterRace())
        reset();
}   // onRaceEnd

// ----------------------------------------------------------------------------
/** Checks if this achievement has been achieved.
 */
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
}   // check

