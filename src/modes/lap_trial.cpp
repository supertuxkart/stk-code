//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2015 SuperTuxKart-Team
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

#include "modes/lap_trial.hpp"

#include "karts/abstract_kart.hpp"
#include "karts/controller/controller.hpp"
#include "utils/string_utils.hpp"
#include "tracks/track.hpp"

LapTrial::LapTrial() : LinearWorld()
{
    WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN, RaceManager::get()->getTimeTarget());
    RaceManager::get()->setNumLaps(99999);
}

bool LapTrial::isRaceOver()
{
    return m_count_down_reached_zero;
}

void LapTrial::countdownReachedZero()
{
    // When countdown reaches zero stop race
    LinearWorld::countdownReachedZero();
    m_count_down_reached_zero = true;
}

void LapTrial::getKartsDisplayInfo(std::vector<RaceGUIBase::KartIconDisplayInfo>* icons)
{
    const unsigned int numKarts = getNumKarts();

    for(unsigned int i = 0; i < numKarts; i++)
    {
        AbstractKart* kart = m_karts[i].get();
        RaceGUIBase::KartIconDisplayInfo& icon = (*icons)[i];
        int laps = getFinishedLapsOfKart(kart->getWorldKartId());
        icon.lap = -1;
        icon.m_outlined_font = true;
        icon.m_color = irr::video::SColor(255,255,255,255);
        if (kart->hasFinishedRace())
        {
            icon.m_text = kart->getController()->getName();
            if (RaceManager::get()->getKartGlobalPlayerId(i) > -1)
            {
                const core::stringw& flag = StringUtils::getCountryFlag(
                    RaceManager::get()->getKartInfo(i).getCountryCode());
                if (!flag.empty())
                {
                    icon.m_text += L" ";
                    icon.m_text += flag;
                }
            }
        }
        else
        {
            icon.m_text = irr::core::stringw((!kart->isEliminated() && laps < 0) ? 0 : laps);
        }
    }
}

void LapTrial::reset(bool restart)
{
    LinearWorld::reset(restart);
    WorldStatus::setClockMode(WorldStatus::CLOCK_COUNTDOWN, RaceManager::get()->getTimeTarget());
    m_count_down_reached_zero = false;
}

void LapTrial::update(int ticks)
{
    LinearWorld::update(ticks);
}

void LapTrial::terminateRace()
{
    for (unsigned int i = 0; i < getNumKarts(); i++)
    {
        getKart(i)->finishedRace(RaceManager::get()->getTimeTarget());
    }
    World::terminateRace();
}
