//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 Joerg Henrichs
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

#ifndef HEADER_POWERUPMANAGER_HPP
#define HEADER_POWERUPMANAGER_HPP

#include "utils/no_copy.hpp"
#include "utils/leak_check.hpp"

#include "btBulletDynamicsCommon.h"

#include <map>
#include <string>
#include <vector>

class Material;
class XMLNode;
namespace irr
{
    namespace scene { class IMesh; }
}

/**
  * \ingroup items
  */

/** This class manages all powerups. It reads in powerup.xml to get the data,
 *  initialise the static member of some flyables (i.e. powerup.xml contains
 *  info about cakes, plunger etc which needs to be stored), and maintains
 *  the 'weights' (used in randomly chosing which item was collected) for all
 *  items depending on position. The latter is done so that as the first player
 *  you get less advantageous items (but no useless ones either, e.g. anchor),
 *  while as the last you get more useful ones.
 *
 *  The weight distribution works as follow:
 *  Depending on the number of karts, 5 reference points are mapped to positions.
 *  For each reference point the weight distribution is read in from powerup.xml:
 *   <!--      bubble cake bowl zipper plunger switch para anvil -->
 *   <last  w="0      1    1    2      2       0      2    2"     />
 *  Then, the weights for the real position are calculated as a linear average
 *  between the weights of the reference positions immediately lower and higher.
 *  e.g. ; if the reference positions are 1 and 4,5 ; the 3rd will have
 *  weights equal to (4,5-3)/(4,5-1) times the weight of the reference position
 *  at 4,5 and (3-1)/(4,5-1) times the weight of the reference position at 1.
 *
 *  At the start of each race three mappings are computed in updateWeightsForRace:
 *  m_position_to_class maps each postion to a list of class using
 *  the function convertPositionToClass, with a class with a higher weight
 *  included more times so picking at random gives us the right class distribution
 *  m_powerups_for_position contains a list of items for each class. A item
 *  with higher weight is included more than once, so at runtime we can
 *  just pick a random item from this list to get the right distribution.
 *  In the example above the list for 'last' will be:
 *  [cake, bowling,zipper,zipper,plunger,plunger,parachute,parachute,
 *   anvil,anvil.
 */

class PowerupManager : public NoCopy
{
public:
    LEAK_CHECK();
private:
    // ------------------------------------------------------------------------
    /** This object stores the weights for each 'section' for a certain
     *  number of karts. */
    class WeightsData
    {
    private:
        /** The number of karts for which this entry is to be used. */
        unsigned int m_num_karts;

        /** Stores for each of the sections the weights from the XML file. */
        std::vector < std::vector<int> > m_weights_for_section;

        /** This field is only populated for the WeightData class that
         *  is used during a race. It contains for each rank the summed
         *  weights for easy lookup during a race. */
        std::vector < std::vector<int> > m_summed_weights_for_rank;

    public:
        WeightsData() { m_num_karts = 0; }
        void reset();
        void readData(int num_karts, const XMLNode *node);
        void interpolate(WeightsData *prev, WeightsData *next, int num_karts);
        int convertRankToSection(int rank, int *prev, int *next,
                                 float *weight);
        void precomputeWeights();
        int getRandomItem(int rank, int random_number);
        // --------------------------------------------------------------------
        /** Sets the number of karts. */
        void setNumKarts(int num_karts) { m_num_karts = num_karts; }
        // --------------------------------------------------------------------
        /** Returns for how many karts this entry is meant for. */
        int getNumKarts() const { return m_num_karts; }
    };   // class WeightsData
    // ------------------------------------------------------------------------

    /** The first key is the race type: race, battle, soccer etc.
     *  The key then contains a mapping from the kart numbers to the
     *  WeightsData object that stores all data for the give kart number.
     */
    std::map<std::string, std::vector<WeightsData*> > m_all_weights;

public:
    // The anvil and parachute must be at the end of the enum, and the
    // zipper just before them (see Powerup::hitBonusBox).
    enum PowerupType {POWERUP_NOTHING,
                      POWERUP_FIRST,
                      POWERUP_BUBBLEGUM = POWERUP_FIRST,
                      POWERUP_CAKE,
                      POWERUP_BOWLING, POWERUP_ZIPPER, POWERUP_PLUNGER,
                      POWERUP_SWITCH, POWERUP_SWATTER, POWERUP_RUBBERBALL,
                      POWERUP_PARACHUTE,
                      POWERUP_ANVIL,      //powerup.cpp assumes these two come last
                      POWERUP_LAST=POWERUP_ANVIL,
                      POWERUP_MAX
    };

private:
    const int     RAND_CLASS_RANGE = 1000;

    /** The icon for each powerup. */
    Material*     m_all_icons [POWERUP_MAX];

    /** Last time the bouncing ball was collected */
    int           m_rubber_ball_collect_ticks;

    /** The mesh for each model (if the powerup has a model), e.g. a switch
        has none. */
    irr::scene::IMesh *m_all_meshes[POWERUP_MAX];

    /** The weight distribution to be used for the current race. */
    WeightsData m_current_item_weights;

    PowerupType   getPowerupType(const std::string &name) const;
public:
                  PowerupManager  ();
                 ~PowerupManager  ();
    void          loadPowerupsModels ();
    void          loadWeights(const XMLNode *node, const std::string &category);
    void          unloadPowerups  ();
    void          computeWeightsForRace(int num_karts);
    void          LoadPowerup     (PowerupType type, const XMLNode &node);
    PowerupManager::PowerupType
        getRandomPowerup(unsigned int pos, unsigned int *n, int random_number);
    // ------------------------------------------------------------------------
    /** Returns the icon(material) for a powerup. */
    Material* getIcon(int type) const {return m_all_icons [type];}
    // ------------------------------------------------------------------------
    /** Returns the mesh for a certain powerup.
     *  \param type Mesh type for which the model is returned. */
    irr::scene::IMesh *getMesh(int type) const {return m_all_meshes[type];}
    // ------------------------------------------------------------------------
    /** Returns the last time a rubber ball was collected. */
    int getBallCollectTicks() const {return m_rubber_ball_collect_ticks;}
    // ------------------------------------------------------------------------
    /** Updates the last time at which a rubber ball was collected. */
    void setBallCollectTicks(int ticks) {m_rubber_ball_collect_ticks=ticks;}
    // ------------------------------------------------------------------------
};   // class PowerupManager

extern PowerupManager* powerup_manager;

#endif
