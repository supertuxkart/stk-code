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

#include "items/powerup_manager.hpp"

#include <cinttypes>
#include <stdexcept>

#include "config/stk_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/sp/sp_base.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "guiengine/engine.hpp"
#include "guiengine/skin.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "items/bowling.hpp"
#include "items/cake.hpp"
#include "items/plunger.hpp"
#include "items/rubber_ball.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

#include <IMesh.h>

PowerupManager* powerup_manager=0;

//-----------------------------------------------------------------------------
/** The constructor initialises everything to zero. */
PowerupManager::PowerupManager()
{
    m_random_seed.store(0);
    for(int i=0; i<POWERUP_MAX; i++)
    {
        m_all_meshes[i] = NULL;
        m_all_icons[i]  = (Material*)NULL;
    }
}   // PowerupManager

//-----------------------------------------------------------------------------
/** Destructor, frees all meshes. */
PowerupManager::~PowerupManager()
{
    for(unsigned int i=POWERUP_FIRST; i<=POWERUP_LAST; i++)
    {
        scene::IMesh *mesh = m_all_meshes[(PowerupType)i];
        if(mesh)
        {
            mesh->drop();
            // If the ref count is 1, the only reference is in
            // irrlicht's mesh cache, from which the mesh can
            // then be deleted.
            // Note that this test is necessary, since some meshes
            // are also used in attachment_manager!!!
            if(mesh->getReferenceCount()==1)
                irr_driver->removeMeshFromCache(mesh);
        }
    }
    
    for(auto key: m_all_weights)
    {
        for(auto p: key.second )
            delete p;
    }
}   // ~PowerupManager

//-----------------------------------------------------------------------------
/** Removes any textures so that they can be reloaded.
 */
void PowerupManager::unloadPowerups()
{
    for(unsigned int i=POWERUP_FIRST; i<=POWERUP_LAST; i++)
    {
        if(m_all_meshes[(PowerupType)i])
            m_all_meshes[(PowerupType)i]->drop();

        //FIXME: I'm not sure if this is OK or if I need to ->drop(),
        //       or delete them, or...
        m_all_icons[i]  = (Material*)NULL;
    }
}   // removeTextures

//-----------------------------------------------------------------------------
/** Determines the powerup type for a given name.
 *  \param name Name of the powerup to look up.
 *  \return The type, or POWERUP_NOTHING if the name is not found
 */
PowerupManager::PowerupType
    PowerupManager::getPowerupType(const std::string &name) const
{
    // Must match the order of PowerupType in powerup_manager.hpp!!
    static std::string powerup_names[] = {
        "",            /* Nothing */
        "bubblegum", "cake", "bowling", "zipper", "plunger", "switch",
        "swatter", "rubber-ball", "parachute", "anchor"
    };

    for(unsigned int i=POWERUP_FIRST; i<=POWERUP_LAST; i++)
    {
        if(powerup_names[i]==name) return(PowerupType)i;
    }
    return POWERUP_NOTHING;
}   // getPowerupType

//-----------------------------------------------------------------------------
/** Loads powerups models and icons from the powerup.xml file.
 */
void PowerupManager::loadPowerupsModels()
{
    const std::string file_name = file_manager->getAsset("powerup.xml");
    XMLNode *root               = file_manager->createXMLTree(file_name);
    for(unsigned int i=0; i<root->getNumNodes(); i++)
    {
        const XMLNode *node=root->getNode(i);
        if(node->getName()!="item") continue;
        std::string name;
        node->get("name", &name);
        PowerupType type = getPowerupType(name);
        // The weight nodes will be also included in this list, so ignore those
        if(type!=POWERUP_NOTHING)
            loadPowerup(type, *node);
        else
        {
            Log::fatal("PowerupManager",
                       "Can't find item '%s' from powerup.xml, entry %d.",
                       name.c_str(), i+1);
            exit(-1);
        }
    }
    
    loadWeights(root, "race-weight-list"    );
    loadWeights(root, "ftl-weight-list"     );
    loadWeights(root, "battle-weight-list"  );
    loadWeights(root, "soccer-weight-list"  );
    loadWeights(root, "tutorial-weight-list");

    delete root;

    if (GUIEngine::isNoGraphics())
    {
        for (unsigned i = POWERUP_FIRST; i <= POWERUP_LAST; i++)
        {
            scene::IMesh *mesh = m_all_meshes[(PowerupType)i];
            if (mesh)
            {
                // After minMax3D from loadPowerup mesh can free its vertex
                // buffer
                mesh->freeMeshVertexBuffer();
            }
        }
    }
}  // loadPowerupsModels

//-----------------------------------------------------------------------------
/** Loads the powerups weights for a given category (race, ft, ...). The data
 *  is stored in m_all_weights.
 *  \param node The top node of the powerup xml file.
 *  \param class_name The name of the attribute with the weights for the
 *         class.
 */
void PowerupManager::loadWeights(const XMLNode *powerup_node,
                                 const std::string &class_name)
{
    const XMLNode *node = powerup_node->getNode(class_name);
    if(!node)
    {
        Log::fatal("PowerupManager",
                   "Cannot find node '%s' in powerup.xml file.",
                   class_name.c_str());
    }

    for (unsigned int i = 0; i < node->getNumNodes(); i++)
    {
        const XMLNode *weights = node->getNode(i);
        int num_karts;
        weights->get("num-karts", &num_karts);
        WeightsData *wd = new WeightsData();
        wd->readData(num_karts, weights);
        m_all_weights[class_name].push_back(wd);
    }    // for i in node->getNumNodes

}  // loadWeights

// ============================================================================
// Implement of WeightsData

/** Deletes all data stored in a WeightsData objects.
 */
void PowerupManager::WeightsData::reset()
{
    m_weights_for_section.clear();
    m_summed_weights_for_rank.clear();
    m_num_karts = 0;
}   // reset

//-----------------------------------------------------------------------------
/** Reads in all weights for a given category and number of karts.
 *  \param num_karts Number of karts for this set of data.
 *  \param node The XML node with the data to read.
 */
void PowerupManager::WeightsData::readData(int num_karts, const XMLNode *node)
{
    m_num_karts = num_karts;
    for (unsigned int i = 0; i < node->getNumNodes(); i++)
    {
        m_weights_for_section.emplace_back();
        const XMLNode *w = node->getNode(i);
        std::string single_item;
        w->get("single", &single_item);
        std::string multi_item;
        w->get("multi", &multi_item);
        std::vector<std::string> l_string =
            StringUtils::split(single_item+" "+multi_item,' ');

        // Keep a reference for shorter access to the list
        std::vector<int> &l = m_weights_for_section.back();
        for(unsigned int i=0; i<l_string.size(); i++)
        {
            if(l_string[i]=="") continue;
            int n;
            StringUtils::fromString(l_string[i], n);
            l.push_back(n);
        }
        // Make sure we have the right number of entries
        if (l.size() < 2 * (int)POWERUP_LAST)
        {
            Log::fatal("PowerupManager",
                       "Not enough entries for '%s' in powerup.xml",
                       node->getName().c_str());
            while (l.size() < 2 * (int)POWERUP_LAST) l.push_back(0);
        }
        if(l.size()>2*(int)POWERUP_LAST)
        {
            Log::fatal("PowerupManager",
                       "Too many entries for '%s' in powerup.xml.",
                       node->getName().c_str());
        }
    }   // for i in getNumNodes()
}   // WeightsData::readData

// ----------------------------------------------------------------------------
/** Defines the weights for this WeightsData object based on a linear
 *  interpolation between the previous and next WeightsData class (depending
 *  on the number of karts in this race and in previous and next).
 *  \param prev The WeightsData object for less karts.
 *  \param next The WeightData object for more karts.
 *  \param num_karts Number of karts to extrapolate for.
 */
void PowerupManager::WeightsData::interpolate(WeightsData *prev,
                                              WeightsData *next, int num_karts)
{
    m_num_karts = num_karts;
    m_weights_for_section.clear();
    float f = float(num_karts - prev->getNumKarts())
            / (next->getNumKarts() - prev->getNumKarts());

    // Outer loop over all classes. Note that 'this' is empty atm, but
    // since all WeightsData have the same number of elements, we use
    // the previous one for loop boundaries and push_back the interpolated
    // values to 'this'.
    for (unsigned int cl = 0; cl < prev->m_weights_for_section.size(); cl++)
    {
        std::vector<int> & w_prev = prev->m_weights_for_section[cl];
        std::vector<int> & w_next = next->m_weights_for_section[cl];
        m_weights_for_section.emplace_back();
        std::vector<int> &l = m_weights_for_section.back();
        for (unsigned int i = 0; i < w_prev.size(); i++)
        {
            float interpolated_weight = w_prev[i] * (1-f) + w_next[i] * f;
            l.push_back(int(interpolated_weight + 0.5f));
        }
    }   // for l < prev->m_weights_for_section.size()
}   // WeightsData::interpolate

// ----------------------------------------------------------------------------
/** For a given rank in the current race this computes the previous and
 *  next entry in the weight list, and the weight necessary to interpolate
 *  between these two values. If the requested rank should exactly match
 *  one entries, previous and next entry will be identical, and weight set
 *  to 1.0.
 *  \param rank Rank that is to be interpolated.
 *  \param prev On return contains the index of the closest weight field
 *         smaller than the given rank.
 *  \param next On return contains the index of the closest weight field
 *         bigger than the given rank.
 *  \param weight On return contains the weight to use to interpolate between
 *         next and previous. The weight is for 'next', so (1-weight) is the
 *         weight that needs to be applied to the previous data.
 */
void PowerupManager::WeightsData::convertRankToSection(int rank, int *prev,
                                                       int *next, float *weight)
{
    // If there is only one section (e.g. in soccer mode etc), use it.
    // If the rank is first, always use the first entry as well.
    if (m_weights_for_section.size() == 1 || rank == 1)
    {
        *prev = *next = 0;
        *weight = 1.0f;
        return;
    }

    // The last kart always uses the data for the last section
    if (rank == (int)m_num_karts)
    {
        *prev = *next = (int)m_weights_for_section.size() - 1;
        *weight = 1.0f;
        return;
    }

    // In FTL mode the first section is for the leader, the 
    // second section is used for the first non-leader kart.
    if (RaceManager::get()->isFollowMode() && rank == 2)
    {
        *prev = *next = 1;
        *weight = 1.0f;
        return;
    }

    // Now we have a rank that needs to be interpolated between
    // two sections.

    // Get the first index that is used for a section (FTL is
    // special since index 2 is for the first non-leader kart):
    int first_section_index = RaceManager::get()->isFollowMode() ? 2 : 1;

    // If we have five points, the first and last assigned to the first
    // and last kart, leaving 3 points 'inside' this interval, which define
    // 4 'sections'. So the number of sections is number_of_points - 2 + 1.
    // If the first two points are assigned to rank 1 and 2 in a FTL race
    // and the last to the last kart, leaving two inner points defining
    // 3 sections, i.e. number_of_points - 3 + 1
    // In both cases the number of sections is:
    int num_sections = ((int)m_weights_for_section.size() - first_section_index);
    float karts_per_fraction = (m_num_karts - first_section_index)
                             / float(num_sections);

    // Now check in which section the current rank is: Test from the first
    // section (section 0) and see if the rank is still greater than the
    // next section. If not, the current section is the section to which
    // this rank belongs. Otherwise increase section and try again:
    int section = 0;
    while (rank - first_section_index >  (section + 1) * karts_per_fraction)
    {
        section++;
    }

    *prev = first_section_index + section - 1;
    *next = *prev + 1;
    *weight = (rank - first_section_index - section * karts_per_fraction)
            / karts_per_fraction;

    return;
}   // WeightsData::convertRankToSection

// ----------------------------------------------------------------------------
/** This function computes the item distribution for each possible rank in the
 *  race. It creates a list which sums for each item the weights of all
 *  previous items.  E.g. if the weight list starts with 20, 30, 0, 10,
 *  the summed array will contains 20, 50, 50, 60. This allows for a quick
 *  look up based on a single random number.
 */
void PowerupManager::WeightsData::precomputeWeights()
{
    m_summed_weights_for_rank.clear();
    for (unsigned int i = 0; i<m_num_karts; i++)
    {
        m_summed_weights_for_rank.emplace_back();
        int prev, next;
        float weight;
        convertRankToSection(i + 1, &prev, &next, &weight);
        int sum = 0;
        for (unsigned int j = 0;
            j <= 2 * POWERUP_LAST - POWERUP_FIRST; j++)
        {
            float av = (1.0f - weight) * m_weights_for_section[prev][j]
                     +         weight  * m_weights_for_section[next][j];
            sum += int(av + 0.5f);
            m_summed_weights_for_rank[i].push_back(sum);
        }
    }
}   // WeightsData::precomputeWeights
//-----------------------------------------------------------------------------
/** Computes a random item dependent on the rank of the kart and a given
 *  random number.
 *  The value returned matches the enum value of the random item if single.
 *  In case of triple-item, the value will be the enum value plus
 *  the number of existing powerups (= POWERUP_LAST-POWERUP_FIRST+1)
 *  \param rank The rank for which an item needs to be picked (between 0
 *         and number_of_karts-1).
 *  \param random_number A random number used to 'randomly' select the item
 *         that was picked.
 */
int PowerupManager::WeightsData::getRandomItem(int rank, uint64_t random_number)
{
    // E.g. for battle mode with only one entry
    if(rank>(int)m_summed_weights_for_rank.size())
        rank = (int)m_summed_weights_for_rank.size()-1;
    else if (rank<0) rank = 0;  // E.g. battle mode, which has rank -1
    const std::vector<unsigned> &summed_weights = m_summed_weights_for_rank[rank];
    // The last entry is the sum of all previous entries, i.e. the maximum
    // value
#undef ITEM_DISTRIBUTION_DEBUG
#ifdef ITEM_DISTRIBUTION_DEBUG
    uint64_t original_random_number = random_number;
#endif
    random_number = random_number % summed_weights.back();
    // Put the random number in range [1;max of summed weights],
    // so for sum = N, there are N possible random numbers <= N.
    random_number++;
    int powerup = 0;
    // Stop at the first inferior or equal sum, before incrementing
    // So stop while powerup is such that
    // summed_weights[powerup-1] < random_number <= summed_weights[powerup]
    while ( random_number > summed_weights[powerup] )
        powerup++;

    // We align with the beginning of the enum and return
    // We don't do more, because it would need to be decoded from enum later
#ifdef ITEM_DISTRIBUTION_DEBUG
    Log::verbose("Powerup", "World %d rank %d random %d %" PRIu64 " item %d",
                 World::getWorld()->getTicksSinceStart(), rank, random_number,
                 original_random_number, powerup);
#endif

    return powerup + POWERUP_FIRST;
}   // WeightsData::getRandomItem

// ============================================================================
/** Loads the data for one particular powerup. For bowling ball, plunger, and
 *  cake static members in the appropriate classes are called to store
 *  additional information for those objects.
 *  \param type The type of the powerup.
 *  \param node The XML node with the data for this powerup.
 */
void PowerupManager::loadPowerup(PowerupType type, const XMLNode &node)
{
    std::string icon_file("");
    node.get("icon", &icon_file);
    icon_file = GUIEngine::getSkin()->getThemedIcon("gui/icons/" + icon_file);

#ifdef DEBUG
    if (icon_file.size() == 0)
    {
        Log::fatal("PowerupManager",
                   "Cannot load powerup %i, no 'icon' attribute under XML node",
                   type);
    }
#endif

    m_all_icons[type] = material_manager->getMaterial(icon_file,
                                  /* full_path */     true,
                                  /*make_permanent */ true,
                                  /*complain_if_not_found*/ true,
                                  /*strip_path*/ false);


    assert(m_all_icons[type] != NULL);
    assert(m_all_icons[type]->getTexture() != NULL);

    std::string model("");
    node.get("model", &model);
    if(model.size()>0)
    {
        std::string full_path = file_manager->getAsset(FileManager::MODEL,model);
        m_all_meshes[type] = irr_driver->getMesh(full_path);
        if(!m_all_meshes[type])
        {
            std::ostringstream o;
            o << "Can't load model '" << model << "' for powerup type '" << type
              << "', aborting.";
            throw std::runtime_error(o.str());
        }
#ifndef SERVER_ONLY
        SP::uploadSPM(m_all_meshes[type]);
#endif
        m_all_meshes[type]->grab();
    }
    else
    {
        m_all_meshes[type] = 0;
    }
    // Load special attributes for certain powerups
    switch (type) {
        case POWERUP_BOWLING:
             Bowling::init(node, m_all_meshes[type]);    break;
        case POWERUP_PLUNGER:
             Plunger::init(node, m_all_meshes[type]);    break;
        case POWERUP_CAKE:
             Cake::init(node, m_all_meshes[type]);       break;
        case POWERUP_RUBBERBALL:
             RubberBall::init(node, m_all_meshes[type]); break;
        default: break;
    }   // switch
}   // loadPowerup

// ----------------------------------------------------------------------------
/** Create a (potentially interpolated) WeightsData objects for the current
 *  race based on the number of karts.
 *  \param num_karts Number of karts in the current race.
 */
void PowerupManager::computeWeightsForRace(int num_karts)
{
    if (num_karts == 0) return;

    std::string class_name="";
    switch (RaceManager::get()->getMinorMode())
    {
    case RaceManager::MINOR_MODE_TIME_TRIAL:       /* fall through */
    case RaceManager::MINOR_MODE_LAP_TRIAL:       /* fall through */
    case RaceManager::MINOR_MODE_NORMAL_RACE:      class_name="race";     break;
    case RaceManager::MINOR_MODE_FOLLOW_LEADER:    class_name="ftl";      break;
    case RaceManager::MINOR_MODE_3_STRIKES:        class_name="battle";   break;
    case RaceManager::MINOR_MODE_FREE_FOR_ALL:     class_name="battle";   break;
    case RaceManager::MINOR_MODE_CAPTURE_THE_FLAG: class_name="battle";   break;
    case RaceManager::MINOR_MODE_TUTORIAL:         class_name="tutorial"; break;
    case RaceManager::MINOR_MODE_EASTER_EGG:       /* fall through */
    case RaceManager::MINOR_MODE_OVERWORLD:
    case RaceManager::MINOR_MODE_CUTSCENE:
    case RaceManager::MINOR_MODE_SOCCER:           class_name="soccer";   break;
    default:
        Log::fatal("PowerupManager", "Invalid minor mode %d - aborting.",
                    RaceManager::get()->getMinorMode());
    }
    class_name +="-weight-list";

    std::vector<WeightsData*> wd = m_all_weights[class_name];

    // Find the two indices closest to the current number of karts
    // so that the right number can be interpolated between the
    // two values.
    int prev_index=0, next_index=0;
    for (unsigned int i = 1; i < wd.size(); i++)
    {
        int n = wd[i]->getNumKarts();
        if ( ( n < wd[prev_index]->getNumKarts() &&
                   wd[prev_index]->getNumKarts() > num_karts)       ||
             ( n > wd[prev_index]->getNumKarts() && n <= num_karts )   )
        {
                prev_index = i;
        }
        if ( ( n > wd[next_index]->getNumKarts()     &&
                   wd[next_index]->getNumKarts() < num_karts  )     ||
             ( n < wd[next_index]->getNumKarts() && n >= num_karts)    )
        {
            next_index = i;
        }
    }

    // Check if we have exactly one entry (e.g. either class with only one
    // set of data specified, or an exact match):
    m_current_item_weights.reset();
    if(prev_index == next_index)
    {
        // Just create a copy of this entry:
        m_current_item_weights = *wd[prev_index];
        // The number of karts might need to be increased to make
        // sure enough weight list for all ranks are created: e.g.
        // in soccer mode there is only one weight list (for 1 kart)
        // but we still need to make sure to create rank weight list
        // for all possible ranks
        m_current_item_weights.setNumKarts(num_karts);
    }
    else
    {
        // We need to interpolate between prev_index and next_index
        m_current_item_weights.interpolate(wd[prev_index], wd[next_index],
                                           num_karts                      );
    }
    m_current_item_weights.precomputeWeights();
}   // computeWeightsForRace

// ----------------------------------------------------------------------------
/** Returns a random powerup for a kart at a given position. If the race mode
 *  is a battle, the position is actually not used and a randomly selected
 *  item for POSITION_BATTLE_MODE is returned. This function takes the weights
 *  specified for all items into account by using a list which contains all
 *  items depending on the weights defined. See updateWeightsForRace()
 *  \param pos Position of the kart (1<=pos<=number of karts).
 *  \param n Number of times this item is given to the kart.
 *  \param random_number A random number used to select the item. Important
 *         for networking to be able to reproduce item selection.
 */
PowerupManager::PowerupType PowerupManager::getRandomPowerup(unsigned int pos,
                                                             unsigned int *n,
                                                             uint64_t random_number)
{
    int powerup = m_current_item_weights.getRandomItem(pos-1, random_number);
    if(powerup > POWERUP_LAST)
    {
        powerup -= (POWERUP_LAST-POWERUP_FIRST+1);
        *n = 3;
    }
    else
        *n=1;

    // Prevents early explosive items
    if (World::getWorld() && 
        stk_config->ticks2Time(World::getWorld()->getTicksSinceStart()) <
                                      stk_config->m_no_explosive_items_timeout)
    {
        if (powerup == POWERUP_CAKE || powerup == POWERUP_RUBBERBALL)
            powerup = POWERUP_BOWLING;
    }
    return (PowerupType)powerup;
}   // getRandomPowerup

// ============================================================================
/** Unit testing is based on deterministic item distributions: if all random
 *  numbers from 0 till sum_of_all_weights - 1 are used, the original weight
 *  distribution must be restored.
 */
void PowerupManager::unitTesting()
{
    // Test 1: Test all possible random numbers for tutorial, and
    // make sure that always three bowling balls are picked.
    // ----------------------------------------------------------
    RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_TUTORIAL);
    powerup_manager->computeWeightsForRace(1);
    WeightsData wd = powerup_manager->m_current_item_weights;
    int num_weights = wd.m_summed_weights_for_rank[0].back();
    for(int i=0; i<num_weights; i++)
    {
#ifdef DEBUG
        unsigned int n;
        assert( powerup_manager->getRandomPowerup(1, &n, i)==POWERUP_BOWLING );
        assert(n==3);
#endif
    }

    // Test 2: Test all possible random numbers for 5 karts and rank 5
    // ---------------------------------------------------------------
    RaceManager::get()->setMinorMode(RaceManager::MINOR_MODE_NORMAL_RACE);
    int num_karts = 5;
    powerup_manager->computeWeightsForRace(num_karts);
    wd = powerup_manager->m_current_item_weights;

    int position = 5;
    int section, next;
    float weight;
    wd.convertRankToSection(position, &section, &next, &weight);
    assert(weight == 1.0f);
    assert(section == next);
    // Get the sum of all weights, which determine the number
    // of different random numbers we need to test.
    num_weights = wd.m_summed_weights_for_rank[section].back();
    std::vector<int> count(2*POWERUP_LAST);
    for (int i = 0; i<num_weights; i++)
    {
        unsigned int n;
        int powerup = powerup_manager->getRandomPowerup(position, &n, i);
        if(n==1)
            count[powerup-1]++;
        else
            count[powerup+POWERUP_LAST-POWERUP_FIRST]++;
    }

    // Now make sure we reproduce the original weight distribution.
    for(unsigned int i=0; i<wd.m_weights_for_section[section].size(); i++)
    {
        assert(count[i] == wd.m_weights_for_section[section][i]);
    }
}   // unitTesting
