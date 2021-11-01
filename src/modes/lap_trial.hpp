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

#ifndef HEADER_LAP_TRIAL_HPP
#define HEADER_LAP_TRIAL_HPP

#include "modes/linear_world.hpp"

class LapTrial : public LinearWorld
{
protected:
    virtual bool isRaceOver() OVERRIDE;
public:
    LapTrial();
    virtual void countdownReachedZero() OVERRIDE;
    virtual void reset(bool restart = false) OVERRIDE;
    virtual void update(int ticks) OVERRIDE;
    virtual void terminateRace() OVERRIDE;
    virtual const std::string& getIdent() const OVERRIDE { return IDENT_LAP_TRIAL; }
    virtual bool showLapsTarget() OVERRIDE { return false; }
    virtual void getKartsDisplayInfo(std::vector<RaceGUIBase::KartIconDisplayInfo>* icons) OVERRIDE;
private:
    bool m_count_down_reached_zero;
};

#endif
