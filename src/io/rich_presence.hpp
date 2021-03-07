#ifdef WIN32
#include <namedpipeapi.h>
#endif

namespace RichPresenceNS {
    // There are more, but we don't need to use them
    enum OPCodes {
        OP_HANDSHAKE = 0,
        OP_DATA = 1,
    };
    struct discordPacket {
        int32_t op;
        int32_t length;
        char data[];
    };
    class RichPresence {
    private:
        bool m_connected;
        bool m_ready;
        time_t m_last;
#ifdef WIN32
        HANDLE m_socket;
#else
        int m_socket;
#endif
        bool tryConnect(std::string path);
        bool doConnect();
        void terminate();
        void sendData(int32_t op, std::string json);
        void handshake();
        void readData();
        static void finishConnection(RichPresence* self);
    public:
        RichPresence();
        ~RichPresence();
        void update(bool force);
        static RichPresence* get();
    };
}
