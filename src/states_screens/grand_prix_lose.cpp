//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 SuperTuxKart-Team
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

#include "states_screens/grand_prix_lose.hpp"

#include "audio/music_manager.hpp"
#include "audio/sfx_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/stk_config.hpp"
#include "graphics/irr_driver.hpp"
#include "graphics/lod_node.hpp"
#include "graphics/lod_node.hpp"
#include <ge_render_info.hpp>
#include "guiengine/engine.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "io/file_manager.hpp"
#include "items/item_manager.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "modes/cutscene_world.hpp"
#include "modes/overworld.hpp"
#include "modes/world.hpp"
#include "race/race_manager.hpp"
#include "states_screens/feature_unlocked.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_object.hpp"
#include "tracks/track_object_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <ISceneManager.h>
#include <SColor.h>
#include <ICameraSceneNode.h>
#include <ILightSceneNode.h>
#include <IMeshSceneNode.h>

using namespace irr::core;
using namespace irr::gui;
using namespace irr::video;
using namespace GUIEngine;

const float DURATION = 15.0f;

const float DISTANCE_BETWEEN_KARTS = 2.0f;

const float KART_SCALE = 0.75f;

const float KART_START_X = -17.0f;
const float KART_END_X = -5.0f;
const float KART_Y = 0.0f;
const float KART_Z = 0.0f;

const int MAX_KART_COUNT = 4;

// -------------------------------------------------------------------------------------

void GrandPrixLose::onCutsceneEnd()
{
    for (int i = 0; i < 4; i++)
    {
        if (m_kart_node[i] != NULL)
            m_kart_node[i]->getPresentation<TrackObjectPresentationSceneNode>()->getNode()->remove();
        m_kart_node[i] = NULL;
    }

    for (unsigned int i = 0; i<m_all_kart_models.size(); i++)
        delete m_all_kart_models[i];

    m_all_kart_models.clear();
}   // onCutsceneEnd

// -------------------------------------------------------------------------------------

void GrandPrixLose::loadedFromFile()
{
    m_kart_node[0] = NULL;
    m_kart_node[1] = NULL;
    m_kart_node[2] = NULL;
    m_kart_node[3] = NULL;
}   // loadedFromFile

// -------------------------------------------------------------------------------------

void GrandPrixLose::init()
{
    std::vector<std::string> parts;
    parts.push_back("gplose");
    ((CutsceneWorld*)World::getWorld())->setParts(parts);
    CutsceneWorld::setUseDuration(false);

    Screen::init();

    World::getWorld()->setPhase(WorldStatus::RACE_PHASE);

    saveGPButton();

    m_phase = 1;
    m_global_time = 0.0f;

    getWidget<ButtonWidget>("continue")->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
}   // init

// -------------------------------------------------------------------------------------

void GrandPrixLose::onUpdate(float dt)
{
    m_global_time += dt;

    const float kartProgression = m_global_time/(DURATION - 6.0f);
    if (kartProgression <= 1.0f)
    {
        m_kart_x = KART_START_X + (KART_END_X - KART_START_X)*kartProgression;

        core::vector3df kart_rot(0.0f, 90.0f, 0.0f);
        core::vector3df kart_scale(KART_SCALE, KART_SCALE, KART_SCALE);
        for (int n=0; n<MAX_KART_COUNT; n++)
        {
            if (m_kart_node[n] != NULL)
            {
                core::vector3df kart_pos(m_kart_x + n*DISTANCE_BETWEEN_KARTS,
                    m_kart_y,
                    m_kart_z);
                m_kart_node[n]->move(kart_pos, kart_rot, kart_scale, false, true);
            }
        }
    }

    // ---- title
    const int w = irr_driver->getFrameSize().Width;
    const int h = irr_driver->getFrameSize().Height;
    const irr::video::SColor color(255, 255, 255, 255);

    static int test_y = 0;

    //I18N: when failing a GP
    GUIEngine::getTitleFont()->draw(_("Better luck next time!"),
                                    core::rect< s32 >( 0, test_y, w, h ),
                                    color,
                                    true/* center h */, false /* center v */ );
}   // onUpdate

// -------------------------------------------------------------------------------------

void GrandPrixLose::setKarts(std::vector<std::pair<std::string, float> > ident_arg)
{
    TrackObjectManager* tobjman = Track::getCurrentTrack()->getTrackObjectManager();

    assert(ident_arg.size() > 0);
    if ((int)ident_arg.size() > MAX_KART_COUNT)
        ident_arg.resize(MAX_KART_COUNT);

    // (there is at least one kart so kart node 0 is sure to be set)
    m_kart_node[1] = NULL;
    m_kart_node[2] = NULL;
    m_kart_node[3] = NULL;

    m_kart_x = KART_START_X;
    m_kart_y = KART_Y;
    m_kart_z = KART_Z;

    const int count = (int)ident_arg.size();
    for (int n=0; n<count; n++)
    {
        const KartProperties* kart = kart_properties_manager->getKart(ident_arg[n].first);
        if (kart != NULL)
        {
            KartModel* kart_model = kart->getKartModelCopy(std::make_shared<GE::GERenderInfo>(ident_arg[n].second));
            m_all_kart_models.push_back(kart_model);
            scene::ISceneNode* kart_main_node = kart_model->attachModel(true, false);
            LODNode* lnode = dynamic_cast<LODNode*>(kart_main_node);
            if (lnode)
            {
                // Lod node has to be animated
                auto* a_node = static_cast<scene::IAnimatedMeshSceneNode*>
                    (lnode->getAllNodes()[0]);
                const unsigned start_frame =
                    kart_model->getFrame(KartModel::AF_LOSE_LOOP_START) > -1 ?
                    kart_model->getFrame(KartModel::AF_LOSE_LOOP_START) :
                    kart_model->getFrame(KartModel::AF_LOSE_START) > -1 ?
                    kart_model->getFrame(KartModel::AF_LOSE_START) :
                    kart_model->getFrame(KartModel::AF_STRAIGHT);
                const unsigned end_frame =
                    kart_model->getFrame(KartModel::AF_LOSE_END) > -1 ?
                    kart_model->getFrame(KartModel::AF_LOSE_END) :
                    kart_model->getFrame(KartModel::AF_STRAIGHT);
                a_node->setLoopMode(true);
                a_node->setFrameLoop(start_frame, end_frame);
            }

            core::vector3df kart_pos(m_kart_x + n*DISTANCE_BETWEEN_KARTS,
                m_kart_y,
                m_kart_z);
            core::vector3df kart_rot(0, 90.0f, 0);
            core::vector3df kart_scale(KART_SCALE, KART_SCALE, KART_SCALE);

            //FIXME: it's not ideal that both the track object and the
            // presentation know the initial coordinates of the object
            TrackObjectPresentationSceneNode* presentation =
                new TrackObjectPresentationSceneNode(kart_pos, kart_rot,
                                                     kart_scale, kart_main_node);
            TrackObject* tobj = new TrackObject(kart_pos, kart_rot, kart_scale,
                "ghost", presentation, false /* isDynamic */, NULL /* physics settings */);
            tobjman->insertObject(tobj);

            m_kart_node[n] = tobj;
        }
        else
        {
            Log::warn("GrandPrixLose", "A kart named '%s' could not be found\n",
                      ident_arg[n].first.c_str());
            m_kart_node[n] = NULL;
        } // if kart != NULL
    }
}   // setKarts

// -------------------------------------------------------------------------------------

MusicInformation* GrandPrixLose::getInGameMenuMusic() const
{
    MusicInformation* mi = stk_config->m_gp_lose_music;
    return mi;
}

// -------------------------------------------------------------------------------------

