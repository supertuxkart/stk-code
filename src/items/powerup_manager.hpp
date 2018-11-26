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

#include "utils/leak_check.hpp"
#include "utils/no_copy.hpp"
#include "utils/types.hpp"

#include "btBulletDynamicsCommon.h"

#include <atomic>
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
 *  you get less advantageous items (but no useless ones either), while as the
 *  last you get more useful ones.
 *
 *  The weights distribution is described in the powerup.xml file in more
 *  detail. All weights are stored in the m_all_weights data structure,
 *  which maps the race mode (race, battle, ...) to a list of WeightsData
 *  instances. Each WeightsData instance stores the data for one specific
 *  number of karts. E.g. m_all_weights['race'] contains 5 WeightsData
 *  instances for 1, 5, 9, 14, and 20 karts.
 *  At race start a new instance of WeightsData is created in
 *  m_current_item_weights. It contains the interpolated values for the
 *  number of karts in the current race (e.g. if the race is with 6 karts
 *  if will use 3/4 the weights for 5 karts, and 1/4 the weights for 9 karts.
 *  Then m_current_item_weights will create a weight distribution for each
 *  possible rank in the race (1 to 6 in the example above). This is the
 *  interpolation of the values within one WeightsData. Atm there are also
 *  5 entries in that list (though it does not have to be the same number
 *  as above - i.e. the 1, 5, 9, 14, 20 weights list). Similarly the actual
 *  distribution used for a kart with a specific rank is based on dividing
 *  the available ranks (so 6 karts --> 6 ranks). With the 5 specified values
 *  the first entry is used for rank 1, the last entry for rank 6, and ranks
 *  2-5 will be interpolated based on an equal distance: in a race with 6
 *  karts for example, the 2nd weight list is used for rank 2.25, the 3nd
 *  for rank 3.5, the 4th for rank 4.75 (and the first and last for rank 1
 *  and 6). It does not matter that the ranks are non integer: the actual
 *  weights used for say rank 2, will then be interplated between the weights
 *  of rank 1 and 2.25 (e.g. 0.8*weights_for 2.25 + 0.2*weights_for 1).
 */

class PowerupManager : public NoCopy
{
public:
    LEAK_CHECK();
private:
    // ------------------------------------------------------------------------
    /** This object stores all the weights for one particular number of
     *  karts. I.e. it has a list of all the weights within the number of karts.
     */
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
        std::vector < std::vector<unsigned> > m_summed_weights_for_rank;

    public:
        // The friend declaration gives the PowerupManager access to the
        // internals, which is ONLY used for testing!!
        friend PowerupManager;
        WeightsData() { m_num_karts = 0; }
        void reset();
        void readData(int num_karts, const XMLNode *node);
        void interpolate(WeightsData *prev, WeightsData *next, int num_karts);
        void convertRankToSection(int rank, int *prev, int *next,
                                 float *weight);
        void precomputeWeights();
        int getRandomItem(int rank, uint64_t random_number);
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

    /** The icon for each powerup. */
    Material*     m_all_icons [POWERUP_MAX];

    /** The mesh for each model (if the powerup has a model), e.g. a switch
        has none. */
    irr::scene::IMesh *m_all_meshes[POWERUP_MAX];

    /** The weight distribution to be used for the current race. */
    WeightsData m_current_item_weights;

    PowerupType   getPowerupType(const std::string &name) const;

    /** Seed for random powerup, for local game it will use a random number,
     *  for network games it will use the start time from server. */
    std::atomic<uint64_t> m_random_seed;

public:
    static void unitTesting();

                  PowerupManager  ();
                 ~PowerupManager  ();
    void          loadPowerupsModels ();
    void          loadWeights(const XMLNode *node, const std::string &category);
    void          unloadPowerups  ();
    void          computeWeightsForRace(int num_karts);
    void          loadPowerup     (PowerupType type, const XMLNode &node);
    PowerupManager::PowerupType
        getRandomPowerup(unsigned int pos, unsigned int *n,
                         uint64_t random_number);
    // ------------------------------------------------------------------------
    /** Returns the icon(material) for a powerup. */
    Material* getIcon(int type) const {return m_all_icons [type];}
    // ------------------------------------------------------------------------
    /** Returns the mesh for a certain powerup.
     *  \param type Mesh type for which the model is returned. */
    irr::scene::IMesh *getMesh(int type) const {return m_all_meshes[type];}
    // ------------------------------------------------------------------------
    uint64_t getRandomSeed() const { return m_random_seed.load(); }
    // ------------------------------------------------------------------------
    void setRandomSeed(uint64_t seed) { m_random_seed.store(seed); }

};   // class PowerupManager

extern PowerupManager* powerup_manager;

#endif
