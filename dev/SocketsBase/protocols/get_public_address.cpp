#include "get_public_address.hpp"

#include "../network_manager.hpp"

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

GetPublicAddress::GetPublicAddress() : Protocol()
{
    m_type = GET_PUBLIC_ADDRESS;
}

GetPublicAddress::~GetPublicAddress()
{
}

void GetPublicAddress::messageReceived(uint8_t* data)
{
    assert(data);
    if (m_state == TEST_SENT && sizeof(data) >= 20)
    {
        
        m_state = ADDRESS_KNOWN;
    }
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
        
        unsigned int dst = 132*256*256*256+177*256*256+123*256+6;
        NetworkManager::sendRawPacket(bytes, 20, dst, 3478);
        m_state = TEST_SENT;
        
        uint8_t* data = NetworkManager::receiveRawPacket();
        assert(data);
        // check that the stun response is a response, contains the magic cookie and the transaction ID
        if (    data[0] == 0b01 &&
                data[1] == 0b01 &&
                data[4] ==  (uint8_t)(m_stunMagicCookie>>24)        &&
                data[5] ==  (uint8_t)(m_stunMagicCookie>>16)        &&
                data[6] ==  (uint8_t)(m_stunMagicCookie>>8)         &&
                data[7] ==  (uint8_t)(m_stunMagicCookie)            &&
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
            printf("the stun server reponded a valid answer\n");
            int messageSize = data[2]*256+data[3];
            printf("the answer is %i bytes long\n", messageSize);
        }
    }
}
