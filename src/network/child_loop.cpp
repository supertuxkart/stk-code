//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2020 SuperTuxKart-Team
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

#include "network/child_loop.hpp"
#include "config/user_config.hpp"
#include "guiengine/engine.hpp"
#include "items/projectile_manager.hpp"
#include "modes/world.hpp"
#include "network/network_config.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/server_lobby.hpp"
#include "network/race_event_manager.hpp"
#include "network/server_config.hpp"
#include "network/stk_host.hpp"
#include "race/race_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/log.hpp"
#include "utils/stk_process.hpp"
#include "utils/time.hpp"
#include "utils/vs.hpp"

// ----------------------------------------------------------------------------
float ChildLoop::getLimitedDt()
{
    m_prev_time = m_curr_time;

    float dt = 0;
    while (1)
    {
        m_curr_time = StkTime::getMonoTimeMs();
        if (m_prev_time > m_curr_time)
        {
            m_prev_time = m_curr_time;
        }
        dt = (float)(m_curr_time - m_prev_time);
        while (dt == 0)
        {
            StkTime::sleep(1);
            m_curr_time = StkTime::getMonoTimeMs();
            if (m_prev_time > m_curr_time)
            {
                Log::error("MainLopp", "System clock keeps backwards!");
                m_prev_time = m_curr_time;
            }
            dt = (float)(m_curr_time - m_prev_time);
        }

        const int current_fps = (int)(1000.0f / dt);
        const int max_fps = UserConfigParams::m_max_fps;
        if (current_fps <= max_fps)
            break;

        int wait_time = 1000 / max_fps - 1000 / current_fps;
        if (wait_time < 1) wait_time = 1;

        StkTime::sleep(wait_time);
    }   // while(1)
    dt *= 0.001f;
    return dt;
}   // getLimitedDt

// ----------------------------------------------------------------------------
void ChildLoop::run()
{
    VS::setThreadName("ChildLoop");
    STKProcess::init(PT_CHILD);

    GUIEngine::disableGraphics();
    RaceManager::create();
    ProjectileManager::create();
    NetworkConfig::get()->setIsServer(true);
    if (m_cl_config->m_lan_server)
        NetworkConfig::get()->setIsLAN();
    else
    {
        if (UserConfigParams::m_default_ip_type == NetworkConfig::IP_NONE)
        {
            NetworkConfig::get()->setIPType(NetworkConfig::IP_V4);
            NetworkConfig::get()->queueIPDetection();
        }
        // Longer timeout for server creation
        NetworkConfig::get()->getIPDetectionResult(4000);
        NetworkConfig::getByType(PT_MAIN)->setIPType(
            NetworkConfig::get()->getIPType());
        NetworkConfig::get()->setIsWAN();
        NetworkConfig::get()->setIsPublicServer();
    }
    NetworkConfig::get()->setCurrentUserId(m_cl_config->m_login_id);
    NetworkConfig::get()->setCurrentUserToken(m_cl_config->m_token);
    NetworkConfig::get()->setNumFixedAI(m_cl_config->m_server_ai);
    // Unused afterwards
    delete m_cl_config;
    m_cl_config = NULL;

    ServerConfig::loadServerLobbyFromConfig();
    StateManager::get()->enterMenuState();

    m_curr_time = StkTime::getMonoTimeMs();
    float left_over_time = 0;
    while (!m_abort)
    {
        if (STKHost::existHost() && STKHost::get()->requestedShutdown())
            break;

        // Tell the main process port and server id
        if (m_port == 0 && STKHost::existHost())
        {
            auto sl = LobbyProtocol::get<ServerLobby>();
            if (sl &&
                sl->getCurrentState() >= ServerLobby::WAITING_FOR_START_GAME)
            {
                m_port = STKHost::get()->getPrivatePort();
                m_server_online_id = sl->getServerIdOnline();
            }
        }

        left_over_time += getLimitedDt();
        int num_steps = stk_config->time2Ticks(left_over_time);
        float dt = stk_config->ticks2Time(1);
        left_over_time -= num_steps * dt;

        for (int i = 0; i < num_steps; i++)
        {
            if (auto pm = ProtocolManager::lock())
                pm->update(1);

            World* w = World::getWorld();
            if (w && w->getPhase() == WorldStatus::SETUP_PHASE)
            {
                // Skip the large num steps contributed by loading time
                w->updateTime(1);
                break;
            }

            if (w)
            {
                auto rem = RaceEventManager::get();
                if (rem && rem->isRunning())
                    RaceEventManager::get()->update(1, false/*fast_forward*/);
                else
                    w->updateWorld(1);
                w->updateTime(1);
            }
            if (m_abort)
                break;
        }
    }

    if (STKHost::existHost())
        STKHost::get()->shutdown();
    if (World::getWorld())
        RaceManager::get()->exitRace();

    RaceManager::destroy();
    ProjectileManager::destroy();
    NetworkConfig::destroy();
    StateManager::deallocate();
}   // run
