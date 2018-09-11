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

#include <vector>

// The order here is important. If all_params is declared later (e.g. after
// the #includes), all elements will be added to all_params, and then
// g_server_params will be initialised, i.e. cleared!
// ============================================================================
class UserConfigParam;
static std::vector<UserConfigParam*> g_server_params;
// ============================================================================

// X-macros
#define SERVER_CFG_PREFIX
#define SERVER_CFG_DEFAULT(X) = X

#include "network/server_config.hpp"
#include "io/file_manager.hpp"

namespace ServerConfig
{
// ============================================================================
std::string g_server_config_path;
// ============================================================================
FloatServerConfigParam::FloatServerConfigParam(float default_value,
                                               const char* param_name,
                                               const char* comment)
                      : FloatUserConfigParam(param_name, comment)
{
    m_value = default_value;
    m_default_value = default_value;
    g_server_params.push_back(this);
}   // FloatServerConfigParam

// ============================================================================
IntServerConfigParam::IntServerConfigParam(int default_value,
                                           const char* param_name,
                                           const char* comment)
                    : IntUserConfigParam(param_name, comment)
{
    m_value = default_value;
    m_default_value = default_value;
    g_server_params.push_back(this);
}   // IntServerConfigParam

// ============================================================================
BoolServerConfigParam::BoolServerConfigParam(bool default_value,
                                             const char* param_name,
                                             const char* comment)
                     : BoolUserConfigParam(param_name, comment)
{
    m_value = default_value;
    m_default_value = default_value;
    g_server_params.push_back(this);
}   // BoolServerConfigParam

// ============================================================================
template<typename T, typename U>
MapServerConfigParam<T, U>::MapServerConfigParam(const char* param_name,
                                                 const char* comment,
                                                 std::map<T, U> default_value)
                          : MapUserConfigParam<T, U>(param_name, comment)
{
    m_elements = default_value;
    g_server_params.push_back(this);
}   // MapServerConfigParam

// ============================================================================
void loadServerConfig(const std::string& path)
{
    if (path.empty())
    {
        g_server_config_path =
            file_manager->getUserConfigFile("server_config.xml");
    }
    else
    {
        g_server_config_path = path;
    }

}   // loadServerConfig

// ----------------------------------------------------------------------------
void writeServerConfig()
{
}   // writeServerConfig

}

