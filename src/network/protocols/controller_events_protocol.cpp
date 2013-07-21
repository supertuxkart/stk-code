#include "network/protocols/controller_events_protocol.hpp"

#include "modes/world.hpp"
#include "karts/abstract_kart.hpp"
#include "network/network_manager.hpp"
#include "network/network_world.hpp"
#include "utils/log.hpp"

//-----------------------------------------------------------------------------

ControllerEventsProtocol::ControllerEventsProtocol() :
        Protocol(NULL, PROTOCOL_CONTROLLER_EVENTS)
{
}

//-----------------------------------------------------------------------------

ControllerEventsProtocol::~ControllerEventsProtocol()
{
}

//-----------------------------------------------------------------------------

void ControllerEventsProtocol::setup()
{
    m_self_controller_index = 0;
    std::vector<AbstractKart*> karts = World::getWorld()->getKarts();
    std::vector<STKPeer*> peers = NetworkManager::getInstance()->getPeers();
    for (unsigned int i = 0; i < karts.size(); i++)
    {
        if (karts[i]->getIdent() == NetworkWorld::getInstance()->m_self_kart)
        {
            Log::info("ControllerEventsProtocol", "My id is %d", i);
            m_self_controller_index = i;
        }
        STKPeer* peer = NULL;
        if (m_listener->isServer())
        {
            for (unsigned int j = 0; j < peers.size(); j++)
            {
                if (peers[j]->getPlayerProfile()->kart_name == karts[i]->getIdent())
                {
                    peer = peers[j];
                }
            }
        }
        else
        {
            if (peers.size() > 0)
                peer = peers[0];
        }
        if (peer == NULL)
        {
            Log::error("ControllerEventsProtocol", "Couldn't find the peer corresponding to the kart.");
        }
        m_controllers.push_back(std::pair<Controller*, STKPeer*>(karts[i]->getController(), peer));
    }
}

//-----------------------------------------------------------------------------

void ControllerEventsProtocol::notifyEvent(Event* event)
{
    if (event->data.size() < 17)
    {
        Log::error("ControllerEventsProtocol", "The data supplied was not complete. Size was %d.", event->data.size());
        return;
    }
    uint32_t token = event->data.gui32();
    NetworkString pure_message = event->data;
    pure_message.removeFront(4);
    if (token != (*event->peer)->getClientServerToken())
    {
        Log::error("ControllerEventsProtocol", "Bad token from peer.");
        return;
    }
    NetworkString ns = pure_message;
    float event_timestamp = ns.getFloat();
    ns.removeFront(4);
    uint8_t client_index = -1;
    while (ns.size() >= 9)
    {
        uint8_t controller_index = ns.gui8();
        client_index = controller_index;
        uint8_t serialized_1 = ns.gui8(1), serialized_2 = ns.gui8(2), serialized_3 = ns.gui8(3);
        PlayerAction action  = (PlayerAction)(ns.gui8(4));
        int action_value = ns.gui32(5);

        KartControl* controls = m_controllers[controller_index].first->getControls();
        controls->m_brake       = serialized_1 & 0b01000000;
        controls->m_nitro       = serialized_1 & 0b00100000;
        controls->m_rescue      = serialized_1 & 0b00010000;
        controls->m_fire        = serialized_1 & 0b00001000;
        controls->m_look_back   = serialized_1 & 0b00000100;
        controls->m_skid        = KartControl::SkidControl(serialized_1 & 0b00000011);

        m_controllers[controller_index].first->action(action, action_value);
        ns.removeFront(9);
        //Log::info("ControllerEventProtocol", "Registered one action.");
    }
    if (ns.size() > 0 && ns.size() != 9)
    {
        Log::warn("ControllerEventProtocol", "The data seems corrupted.");
        return;
    }
    if (client_index < 0)
    {
        Log::warn("ControllerEventProtocol", "Couldn't have a client id.");
        return;
    }
    if (m_listener->isServer())
    {
        // notify everybody of the event :
        for (unsigned int i = 0; i < m_controllers.size(); i++)
        {
            if (i == client_index) // don't send that message to the sender
                continue;
            NetworkString ns2;
            ns2.ai32(m_controllers[i].second->getClientServerToken());
            ns2 += pure_message;
            m_listener->sendMessage(this, m_controllers[i].second, ns2, false);
        }
    }
}

//-----------------------------------------------------------------------------

void ControllerEventsProtocol::update()
{
}

//-----------------------------------------------------------------------------

void ControllerEventsProtocol::controllerAction(Controller* controller,
        PlayerAction action, int value)
{
    assert(!m_listener->isServer());

    KartControl* controls = controller->getControls();
    uint8_t serialized_1 = 0;
    serialized_1 |= (controls->m_brake==true);
    serialized_1 <<= 1;
    serialized_1 |= (controls->m_nitro==true);
    serialized_1 <<= 1;
    serialized_1 |= (controls->m_rescue==true);
    serialized_1 <<= 1;
    serialized_1 |= (controls->m_fire==true);
    serialized_1 <<= 1;
    serialized_1 |= (controls->m_look_back==true);
    serialized_1 <<= 2;
    serialized_1 += controls->m_skid;
    uint8_t serialized_2 = (uint8_t)(controls->m_accel*255.0);
    uint8_t serialized_3 = (uint8_t)(controls->m_steer*127.0);

    NetworkString ns;
    ns.ai32(m_controllers[m_self_controller_index].second->getClientServerToken());
    ns.af(World::getWorld()->getTime());
    ns.ai8(m_self_controller_index);
    ns.ai8(serialized_1).ai8(serialized_2).ai8(serialized_3);
    ns.ai8((uint8_t)(action)).ai32(value);

    m_listener->sendMessage(this, ns, false); // send message to server
}


