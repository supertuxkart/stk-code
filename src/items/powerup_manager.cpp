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

#include <stdexcept>

#include <irrlicht.h>

#include "graphics/irr_driver.hpp"
#include "graphics/material.hpp"
#include "graphics/material_manager.hpp"
#include "io/file_manager.hpp"
#include "io/xml_node.hpp"
#include "items/bowling.hpp"
#include "items/cake.hpp"
#include "items/plunger.hpp"
#include "items/rubber_ball.hpp"
#include "modes/world.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"

PowerupManager* powerup_manager=0;

//-----------------------------------------------------------------------------
/** The constructor initialises everything to zero. */
PowerupManager::PowerupManager()
{
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

        //FIXME: I'm not sure if this is OK or if I need to ->drop(), or delete them, or...
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
/** Loads all powerups from the powerup.xml file.
 *  Uses num_karts to get the good weights
 */
void PowerupManager::loadAllPowerups(unsigned int num_karts)
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
            LoadPowerup(type, *node);
        else
        {
            Log::fatal("[PowerupManager]", "Can't find item '%s' from powerup.xml, entry %d/",
                        name.c_str(), i+1);
            exit(-1);
        }
    }
    //loadWeights takes care of loading specific weight in follow-the-leader
    //They also vary depending on rank in race, so they use the usual position classes
    loadWeights(*root, num_karts, "first",   POSITION_FIRST      );
    loadWeights(*root, num_karts, "top33",   POSITION_TOP33      );
    loadWeights(*root, num_karts, "mid33",   POSITION_MID33      );
    loadWeights(*root, num_karts, "end33",   POSITION_END33      );
    loadWeights(*root, num_karts, "last" ,   POSITION_LAST       );
    loadWeights(*root, num_karts, "battle" , POSITION_BATTLE_MODE);
    loadWeights(*root, num_karts, "soccer" , POSITION_SOCCER_MODE);
    loadWeights(*root, num_karts, "tuto",    POSITION_TUTORIAL_MODE);

    delete root;

}  // loadAllPowerups

//-----------------------------------------------------------------------------
/** Loads the data for one particular powerup. For bowling ball, plunger, and
 *  cake static members in the appropriate classes are called to store
 *  additional information for those objects.
 *  \param type The type of the powerup.
 *  \param node The XML node with the data for this powerup.
 */
void PowerupManager::LoadPowerup(PowerupType type, const XMLNode &node)
{
    std::string icon_file("");
    node.get("icon", &icon_file);

#ifdef DEBUG
    if (icon_file.size() == 0)
    {
        Log::debug("[PowerupManager]", "Cannot load powerup %i, no 'icon' attribute under XML node", type);
        assert(false);
    }
#endif

    m_all_icons[type] = material_manager->getMaterial(icon_file,
                                  /* full_path */     false,
                                  /*make_permanent */ true);


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
            o<<"Can't load model '"<<model<<"' for powerup type '"<<type<<"', aborting.";
            throw std::runtime_error(o.str());
        }
        m_all_meshes[type]->grab();
    }
    else
    {
        m_all_meshes[type] = 0;
        m_all_extends[type] = btVector3(0.0f,0.0f,0.0f);
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
        default:;
    }   // switch
}   // LoadNode

// ----------------------------------------------------------------------------
/** Loads a weight list specified in powerup.xml.
 *  Calculates a linear average of the weight specified for the reference points
 *  of num_karts higher and lower (unless it exactly matches one)
 *  \param root The root node of powerup.xml
 *  \param num_karts The number of karts in the race
 *  \param class_name The name of the position class to load.
 *  \param position_class The class for which the weights are read.
 */
void PowerupManager::loadWeights(const XMLNode &root,
                                 unsigned int num_karts,
                                 const std::string &class_name,
                                 PositionClass position_class)
{
    const XMLNode *class_node = root.getNode(class_name), *reference_points_node = root.getNode("reference_points");
    std::string ref;
    // Those are the strings which will contain the names of the nodes to load inside the class node
    std::string w_inf("w"), w_sup("w"), w_inf_multi("w-multi"), w_sup_multi("w-multi");
    // Those are the strings where the raw weight values will be put when read from the XML nodes
    std::string values_inf(""), values_inf_multi(""), values_sup(""), values_sup_multi("");
    int inf_num = 1, sup_num = 1; //The number of karts associated with the reference points

    /** Adds to w_inf and w_sup the suffixe of the closest num_karts reference classes
     */
    if(position_class!=POSITION_BATTLE_MODE &&
       position_class!=POSITION_TUTORIAL_MODE &&
       position_class!=POSITION_SOCCER_MODE)
    {
        //First step : get the reference points

        if(!reference_points_node)
        {
            Log::error("[PowerupManager]","No reference points found, "
                         "probabilities will be incorrect");
        }

        if (reference_points_node)
        {
            if(race_manager->isFollowMode())
            {
                w_inf = w_sup = "wf";
            }
            
            else
            {
                int i = 0;
                while(1)
                {
                    i++;
                    ref = "ref" + StringUtils::toString(i);
                    std::string ref_pos;
                    if (!reference_points_node->get(ref, &ref_pos))
                        break;
                    unsigned int ref_value;
                    if (!StringUtils::parseString(ref_pos.c_str(), &ref_value))
                    {
                        Log::error("[PowerupManager]","Incorrect reference point values, "
                                     "probabilities will be incorrect");
                        continue;
                    }
                    if (ref_value <= num_karts)
                    {
                        inf_num = ref_value;
                        //Useful if config is edited so num_kart > highest reference position
                        sup_num = ref_value;
                        w_inf = "w" + StringUtils::toString(i);
                        w_inf_multi = "w-multi" + StringUtils::toString(i);
                    }
                    //No else if, it can be equal
                    if (ref_value >= num_karts)
                    {
                        sup_num = ref_value;
                        w_sup = w_sup + StringUtils::toString(i);
                        w_sup_multi = w_sup_multi + StringUtils::toString(i);
                        break;
                    }
                }
            } //else
        }

        if(w_inf=="w" || w_sup=="w")
        {
            w_inf = "w3";            //fallback values
            w_inf_multi = "w-multi3";//fallback values
            w_sup = "w3";            //fallback values
            w_sup_multi = "w-multi3";//fallback values
            inf_num = sup_num = num_karts;//fallback values
            Log::error("[PowerupManager]","Uncorrect reference points in powerup.xml."
                      "%d karts - fallback probabilities will be used.",
                      num_karts);
        }

        // Now w_inf and w_sup contain the weight classes to use
        // for the weights calculations

    }
    
    //Second step : load the class_node values
    if(class_node)
    {
        class_node->get(w_inf,       &values_inf       );
        class_node->get(w_inf_multi, &values_inf_multi );
        class_node->get(w_sup,       &values_sup       );
        class_node->get(w_sup_multi, &values_sup_multi );
    }

    if(!class_node || values_inf=="" || values_inf_multi==""
                   || values_sup=="" || values_sup_multi=="")
    {
        Log::error("[PowerupManager]", "No weights found for class '%s'"
                    " - probabilities will be incorrect.",
                    class_name.c_str());
        return;
    }

    //Third step : split in discrete values
    std::vector<std::string> weight_list_inf = StringUtils::split(values_inf+" "+values_inf_multi,' ');
    std::vector<std::string> weight_list_sup = StringUtils::split(values_sup+" "+values_sup_multi,' ');

    std::vector<std::string>::iterator i=weight_list_inf.begin();
    while(i!=weight_list_inf.end())
    {
        if(*i=="")
        {
            std::vector<std::string>::iterator next=weight_list_inf.erase(i);
            i=next;
        }
        else
            i++;
    }

    i=weight_list_sup.begin();
    while(i!=weight_list_sup.end())
    {
        if(*i=="")
        {
            std::vector<std::string>::iterator next=weight_list_sup.erase(i);
            i=next;
        }
        else
            i++;
    }

    // Fill missing entries with 0s
    while(weight_list_inf.size()<2*(int)POWERUP_LAST)
        weight_list_inf.push_back(0);

    while(weight_list_sup.size()<2*(int)POWERUP_LAST)
        weight_list_sup.push_back(0);

    //This also tests that the two lists are of equal size
    if(weight_list_inf.size()!=2*(int)POWERUP_LAST ||
       weight_list_sup.size()!=2*(int)POWERUP_LAST)
    {
        Log::error("[PowerupManager]", "Incorrect number of weights found in class '%s':",
               class_name.c_str());
        Log::error("[PowerupManager]", "%d and %d instead of twice %d - probabilities will be incorrect.",
               (int)weight_list_inf.size(), (int)weight_list_sup.size(), (int)POWERUP_LAST);
        return;
    }

    //Fourth step : finally compute the searched values
    for(unsigned int i=0; i<2*(int)POWERUP_LAST; i++)
    {
        int weight_inf, weight_sup;

        if (!StringUtils::parseString(weight_list_inf[i].c_str(), &weight_inf))
        {
            Log::error("[PowerupManager]","Incorrect powerup weight values, "
                                     "probabilities will be incorrect");
            weight_inf = 0;//default to zero
        }

        if (!StringUtils::parseString(weight_list_sup[i].c_str(), &weight_sup))
        {
            Log::error("[PowerupManager]","Incorrect powerup weight values, "
                                     "probabilities will be incorrect");
            weight_sup = 0;
        }

        //Do a linear average of the inf and sup values
        //Use float calculations to reduce the rounding errors
        float ref_diff = (float) (sup_num - inf_num);
        float sup_diff = (float) (sup_num - num_karts);
        float inf_diff = (float) (num_karts - inf_num);
        float weight = (sup_diff/ref_diff)*weight_inf
                      +(inf_diff/ref_diff)*weight_sup;
        int rounded_weight = (int) weight;

        m_weights[position_class].push_back(rounded_weight);
    }
}   // loadWeights


// ----------------------------------------------------------------------------
/** This function set up various arrays for faster lookup later. It first
 *  determines which race position correspond to which position class, and
 *  then filters which powerups are available (not all powerups might be
 *  available in all races). It sets up two arrays: m_position_to_class_[inf/sup]
 *  which maps which race position corresponds to which position class
 *  \param num_kart Number of karts.
 */
void PowerupManager::updateWeightsForRace(unsigned int num_karts)
{
    if (num_karts == 0)
        return;

    //This loads the appropriate weights
    loadAllPowerups(num_karts);

    // In battle mode no positions exist, so use only position 1
    unsigned int end_position = (race_manager->isBattleMode() ||
        race_manager->isSoccerMode()) ? 1 : num_karts;

    m_position_to_class_inf.clear();
    m_position_to_class_sup.clear();
    m_position_to_class_cutoff.clear();

    for(unsigned int position =1; position <= end_position; position++)
    {
        // Set up the mapping of position to position class,
        // Using a linear average of the superior and inferor position class
        // -------------------------------------------------
        m_position_to_class_inf.push_back(convertPositionToClass(num_karts, position, false));
        m_position_to_class_sup.push_back(convertPositionToClass(num_karts, position, true));
        m_position_to_class_cutoff.push_back(convertPositionToClassWeight(num_karts, position));
    }

    // Then determine which items are available.
    if (race_manager->isBattleMode())
         updatePowerupClass(POSITION_BATTLE_MODE);
    else if(race_manager->isSoccerMode())
         updatePowerupClass(POSITION_SOCCER_MODE);
    else if(race_manager->isTutorialMode())
         updatePowerupClass(POSITION_TUTORIAL_MODE);
    else
    {
        //TODO : replace this by a nice loop   
        updatePowerupClass(POSITION_FIRST);
        updatePowerupClass(POSITION_TOP33);
        updatePowerupClass(POSITION_MID33);
        updatePowerupClass(POSITION_END33);
        updatePowerupClass(POSITION_LAST);
    }
}   // updateWeightsForRace

// ----------------------------------------------------------------------------
/** This function update the powerup array of a position class
 *  \param pos_class The position class to update
 */
void PowerupManager::updatePowerupClass(PowerupManager::PositionClass pos_class)
{

    m_powerups_for_reference_pos[pos_class].clear();

    // This loop actually goes over the powerups twice: first the single item version,
    // then the multi-item version. The assignment to type takes care of this
    for(unsigned int i= POWERUP_FIRST; i<=2*POWERUP_LAST; i++)
    {
        PowerupType type =
            (PowerupType) ((i<=POWERUP_LAST) ? i
                                             : i+POWERUP_FIRST);
        unsigned int w =m_weights[pos_class][i-POWERUP_FIRST];
        for(unsigned int j=0; j<w; j++)
            m_powerups_for_reference_pos[pos_class].push_back(type);
    }   // for type in [POWERUP_FIRST, POWERUP_LAST]
} //updatePowerupClass

// ----------------------------------------------------------------------------
/** Determines the position class for a given position. If the race is a
 *  battle mode (in which case we don't have a position), always return
 *  'POSITION_BATTLE_MODE' (and in this case only position 1 will be used
 *  for all karts).
 *  \param num_karts  Number of karts in the race.
 *  \param position The position for which to determine the position class.
 *  \param class_sup true if it should send a class matching a >= position
 *                   false if it should match a <= position
 */
PowerupManager::PositionClass
               PowerupManager::convertPositionToClass(unsigned int num_karts,
                                                     unsigned int position, bool class_sup)
{
    if(race_manager->isBattleMode()) return POSITION_BATTLE_MODE;
    if(race_manager->isSoccerMode()) return POSITION_SOCCER_MODE;
    if(race_manager->isTutorialMode()) return POSITION_TUTORIAL_MODE;
    if(position==1)         return POSITION_FIRST;
    if(position==num_karts) return POSITION_LAST;

    // Now num_karts must be >=3, since position <=num_players

    //Special case for Follow the Leader : top33 is mapped to the first non-leader kart
    if (race_manager->isFollowMode())
    {
        float third = (float) (num_karts-2)/3.0f;

        if(position == 2) return POSITION_TOP33;

        if (class_sup)
        {
            if(position <= 2 + third) return POSITION_MID33;
            else if(position <= 2 + 2*third) return POSITION_END33;
            else return POSITION_LAST;
        }
        else
        {
            if(position >= 2 + 2*third) return POSITION_END33;
            else if(position >= 2 + third) return POSITION_MID33;
            else return POSITION_TOP33;
        }
    }

    //Three points divide a line in 4 sections
    float quarter = (float) (num_karts-1)/4.0f;
    if (class_sup)
    {
        if(position <= 1 + quarter) return POSITION_TOP33;
        else if(position <= 1 + 2*quarter) return POSITION_MID33;
        else if(position <= 1 + 3*quarter) return POSITION_END33;
        else return POSITION_LAST;
    }
    else
    {
        if(position >= 1 + 3*quarter) return POSITION_END33;
        else if(position >= 1 + 2*quarter) return POSITION_MID33;
        else if(position >= 1 + 1*quarter) return POSITION_TOP33;
        else return POSITION_FIRST;
    }
}   // convertPositionToClass

// ----------------------------------------------------------------------------
/** Computes the proportion of the inf position class for a given position.
 *  \param num_karts  Number of karts in the race.
 *  \param position The position for which to determine the proportion.
 */
unsigned int PowerupManager::convertPositionToClassWeight(unsigned int num_karts,
                                                     unsigned int position)
{
    if(race_manager->isBattleMode() || race_manager->isSoccerMode() ||
       race_manager->isTutorialMode() || position==1 || position==num_karts)
        return 1;

    // Now num_karts must be >=3, since position <=num_players

    //Special case for Follow the Leader : top33 is mapped to the first non-leader kart
    if (race_manager->isFollowMode())
    {
        float third = (float) (num_karts-2)/3.0f;

        if(position == 2) return 1;

        if(position <= 2 + third) return (int) RAND_CLASS_RANGE-((RAND_CLASS_RANGE*(position - 2))/third);
        else if(position <= 2 + 2*third) return (int) RAND_CLASS_RANGE-((RAND_CLASS_RANGE*(position - (2+third)))/third);
        else return (int) RAND_CLASS_RANGE-((RAND_CLASS_RANGE*(position - (2+2*third)))/third);
    }

    //Three points divide a line in 4 sections
    float quarter = (float) (num_karts-1)/4.0f;

    if(position <= 1 + quarter) return (int) RAND_CLASS_RANGE-((RAND_CLASS_RANGE*(position - 1))/quarter);
    else if(position <= 1 + 2*quarter) return (int) RAND_CLASS_RANGE-((RAND_CLASS_RANGE*(position - (1+quarter)))/quarter);
    else if(position <= 1 + 3*quarter) return (int) RAND_CLASS_RANGE-((RAND_CLASS_RANGE*(position - (1+2*quarter)))/quarter);
    else return  (int) RAND_CLASS_RANGE-(RAND_CLASS_RANGE*((position - (1+3*quarter)))/quarter);
}   // convertPositionToClassWeight

// ----------------------------------------------------------------------------
/** Returns a random powerup for a kart at a given position. If the race mode
 *  is a battle, the position is actually not used and a randomly selected
 *  item for POSITION_BATTLE_MODE is returned. This function takes the weights
 *  specified for all items into account by using a list which contains all
 *  items depending on the weights defined. See updateWeightsForRace()
 *  \param pos Position of the kart (1<=pos<=number of karts) - ignored in
 *         case of a battle mode.
 *  \param n Number of times this item is given to the kart
 */
PowerupManager::PowerupType PowerupManager::getRandomPowerup(unsigned int pos,
                                                             unsigned int *n)
{
   int random = rand();
   int random_class = random%RAND_CLASS_RANGE;

    //First step : select at random a class according to the weights of class for this position
    // Positions start with 1, while the index starts with 0 - so subtract 1
    PositionClass pos_class =
        (race_manager->isBattleMode() ? POSITION_BATTLE_MODE :
         race_manager->isSoccerMode() ? POSITION_SOCCER_MODE :
         race_manager->isTutorialMode() ? POSITION_TUTORIAL_MODE :
         random_class > m_position_to_class_cutoff[pos-1] ? m_position_to_class_inf[pos-1] :
                                                         m_position_to_class_sup[pos-1]);

    //Second step : select an item at random
    
   int random_item = (random/RAND_CLASS_RANGE)%m_powerups_for_reference_pos[pos_class].size();

    int i=m_powerups_for_reference_pos[pos_class][random_item];
    if(i>=POWERUP_MAX)
    {
        i -= POWERUP_MAX;
        *n = 3;
    }
    else
        *n=1;
    return (PowerupType)i;
}   // getRandomPowerup
