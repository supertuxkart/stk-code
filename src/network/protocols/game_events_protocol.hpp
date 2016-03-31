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
        GE_START_READY_SET_GO = 0x01,
        GE_ITEM_COLLECTED     = 0x02,
        GE_KART_FINISHED_RACE = 0x03
    };   // GameEventType

public:
             GameEventsProtocol();
    virtual ~GameEventsProtocol();

    virtual bool notifyEvent(Event* event) OVERRIDE;
    void collectedItem(Item* item, AbstractKart* kart);
    void collectedItem(const NetworkString &ns);
    void kartFinishedRace(AbstractKart *kart, float time);
    void kartFinishedRace(const NetworkString &ns);
    void startReadySetGo();
    void receivedReadySetGo();
    virtual void setup() OVERRIDE {};
    virtual void update(float dt) OVERRIDE {};
    virtual void asynchronousUpdate() OVERRIDE{}
    // ------------------------------------------------------------------------
    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE 
    {
        return false; 
    }   // notifyEventAsynchronous


};   // class GameEventsProtocol

#endif // GAME_EVENTS_PROTOCOL_HPP
