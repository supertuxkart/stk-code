#include "utils/time.hpp"
#include "utils/string_utils.hpp"
#include "race/race_manager.hpp"
#include "io/rich_presence.hpp"
#include "config/player_manager.hpp"
#include "config/player_profile.hpp"
#include "modes/world.hpp"
#include "config/hardware_stats.hpp"
#include "config/user_config.hpp"
#include "tracks/track_manager.hpp"
#include "tracks/track.hpp"
#include "karts/abstract_kart.hpp"
#include "karts/kart_properties.hpp"
#include "utils/translation.hpp"

#include <locale>
#include <codecvt>

#if defined(__SWITCH__) || defined(MOBILE_STK) || defined(SERVER_ONLY)
#define DISABLE_RPC
#endif

#if !defined(WIN32) && !defined(DISABLE_RPC)
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#elif defined(WIN32)
#include <process.h>
#include <fileapi.h>
#include <namedpipeapi.h>
#endif

namespace RichPresenceNS {
RichPresence* g_rich_presence = nullptr;

RichPresence* RichPresence::get() {
    if (g_rich_presence == nullptr)
    {  
        g_rich_presence = new RichPresence();
    }
    return g_rich_presence;
}

RichPresence::RichPresence() : m_connected(false), m_ready(false), m_last(0) {
    doConnect();
}
RichPresence::~RichPresence() {
    terminate();
}

void RichPresence::terminate() {
    if (m_connected)
    {
        Log::info("RichPresence", "Terminating");
#ifndef WIN32
        close(m_socket);
#else
        CloseHandle(m_socket);
#endif
        m_connected = false;
        m_ready = false;
    }
}

bool RichPresence::doConnect() {
    if (std::string(UserConfigParams::m_discord_client_id) == "-1")
        return false;
#ifndef DISABLE_RPC
    // Just in case we're retrying or something:
    terminate();
#if !defined(WIN32) && defined(AF_UNIX)
    m_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_socket < 0)
    {
        if (UserConfigParams::m_rich_presence_debug)
            perror("Couldn't open a Unix socket!");
        return false;
    }
    // Discord tries these env vars in order:
    char* env;
    std::string basePath = "";
#define TRY_ENV(path) env = std::getenv(path); \
    if (env != nullptr) \
    { \
        basePath = env; \
        goto completed; \
    }

    TRY_ENV("XDG_RUNTIME_DIR")
    TRY_ENV("TMPDIR")
    TRY_ENV("TMP")
    TRY_ENV("TEMP")
#undef TRY_ENV
    // Falls back to /tmp
    basePath = "/tmp";
    completed:
    basePath = basePath + "/";
#elif defined(WIN32)
    // Windows uses named pipes
    std::string basePath = "\\\\?\\pipe\\";
#endif
    // Discord will only bind up to socket 9
    for (int i = 0; i < 10; ++i)
    {
        if (tryConnect(basePath + "discord-ipc-" + StringUtils::toString(i)))
            break;
    }

    if (m_connected)
    {  
        if (UserConfigParams::m_rich_presence_debug)
            Log::info("RichPresence", "Connection opened with Discord!");
        std::thread(finishConnection, this).join();
        return true;
    }
    else
    {  
        terminate();
        return false;
    }
#else
    return false;
#endif
}

void RichPresence::readData() {
#ifndef DISABLE_RPC
    size_t baseLength = sizeof(int32_t) * 2;
    struct discordPacket* basePacket = (struct discordPacket*) malloc(baseLength);
#ifdef WIN32
    DWORD read;
    if (!ReadFile(m_socket, basePacket, baseLength, &read, NULL))
    {
        Log::error("RichPresence", "Couldn't read from pipe! Error %x", GetLastError());
        free(basePacket);
        return;
    }
#else
    int read = recv(m_socket, basePacket, baseLength, 0);
    if (read == -1)
    {  
        if (UserConfigParams::m_rich_presence_debug)
            perror("Couldn't read data from socket!");
        return;
    }
#endif
    // Add one char so we can printf easy
    struct discordPacket* packet = (struct discordPacket*)
        malloc(baseLength + basePacket->length + sizeof(char));
    // Copy over length and opcode from base packet
    memcpy(packet, basePacket, baseLength);
    free(basePacket);
#ifdef WIN32
    if (!ReadFile(m_socket, packet->data, packet->length, &read, NULL))
    {
        Log::error("RichPresence", "Couldn't read from pipe! Error %x", GetLastError());
        free(packet);
        return;
    }
#else
    read = recv(m_socket, packet->data, packet->length, 0);
#endif

    packet->data[packet->length] = '\0';
    if (UserConfigParams::m_rich_presence_debug)
        Log::debug("RichPresence", "<= (OP %d len=%d) %s is data (READ %d bytes)",
                   packet->op, packet->length, packet->data, read);

    free(packet);
#endif
}

void RichPresence::finishConnection(RichPresence* self) {
    // We read all the data from the socket. We're clear now to handshake!
    self->handshake();

    // Make sure we get a response!
    self->readData();

    // Okay to go nonblocking now!
#if !defined(WIN32) && defined(O_NONBLOCK)
    fcntl(self->m_socket, F_SETFL, O_NONBLOCK);
#endif

    self->m_ready = true;
}

bool RichPresence::tryConnect(std::string path) {
#if !defined(WIN32) && defined(AF_UNIX)
    struct sockaddr_un addr = {
        .sun_family = AF_UNIX
    };
    memset(addr.sun_path, 0, sizeof(addr.sun_path));
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path.c_str());
    if(connect(m_socket, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    { 
        // Something is probably wrong:
        if (errno != ENOENT && errno != ECONNREFUSED)
        {
            perror("Couldn't open Discord socket!");
        }
        return false;
    }
    // Connected!
    m_connected = true;
#elif defined(WIN32)
    // Windows
    m_socket = CreateFileA(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (m_socket == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();
        if (error != ERROR_FILE_NOT_FOUND)
        {
            LPSTR errorText = NULL;
            FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&errorText,
                0, NULL);

            Log::warn("RichPresence", "Couldn't open file! %s Error: %ls", path.c_str(), errorText);
            return false;
        }
    }
    m_connected = true;
#endif
    return m_connected;
}

void RichPresence::handshake() {
    if (UserConfigParams::m_rich_presence_debug)
        Log::debug("RichPresence", "Starting handshake...");
    HardwareStats::Json *json = new HardwareStats::Json();
    json->add<int>("v", 1);
    json->add("client_id", std::string(UserConfigParams::m_discord_client_id));
    json->finish();
    std::string data = json->toString();
    sendData(OP_HANDSHAKE, data);
}

void RichPresence::sendData(int32_t op, std::string json) {
    // Handshake will make us ready:
    if (op != OP_HANDSHAKE && !m_ready)
    {    
        Log::warn("RichPresence", "Tried sending data while not ready?");
        return;
    }
    if (UserConfigParams::m_rich_presence_debug)
        Log::debug("RichPresence", "=> %s", json.c_str());
    int32_t size = json.size();
    size_t length = (sizeof(int32_t) * 2) + (size * sizeof(char));
    struct discordPacket* packet = (struct discordPacket*) malloc(
        length
    );
    packet->op = op;
    packet->length = size;
    // Note we aren't copying the NUL at the end
    memcpy(&packet->data, json.c_str(), json.size());
#if !defined(WIN32) && defined(AF_UNIX)
    int flags = 0;
#ifdef MSG_NOSIGNAL
    flags |= MSG_NOSIGNAL;
#endif
    if (send(m_socket, packet, length, flags) == -1)
    {
        if (errno != EPIPE)
            perror("Couldn't send data to Discord socket!");
        else
        {  
            if (UserConfigParams::m_rich_presence_debug)
                Log::debug("RichPresence", "Got an EPIPE, closing");
            // EPIPE, cleanup!
            terminate();
        }
    }
#elif defined(WIN32)
    DWORD written;
    WriteFile(m_socket, packet, length, &written, NULL);
    // TODO
    if(written != length)
    { 
        Log::warn("RichPresence", "Amount written != data size!");
    }
#endif // AF_UNIX
}

void RichPresence::update(bool force) {
#ifndef DISABLE_RPC
    if (STKProcess::getType() != PT_MAIN)
    {
        // Don't update on server thread
        return;
    }
    time_t now = time(NULL);
    if ((now - m_last) < 10 && !force)
    {  
        // Only update every 10s
        return;
    }
    // Retry connection:
    if (!m_connected)
    {  
        doConnect();
    }
    if (!m_ready)
    {  
        if (m_connected)
            Log::debug("RichPresence", "Connected but not ready! Waiting...");
        return;
    }
    if (UserConfigParams::m_rich_presence_debug)
        Log::debug("RichPresence", "Updating status!");
    // Update timer
    m_last = now;

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;

    std::string playerName;
    PlayerProfile *player = PlayerManager::getCurrentPlayer();
    if(PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_GUEST  ||
       PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_SIGNED_IN)
    {
        playerName = convert.to_bytes(player->getLastOnlineName().c_str()) + "@stk";
    }
    else
    {
        playerName = convert.to_bytes(player->getName().c_str());
    }
    World* world = World::getWorld();
    RaceManager *raceManager = RaceManager::get();
    std::string trackId = raceManager->getTrackName();
    std::string difficulty = convert.to_bytes(raceManager->getDifficultyName(
        raceManager->getDifficulty()
    ).c_str());
    std::string minorModeName = convert.to_bytes(raceManager->getNameOf(
        raceManager->getMinorMode()
    ).c_str());
    // Discord takes the time when we started as unix timestamp
    uint64_t since = (now * 1000) - StkTime::getMonoTimeMs();
    if (world)
    {  
      since += world->getStart();
    }

    // {cmd:SET_ACTIVITY,args:{activity:{},pid:0},nonce:0}

    HardwareStats::Json *base = new HardwareStats::Json();
    base->add("cmd", "SET_ACTIVITY");

    HardwareStats::Json *args = new HardwareStats::Json();
    HardwareStats::Json *activity = new HardwareStats::Json();

    std::string trackName = convert.to_bytes(_("Getting ready to race").c_str());
    if (world)
    {
        Track* track = track_manager->getTrack(trackId);
        if (track)
            trackName = convert.to_bytes(track->getName().c_str());
    }

    activity->add("state", std::string(trackName.c_str()));
    if (world)
        activity->add("details", minorModeName + " (" + difficulty + ")");

    HardwareStats::Json *assets = new HardwareStats::Json();
    if (world)
    {  
        Track* track = track_manager->getTrack(trackId);
        assets->add("large_text", trackName);
        assets->add("large_image", track->isAddon() ?
                    "addons" : "track_" + trackId);
        AbstractKart *abstractKart = world->getLocalPlayerKart(0);
        if (abstractKart)
        {  
            const KartProperties* kart = abstractKart->getKartProperties();
            assets->add("small_image", kart->isAddon() ?
                        "addons" : "kart_" + abstractKart->getIdent());
            std::string kartName = convert.to_bytes(kart->getName().c_str());
            assets->add("small_text", kartName + " (" + playerName + ")");
        }
    }
    else
    {
        assets->add("large_text", "SuperTuxKart");
        assets->add("large_image", "logo");
        assets->add("small_text", playerName);
        // std::string filename = std::string(basename(player->getIconFilename().c_str()));
        // assets->add("small_image", "kart_" + filename);
    }
    assets->finish();
    activity->add<std::string>("assets", assets->toString());

    HardwareStats::Json *timestamps = new HardwareStats::Json();
    timestamps->add<std::string>("start", std::to_string(since));
    
    timestamps->finish();
    activity->add<std::string>("timestamps", timestamps->toString());
    
    activity->finish();
    args->add<std::string>("activity", activity->toString());
    int pid = 0;
#ifdef WIN32
    pid = _getpid();
#elif !defined(DISABLE_RPC)
    pid = getppid();
#endif
    args->add<int>("pid", pid);
    args->finish();
    base->add<int>("nonce", now);
    base->add<std::string>("args", args->toString());
    base->finish();

    sendData(OP_DATA, base->toString());

    // Good to flush the rest of the data...
    readData();
#endif
}

} // namespace
