//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 SuperTuxKart-Team
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

#include "network/network_manager.hpp"
#include "network/client_network_manager.hpp"
#include "network/protocols/connect_to_server.hpp"
#include "network/network_interface.hpp"

#include "utils/log.hpp"

#include <assert.h>

int stunRand()
{
    static bool init = false;
    if (!init)
    {
        srand((unsigned int)time(NULL));
        init = true;
    }
    return rand();
}

GetPublicAddress::GetPublicAddress(CallbackObject* callback_object) : Protocol(callback_object, PROTOCOL_SILENT)
{
}

GetPublicAddress::~GetPublicAddress()
{
}

void GetPublicAddress::setup()
{
    m_state = NOTHING_DONE;
}

void GetPublicAddress::asynchronousUpdate()
{
    if (m_state == NOTHING_DONE)
    {
        // format :               00MMMMMCMMMCMMMM (cf rfc 5389)
        uint16_t message_type = 0x0001; // binding request
        m_stun_tansaction_id[0] = stunRand();
        m_stun_tansaction_id[1] = stunRand();
        m_stun_tansaction_id[2] = stunRand();
        uint16_t message_length = 0x0000;

        uint8_t bytes[21]; // the message to be sent
        // bytes 0-1 : the type of the message,
        bytes[0] = (uint8_t)(message_type>>8);
        bytes[1] = (uint8_t)(message_type);

        // bytes 2-3 : message length added to header (attributes)
        bytes[2] = (uint8_t)(message_length>>8);
        bytes[3] = (uint8_t)(message_length);

        // bytes 4-7 : magic cookie to recognize the stun protocol
        bytes[4] = (uint8_t)(m_stun_magic_cookie>>24);
        bytes[5] = (uint8_t)(m_stun_magic_cookie>>16);
        bytes[6] = (uint8_t)(m_stun_magic_cookie>>8);
        bytes[7] = (uint8_t)(m_stun_magic_cookie);

        // bytes 8-19 : the transaction id
        bytes[8] = (uint8_t)(m_stun_tansaction_id[0]>>24);
        bytes[9] = (uint8_t)(m_stun_tansaction_id[0]>>16);
        bytes[10] = (uint8_t)(m_stun_tansaction_id[0]>>8);
        bytes[11] = (uint8_t)(m_stun_tansaction_id[0]);
        bytes[12] = (uint8_t)(m_stun_tansaction_id[1]>>24);
        bytes[13] = (uint8_t)(m_stun_tansaction_id[1]>>16);
        bytes[14] = (uint8_t)(m_stun_tansaction_id[1]>>8);
        bytes[15] = (uint8_t)(m_stun_tansaction_id[1]);
        bytes[16] = (uint8_t)(m_stun_tansaction_id[2]>>24);
        bytes[17] = (uint8_t)(m_stun_tansaction_id[2]>>16);
        bytes[18] = (uint8_t)(m_stun_tansaction_id[2]>>8);
        bytes[19] = (uint8_t)(m_stun_tansaction_id[2]);
        bytes[20] = '\0';

        Log::verbose("GetPublicAddress", "Querrying STUN server 132.177.123.6");
        unsigned int dst = (132<<24)+(177<<16)+(123<<8)+6;
        NetworkManager::getInstance()->setManualSocketsMode(true);
        NetworkManager::getInstance()->getHost()->sendRawPacket(bytes, 20, TransportAddress(dst, 3478));
        m_state = TEST_SENT;
    }
    if (m_state == TEST_SENT)
    {
        unsigned int dst = (132<<24)+(177<<16)+(123<<8)+6;
        uint8_t* data = NetworkManager::getInstance()->getHost()->receiveRawPacket(TransportAddress(dst, 3478));
        assert(data);

        // check that the stun response is a response, contains the magic cookie and the transaction ID
        if (    data[0] == 0x01 &&
                data[1] == 0x01 &&
                data[4] ==  (uint8_t)(m_stun_magic_cookie>>24)        &&
                data[5] ==  (uint8_t)(m_stun_magic_cookie>>16)        &&
                data[6] ==  (uint8_t)(m_stun_magic_cookie>>8)         &&
                data[7] ==  (uint8_t)(m_stun_magic_cookie)               )
        {
            if(
                data[8] ==  (uint8_t)(m_stun_tansaction_id[0]>>24)   &&
                data[9] ==  (uint8_t)(m_stun_tansaction_id[0]>>16)   &&
                data[10] == (uint8_t)(m_stun_tansaction_id[0]>>8 )   &&
                data[11] == (uint8_t)(m_stun_tansaction_id[0]    )   &&
                data[12] == (uint8_t)(m_stun_tansaction_id[1]>>24)   &&
                data[13] == (uint8_t)(m_stun_tansaction_id[1]>>16)   &&
                data[14] == (uint8_t)(m_stun_tansaction_id[1]>>8 )   &&
                data[15] == (uint8_t)(m_stun_tansaction_id[1]    )   &&
                data[16] == (uint8_t)(m_stun_tansaction_id[2]>>24)   &&
                data[17] == (uint8_t)(m_stun_tansaction_id[2]>>16)   &&
                data[18] == (uint8_t)(m_stun_tansaction_id[2]>>8 )   &&
                data[19] == (uint8_t)(m_stun_tansaction_id[2]    ))
            {
                Log::verbose("GetPublicAddress", "The STUN server responded with a valid answer");
                int message_size = data[2]*256+data[3];

                // parse the stun message now:
                bool finish = false;
                uint8_t* attributes = data+20;
                if (message_size == 0)
                {
                    Log::error("GetPublicAddress", "STUN answer does not contain any information.");
                    finish = true;
                }
                if (message_size < 4) // cannot even read the size
                {
                    Log::error("GetPublicAddress", "STUN message is not valid.");
                    finish = true;
                }
                uint16_t port;
                uint32_t address;
                bool valid = false;
                while(!finish)
                {
                    int type = attributes[0]*256+attributes[1];
                    int size = attributes[2]*256+attributes[3];
                    switch(type)
                    {
                        case 0:
                        case 1:
                            assert(size == 8);
                            assert(attributes[5] = 0x01); // IPv4 only
                            port = attributes[6]*256+attributes[7];
                            address = (attributes[8]<<24 & 0xFF000000)+(attributes[9]<<16 & 0x00FF0000)+(attributes[10]<<8 & 0x0000FF00)+(attributes[11] & 0x000000FF);
                            finish = true;
                            valid = true;
                            continue;
                            break;
                        default:
                            break;
                    }
                    attributes = attributes + 4 + size;
                    message_size -= 4 + size;
                    if (message_size == 0)
                        finish = true;
                    if (message_size < 4) // cannot even read the size
                    {
                        Log::error("GetPublicAddress", "STUN message is not valid.");
                        finish = true;
                    }
                }
                // finished parsing, we know our public transport address
                if (valid)
                {
                    Log::debug("GetPublicAddress", "The public address has been found : %i.%i.%i.%i:%i", address>>24&0xff, address>>16&0xff, address>>8&0xff, address&0xff, port);
                    m_state = ADDRESS_KNOWN;
                    NetworkManager::getInstance()->setManualSocketsMode(false);
                    TransportAddress* addr = static_cast<TransportAddress*>(m_callback_object);
                    addr->ip = address;
                    addr->port = port;
                }
                else
                    m_state = NOTHING_DONE; // need to re-send the stun request
            }
            else
            {
                m_state = NOTHING_DONE; // need to re-send the stun request
            }
        }
    }
    if (m_state == ADDRESS_KNOWN)
    {
        m_state = EXITING;
        // terminate the protocol
        m_listener->requestTerminate(this);
    }
    if (m_state == EXITING)
    {
    }
}
