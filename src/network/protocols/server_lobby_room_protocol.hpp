#ifndef SERVER_LOBBY_ROOM_PROTOCOL_HPP
#define SERVER_LOBBY_ROOM_PROTOCOL_HPP

#include "network/protocols/lobby_room_protocol.hpp"

class ServerLobbyRoomProtocol : public LobbyRoomProtocol
{
    public:
        ServerLobbyRoomProtocol();
        virtual ~ServerLobbyRoomProtocol();

        virtual void notifyEvent(Event* event);
        virtual void setup();
        virtual void update();

    protected:
        void kartDisconnected(Event* event);
        void connectionRequested(Event* event);
        void kartSelectionRequested(Event* event);

        uint8_t m_next_id; //!< Next id to assign to a peer.
        std::vector<TransportAddress> m_peers;
        std::vector<uint32_t> m_incoming_peers_ids;
        uint32_t m_current_protocol_id;
        TransportAddress m_public_address;

        enum STATE
        {
            NONE,
            GETTING_PUBLIC_ADDRESS,
            LAUNCHING_SERVER,
            WORKING,
            DONE
        };
        STATE m_state;
};
#endif // SERVER_LOBBY_ROOM_PROTOCOL_HPP
