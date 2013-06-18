#include "get_public_address.hpp"

#include "../network_manager.hpp"
#include "connect_to_server.hpp"

#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int stunRand()
{
    static bool init = false;
    if (!init)
    {
        srand(time(NULL));
        init = true;
    }
    return rand();
}

GetPublicAddress::GetPublicAddress(CallbackObject* callbackObject) : Protocol(callbackObject)
{
    m_type = GET_PUBLIC_ADDRESS;
}

GetPublicAddress::~GetPublicAddress()
{
}

void GetPublicAddress::messageReceived(uint8_t* data)
{

}

void GetPublicAddress::setup()
{
    m_state = NOTHING_DONE;
}

void GetPublicAddress::start()
{
}
    
void GetPublicAddress::update()
{
    if (m_state == NOTHING_DONE)
    {
        // format :               00MMMMMCMMMCMMMM (cf rfc 5389)
        uint16_t message_type = 0b0000000000000001; // binding request
        m_stunTransactionID[0] = stunRand();
        m_stunTransactionID[1] = stunRand();
        m_stunTransactionID[2] = stunRand();
        uint16_t message_length = 0x0000;
        
        uint8_t bytes[21]; // the message to be sent
        // bytes 0-1 : the type of the message, 
        bytes[0] = (uint8_t)(message_type>>8);
        bytes[1] = (uint8_t)(message_type);
        
        // bytes 2-3 : message length added to header (attributes)
        bytes[2] = (uint8_t)(message_length>>8);
        bytes[3] = (uint8_t)(message_length);
        
        // bytes 4-7 : magic cookie to recognize the stun protocol
        bytes[4] = (uint8_t)(m_stunMagicCookie>>24); 
        bytes[5] = (uint8_t)(m_stunMagicCookie>>16);
        bytes[6] = (uint8_t)(m_stunMagicCookie>>8);
        bytes[7] = (uint8_t)(m_stunMagicCookie);
        
        // bytes 8-19 : the transaction id
        bytes[8] = (uint8_t)(m_stunTransactionID[0]>>24);
        bytes[9] = (uint8_t)(m_stunTransactionID[0]>>16);
        bytes[10] = (uint8_t)(m_stunTransactionID[0]>>8);
        bytes[11] = (uint8_t)(m_stunTransactionID[0]);
        bytes[12] = (uint8_t)(m_stunTransactionID[1]>>24);
        bytes[13] = (uint8_t)(m_stunTransactionID[1]>>16);
        bytes[14] = (uint8_t)(m_stunTransactionID[1]>>8);
        bytes[15] = (uint8_t)(m_stunTransactionID[1]);
        bytes[16] = (uint8_t)(m_stunTransactionID[2]>>24);
        bytes[17] = (uint8_t)(m_stunTransactionID[2]>>16);
        bytes[18] = (uint8_t)(m_stunTransactionID[2]>>8);
        bytes[19] = (uint8_t)(m_stunTransactionID[2]);
        bytes[20] = '\0'; 
        
        printf("Querrying STUN server 132.177.123.6\n");
        unsigned int dst = 132*256*256*256+177*256*256+123*256+6;
        NetworkManager::setManualSocketsMode(true);
        NetworkManager::getHost()->sendRawPacket(bytes, 20, dst, 3478);
        m_state = TEST_SENT;
    }
    if (m_state == TEST_SENT)
    {
        unsigned int dst = 132*256*256*256+177*256*256+123*256+6;
        uint8_t* data = NetworkManager::getHost()->receiveRawPacket(dst, 3478);
        assert(data);
        
        // check that the stun response is a response, contains the magic cookie and the transaction ID
        if (    data[0] == 0b01 &&
                data[1] == 0b01 &&
                data[4] ==  (uint8_t)(m_stunMagicCookie>>24)        &&
                data[5] ==  (uint8_t)(m_stunMagicCookie>>16)        &&
                data[6] ==  (uint8_t)(m_stunMagicCookie>>8)         &&
                data[7] ==  (uint8_t)(m_stunMagicCookie))
        {
            if(
                data[8] ==  (uint8_t)(m_stunTransactionID[0]>>24)   &&
                data[9] ==  (uint8_t)(m_stunTransactionID[0]>>16)   &&
                data[10] == (uint8_t)(m_stunTransactionID[0]>>8 )   &&
                data[11] == (uint8_t)(m_stunTransactionID[0]    )   &&
                data[12] == (uint8_t)(m_stunTransactionID[1]>>24)   &&
                data[13] == (uint8_t)(m_stunTransactionID[1]>>16)   &&
                data[14] == (uint8_t)(m_stunTransactionID[1]>>8 )   &&
                data[15] == (uint8_t)(m_stunTransactionID[1]    )   &&
                data[16] == (uint8_t)(m_stunTransactionID[2]>>24)   &&
                data[17] == (uint8_t)(m_stunTransactionID[2]>>16)   &&
                data[18] == (uint8_t)(m_stunTransactionID[2]>>8 )   &&
                data[19] == (uint8_t)(m_stunTransactionID[2]    ))
            {
                printf("The STUN server responded with a valid answer\n");
                int messageSize = data[2]*256+data[3];
                
                // parse the stun message now:
                bool finish = false;
                uint8_t* attributes = data+20;
                if (messageSize == 0)
                {
                    printf("STUN answer does not contain any information.\n");
                    finish = true;
                }
                if (messageSize < 4) // cannot even read the size
                {
                    printf("STUN message is not valid.\n");
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
                    messageSize -= 4 + size;
                    if (messageSize == 0)
                        finish = true;
                    if (messageSize < 4) // cannot even read the size
                    {
                        printf("STUN message is not valid.\n");
                        finish = true;
                    }
                }
                // finished parsing, we know our public transport address
                if (valid)
                {
                    printf("The public address has been found : %i.%i.%i.%i:%i\n", address>>24&0xff, address>>16&0xff, address>>8&0xff, address&0xff, port);
                    m_state = ADDRESS_KNOWN;
                    NetworkManager::setManualSocketsMode(false); 
                    ConnectToServer* cbObj = static_cast<ConnectToServer*>(m_callbackObject);
                    cbObj->setSelfAddress(address, port);
                    m_listener->runProtocol(cbObj);
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
        // return the information and terminate the protocol
        m_listener->protocolTerminated(this);
    }
}
