//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

#include "karts/controller/network_ai_controller.hpp"
#include "graphics/camera.hpp"
#include "guiengine/engine.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/controller/kart_control.hpp"
#include "karts/controller/skidding_ai.hpp"
#include "modes/world.hpp"
#include "network/protocols/game_protocol.hpp"
#include "network/network_config.hpp"
#include "network/rewind_manager.hpp"

// ============================================================================
int NetworkAIController::m_ai_frequency = 30;
// ----------------------------------------------------------------------------
NetworkAIController::NetworkAIController(AbstractKart *kart,
                                         int local_player_id,
                                         AIBaseController* ai)
                   : PlayerController(kart)
{
    m_ai_controller = ai;
    m_ai_controls = new KartControl;
    // We only need camera for real AI instance for debugging view
    if (!GUIEngine::isNoGraphics() &&
        NetworkConfig::get()->isNetworkAIInstance())
        Camera::createCamera(kart, local_player_id);
    ai->setControls(m_ai_controls);
}   // NetworkAIController

// ----------------------------------------------------------------------------
NetworkAIController::~NetworkAIController()
{
    delete m_ai_controller;
    delete m_ai_controls;
}   // ~NetworkAIController

// ----------------------------------------------------------------------------
bool NetworkAIController::isLocalPlayerController() const
{
    return NetworkConfig::get()->isNetworkAIInstance();
}   // isLocalPlayerController

// ----------------------------------------------------------------------------
void NetworkAIController::update(int ticks)
{
    if (!RewindManager::get()->isRewinding())
    {
        if (World::getWorld()->isStartPhase() ||
            World::getWorld()->getTicksSinceStart() > m_prev_update_ticks)
        {
            m_prev_update_ticks = World::getWorld()->getTicksSinceStart() +
                m_ai_frequency;
            m_ai_controller->update(m_ai_frequency);
            convertAIToPlayerActions();
        }
    }
    PlayerController::update(ticks);
}   // update

// ----------------------------------------------------------------------------
void NetworkAIController::reset()
{
    m_prev_update_ticks = 0;
    m_ai_controller->reset();
    m_ai_controller->setNetworkAI(true);
    m_ai_controls->reset();
    PlayerController::reset();
}   // reset

// ----------------------------------------------------------------------------
void NetworkAIController::convertAIToPlayerActions()
{
    std::vector<std::pair<PlayerAction, int> > all_actions;
    if (m_ai_controls->getSteer() < 0.0f)
    {
        all_actions.emplace_back(PA_STEER_LEFT,
            int(fabsf(m_ai_controls->getSteer()) * 32768));
    }
    else
    {
        all_actions.emplace_back(PA_STEER_RIGHT,
            int(fabsf(m_ai_controls->getSteer()) * 32768));
    }
    all_actions.emplace_back(PA_ACCEL,
        int(m_ai_controls->getAccel() * 32768));
    all_actions.emplace_back(PA_BRAKE,
        m_ai_controls->getBrake() ? 32768 : 0);
    all_actions.emplace_back(PA_FIRE,
        m_ai_controls->getFire() ? 32768 : 0);
    all_actions.emplace_back(PA_NITRO,
        m_ai_controls->getNitro() ? 32768 : 0);
    all_actions.emplace_back(PA_DRIFT,
        m_ai_controls->getSkidControl() == KartControl::SC_NONE ?
        0 : 32768);
    all_actions.emplace_back(PA_RESCUE,
        m_ai_controls->getRescue() ? 32768 : 0);
    all_actions.emplace_back(PA_LOOK_BACK,
        m_ai_controls->getLookBack() ? 32768 : 0);

    for (const auto& a : all_actions)
    {
        if (!PlayerController::action(a.first, a.second, /*dry_run*/true))
            continue;

        if (NetworkConfig::get()->isNetworking() &&
            NetworkConfig::get()->isClient() &&
            !RewindManager::get()->isRewinding())
        {
            if (auto gp = GameProtocol::lock())
            {
                gp->controllerAction(m_kart->getWorldKartId(),
                    a.first, a.second,
                    m_steer_val_l, m_steer_val_r);
            }
        }
        PlayerController::action(a.first, a.second, false);
    }
}   // convertAIToPlayerActions
