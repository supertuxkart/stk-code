#ifndef HEADER_REQUEST_CONNECTION_HPP
#define HEADER_REQUEST_CONNECTION_HPP

#include "network/protocol.hpp"
#include "online/xml_request.hpp"

#include <memory>
class Server;

class RequestConnection : public Protocol
{
protected:
    /** Id of the server to join. */
    std::shared_ptr<Server> m_server;
    enum STATE
    {
        NONE,
        EXITING
    };

    /** State of this connection. */
    STATE m_state;

public:
    // ------------------------------------------------------------------------
             RequestConnection(std::shared_ptr<Server> server);
    virtual ~RequestConnection();
    virtual void setup() OVERRIDE;
    virtual void asynchronousUpdate() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool notifyEvent(Event* event) OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE { return true; }
    // ------------------------------------------------------------------------
    virtual void update(int ticks) OVERRIDE {}


};   // RequestConnection

#endif // REQUEST_CONNECTION_HPP
