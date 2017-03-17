#ifndef GAME_EVENTS_PROTOCOL_HPP
#define GAME_EVENTS_PROTOCOL_HPP

#include "network/protocol.hpp"
#include "utils/cpp2011.hpp"

class AbstractKart;
class Item;

class GameEventsProtocol : public Protocol
{
private:
    enum GameEventType {
        GE_CLIENT_STARTED_RSG = 0x01,
        GE_ITEM_COLLECTED     = 0x02,
        GE_KART_FINISHED_RACE = 0x03
    };   // GameEventType

    /** Count how many clients have started 'ready'. The server
     *  will only go to its 'ready' phase if all client shave done so.
     *  This means the server time is far enough behind the clients
     *  that at time T all client messages for time T have been 
     *  received (short of latency spikes). */
    int m_count_ready_clients;

public:
             GameEventsProtocol();
    virtual ~GameEventsProtocol();

    virtual bool notifyEvent(Event* event) OVERRIDE;
    void collectedItem(Item* item, AbstractKart* kart);
    void collectedItem(const NetworkString &ns);
    void kartFinishedRace(AbstractKart *kart, float time);
    void clientHasStarted();
    void receivedClientHasStarted(Event *event);
    void kartFinishedRace(const NetworkString &ns);
    virtual void setup() OVERRIDE;
    virtual void update(float dt) OVERRIDE {};
    virtual void asynchronousUpdate() OVERRIDE{}
    // ------------------------------------------------------------------------
    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE 
    {
        return false; 
    }   // notifyEventAsynchronous


};   // class GameEventsProtocol

#endif // GAME_EVENTS_PROTOCOL_HPP
