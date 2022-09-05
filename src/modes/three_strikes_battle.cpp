//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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

#include "modes/three_strikes_battle.hpp"

#include "main_loop.hpp"
#include "audio/music_manager.hpp"
#include "config/user_config.hpp"
#include "graphics/camera.hpp"
#include "graphics/irr_driver.hpp"
#include <ge_render_info.hpp>
#include "io/file_manager.hpp"
#include "karts/kart.hpp"
#include "karts/controller/spare_tire_ai.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "physics/physics.hpp"
#include "states_screens/race_gui_base.hpp"
#include "tracks/arena_graph.hpp"
#include "tracks/arena_node.hpp"
#include "tracks/terrain_info.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/constants.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <algorithm>
#include <deque>
#include <string>
#include <IMeshSceneNode.h>

//-----------------------------------------------------------------------------
/** Constructor. Sets up the clock mode etc.
 */
ThreeStrikesBattle::ThreeStrikesBattle() : WorldWithRank()
{
    WorldStatus::setClockMode(CLOCK_CHRONO);
    m_use_highscores = false;
    m_insert_tire = 0;

    m_tire = irr_driver->getMesh(file_manager->getAsset(FileManager::MODEL,
                                 "tire.spm") );
    irr_driver->grabAllTextures(m_tire);

    m_total_rescue = 0;
    m_frame_count = 0;
    m_start_time = irr_driver->getRealTime();
    m_total_hit = 0;

}   // ThreeStrikesBattle

//-----------------------------------------------------------------------------
/** Initialises the three strikes battle. It sets up the data structure
 *  to keep track of points etc. for each kart.
 */
void ThreeStrikesBattle::init()
{
    WorldWithRank::init();
    m_display_rank = false;
    m_kart_info.resize(m_karts.size());
}   // ThreeStrikesBattle

//-----------------------------------------------------------------------------
/** Destructor. Clears all internal data structures, and removes the tire mesh
 *  from the mesh cache.
 */
ThreeStrikesBattle::~ThreeStrikesBattle()
{
    m_tires.clearWithoutDeleting();
    m_spare_tire_karts.clear();

    irr_driver->dropAllTextures(m_tire);
    // Remove the mesh from the cache so that the mesh is properly
    // freed once all refernces to it (which will happen once all
    // karts are being freed, which would have a pointer to this mesh)
    irr_driver->removeMeshFromCache(m_tire);
}   // ~ThreeStrikesBattle

//-----------------------------------------------------------------------------
/** Called when a battle is restarted.
 */
void ThreeStrikesBattle::reset(bool restart)
{
    WorldWithRank::reset(restart);

    float next_spawn_time =
        RaceManager::get()->getDifficulty() == RaceManager::DIFFICULTY_BEST ? 40.0f :
        RaceManager::get()->getDifficulty() == RaceManager::DIFFICULTY_HARD ? 30.0f :
        RaceManager::get()->getDifficulty() == RaceManager::DIFFICULTY_MEDIUM ?
        25.0f : 20.0f;
    m_next_sta_spawn_ticks = stk_config->time2Ticks(next_spawn_time);

    const unsigned int kart_amount = (unsigned int)m_karts.size();
    for(unsigned int n=0; n<kart_amount; n++)
    {
        if (dynamic_cast<SpareTireAI*>(m_karts[n]->getController()) != NULL)
        {
            // STA has no life
            m_kart_info[n].m_lives = 0;
        }
        else
        {
            m_kart_info[n].m_lives = 3;
        }

        // no positions in this mode
        m_karts[n]->setPosition(-1);

        scene::ISceneNode* kart_node = m_karts[n]->getNode();

        for (unsigned i = 0; i < kart_node->getChildren().size(); i++)
        {
            scene::ISceneNode* curr = kart_node->getChildren()[i];

            if (core::stringc(curr->getName()) == "tire1")
            {
                curr->setVisible(true);
            }
            else if (core::stringc(curr->getName()) == "tire2")
            {
                curr->setVisible(true);
            }
        }

    }// next kart

    // remove old battle events
    m_battle_events.clear();

    // add initial battle event
    BattleEvent evt;
    evt.m_time = 0.0f;
    evt.m_kart_info = m_kart_info;
    m_battle_events.push_back(evt);

    TrackObject *obj;
    for_in(obj, m_tires)
    {
        Track::getCurrentTrack()->getTrackObjectManager()->removeObject(obj);
    }
    m_tires.clearWithoutDeleting();

    // Finish all spare tire karts first
    if (!m_spare_tire_karts.empty())
    {
        updateKartRanks();
        for (unsigned int i = 0; i < m_spare_tire_karts.size(); i++)
        {
             m_spare_tire_karts[i]->finishedRace(0.0f);
             m_spare_tire_karts[i]->getNode()->setVisible(false);
             m_eliminated_karts++;
        }
    }
}   // reset

//-----------------------------------------------------------------------------
/** Adds two tires to each of the kart. The tires are used to represent
 *  lifes.
 *  \param kart The pointer to the kart (not used here).
 *  \param node The scene node of this kart.
 */
void ThreeStrikesBattle::kartAdded(AbstractKart* kart, scene::ISceneNode* node)
{
    if (!node)
        return;
    if (kart->getType() == RaceManager::KartType::KT_SPARE_TIRE)
    {
        // Add heart billboard above it
        std::string heart_path =
            file_manager->getAsset(FileManager::GUI_ICON, "heart.png");
        float height = kart->getKartHeight() + 0.5f;

        scene::ISceneNode* billboard = irr_driver->addBillboard
            (core::dimension2d<irr::f32>(0.8f, 0.8f), heart_path,
            kart->getNode());
        billboard->setPosition(core::vector3df(0, height, 0));
        return;
    }

    float coord = -kart->getKartLength()*0.5f;

    scene::ISceneNode* tire_node = irr_driver->addMesh(m_tire, "3strikestire", node);
    tire_node->setPosition(core::vector3df(-0.16f, 0.3f, coord - 0.25f));
    tire_node->setScale(core::vector3df(0.4f, 0.4f, 0.4f));
    tire_node->setRotation(core::vector3df(90.0f, 0.0f, 0.0f));
    tire_node->setName("tire1");

    tire_node = irr_driver->addMesh(m_tire, "3strikestire", node);
    tire_node->setPosition(core::vector3df(0.16f, 0.3f, coord - 0.25f));
    tire_node->setScale(core::vector3df(0.4f, 0.4f, 0.4f));
    tire_node->setRotation(core::vector3df(90.0f, 0.0f, 0.0f));
    tire_node->setName("tire2");
}   // kartAdded

//-----------------------------------------------------------------------------
/** Called when a kart is hit.
 *  \param kart_id The world kart id of the kart that was hit.
 *  \param hitter The world kart id of the kart who hit(-1 if none).
 */
bool ThreeStrikesBattle::kartHit(int kart_id, int hitter)
{
    if (isRaceOver()) return false;

    SpareTireAI* sta =
        dynamic_cast<SpareTireAI*>(m_karts[kart_id]->getController());
    if (sta)
    {
        // Unspawn the spare tire kart if it get hit
        sta->unspawn();
        return false;
    }

    assert(kart_id < (int)m_karts.size());
    // make kart lose a life, ignore if in profiling mode
    if (!UserConfigParams::m_arena_ai_stats)
        m_kart_info[kart_id].m_lives--;
    else
        m_total_hit++;

    // record event
    BattleEvent evt;
    evt.m_time = getTime();
    evt.m_kart_info = m_kart_info;
    m_battle_events.push_back(evt);

    updateKartRanks();
    // check if kart is 'dead'
    if (m_kart_info[kart_id].m_lives < 1)
    {
        if (getCurrentNumPlayers())
            eliminateKart(kart_id, /*notify_of_elimination*/ true);
        m_karts[kart_id]->finishedRace(WorldStatus::getTime());
        scene::ISceneNode** wheels = m_karts[kart_id]->getKartModel()
                                                     ->getWheelNodes();
        if(wheels[0]) wheels[0]->setVisible(false);
        if(wheels[1]) wheels[1]->setVisible(false);
        if(wheels[2]) wheels[2]->setVisible(false);
        if(wheels[3]) wheels[3]->setVisible(false);
        // Find a camera of the kart with the most lives ("leader"), and
        // attach all cameras for this kart to the leader.
        int max_lives = 0;
        AbstractKart *leader = NULL;
        for(unsigned int i=0; i<getNumKarts(); i++)
        {
            AbstractKart * const kart = getKart(i);
            if(kart->isEliminated() || kart->hasFinishedRace() ||
                kart->getWorldKartId()==(unsigned)kart_id) continue;
            if(m_kart_info[i].m_lives > max_lives)
            {
                leader = kart;
                max_lives = m_kart_info[i].m_lives;
            }
        }
        // leader could be 0 if the last two karts hit each other in
        // the same frame
        if(leader && getCurrentNumPlayers())
        {
            for(unsigned int i=0; i<Camera::getNumCameras(); i++)
            {
                Camera *camera = Camera::getCamera(i);
                if(camera->getKart()->getWorldKartId()==(unsigned)kart_id)
                {
                    camera->setMode(Camera::CM_NORMAL);
                    camera->setKart(leader);
                }
            }   // for in < number of cameras
        }   // if leader
        m_insert_tire = 4;
    }

    const unsigned int NUM_KARTS = getNumKarts();
    int num_karts_many_lives = 0;

    for (unsigned int n = 0; n < NUM_KARTS; ++n)
    {
        if (m_kart_info[n].m_lives > 1) num_karts_many_lives++;
    }

    // when almost over, use fast music
    if (num_karts_many_lives<=1 && !m_faster_music_active)
    {
        music_manager->switchToFastMusic();
        m_faster_music_active = true;
    }

    scene::ISceneNode* kart_node = m_karts[kart_id]->getNode();
    for (unsigned i = 0; i < kart_node->getChildren().size(); i++)
    {
        scene::ISceneNode* curr = kart_node->getChildren()[i];

        if (core::stringc(curr->getName()) == "tire1")
        {
            curr->setVisible(m_kart_info[kart_id].m_lives >= 3);
        }
        else if (core::stringc(curr->getName()) == "tire2")
        {
            curr->setVisible(m_kart_info[kart_id].m_lives >= 2);
        }
    }

    // schedule a tire to be thrown away (but can't do it in this callback
    // because the caller is currently iterating the list of track objects)
    m_insert_tire++;
    core::vector3df wheel_pos(m_karts[kart_id]->getKartWidth()*0.5f,
                              0.0f, 0.0f);
    m_tire_position = kart_node->getPosition() + wheel_pos;
    m_tire_rotation = 0;
    if(m_insert_tire > 1)
    {
        m_tire_position = kart_node->getPosition();
        m_tire_rotation = m_karts[kart_id]->getHeading();
    }

    for(unsigned int i=0; i<4; i++)
    {
        m_tire_offsets[i] = m_karts[kart_id]->getKartModel()
                            ->getWheelGraphicsPosition(i).toIrrVector();
        m_tire_offsets[i].rotateXZBy(-m_tire_rotation / M_PI * 180 + 180);
        m_tire_radius[i] = m_karts[kart_id]->getKartModel()
                                           ->getWheelGraphicsRadius(i);
    }

    m_tire_dir = m_karts[kart_id]->getKartProperties()->getKartDir();
    if(m_insert_tire == 5 && m_karts[kart_id]->isWheeless())
        m_insert_tire = 0;
    return true;
}   // kartHit

//-----------------------------------------------------------------------------
/** Returns the internal identifier for this race.
 */
const std::string& ThreeStrikesBattle::getIdent() const
{
    return IDENT_STRIKES;
}   // getIdent

//-----------------------------------------------------------------------------
/** Update the world and the track.
 *  \param ticks Number of physics time step - should be 1.
 */
void ThreeStrikesBattle::update(int ticks)
{
    WorldWithRank::update(ticks);
    WorldWithRank::updateTrack(ticks);

    spawnSpareTireKarts();
    if (Track::getCurrentTrack()->hasNavMesh())
        updateSectorForKarts();

    // insert blown away tire(s) now if was requested
    while (m_insert_tire > 0)
    {
        std::string tire;
        core::vector3df tire_offset;
        float scale = 0.5f;
        float radius = 0.5f;
        PhysicalObject::BodyTypes body_shape;
        if(m_insert_tire == 1)
        {
            tire_offset = core::vector3df(0.0f, 0.0f, 0.0f);
            tire = file_manager->getAsset(FileManager::MODEL,"tire.spm");
            scale = 0.5f;
            radius = 0.5f;
            body_shape = PhysicalObject::MP_CYLINDER_Y;
        }
        else
        {
            scale = 1.0f;
            body_shape = PhysicalObject::MP_CYLINDER_X;
            radius = m_tire_radius[m_insert_tire-2];
            tire_offset = m_tire_offsets[m_insert_tire-2];
            if     (m_insert_tire == 2)
                tire = m_tire_dir+"/wheel-rear-left.spm";
            else if(m_insert_tire == 3)
                tire = m_tire_dir+"/wheel-front-left.spm";
            else if(m_insert_tire == 4)
                tire = m_tire_dir+"/wheel-front-right.spm";
            else if(m_insert_tire == 5)
                tire = m_tire_dir+"/wheel-rear-right.spm";
            if(!file_manager->fileExists(tire))
            {
                m_insert_tire--;
                if(m_insert_tire == 1)
                    m_insert_tire = 0;
                continue;
            }
        }


        core::vector3df tire_xyz = m_tire_position + tire_offset;
        core::vector3df tire_hpr = core::vector3df(800.0f,0,
                                                   m_tire_rotation *RAD_TO_DEGREE + 180);
        core::vector3df tire_scale(scale,scale,scale);

        PhysicalObject::Settings physics_settings(body_shape,
                                                  radius, /*mass*/15.0f);

        TrackObjectPresentationMesh* tire_presentation =
            new TrackObjectPresentationMesh(tire, tire_xyz, tire_hpr, tire_scale);

#ifdef DEBUG
        tire_presentation->getNode()->setName("Tire on ground");
#endif

        TrackObject* tire_obj = new TrackObject(tire_xyz, tire_hpr, tire_scale,
                                                "movable", tire_presentation,
                                                true /* is_dynamic */,
                                                &physics_settings);
        Track::getCurrentTrack()->getTrackObjectManager()->insertObject(tire_obj);

        // FIXME: orient the force relative to kart orientation
        tire_obj->getPhysicalObject()->getBody()
                ->applyCentralForce(btVector3(60.0f, 0.0f, 0.0f));

        m_insert_tire--;
        if(m_insert_tire == 1)
            m_insert_tire = 0;

        m_tires.push_back(tire_obj);
    }   // while
    if (UserConfigParams::m_arena_ai_stats)
        m_frame_count++;

}   // update

//-----------------------------------------------------------------------------
/** Updates the ranking of the karts.
 */
void ThreeStrikesBattle::updateKartRanks()
{
    beginSetKartPositions();
    // sort karts by their times then give each one its position.
    // in battle-mode, long time = good (meaning he survived longer)

    const unsigned int NUM_KARTS = getNumKarts();

    std::vector<KartValues> karts_list;
    for( unsigned int n = 0; n < NUM_KARTS; ++n )
    {
        KartValues k;
        k.id = n;
        k.time = m_karts[n]->hasFinishedRace() ? (int)m_karts[n]->getFinishTime()
                                               : (int)WorldStatus::getTime();
        k.lives = m_kart_info[n].m_lives;
        karts_list.push_back(k);
    }

    std::sort(karts_list.rbegin(), karts_list.rend());

    for( unsigned int n = 0; n < NUM_KARTS; ++n )
    {
        setKartPosition(karts_list[n].id, n+1);
    }

    endSetKartPositions();
}   // updateKartRank

//-----------------------------------------------------------------------------
/** The battle is over if only one kart is left, or no player kart.
 */
bool ThreeStrikesBattle::isRaceOver()
{
    if (UserConfigParams::m_arena_ai_stats)
        return (irr_driver->getRealTime()-m_start_time)*0.001f > 20.0f;

    // for tests : never over when we have a single player there :)
    if (RaceManager::get()->getNumberOfKarts() - m_spare_tire_karts.size () ==1 &&
        getCurrentNumKarts()==1 &&
        UserConfigParams::m_artist_debug_mode)
    {
        return false;
    }

    return getCurrentNumKarts()==1 || getCurrentNumPlayers()==0;
}   // isRaceOver

//-----------------------------------------------------------------------------
/** Called when the race finishes, i.e. after playing (if necessary) an
 *  end of race animation. It updates the time for all karts still racing,
 *  and then updates the ranks.
 */
void ThreeStrikesBattle::terminateRace()
{
    updateKartRanks();
    WorldWithRank::terminateRace();
}   // terminateRace

//-----------------------------------------------------------------------------
/** Returns the data to display in the race gui.
 */
void ThreeStrikesBattle::getKartsDisplayInfo(
                           std::vector<RaceGUIBase::KartIconDisplayInfo> *info)
{
    const unsigned int kart_amount = getNumKarts();
    for(unsigned int i = 0; i < kart_amount ; i++)
    {
        RaceGUIBase::KartIconDisplayInfo& rank_info = (*info)[i];

        // reset color
        rank_info.lap = -1;

        switch(m_kart_info[i].m_lives)
        {
            case 3:
                rank_info.m_color = video::SColor(255, 0, 255, 0);
                break;
            case 2:
                rank_info.m_color = video::SColor(255, 255, 229, 0);
                break;
            case 1:
                rank_info.m_color = video::SColor(255, 255, 0, 0);
                break;
            case 0:
                rank_info.m_color = video::SColor(128, 128, 128, 0);
                break;
        }

        std::ostringstream oss;
        oss << m_kart_info[i].m_lives;

        rank_info.m_text = oss.str().c_str();
    }
}   // getKartsDisplayInfo

//-----------------------------------------------------------------------------
void ThreeStrikesBattle::enterRaceOverState()
{
    WorldWithRank::enterRaceOverState();

    // Unspawn all spare tire karts if neccesary
    for (unsigned int i = 0; i < m_spare_tire_karts.size(); i++)
    {
        SpareTireAI* sta =
            dynamic_cast<SpareTireAI*>(m_spare_tire_karts[i]->getController());
        assert(sta);
        if (sta->isMoving())
            sta->unspawn();
    }

    if (UserConfigParams::m_arena_ai_stats)
    {
        float runtime = (irr_driver->getRealTime()-m_start_time)*0.001f;
        Log::verbose("Battle AI profiling", "Number of frames: %d, Average FPS: %f",
            m_frame_count, (float)m_frame_count/runtime);
        Log::verbose("Battle AI profiling", "Total rescue: %d , hits %d in %f seconds",
            m_total_rescue, m_total_hit, runtime);
        delete this;
        main_loop->abort();
    }

}   // enterRaceOverState

//-----------------------------------------------------------------------------
bool ThreeStrikesBattle::spareTireKartsSpawned() const
{
    if (m_spare_tire_karts.empty()) return false;

    // Spare tire karts are spawned if at least 1 of them needs update
    SpareTireAI* sta =
        dynamic_cast<SpareTireAI*>(m_spare_tire_karts[0]->getController());
    assert(sta);

    return sta->isMoving();
}   // spareTireKartsSpawned

//-----------------------------------------------------------------------------
void ThreeStrikesBattle::addKartLife(unsigned int id)
{
    m_kart_info[id].m_lives++;
    updateKartRanks();

    scene::ISceneNode* kart_node = m_karts[id]->getNode();
    for (unsigned i = 0; i < kart_node->getChildren().size(); i++)
    {
        scene::ISceneNode* curr = kart_node->getChildren()[i];
        if (core::stringc(curr->getName()) == "tire1")
        {
            curr->setVisible(m_kart_info[id].m_lives >= 3);
        }
        else if (core::stringc(curr->getName()) == "tire2")
        {
            curr->setVisible(m_kart_info[id].m_lives >= 2);
        }
    }

}   // addKartLife

//-----------------------------------------------------------------------------
void ThreeStrikesBattle::spawnSpareTireKarts()
{
    if (m_spare_tire_karts.empty() ||
        getTicksSinceStart() < m_next_sta_spawn_ticks)
        return;

    // The lifespan for sta: inc_factor / period * 1000 / 2
    // So in easier mode the sta lasts longer than spawn period
    float inc_factor, lifespan;
    switch (RaceManager::get()->getDifficulty())
    {
    case RaceManager::DIFFICULTY_BEST: inc_factor = 0.7f;  lifespan = 17.5f;  break;
    case RaceManager::DIFFICULTY_HARD: inc_factor = 0.65f; lifespan = 21.66f; break;
    case RaceManager::DIFFICULTY_EASY: inc_factor = 0.6f;  lifespan = 24.0f;  break;
    default:                           inc_factor = 0.55f; lifespan = 27.5f;  break;
    }

    int lifespan_ticks = stk_config->time2Ticks(lifespan);
    // Spawn spare tire kart when necessary
    m_next_sta_spawn_ticks = int( lifespan_ticks
                                + getTicksSinceStart() * inc_factor
                                + getTicksSinceStart()             );
    int kart_has_few_lives = 0;
    for (unsigned int i = 0; i < m_kart_info.size(); i++)
    {
        if (m_kart_info[i].m_lives > 0 && m_kart_info[i].m_lives < 3)
            kart_has_few_lives++;
    }

    float ratio = kart_has_few_lives / (inc_factor * 2);
    if (ratio < 1.5f) return;
    unsigned int spawn_sta = unsigned(ratio);
    if (spawn_sta > m_spare_tire_karts.size())
        spawn_sta = (int)m_spare_tire_karts.size();
    if (m_race_gui)
    {
        m_race_gui->addMessage(_P("%i spare tire kart has been spawned!",
                                "%i spare tire karts have been spawned!",
                                spawn_sta), NULL, 2.0f);
    }
    for (unsigned int i = 0; i < spawn_sta; i++)
    {
        SpareTireAI* sta = dynamic_cast<SpareTireAI*>
            (m_spare_tire_karts[i]->getController());
        assert(sta);
        sta->spawn(lifespan_ticks);
    }
}   // spawnSpareTireKarts

//-----------------------------------------------------------------------------
void ThreeStrikesBattle::loadCustomModels()
{
    // Pre-add spare tire karts if there are more than certain number of karts
    ArenaGraph* ag = ArenaGraph::get();
    if (ag && m_karts.size() > 4)
    {
        // Spare tire karts only added with large arena
        const int all_nodes = ag->getNumNodes();
        if (all_nodes > 500)
        {
            // Don't create too many spare tire karts
            const unsigned int max_sta_num = unsigned(m_karts.size() * 0.8f);
            unsigned int pos_created = 0;
            std::deque<int> sta_possible_nodes;
            for (int i = 0; i < all_nodes; i++)
                sta_possible_nodes.push_back(i);
            std::vector<btTransform> pos;

            // Fill all current starting position into used first
            for (unsigned int i = 0; i < getNumberOfRescuePositions(); i++)
            {
                int node = -1;
                ag->findRoadSector(getRescueTransform(i).getOrigin(), &node,
                    NULL, true);
                assert(node != -1);
                sta_possible_nodes.erase(std::remove_if(
                    sta_possible_nodes.begin(), sta_possible_nodes.end(),
                    [node](const int n) { return n == node; }),
                    sta_possible_nodes.end());
            }

            // Find random nodes to pre-spawn spare tire karts
            std::random_shuffle(sta_possible_nodes.begin(),
                sta_possible_nodes.end());

            // Compute a random kart list
            std::vector<std::string> sta_list;
            kart_properties_manager->getRandomKartList(max_sta_num, NULL,
                &sta_list);
            if (sta_list.size() != max_sta_num)
                return;

            TerrainInfo terrain;
            while (!sta_possible_nodes.empty())
            {
                const int node = sta_possible_nodes.front();
                const ArenaNode* n = ag->getNode(node);
                btTransform t;
                t.setOrigin(n->getCenter());
                t.setRotation(shortestArcQuat(Vec3(0, 1, 0), n->getNormal()));

                // Make sure starting position is valid for spare tire karts,
                // see #4615
                terrain.update(t.getBasis(),
                    t.getOrigin() + t.getBasis() * Vec3(0, 0.3f, 0));
                Vec3 from = (Vec3)t.getOrigin();
                const KartProperties* kp = kart_properties_manager->getKart(
                    sta_list[pos.size()]);
                if (!kp)
                    return;
                float kh = kp->getMasterKartModel().getHeight();
                //start projection from top of kart
                Vec3 up_offset = terrain.getNormal() * (0.5f * kh);
                from += up_offset;
                Vec3 down = t.getBasis() * Vec3(0, -10000.0f, 0);

                Vec3 hit_point, normal;
                if (!Track::getCurrentTrack()->isOnGround(from, down,
                    &hit_point, &normal, false/*print_warning*/))
                {
                    sta_possible_nodes.pop_front();
                    continue;
                }
                pos.push_back(t);
                pos_created++;
                sta_possible_nodes.pop_front();
                if (pos_created == max_sta_num) break;
            }

            if (pos_created != max_sta_num)
                return;
            assert(sta_list.size() == pos.size());
            // Now add them
            for (unsigned int i = 0; i < pos.size(); i++)
            {
                auto sta = std::make_shared<Kart>(sta_list[i], (int)m_karts.size(),
                    (int)m_karts.size() + 1, pos[i], HANDICAP_NONE,
                    std::make_shared<GE::GERenderInfo>(1.0f));
                sta->init(RaceManager::KartType::KT_SPARE_TIRE);
                sta->setController(new SpareTireAI(sta.get()));

                m_karts.push_back(sta);
                RaceManager::get()->addSpareTireKart(sta_list[i]);

                // Copy STA pointer to m_spare_tire_karts array, allowing them
                // to respawn easily
                m_spare_tire_karts.push_back(sta.get());
            }
            unsigned int sta_num = RaceManager::get()->getNumSpareTireKarts();
            assert(m_spare_tire_karts.size() == sta_num);
            Log::info("ThreeStrikesBattle","%d spare tire kart(s) created.",
                sta_num);
        }
    }
}   // loadCustomModels
