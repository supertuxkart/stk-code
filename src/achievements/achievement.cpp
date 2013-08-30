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

#include "utils/log.hpp"
#include "utils/translation.hpp"

#include <sstream>
#include <stdlib.h>
#include <assert.h>

// ============================================================================
Achievement::Achievement(const XMLNode * input)
{
    input->get("id", &m_id);
    m_achieved = false;
}

// ============================================================================
Achievement::~Achievement()
{

}

// ============================================================================
void Achievement::onAchieving()
{
    if(!m_achieved)
    {
        //show achievement
        //send to server
        m_achieved = true;
    }
}

// ============================================================================
SingleAchievement::SingleAchievement(const XMLNode * input)
    : Achievement(input)
{
}

// ============================================================================
void SingleAchievement::check()
{
    if(m_achieved)
        return;
    if(m_progress >= m_goal)
        onAchieving();
}


// ============================================================================
MapAchievement::MapAchievement(const XMLNode * input)
    : Achievement(input)
{
}

// ============================================================================
void MapAchievement::check()
{
    if(m_achieved)
        return;

    ProgressMap::iterator iter;
    for (iter = m_progress_map.begin(); iter != m_progress_map.end(); ++iter)
    {
       if (!iter->second)
           return;
    }
    onAchieving();
}
