//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
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

#ifndef EASTER_EGG_HUNT_HPP
#define EASTER_EGG_HUNT_HPP

#include "modes/linear_world.hpp"
#include "states_screens/race_gui_base.hpp"

#include <string>
#include <vector>

class AbstractKart;

/**
 * \brief An implementation of World to provide an easter egg hunt like mode
 * \ingroup modes
 */
class EasterEggHunt: public LinearWorld
{
private:
    /** Keeps track of how many eggs each kart has found. */
    std::vector<int>  m_eggs_collected;

    /** Overall number of easter eggs. */
    int   m_number_of_eggs;

    /** Number of eggs found so far. */
    int   m_eggs_found;
public:
             EasterEggHunt();
    virtual ~EasterEggHunt();

    virtual void init() OVERRIDE;

    virtual bool isRaceOver() OVERRIDE;

    // overriding World methods
    virtual void reset() OVERRIDE;

    virtual bool raceHasLaps() OVERRIDE { return false; }

    virtual const std::string& getIdent() const OVERRIDE;
    virtual void terminateRace() OVERRIDE;
    virtual void update(float dt) OVERRIDE;
    virtual void getKartsDisplayInfo(
                          std::vector<RaceGUIBase::KartIconDisplayInfo> *info) OVERRIDE;

    void updateKartRanks();
    void collectedEasterEgg(const AbstractKart *kart);
    void readData(const std::string &filename);

    virtual void checkForWrongDirection(unsigned int i, float dt) OVERRIDE;
    virtual float estimateFinishTimeForKart(AbstractKart* kart) OVERRIDE;

};   // EasterEggHunt


#endif
