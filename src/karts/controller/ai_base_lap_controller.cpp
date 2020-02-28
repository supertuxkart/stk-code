//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2009  Eduardo Hernandez Munoz
//  Copyright (C) 2009-2015  Joerg Henrichs
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

#include "karts/controller/ai_base_lap_controller.hpp"

#include <assert.h>

#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "karts/controller/ai_properties.hpp"
#include "modes/linear_world.hpp"
#include "tracks/drive_graph.hpp"
#include "tracks/track.hpp"
#include "utils/constants.hpp"


/**
This is the base class for all AIs. At this stage there are two similar
AIs: one is the SkiddingAI, which is the AI used in lap based races
(including follow-the-leader mode), the other one is the end controller,
I.e. the controller that takes over from a player (or AI) when the race is
finished.

This base class defines some basic operations:
- It takes care on which part of the DriveGraph the AI currently is.
- It determines which path the AI should take (in case of shortcuts
  or forks in the road).

At race start and every time a new lap is started, the AI will compute the
path the kart is taking this lap (computePath). At this stage the decision
which road in case of shortcut to take is purely random. It stores the
information in two arrays:
  m_successor_index[i] stores which successor to take from node i.
       The successor is a number between 0 and number_of_successors - 1.
  m_next_node_index[i] stores the actual index of the graph node that
       follows after node i.
Depending on operation one of the other data is more useful, so this
class stores both information to avoid looking it up over and over.
Once this is done (still in computePath), the array m_all_look_aheads is
computed. This array stores for each quad a list of the next (atm) 10 quads.
This is used when the AI is selecting where to drive next, and it will just
pass the list of next quads to findRoadSector.

Note that the quad graph information is stored for every quad in the quad
graph, even if the quad is not on the path chosen. This is necessary since
it can happen that a kart ends up on a path not choses (e.g. perhaps it was
pushed on that part, or couldn't get a sharp corner).

In update(), which gets called one per frame per AI, this object will
determine the quad the kart is currently on (which is then used to determine
where the kart will be driving to). This uses the m_all_look_aheads to
speed up this process (since the kart is likely to be either on the same
quad as it was before, or the next quad in the m_all_look_aheads list).

It will also check if the kart is stuck:
this is done by maintaining a list of times when the kart hits the track. If
(atm) more than 3 collisions happen in 1.5 seconds, the kart is considered
stuck and will trigger a rescue (due to the pushback from the track it will
take some time if a kart is really stuck before it will hit the track again).

This base class also contains some convenience functions which are useful
in all AIs, e.g.:
-  steerToPoint: determine the steering angle to use depending on the
   current location and the point the kart is driving to.
-  normalizeAngle: To normalise the steering angle to be in [-PI,PI].
-  setSteering: Converts the steering angle into a steering fraction
   in [-1,1].

*/
AIBaseLapController::AIBaseLapController(AbstractKart *kart)
                   : AIBaseController(kart)
{

    if (!RaceManager::get()->isBattleMode() &&
        RaceManager::get()->getMinorMode()!=RaceManager::MINOR_MODE_SOCCER)
    {
        m_world     = dynamic_cast<LinearWorld*>(World::getWorld());
        m_track     = Track::getCurrentTrack();
        computePath();
    }
    else
    {
        // Those variables are not defined in a battle mode (m_world is
        // a linear world, since it assumes the existance of drivelines)
        m_world           = NULL;
        m_track           = NULL;
        m_next_node_index.clear();
        m_all_look_aheads.clear();
        m_successor_index.clear();
    }   // if battle mode
    // Don't call our own setControllerName, since this will add a
    // billboard showing 'AIBaseLapController' to the kar.
    Controller::setControllerName("AIBaseLapController");
}   // AIBaseLapController

//-----------------------------------------------------------------------------
void AIBaseLapController::reset()
{
    AIBaseController::reset();
}   // reset



//-----------------------------------------------------------------------------
/** Triggers a recomputation of the path to use, so that the AI does not
 *  always use the same way.
 */
void  AIBaseLapController::newLap(int lap)
{
    if(lap>0)
    {
        computePath();
    }
}   // newLap

//-----------------------------------------------------------------------------
/** Computes a path for the AI to follow. This function is called at race
 *  start and every time a new lap is started. Recomputing the path every
 *  time will mean that the kart will not always take the same path, but
 *  (potentially) vary from lap to lap. At this stage the decision is done
 *  randomly. The AI could be improved by collecting more information about
 *  each branch of a track, and selecting the 'appropriate' one (e.g. if the
 *  AI is far ahead, chose a longer/slower path).
 */
void AIBaseLapController::computePath()
{
    m_next_node_index.resize(DriveGraph::get()->getNumNodes());
    m_successor_index.resize(DriveGraph::get()->getNumNodes());
    std::vector<unsigned int> next;
    for(unsigned int i=0; i<DriveGraph::get()->getNumNodes(); i++)
    {
        next.clear();
        // Get all successors the AI is allowed to take.
        DriveGraph::get()->getSuccessors(i, next, /*for_ai*/true);
        // In case of short cuts hidden for the AI it can be that a node
        // might not have a successor (since the first and last edge of
        // a hidden shortcut is ignored). Since in the case that the AI
        // ends up on a short cut (e.g. by accident) and doesn't have an
        // allowed way to drive, it should still be able to drive, so add
        // the non-AI successors of that node in this case.
        if(next.size()==0)
            DriveGraph::get()->getSuccessors(i, next, /*for_ai*/false);
        // For now pick one part on random, which is not adjusted during the
        // race. Long term statistics might be gathered to determine the
        // best way, potentially depending on race position etc.
        int r = rand();
        int indx = (int)( r / ((float)(RAND_MAX)+1.0f) * next.size() );
        // In case of rounding errors0
        if(indx>=(int)next.size()) indx--;
        m_successor_index[i] = indx;
        assert(indx <(int)next.size() && indx>=0);
        m_next_node_index[i] = next[indx];
    }

    const unsigned int look_ahead=10;
    // Now compute for each node in the graph the list of the next 'look_ahead'
    // graph nodes. This is the list of node that is tested in checkCrashes.
    // If the look_ahead is too big, the AI can skip loops (see
    // Graph::findRoadSector for details), if it's too short the AI won't
    // find too good a driveline. Note that in general this list should
    // be computed recursively, but since the AI for now is using only
    // (randomly picked) path this is fine
    m_all_look_aheads.resize(DriveGraph::get()->getNumNodes());
    for(unsigned int i=0; i<DriveGraph::get()->getNumNodes(); i++)
    {
        std::vector<int> l;
        int current = i;
        for(unsigned int j=0; j<look_ahead; j++)
        {
            assert(current < (int)m_next_node_index.size());
            l.push_back(m_next_node_index[current]);
            current = m_next_node_index[current];
        }   // for j<look_ahead
        m_all_look_aheads[i] = l;
    }
}   // computePath

//-----------------------------------------------------------------------------
/** Updates the ai base controller each time step. Note that any calls to
 *  isStuck() must be done before update is called, since update will call
 *  AIBaseController::update() which will reset the isStuck flag!
 *  \param ticks Number of physics time steps - should be 1.
 */
void AIBaseLapController::update(int ticks)
{
    AIBaseController::update(ticks);
    if(DriveGraph::get() && m_world)
    {
        // Update the current node:
        int old_node = m_track_node;
        if(m_track_node!=Graph::UNKNOWN_SECTOR)
        {
            DriveGraph::get()->findRoadSector(m_kart->getXYZ(), &m_track_node,
                &m_all_look_aheads[m_track_node]);
        }
        // If we can't find a proper place on the track, to a broader search
        // on off-track locations.
        if(m_track_node==Graph::UNKNOWN_SECTOR)
        {
            m_track_node = DriveGraph::get()->findOutOfRoadSector(m_kart->getXYZ());
        }
        // IF the AI is off track (or on a branch of the track it did not
        // select to be on), keep the old position.
        if(m_track_node==Graph::UNKNOWN_SECTOR ||
            m_next_node_index[m_track_node]==-1)
            m_track_node = old_node;
    }
}   // update

//-----------------------------------------------------------------------------
/** Returns the next sector of the given sector index. This is used
 *  for branches in the quad graph to select which way the AI kart should
 *  go. This is a very simple implementation that always returns the first
 *  successor, but it can be overridden to allow a better selection.
 *  \param index Index of the graph node for which the successor is searched.
 *  \return Returns the successor of this graph node.
 */
unsigned int AIBaseLapController::getNextSector(unsigned int index)
{
    std::vector<unsigned int> successors;
    DriveGraph::get()->getSuccessors(index, successors);
    return successors[0];
}   // getNextSector

//-----------------------------------------------------------------------------
/** This function steers towards a given angle. It also takes a plunger
 ** attached to this kart into account by modifying the actual steer angle
 *  somewhat to simulate driving without seeing.
 */
float AIBaseLapController::steerToAngle(const unsigned int sector,
                                     const float add_angle)
{
    float angle = DriveGraph::get()->getAngleToNext(sector,
                                                    getNextSector(sector));

    //Desired angle minus current angle equals how many angles to turn
    float steer_angle = angle - m_kart->getHeading();

    if(m_kart->getBlockedByPlungerTicks()>0)
        steer_angle += add_angle*0.2f;
    else
        steer_angle += add_angle;
    steer_angle = normalizeAngle( steer_angle );

    return steer_angle;
}   // steerToAngle
