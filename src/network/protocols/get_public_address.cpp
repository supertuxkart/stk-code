//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 SuperTuxKart-Team
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

#include "network/protocols/get_public_address.hpp"

#include "config/user_config.hpp"
#include "network/network.hpp"
#include "network/network_config.hpp"
#include "network/network_string.hpp"
#include "network/protocols/connect_to_server.hpp"
#include "utils/log.hpp"
#include "utils/string_utils.hpp"

#include <assert.h>
#include <string>

#ifdef __MINGW32__
#  undef _WIN32_WINNT
#  define _WIN32_WINNT 0x501
#endif

#ifdef WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <netdb.h>
#  include <sys/socket.h>
#endif
#include <sys/types.h>

// make the linker happy
const uint32_t   GetPublicAddress::m_stun_magic_cookie = 0x2112A442;
TransportAddress GetPublicAddress::m_my_address(0, 0);

void GetPublicAddress::setMyIPAddress(const std::string &s)
{
    std::vector<std::string> l = StringUtils::split(s, ':');
    if (l.size() != 2)
    {
        Log::fatal("Invalid IP address '%s'.", s.c_str());
    }
    std::vector<std::string> ip = StringUtils::split(l[0], '.');
    if (ip.size() != 4)
    {
        Log::fatal("Invalid IP address '%s'.", s.c_str());
    }
    uint32_t u = 0;
    for (unsigned int i = 0; i < 4; i++)
    {
        int k;
        StringUtils::fromString(ip[i], k);
        u = (u << 8) + k;
    }
    m_my_address.setIP(u);
    int p;
    StringUtils::fromString(l[1], p);
    m_my_address.setPort(p);
}   // setMyIPAddress

// ============================================================================
GetPublicAddress::GetPublicAddress(CallbackObject *callback)
                : Protocol(PROTOCOL_SILENT, callback)
{
    m_state = NOTHING_DONE;
}   // GetPublicAddress

// ----------------------------------------------------------------------------
/** Creates a STUN request and sends it to a random STUN server selected from
 *  the list stored in the config file. See
 *  https://tools.ietf.org/html/rfc5389#section-6
 *  for details on the message structure.
 *  The request is send through m_transaction_host, from which the answer
 *  will be retrieved by parseStunResponse()
 */
void GetPublicAddress::createStunRequest()
{
    // Pick a random stun server
    std::vector<std::string> stun_servers = UserConfigParams::m_stun_servers;

    const char* server_name = stun_servers[rand() % stun_servers.size()].c_str();
    Log::debug("GetPublicAddress", "Using STUN server %s", server_name);

    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    // Resolve the stun server name so we can send it a STUN request
    int status = getaddrinfo(server_name, NULL, &hints, &res);
    if (status != 0)
    {
        Log::error("GetPublicAddress", "Error in getaddrinfo: %s",
                   gai_strerror(status));
        return;
    }
    // documentation says it points to "one or more addrinfo structures"
    assert(res != NULL);
    struct sockaddr_in* current_interface = (struct sockaddr_in*)(res->ai_addr);
    m_stun_server_ip = ntohl(current_interface->sin_addr.s_addr);

    // Create a new socket for the stun server.
    m_transaction_host = new Network(1, 1, 0, 0);

    // Assemble the message for the stun server
    BareNetworkString s(20);

    // bytes 0-1: the type of the message
    // bytes 2-3: message length added to header (attributes)
    uint16_t message_type = 0x0001; // binding request
    uint16_t message_length = 0x0000;
    s.addUInt16(message_type).addUInt16(message_length)
                             .addUInt32(0x2112A442);
    // bytes 8-19: the transaction id
    for (int i = 0; i < 12; i++)
    {
        uint8_t random_byte = rand() % 256;
        s.addUInt8(random_byte);
        m_stun_tansaction_id[i] = random_byte;
    }

    m_transaction_host->sendRawPacket(s,
                                      TransportAddress(m_stun_server_ip,
                                                       m_stun_server_port) );
    freeaddrinfo(res);
    m_state = STUN_REQUEST_SENT;
}   // createStunRequest

// ----------------------------------------------------------------------------
/**
 * Gets the response from the STUN server, checks it for its validity and
 * then parses the answer into address and port
 * \return "" if the address could be parsed or an error message
*/
std::string GetPublicAddress::parseStunResponse()
{
    TransportAddress sender;
    const int LEN = 2048;
    char buffer[LEN];
    int len = m_transaction_host->receiveRawPacket(buffer, LEN, &sender, 2000);

    if(sender.getIP()!=m_stun_server_ip)
    {
        TransportAddress stun(m_stun_server_ip, m_stun_server_port);
        Log::warn("GetPublicAddress", 
                  "Received stun response from %s instead of %s.",
                  sender.toString().c_str(), stun.toString().c_str());
    }

    if (len<0)
        return "STUN response contains no data at all";

    // Convert to network string.
    BareNetworkString datas(buffer, len);

    // check that the stun response is a response, contains the magic cookie
    // and the transaction ID
    if (datas.getUInt16() != 0x0101)
        return "STUN response doesn't contain the magic cookie";
    int message_size = datas.getUInt16();
    if (datas.getUInt32() != m_stun_magic_cookie)
    {
        return "STUN response doesn't contain the magic cookie";
    }

    for (int i = 0; i < 12; i++)
    {
        if (datas.getUInt8() != m_stun_tansaction_id[i])
            return "STUN response doesn't contain the transaction ID";
    }

    Log::debug("GetPublicAddress",
               "The STUN server responded with a valid answer");

    // The stun message is valid, so we parse it now:
    if (message_size == 0)
        return "STUN response does not contain any information.";
    if (message_size < 4) // cannot even read the size
        return "STUN response is too short.";

    // Those are the port and the address to be detected
    
    int pos = 20;
    while (true)
    {
        int type = datas.getUInt16();
        int size = datas.getUInt16();
        if (type == 0 || type == 1)
        {
            assert(size == 8);
            datas.getUInt8();    // skip 1 byte
            assert(datas.getUInt8() == 0x01); // Family IPv4 only
            uint16_t port = datas.getUInt16();
            uint32_t ip   = datas.getUInt32();
            TransportAddress address(ip, port);
            // finished parsing, we know our public transport address
            Log::debug("GetPublicAddress", 
                       "The public address has been found: %s",
                        address.toString().c_str());
            NetworkConfig::get()->setMyAddress(address);
            break;
        }   // type = 0 or 1
        datas.skip(4 + size);
        message_size -= 4 + size;
        if (message_size == 0)
            return "STUN response is invalid.";
        if (message_size < 4) // cannot even read the size
            return "STUN response is invalid.";
    }   // while true

    return "";
}   // parseStunResponse

// ----------------------------------------------------------------------------
/** Detects public IP-address and port by first sending a request to a randomly
 * selected STUN server and then parsing and validating the response */
void GetPublicAddress::asynchronousUpdate()
{
    // If the user has specified an address, use it instead of the stun protocol.
    if (m_my_address.getIP() != 0 && m_my_address.getPort() != 0)
    {
        NetworkConfig::get()->setMyAddress(m_my_address);
        m_state = EXITING;
        requestTerminate();
    }
//#define LAN_TEST
#ifdef LAN_TEST
    TransportAddress address(0x7f000001, 4);
    NetworkConfig::get()->setMyAddress(address);
    m_state = EXITING;
    requestTerminate();
    return;
#endif

    if (m_state == NOTHING_DONE)
    {
        createStunRequest();
    }
    if (m_state == STUN_REQUEST_SENT)
    {
        std::string message = parseStunResponse();
        delete m_transaction_host;
        if (message != "")
        {
            Log::warn("GetPublicAddress", "%s", message.c_str());
            m_state = NOTHING_DONE;  // try again
        }
        else
        {
            // The address and the port are known, so the connection can be closed
            m_state = EXITING;
            requestTerminate();
        }
    }
}   // asynchronousUpdate
