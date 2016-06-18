#ifndef STOP_SERVER_HPP
#define STOP_SERVER_HPP

#include "network/protocol.hpp"
#include "utils/cpp2011.hpp"

namespace Online { class XMLRequest;  }

/*! \brief Removes the server info from the database
 */

class StopServer : public Protocol
{
private:
    Online::XMLRequest* m_request;
    enum STATE
    {
        NONE,
        REQUEST_PENDING,
        DONE,
        EXITING
    };
    STATE m_state;
public:
    StopServer();
    virtual ~StopServer();

    virtual bool notifyEventAsynchronous(Event* event) OVERRIDE;
    virtual void setup() OVERRIDE;
    virtual void asynchronousUpdate() OVERRIDE;
    // --------------------------------------------------------------------
    virtual void update(float dt) OVERRIDE {}

};

#endif // STOP_SERVER_HPP
