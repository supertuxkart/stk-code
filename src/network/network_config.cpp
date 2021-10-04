//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2015 Joerg Henrichs
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

#include "network/network_config.hpp"
#include "config/stk_config.hpp"
#include "config/user_config.hpp"
#include "input/device_manager.hpp"
#include "modes/world.hpp"
#include "network/network.hpp"
#include "network/network_string.hpp"
#include "network/rewind_manager.hpp"
#include "network/server_config.hpp"
#include "network/socket_address.hpp"
#include "network/stk_host.hpp"
#include "network/stk_ipv6.hpp"
#include "network/stun_detection.hpp"
#include "online/xml_request.hpp"
#include "states_screens/main_menu_screen.hpp"
#include "states_screens/online/networking_lobby.hpp"
#include "states_screens/online/online_lan.hpp"
#include "states_screens/online/online_profile_servers.hpp"
#include "states_screens/online/online_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/time.hpp"
#include "utils/utf8/unchecked.h"

#include <fcntl.h>

#ifdef DNS_C
#  include <dns.h>
#else
#ifdef WIN32
#  include <windns.h>
#  include <ws2tcpip.h>
#ifndef __MINGW32__
#  pragma comment(lib, "dnsapi.lib")
#endif
#else
#ifndef __SWITCH__
#  include <arpa/nameser.h>
#  include <arpa/nameser_compat.h>
#endif
#  include <netdb.h>
#  include <netinet/in.h>
#ifndef __SWITCH__
#  include <resolv.h>
#endif
#endif
#endif

#ifdef ANDROID
#include <jni.h>
#include "SDL_system.h"

std::vector<std::pair<std::string, int> >* g_list = NULL;
extern "C" JNIEXPORT void JNICALL addDNSSrvRecords(JNIEnv* env, jclass cls, jstring name, jint weight)
{
    if (!g_list || name == NULL)
        return;
    const uint16_t* utf16_text =
        (const uint16_t*)env->GetStringChars(name, NULL);
    if (utf16_text == NULL)
        return;
    const size_t str_len = env->GetStringLength(name);
    std::string tmp;
    utf8::unchecked::utf16to8(
        utf16_text, utf16_text + str_len, std::back_inserter(tmp));
    g_list->emplace_back(tmp, weight);
    env->ReleaseStringChars(name, utf16_text);
}

#endif

NetworkConfig *NetworkConfig::m_network_config[PT_COUNT];
bool NetworkConfig::m_system_ipv4 = false;
bool NetworkConfig::m_system_ipv6 = false;

/** Initialize detection of system IPv4 or IPv6 support. */
void NetworkConfig::initSystemIP()
{
    // It calls WSAStartup in enet, for the rest new Network function we don't
    // need this because request manager runs curl_global_init which will do
    // WSAStartup too
    if (enet_initialize() != 0)
    {
        Log::error("NetworkConfig", "Could not initialize enet.");
        return;
    }
    ENetAddress eaddr = {};
    setIPv6Socket(0);
    auto ipv4 = std::unique_ptr<Network>(new Network(1, 1, 0, 0, &eaddr));
    setIPv6Socket(1);
    auto ipv6 = std::unique_ptr<Network>(new Network(1, 1, 0, 0, &eaddr));
    setIPv6Socket(0);
    if (ipv4 && ipv4->getENetHost())
        m_system_ipv4 = true;
    if (ipv6 && ipv6->getENetHost())
        m_system_ipv6 = true;
    // If any 1 of them is missing set default network setting accordingly
    if (!m_system_ipv4)
    {
        Log::warn("NetworkConfig", "System doesn't support IPv4");
        if (m_system_ipv6)
        {
            UserConfigParams::m_ipv6_lan = true;
            ServerConfig::m_ipv6_connection = true;
        }
    }
    else if (!m_system_ipv6)
    {
        Log::warn("NetworkConfig", "System doesn't support IPv6");
        UserConfigParams::m_ipv6_lan = false;
        ServerConfig::m_ipv6_connection = false;
    }
    enet_deinitialize();
}   // initSystemIP

/** \class NetworkConfig
 *  This class is the interface between STK and the online code, particularly
 *  STKHost. It stores all online related properties (e.g. if this is a server
 *  or a host, name of the server, maximum number of players, ip address, ...).
 *  They can either be set from the GUI code, or via the command line (for a
 *  stand-alone server).
 *  When STKHost is created, it takes all necessary information from this
 *  instance.
 */
// ============================================================================
/** Constructor.
 */
NetworkConfig::NetworkConfig()
{
    m_ip_type               = IP_NONE;
    m_network_type          = NETWORK_NONE;
    m_auto_connect          = false;
    m_is_server             = false;
    m_is_public_server      = false;
    m_done_adding_network_players = false;
    m_cur_user_id           = 0;
    m_cur_user_token        = "";
    m_client_port = 0;
    m_joined_server_version = 0;
    m_network_ai_instance = false;
    m_state_frequency = 10;
    m_nat64_prefix_data.fill(-1);
    m_num_fixed_ai = 0;
    m_tux_hitbox_addon = false;
}   // NetworkConfig

// ----------------------------------------------------------------------------
/** Separated from constructor because this needs to be run after user config
 *  is load.
 */
void NetworkConfig::initClientPort()
{
    m_client_port = UserConfigParams::m_random_client_port ?
        0 : stk_config->m_client_port;
}   // initClientPort

// ----------------------------------------------------------------------------
/** Set that this is not a networked game.
 */
void NetworkConfig::unsetNetworking()
{
    clearServerCapabilities();
    m_network_type = NETWORK_NONE;
    ServerConfig::m_private_server_password = "";
}   // unsetNetworking

// ----------------------------------------------------------------------------
void NetworkConfig::setUserDetails(std::shared_ptr<Online::XMLRequest> r,
                                   const std::string& name)
{
    assert(!m_cur_user_token.empty());
    r->setApiURL(Online::API::USER_PATH, name);
    r->addParameter("userid", m_cur_user_id);
    r->addParameter("token", m_cur_user_token);
}   // setUserDetails

// ----------------------------------------------------------------------------
void NetworkConfig::setServerDetails(std::shared_ptr<Online::XMLRequest> r,
                                     const std::string& name)
{
    assert(!m_cur_user_token.empty());
    r->setApiURL(Online::API::SERVER_PATH, name);
    r->addParameter("userid", m_cur_user_id);
    r->addParameter("token", m_cur_user_token);
}   // setServerDetails

// ----------------------------------------------------------------------------
std::vector<GUIEngine::Screen*>
    NetworkConfig::getResetScreens(bool lobby) const
{
    if (lobby)
    {
        if (isWAN())
        {
            return
                {
                    MainMenuScreen::getInstance(),
                    OnlineScreen::getInstance(),
                    OnlineProfileServers::getInstance(),
                    NetworkingLobby::getInstance(),
                    nullptr
                };
        }
        else
        {
            return
                {
                    MainMenuScreen::getInstance(),
                    OnlineScreen::getInstance(),
                    OnlineLanScreen::getInstance(),
                    NetworkingLobby::getInstance(),
                    nullptr
                };
        }
    }
    else
    {
        if (isWAN())
        {
            return
                {
                    MainMenuScreen::getInstance(),
                    OnlineScreen::getInstance(),
                    OnlineProfileServers::getInstance(),
                    nullptr
                };
        }
        else
        {
            return
                {
                    MainMenuScreen::getInstance(),
                    OnlineScreen::getInstance(),
                    OnlineLanScreen::getInstance(),
                    nullptr
                };
        }
    }
}   // getResetScreens

// ----------------------------------------------------------------------------
/** Called before (re)starting network race, must be used before adding
 *  split screen players. */
void NetworkConfig::clearActivePlayersForClient() const
{
    if (!isClient())
        return;
    StateManager::get()->resetActivePlayers();
    if (input_manager)
    {
        input_manager->getDeviceManager()->setAssignMode(NO_ASSIGN);
        input_manager->getDeviceManager()->setSinglePlayer(NULL);
        input_manager->setMasterPlayerOnly(false);
        input_manager->getDeviceManager()->clearLatestUsedDevice();
    }
}   // clearActivePlayersForClient

// ----------------------------------------------------------------------------
/** True when client needs to round the bodies phyiscal info for current
 *  ticks, server doesn't as it will be done implictly in save state. */
bool NetworkConfig::roundValuesNow() const
{
    return isNetworking() && !isServer() && RewindManager::get()
        ->shouldSaveState(World::getWorld()->getTicksSinceStart());
}   // roundValuesNow

// ----------------------------------------------------------------------------
#ifdef ENABLE_IPV6
std::vector<std::unique_ptr<StunDetection> > g_ipv4_detection;
std::vector<std::unique_ptr<StunDetection> > g_ipv6_detection;
#endif

void NetworkConfig::clearDetectIPThread(bool quit_stk)
{
#ifdef ENABLE_IPV6
    if (!quit_stk)
    {
        auto it = g_ipv4_detection.begin();
        while (it != g_ipv4_detection.end())
        {
            if ((*it)->isConnecting())
            {
                it++;
                continue;
            }
            it = g_ipv4_detection.erase(it);
        }
        it = g_ipv6_detection.begin();
        while (it != g_ipv6_detection.end())
        {
            if ((*it)->isConnecting())
            {
                it++;
                continue;
            }
            it = g_ipv6_detection.erase(it);
        }
    }
    else
    {
        g_ipv4_detection.clear();
        g_ipv6_detection.clear();
    }
#endif
}   // clearDetectIPThread

// ----------------------------------------------------------------------------
/** Use stun servers to detect current ip type.
 */
void NetworkConfig::queueIPDetection()
{
    if (UserConfigParams::m_default_ip_type != IP_NONE ||
        !m_system_ipv4 || !m_system_ipv6)
        return;

#ifdef ENABLE_IPV6
    auto& stunv4_map = UserConfigParams::m_stun_servers_v4;
    for (auto& s : getStunList(true/*ipv4*/))
    {
        if (s.second == 0)
            stunv4_map.erase(s.first);
        else if (stunv4_map.find(s.first) == stunv4_map.end())
            stunv4_map[s.first] = 0;
    }
    if (stunv4_map.empty())
        return;
    auto ipv4_it = stunv4_map.begin();
    int adv = StkTime::getMonoTimeMs() % stunv4_map.size();
    std::advance(ipv4_it, adv);

    auto& stunv6_map = UserConfigParams::m_stun_servers;
    for (auto& s : getStunList(false/*ipv4*/))
    {
        if (s.second == 0)
            stunv6_map.erase(s.first);
        else if (stunv6_map.find(s.first) == stunv6_map.end())
            stunv6_map[s.first] = 0;
    }
    if (stunv6_map.empty())
        return;
    auto ipv6_it = stunv6_map.begin();
    adv = StkTime::getMonoTimeMs() % stunv6_map.size();
    std::advance(ipv6_it, adv);

    SocketAddress::g_ignore_error_message = true;
    std::unique_ptr<StunDetection> ipv4_detect(
        new StunDetection(ipv4_it->first, true/*ipv4*/));
    std::unique_ptr<StunDetection> ipv6_detect(
        new StunDetection(ipv6_it->first, false/*ipv4*/));
    SocketAddress::g_ignore_error_message = false;
    Log::debug("NetworkConfig", "Using TCP stun IPv4: %s, IPv6: %s",
        ipv4_it->first.c_str(), ipv6_it->first.c_str());
    g_ipv4_detection.emplace_back(std::move(ipv4_detect));
    g_ipv6_detection.emplace_back(std::move(ipv6_detect));
#endif
}   // queueIPDetection

// ----------------------------------------------------------------------------
/** Use stun servers to detect current ip type.
 */
void NetworkConfig::getIPDetectionResult(uint64_t timeout)
{
    if (UserConfigParams::m_default_ip_type != IP_NONE)
    {
        int ip_type = UserConfigParams::m_default_ip_type;
        m_nat64_prefix.clear();
        m_nat64_prefix_data.fill(-1);
        m_ip_type.store((IPType)ip_type);
        return;
    }
#ifdef ENABLE_IPV6
    bool has_ipv4 = false;
    bool has_ipv6 = false;
    if (!m_system_ipv4 || !m_system_ipv6)
    {
        has_ipv4 = m_system_ipv4;
        has_ipv6 = m_system_ipv6;
        goto end;
    }

    if (g_ipv4_detection.empty() || g_ipv6_detection.empty())
        goto end;

    timeout += StkTime::getMonoTimeMs();
    do
    {
        has_ipv4 = g_ipv4_detection.back()->connectionSucceeded();
        has_ipv6 = g_ipv6_detection.back()->connectionSucceeded();
        // Exit early if socket closed
        if (g_ipv4_detection.back()->socketClosed() &&
            g_ipv6_detection.back()->socketClosed())
            break;
    } while (timeout > StkTime::getMonoTimeMs());
    clearDetectIPThread(false/*quit_stk*/);

end:
    if (has_ipv6)
    {
        // For non dual stack IPv6 we try to get a NAT64 prefix to connect
        // to IPv4 only servers
        if (!has_ipv4)
        {
            // Detect NAT64 prefix by using ipv4only.arpa (RFC 7050)
            m_nat64_prefix.clear();
            m_nat64_prefix_data.fill(-1);
            SocketAddress nat64("ipv4only.arpa", 0/*port*/, AF_INET6);
            if (nat64.getFamily() == AF_INET6)
            {
                // Remove last 4 bytes which is IPv4 format
                struct sockaddr_in6* in6 =
                    (struct sockaddr_in6*)nat64.getSockaddr();
                uint8_t* byte = &(in6->sin6_addr.s6_addr[0]);
                byte[12] = 0;
                byte[13] = 0;
                byte[14] = 0;
                byte[15] = 0;
                m_nat64_prefix_data[0] = ((uint32_t)(byte[0]) << 8) | byte[1];
                m_nat64_prefix_data[1] = ((uint32_t)(byte[2]) << 8) | byte[3];
                m_nat64_prefix_data[2] = ((uint32_t)(byte[4]) << 8) | byte[5];
                m_nat64_prefix_data[3] = ((uint32_t)(byte[6]) << 8) | byte[7];
                m_nat64_prefix_data[4] = ((uint32_t)(byte[8]) << 8) | byte[9];
                m_nat64_prefix_data[5] = ((uint32_t)(byte[10]) << 8) | byte[11];
                m_nat64_prefix_data[6] = 0;
                m_nat64_prefix_data[7] = 0;
                m_nat64_prefix = getIPV6ReadableFromIn6(in6);
            }
        }
    }

    if (has_ipv4 && has_ipv6)
    {
        Log::info("NetworkConfig", "System is dual stack network.");
        m_nat64_prefix.clear();
        m_nat64_prefix_data.fill(-1);
        m_ip_type = IP_DUAL_STACK;
    }
    else if (has_ipv4)
    {
        Log::info("NetworkConfig", "System is IPv4 only.");
        m_nat64_prefix.clear();
        m_nat64_prefix_data.fill(-1);
        m_ip_type = IP_V4;
    }
    else if (has_ipv6)
    {
        Log::info("NetworkConfig", "System is IPv6 only.");
        if (m_nat64_prefix.empty())
            m_ip_type = IP_V6;
    }
    else
    {
        Log::error("NetworkConfig", "Cannot detect network type using stun, "
            "using previously detected type: %d", (int)m_ip_type.load());
    }
    if (has_ipv6)
    {
        if (!has_ipv4 && m_nat64_prefix.empty())
        {
            Log::warn("NetworkConfig", "NAT64 prefix not found, "
                "you may not be able to join any IPv4 only servers.");
        }
        if (!m_nat64_prefix.empty())
        {
            m_ip_type = IP_V6_NAT64;
            Log::info("NetworkConfig",
                "NAT64 prefix is %s.", m_nat64_prefix.c_str());
        }
    }
#else
    m_ip_type = IP_V4;
#endif
}   // getIPDetectionResult

// ----------------------------------------------------------------------------
void NetworkConfig::fillStunList(std::vector<std::pair<std::string, int> >* l,
                                 const std::string& dns)
{
#if defined(WIN32)
    PDNS_RECORD dns_record = NULL;
    DnsQuery(StringUtils::utf8ToWide(dns).c_str(), DNS_TYPE_SRV,
        DNS_QUERY_STANDARD, NULL, &dns_record, NULL);
    if (dns_record)
    {
        for (PDNS_RECORD curr = dns_record; curr; curr = curr->pNext)
        {
            if (curr->wType == DNS_TYPE_SRV)
            {
                l->emplace_back(
                    StringUtils::wideToUtf8(curr->Data.SRV.pNameTarget) +
                    ":" + StringUtils::toString(curr->Data.SRV.wPort),
                    curr->Data.SRV.wWeight);
            }
        }
        DnsRecordListFree(dns_record, DnsFreeRecordListDeep);
    }

#elif defined(ANDROID)
    JNIEnv* env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    if (env == NULL)
    {
        Log::error("NetworkConfig",
            "getDNSSrvRecords unable to SDL_AndroidGetJNIEnv.");
        return;
    }

    jobject native_activity = (jobject)SDL_AndroidGetActivity();
    if (native_activity == NULL)
    {
        Log::error("NetworkConfig",
            "getDNSSrvRecords unable to SDL_AndroidGetActivity.");
        return;
    }

    jclass class_native_activity = env->GetObjectClass(native_activity);
    if (class_native_activity == NULL)
    {
        Log::error("NetworkConfig",
            "getDNSSrvRecords unable to find object class.");
        env->DeleteLocalRef(native_activity);
        return;
    }

    jmethodID method_id = env->GetMethodID(class_native_activity,
        "getDNSSrvRecords", "(Ljava/lang/String;)V");

    if (method_id == NULL)
    {
        Log::error("NetworkConfig",
            "getDNSSrvRecords unable to find method id.");
        env->DeleteLocalRef(class_native_activity);
        env->DeleteLocalRef(native_activity);
        return;
    }

    std::vector<uint16_t> jstr_data;
    utf8::unchecked::utf8to16(
        dns.c_str(), dns.c_str() + dns.size(), std::back_inserter(jstr_data));
    jstring text =
        env->NewString((const jchar*)jstr_data.data(), jstr_data.size());
    if (text == NULL)
    {
        Log::error("NetworkConfig",
            "Failed to create text for domain name.");
        env->DeleteLocalRef(class_native_activity);
        env->DeleteLocalRef(native_activity);
        return;
    }

    g_list = l;
    env->CallVoidMethod(native_activity, method_id, text);
    env->DeleteLocalRef(text);
    env->DeleteLocalRef(class_native_activity);
    env->DeleteLocalRef(native_activity);
    g_list = NULL;
#elif defined(DNS_C)
    int err = 0;
    struct dns_socket* so = NULL;
    struct dns_packet* ans = NULL;
    struct dns_packet* quest = NULL;

    dns_options options = {};
    struct dns_rr_i rri = {};
    struct dns_rr rr = {};
    dns_resolv_conf* conf = dns_resconf_open(&err);
    if (!conf)
    {
        Log::error("ConnectToServer", "Error dns_resconf_open: %s",
            dns_strerror(err));
        goto cleanup;
    }
    if ((err = dns_resconf_loadpath(conf, "/etc/resolv.conf")))
    {
        err = 0;
        // Fallback name server
        SocketAddress addr("8.8.8.8:53");
        memcpy(&conf->nameserver[0], addr.getSockaddr(), addr.getSocklen());
    }

    so = dns_so_open((sockaddr*)&conf->iface, 0, &options, &err);
    if (!so)
    {
        Log::error("ConnectToServer", "Error dns_so_open: %s",
            dns_strerror(err));
        goto cleanup;
    }

    quest = dns_p_make(512, &err);
    if (err)
    {
        Log::error("ConnectToServer", "Error dns_p_make: %s",
            dns_strerror(err));
        goto cleanup;
    }
    if ((err = dns_p_push(quest, DNS_S_QD, dns.c_str(), dns.size(),
        DNS_T_SRV, DNS_C_IN, 0, 0)))
    {
        Log::error("ConnectToServer", "Error dns_p_push: %s",
            dns_strerror(err));
        goto cleanup;
    }

    dns_header(quest)->rd = 1;
    while (!(ans = dns_so_query(so, quest, (sockaddr*)&conf->nameserver[0],
        &err)))
    {
        if (err == 0)
        {
            Log::error("ConnectToServer", "Error dns_so_query: %s",
                dns_strerror(err));
            goto cleanup;
        }
        if (dns_so_elapsed(so) > 10)
        {
            Log::error("ConnectToServer", "Timeout dns_so_query.");
            goto cleanup;
        }
        dns_so_poll(so, 1);
    }
    if (!ans)
    {
        Log::error("ConnectToServer", "Timeout dns_so_query.");
        goto cleanup;
    }

    rri.sort = dns_rr_i_packet;
    err = 0;
    while (dns_rr_grep(&rr, 1, &rri, ans, &err))
    {
        if (err != 0)
        {
            Log::error("ConnectToServer", "Error dns_rr_grep: %s",
                dns_strerror(err));
            goto cleanup;
        }
        if (rr.section != DNS_S_ANSWER)
            continue;
        dns_srv srv = {};
        if (dns_srv_parse(&srv, &rr, ans) != 0)
            goto cleanup;
        std::string addr = srv.target;
        if (!addr.empty() && addr.back() == '.')
            addr.pop_back();
        l->emplace_back(addr + ":" + StringUtils::toString(srv.port),
            srv.weight);
    }

cleanup:
    free(ans);
    free(quest);
    dns_so_close(so);
    dns_resconf_close(conf);
#else
#define SRV_WEIGHT (RRFIXEDSZ+2)
#define SRV_PORT (RRFIXEDSZ+4)
#define SRV_SERVER (RRFIXEDSZ+6)
#define SRV_FIXEDSZ (RRFIXEDSZ+6)

    unsigned char response[512] = {};
    int response_len = res_query(dns.c_str(), C_IN, T_SRV, response, 512);
    if (response_len > 0)
    {
        HEADER* header = (HEADER*)response;
        unsigned char* start = response + NS_HFIXEDSZ;

        if ((header->tc) || (response_len < NS_HFIXEDSZ))
            return;

        if (header->rcode >= 1 && header->rcode <= 5)
            return;

        int ancount = ntohs(header->ancount);
        int qdcount = ntohs(header->qdcount);
        if (ancount == 0)
            return;

        if (ancount > NS_PACKETSZ)
            return;

        for (int count = qdcount; count > 0; count--)
        {
            int str_len = dn_skipname(start, response + response_len);
            start += str_len + NS_QFIXEDSZ;
        }

        std::vector<unsigned char*> srv;
        for (int count = ancount; count > 0; count--)
        {
            int str_len = dn_skipname(start, response + response_len);
            start += str_len;
            srv.push_back(start);
            start += SRV_FIXEDSZ;
            start += dn_skipname(start, response + response_len);
        }

        for (unsigned i = 0; i < srv.size(); i++)
        {
            char server_name[512] = {};
            if (ns_name_uncompress(response, response + response_len, srv[i] + SRV_SERVER, server_name, 512) < 0)
                continue;
            uint16_t port = ns_get16(srv[i] + SRV_PORT);
            uint16_t weight = ns_get16(srv[i] + SRV_WEIGHT);
            l->emplace_back(std::string(server_name) + ":" +
                StringUtils::toString(port), weight);
        }
    }
#endif
}   // fillStunList

// ----------------------------------------------------------------------------
const std::vector<std::pair<std::string, int> >&
                                          NetworkConfig::getStunList(bool ipv4)
{
    static std::vector<std::pair<std::string, int> > ipv4_list;
    static std::vector<std::pair<std::string, int> > ipv6_list;
    if (ipv4)
    {
        if (ipv4_list.empty())
            NetworkConfig::fillStunList(&ipv4_list, stk_config->m_stun_ipv4);
        return ipv4_list;
    }
    else
    {
        if (ipv6_list.empty())
            NetworkConfig::fillStunList(&ipv6_list, stk_config->m_stun_ipv6);
        return ipv6_list;
    }
}   // getStunList
