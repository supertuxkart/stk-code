//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2024 SuperTuxKart-Team
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

#ifdef ENABLE_SQLITE3

#ifndef DATABASE_CONNECTOR_HPP
#define DATABASE_CONNECTOR_HPP

#include "utils/string_utils.hpp"
#include "utils/time.hpp"

#include <vector>
#include <iostream>
#include <functional>
#include <memory>
#include <string>
#include <sqlite3.h>

class SocketAddress;
class STKPeer;
class NetworkPlayerProfile;


class DatabaseConnector
{
private:
    sqlite3* m_db;
    std::string m_server_stats_table;
    bool m_ip_ban_table_exists;
    bool m_ipv6_ban_table_exists;
    bool m_online_id_ban_table_exists;
    bool m_ip_geolocation_table_exists;
    bool m_ipv6_geolocation_table_exists;
    bool m_player_reports_table_exists;
    uint64_t m_last_poll_db_time;

public:
    struct IpBanTableData
    {
        int row_id;
        uint32_t ip_start;
        uint32_t ip_end;
        std::string reason;
        std::string description;
    };
    struct Ipv6BanTableData {
        int row_id;
        std::string ipv6_cidr;
        std::string reason;
        std::string description;
    };
    struct OnlineIdBanTableData {
        int row_id;
        uint32_t online_id;
        std::string reason;
        std::string description;
    };
    void initDatabase();
    void destroyDatabase();

    bool easySQLQuery(const std::string& query,
        std::function<void(sqlite3_stmt* stmt)> bind_function = nullptr) const;

    void checkTableExists(const std::string& table, bool& result);

    std::string ip2Country(const SocketAddress& addr) const;

    std::string ipv62Country(const SocketAddress& addr) const;

    static void upperIPv6SQL(sqlite3_context* context, int argc,
                         sqlite3_value** argv);
    static void insideIPv6CIDRSQL(sqlite3_context* context, int argc,
                       sqlite3_value** argv);
    void writeDisconnectInfoTable(STKPeer* peer);
    void initServerStatsTable();
    bool writeReport(STKPeer* reporter, std::shared_ptr<NetworkPlayerProfile> reporter_npp,
                     STKPeer* reporting, std::shared_ptr<NetworkPlayerProfile> reporting_npp,
                     irr::core::stringw& info);
    bool hasDatabase() const                        { return m_db != nullptr; }
    bool hasServerStatsTable() const  { return !m_server_stats_table.empty(); }
    bool hasPlayerReportsTable() const
                                      { return m_player_reports_table_exists; }
    bool hasIpBanTable() const                { return m_ip_ban_table_exists; }
    bool hasIpv6BanTable() const            { return m_ipv6_ban_table_exists; }
    bool hasOnlineIdBanTable() const   { return m_online_id_ban_table_exists; }
    bool isTimeToPoll() const
            { return StkTime::getMonoTimeMs() >= m_last_poll_db_time + 60000; }
    void updatePollTime()   { m_last_poll_db_time = StkTime::getMonoTimeMs(); }
    std::vector<IpBanTableData> getIpBanTableData(uint32_t ip = 0) const;
    std::vector<Ipv6BanTableData> getIpv6BanTableData(std::string ipv6 = "") const;
    std::vector<OnlineIdBanTableData> getOnlineIdBanTableData(uint32_t online_id = 0) const;
    void increaseIpBanTriggerCount(uint32_t ip_start, uint32_t ip_end) const;
    void increaseIpv6BanTriggerCount(const std::string& ipv6_cidr) const;
    void increaseOnlineIdBanTriggerCount(uint32_t online_id) const;
    void clearOldReports();
    void setDisconnectionTimes(std::vector<uint32_t>& present_hosts);
    void saveAddressToIpBanTable(const SocketAddress& addr);
    void onPlayerJoinQueries(std::shared_ptr<STKPeer> peer, uint32_t online_id,
        unsigned player_count, const std::string& country_code);
    void listBanTable();
};



#endif // ifndef DATABASE_CONNECTOR_HPP
#endif // ifdef ENABLE_SQLITE3
