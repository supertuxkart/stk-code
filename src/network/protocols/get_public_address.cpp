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
#include "network/network_manager.hpp"
#include "network/client_network_manager.hpp"
#include "network/protocols/connect_to_server.hpp"
#include "network/network_interface.hpp"
#include "utils/log.hpp"

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

const uint8_t GetPublicAddress::m_stun_magic_cookie[4] = {0x21, 0x12, 0xA4, 0x42}; // make the linker happy

/** Creates a STUN request and sends it to a random STUN server selected from
 *  the list stored in the config file. See https://tools.ietf.org/html/rfc5389#section-6
 *  for details on the message structure.
 *  The request is send through m_transaction_host, from which the answer
 *  will be retrieved by parseStunResponse()
 */
void GetPublicAddress::createStunRequest()
{
    uint8_t bytes[21]; // the message to be sent
    // bytes 0-1: the type of the message
    uint16_t message_type = 0x0001; // binding request
    bytes[0] = (uint8_t)(message_type>>8);
    bytes[1] = (uint8_t)(message_type);

    // bytes 2-3: message length added to header (attributes)
    uint16_t message_length = 0x0000;
    bytes[2] = (uint8_t)(message_length>>8);
    bytes[3] = (uint8_t)(message_length);

    // bytes 4-7: magic cookie to recognize the stun protocol
    for (int i = 0; i < 4; i++)
        bytes[i + 4] = m_stun_magic_cookie[i];


    // bytes 8-19: the transaction id
    for (int i = 0; i < 12; i++)
    {
        uint8_t random_byte = rand() % 256;
        bytes[i+8] = random_byte;
        m_stun_tansaction_id[i] = random_byte;
    }
    bytes[20] = '\0';

    // time to pick a random stun server
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
        Log::error("GetPublicAddress", "Error in getaddrinfo: %s", gai_strerror(status));
        return;
    }
    assert (res != NULL); // documentation says it points to "one or more addrinfo structures"

    struct sockaddr_in* current_interface = (struct sockaddr_in*)(res->ai_addr);
    m_stun_server_ip = ntohl(current_interface->sin_addr.s_addr);
    m_transaction_host = new STKHost();
    m_transaction_host->setupClient(1, 1, 0, 0);
    m_transaction_host->sendRawPacket(bytes, 20, TransportAddress(m_stun_server_ip, m_stun_server_port));
    freeaddrinfo(res);
    m_state = STUN_REQUEST_SENT;
}

/**
 * Gets the response from the STUN server, checks it for its validity and
 * then parses the answer into address and port
 * \return "" if the address could be parsed or an error message
*/
std::string GetPublicAddress::parseStunResponse()
{
    uint8_t* data = m_transaction_host->receiveRawPacket(TransportAddress(m_stun_server_ip, m_stun_server_port), 2000);
    if (!data)
        return "STUN response contains no data at all";

    // check that the stun response is a response, contains the magic cookie and the transaction ID
    if (data[0] != 0x01 || data[1] != 0x01)
        return "STUN response doesn't contain the magic cookie";

    for (int i = 0; i < 4; i++)
    {
        if (data[i + 4] != m_stun_magic_cookie[i])
            return "STUN response doesn't contain the magic cookie";
    }

    for (int i = 0; i < 12; i++)
    {
        if (data[i+8] != m_stun_tansaction_id[i])
            return "STUN response doesn't contain the transaction ID";
    }

    Log::debug("GetPublicAddress", "The STUN server responded with a valid answer");
    int message_size = data[2]*256+data[3];

    // The stun message is valid, so we parse it now:
    uint8_t* attributes = data+20;
    if (message_size == 0)
        return "STUN response does not contain any information.";
    if (message_size < 4) // cannot even read the size
        return "STUN response is too short.";


    // Those are the port and the address to be detected
    uint16_t port;
    uint32_t address;
    while (true)
    {
        int type = attributes[0]*256+attributes[1];
        int size = attributes[2]*256+attributes[3];
        if (type == 0 || type == 1)
        {
            assert(size == 8);
            assert(attributes[5] == 0x01); // IPv4 only
            port = attributes[6]*256+attributes[7];
            // The (IPv4) address was sent as 4 distinct bytes,
            // but needs to be packed into one 4-byte int
            address = (attributes[8]<<24 & 0xFF000000) +
                      (attributes[9]<<16 & 0x00FF0000) +
                      (attributes[10]<<8 & 0x0000FF00) +
                      (attributes[11]    & 0x000000FF);
            break;
        }
        attributes = attributes + 4 + size;
        message_size -= 4 + size;
        if (message_size == 0)
            return "STUN response is invalid.";
        if (message_size < 4) // cannot even read the size
            return "STUN response is invalid.";
    }

    // finished parsing, we know our public transport address
    Log::debug("GetPublicAddress", "The public address has been found: %i.%i.%i.%i:%i",
                 address>>24&0xff, address>>16&0xff, address>>8&0xff, address&0xff, port);
    TransportAddress* addr = static_cast<TransportAddress*>(m_callback_object);
    addr->ip = address;
    addr->port = port;

    // The address and the port are known, so the connection can be closed
    m_state = EXITING;
    m_listener->requestTerminate(this);

    return "";
}

/** Detects public IP-address and port by first sending a request to a randomly
 * selected STUN server and then parsing and validating the response */
void GetPublicAddress::asynchronousUpdate()
{
    if (m_state == NOTHING_DONE)
    {
        createStunRequest();
    }
    if (m_state == STUN_REQUEST_SENT)
    {
        std::string message = parseStunResponse();
        if (message != "")
        {
            Log::warn("GetPublicAddress", "%s", message.c_str());
            m_state = NOTHING_DONE;
        }
    }
}
