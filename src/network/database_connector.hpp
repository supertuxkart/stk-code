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

#include <functional>
#include <iostream>
#include <memory>
#include <sqlite3.h>
#include <string>
#include <vector>

class SocketAddress;
class STKPeer;
class NetworkPlayerProfile;

/** The purpose of Binder and BinderCollection structures is to allow
 *   putting values to bind inside StringUtils::insertValues, which is commonly
 *   used for values that don't require binding (such as integers).
 *  Unlike previously used approach with separate query formation and binding,
 *   the arguments are written in the code in the order of query appearance
 *   (even though real binding still happens later). It also avoids repeated
 *   binding code.
 *
 *  Syntax looks as follows:
 *  std::shared_ptr<BinderCollection> coll = std::make_shared...;
 *  std::string query_string = StringUtils::insertValues(
 *      "query contents with wildcards of type %d, %s, %u, ..."
 *      "where %s is put for values that will be bound later",
 *      values to insert, ..., Binder(coll, other parameters), ...);
 *  Then the bind function (e.g. for usage in easySQLQuery) should be
 *  coll->getBindFunction().
 */

struct Binder;

/** BinderCollection is a structure that collects Binder objects used in an
 *   SQL query formed with insertValues() (see above). For a single query, a
 *   single instance of BinderCollection should be used. After a query is
 *   formed, BinderCollection can produce bind function to use with sqlite3.
 */
struct BinderCollection
{
    std::vector<std::shared_ptr<Binder>> m_binders;

    std::function<void(sqlite3_stmt* stmt)> getBindFunction() const;
};

/** Binder is a wrapper for a string to be bound into an SQL query. See above
 *   for its usage in insertValues(). When it's printed to an output stream
 *   (in particular, this is done in insertValues implementation), this Binder
 *   is added to the query's BinderCollection, and the '?'-placeholder is added
 *   to the query string instead of %s.
 *
 *  When using Binder, make sure that:
 *   - operator << is invoked on it exactly once;
 *   - operator << is invoked on several Binders in the order in which they go
 *     in the query;
 *   - before calling insertValues, there is a %-wildcard corresponding to the
 *     Binder in the query string (and not '?').
 *  For example, when the query formed inside of a function depends on its
 *   arguments, it should be formed part by part, from left to right.
 *  Of course, you can choose the "default" way, binding values separately from
 *   insertValues() call.
 */
struct Binder
{
    std::weak_ptr<BinderCollection> m_collection;
    std::string m_value;
    std::string m_name;
    bool m_use_null_if_empty;

    Binder(std::shared_ptr<BinderCollection> collection, std::string value,
           std::string name = "", bool use_null_if_empty = false):
        m_collection(collection), m_value(value),
        m_name(name), m_use_null_if_empty(use_null_if_empty) {}
};

std::ostream& operator << (std::ostream& os, const Binder& binder);

/** A class that manages the database operations needed for the server to work.
 *   The SQL queries are intended to be placed only within the implementation
 *   of this class, while the logic corresponding to those queries should not
 *   belong here.
 */
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
    /** Corresponds to the row of IPv4 ban table. */
    struct IpBanTableData
    {
        int row_id;
        uint32_t ip_start;
        uint32_t ip_end;
        std::string reason;
        std::string description;
    };
    /** Corresponds to the row of IPv6 ban table. */
    struct Ipv6BanTableData
    {
        int row_id;
        std::string ipv6_cidr;
        std::string reason;
        std::string description;
    };
    /** Corresponds to the row of online id ban table. */
    struct OnlineIdBanTableData
    {
        int row_id;
        uint32_t online_id;
        std::string reason;
        std::string description;
    };
    void initDatabase();
    void destroyDatabase();

    bool easySQLQuery(const std::string& query,
                       std::vector<std::vector<std::string>>* output = nullptr,
               std::function<void(sqlite3_stmt* stmt)> bind_function = nullptr,
                                            std::string null_value = "") const;

    void checkTableExists(const std::string& table, bool& result);

    std::string ip2Country(const SocketAddress& addr) const;

    std::string ipv62Country(const SocketAddress& addr) const;

    static void upperIPv6SQL(sqlite3_context* context, int argc,
                                                         sqlite3_value** argv);
    static void insideIPv6CIDRSQL(sqlite3_context* context, int argc,
                                                         sqlite3_value** argv);
    void writeDisconnectInfoTable(STKPeer* peer);
    void initServerStatsTable();
    bool writeReport(
         STKPeer* reporter, std::shared_ptr<NetworkPlayerProfile> reporter_npp,
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
