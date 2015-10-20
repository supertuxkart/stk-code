#include "network/protocols/game_events_protocol.hpp"

#include "karts/abstract_kart.hpp"
#include "items/attachment.hpp"
#include "items/item.hpp"
#include "items/item_manager.hpp"
#include "items/powerup.hpp"
#include "modes/world.hpp"
#include "network/network_manager.hpp"
#include <stdint.h>

GameEventsProtocol::GameEventsProtocol() : Protocol(NULL, PROTOCOL_GAME_EVENTS)
{
}

GameEventsProtocol::~GameEventsProtocol()
{
}

bool GameEventsProtocol::notifyEvent(Event* event)
{
    if (event->getType() != EVENT_TYPE_MESSAGE)
        return true;
    NetworkString data = event->data();
    if (data.size() < 5) // for token and type
    {
        Log::warn("GameEventsProtocol", "Too short message.");
        return true;
    }
    if ( event->getPeer()->getClientServerToken() != data.gui32())
    {
        Log::warn("GameEventsProtocol", "Bad token.");
        return true;
    }
    int8_t type = data.gui8(4);
    data.removeFront(5);
    switch (type)
    {
        case 0x01: // item picked
        {
            if (data.size() < 6)
            {
                Log::warn("GameEventsProtocol", "Too short message.");
                return true;
            }
            uint32_t item_id = data.gui32();
            uint8_t powerup_type = data.gui8(4);
            uint8_t kart_race_id = data.gui8(5);
            // now set the kart powerup
            AbstractKart* kart = World::getWorld()->getKart(
                NetworkManager::getInstance()->getGameSetup()->getProfile(kart_race_id)->world_kart_id);
            ItemManager::get()->collectedItem(
                ItemManager::get()->getItem(item_id),
                kart,
                powerup_type);
            Log::info("GameEventsProtocol", "Item %d picked by a player.", powerup_type);
        }   break;
        default:
            Log::warn("GameEventsProtocol", "Unkown message type.");
            break;
    }
    return true;
}

void GameEventsProtocol::setup()
{
}

void GameEventsProtocol::update()
{
}

void GameEventsProtocol::collectedItem(Item* item, AbstractKart* kart)
{
    GameSetup* setup = NetworkManager::getInstance()->getGameSetup();
    assert(setup);
    const NetworkPlayerProfile* player_profile = setup->getProfile(kart->getIdent()); // use kart name

    std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();
    for (unsigned int i = 0; i < peers.size(); i++)
    {
        NetworkString ns(11);
        ns.ai32(peers[i]->getClientServerToken());
        // 0x01 : item picked : send item id, powerup type and kart race id
        uint8_t powerup = 0;
        if (item->getType() == Item::ITEM_BANANA)
            powerup = (int)(kart->getAttachment()->getType());
        else if (item->getType() == Item::ITEM_BONUS_BOX)
            powerup = (((int)(kart->getPowerup()->getType()) << 4)&0xf0) + (kart->getPowerup()->getNum()&0x0f);

        ns.ai8(0x01).ai32(item->getItemId()).ai8(powerup).ai8(player_profile->race_id); // send item,
        m_listener->sendMessage(this, peers[i], ns, true); // reliable
        Log::info("GameEventsProtocol", "Notified a peer that a kart collected item %d.", (int)(kart->getPowerup()->getType()));
    }
}
