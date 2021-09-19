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
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#define STK_UTF8_GETTEXT 1
#include "utils/translation.hpp"
#undef STK_UTF8_GETTEXT
#include "network/protocols/client_lobby.hpp"
#include "network/protocols/lobby_protocol.hpp"
#include "network/server.hpp"
#include "online/request_manager.hpp"
#include "online/http_request.hpp"

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

namespace RichPresenceNS
{
class AssetRequest : public Online::HTTPRequest {
private:
    std::string* m_data;
    RichPresence* m_rpc;
    virtual void callback() OVERRIDE
    {
        if (UserConfigParams::m_rich_presence_debug)
            Log::info("RichPresence", "Got asset list!");
        m_data->append(Online::HTTPRequest::getData());
        // Updated asset list! Maybe using addon, so we update:
        m_rpc->update(true);
    }
public:
    AssetRequest(const std::string& url, std::string* data, RichPresence* rpc) :
        Online::HTTPRequest(0), m_data(data), m_rpc(rpc)
    {
        setURL(url);
        setDownloadAssetsRequest(true);
    }
};
RichPresence* g_rich_presence = nullptr;

RichPresence* RichPresence::get()
{
    if (g_rich_presence == nullptr)
    {
        g_rich_presence = new RichPresence();
    }
    return g_rich_presence;
}

void RichPresence::destroy()
{
    if (g_rich_presence != nullptr)
    {
        delete g_rich_presence;
    }
}

RichPresence::RichPresence() : m_connected(false), m_ready(false), m_last(0),
#ifdef WIN32
    m_socket(INVALID_HANDLE_VALUE),
#else
    m_socket(-1),
#endif
    m_assets_request(nullptr),
    m_thread(nullptr),
    m_asset_cache(),
    m_assets()
{
    doConnect();
}

RichPresence::~RichPresence()
{
    terminate();
}

void RichPresence::terminate()
{
#ifndef DISABLE_RPC
#ifdef WIN32
#define UNCLEAN m_socket != INVALID_HANDLE_VALUE
#else
#define UNCLEAN m_socket != -1
#endif
    if(m_thread != nullptr && STKProcess::getType() == PT_MAIN)
    {
        m_thread->join();
        delete m_thread;
        m_thread = nullptr;
    }
    if (m_connected || UNCLEAN)
    {
        if (UNCLEAN && !m_connected)
            Log::fatal("RichPresence", "RichPresence terminated uncleanly! Socket is %d", m_socket);
#ifndef WIN32
        close(m_socket);
        m_socket = -1;
#else
        CloseHandle(m_socket);
        m_socket = INVALID_HANDLE_VALUE;
#endif
        m_connected = false;
        m_ready = false;
    }
#endif // DISABLE_RPC
}

bool RichPresence::doConnect()
{
#ifndef DISABLE_RPC
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
#ifdef SO_NOSIGPIPE
    const int set = 1;
    setsockopt(m_socket, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof(set));
#endif

    // Discord tries these env vars in order:
    char* env;
    std::string basePath = "";
#define TRY_ENV(path) env = std::getenv(path); \
    if (env != nullptr) \
    {\
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
        m_thread = new std::thread(finishConnection, this);
        return true;
    }
    else
    {
        // Force cleanup:
        m_connected = true;
        terminate();
        return false;
    }
#else
    return false;
#endif
#else
    return false;
#endif // DISABLE_RPC
}

void RichPresence::readData()
{
#ifndef DISABLE_RPC
    size_t baseLength = sizeof(int32_t) * 2;
    struct discordPacket* basePacket = (struct discordPacket*) malloc(baseLength);
#ifdef WIN32
    DWORD read;
    if (!ReadFile(m_socket, basePacket, baseLength, &read, NULL))
    {
        Log::error("RichPresence", "Couldn't read from pipe! Error %x", GetLastError());
        free(basePacket);
        terminate();
        return;
    }
#else
    int read = recv(m_socket, basePacket, baseLength, 0);
    if (read == -1)
    {
        if (UserConfigParams::m_rich_presence_debug)
            perror("Couldn't read data from socket!");
        terminate();
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
        terminate();
        return;
    }
#else
    read = recv(m_socket, packet->data, packet->length, 0);
    if (read == -1)
    {
        terminate();
    }
#endif

    packet->data[packet->length] = '\0';
    if (UserConfigParams::m_rich_presence_debug)
        Log::debug("RichPresence", "<= (OP %d len=%d) %s is data (READ %d bytes)",
                   packet->op, packet->length, packet->data, read);

    free(packet);
#endif
}

void RichPresence::finishConnection(RichPresence* self)
{
#ifndef DISABLE_RPC
    // We read all the data from the socket. We're clear now to handshake!
    self->handshake();

    // Make sure we get a response!
    self->readData();

    // Okay to go nonblocking now!
#if !defined(WIN32) && defined(O_NONBLOCK)
    fcntl(self->m_socket, F_SETFL, O_NONBLOCK);
#endif

    self->m_ready = true;
#endif
}

bool RichPresence::tryConnect(std::string path)
{
#ifndef DISABLE_RPC
#if !defined(WIN32) && defined(AF_UNIX)
    struct sockaddr_un addr =
    {
        .sun_family = AF_UNIX
    };
    memset(addr.sun_path, 0, sizeof(addr.sun_path));
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path.c_str());
    if(connect(m_socket, (struct sockaddr *) &addr, sizeof(addr)) == -1)
    {
        // Something is probably wrong:
        if (errno != ENOENT && errno != ECONNREFUSED)
        {
            Log::error("RichPresence", "Couldn't read data from socket! %s",
                strerror(errno));
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
        }
        return false;
    }
    m_connected = true;
#endif
#endif // DISABLE_RPC
    return m_connected;
}

void RichPresence::handshake()
{
#ifndef DISABLE_RPC
    if (UserConfigParams::m_rich_presence_debug)
        Log::debug("RichPresence", "Starting handshake...");
    HardwareStats::Json json;
    json.add<int>("v", 1);
    json.add("client_id", std::string(UserConfigParams::m_discord_client_id));
    json.finish();
    sendData(OP_HANDSHAKE, json.toString());
#endif
}

void RichPresence::sendData(int32_t op, std::string json)
{
#ifndef DISABLE_RPC
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
        if (UserConfigParams::m_rich_presence_debug)
            Log::debug("RichPresence", "Amount written != data size! Closing");
        terminate();
    }
#endif // AF_UNIX
    free(packet);
#endif
}

void RichPresence::ensureCache()
{
    if (m_assets_request != nullptr ||
        Online::RequestManager::get() == nullptr) return;
    std::string url = "https://discord.com/api/v8/oauth2/applications/";
    url.append(UserConfigParams::m_discord_client_id);
    url.append("/assets");
    m_assets_request = std::make_shared<AssetRequest>(url, &m_assets, this);
    m_assets_request->queue();
}

void RichPresence::update(bool force)
{
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
    // Check more often if we're not ready
    if (m_ready || !m_connected)
    {
        // Update timer
        m_last = now;
    }
    // Retry connection:
    if (!m_connected)
    {
        doConnect();
    }
    else
    {
        // Connected but not ready, ensure we have cache
        ensureCache();
    }
    if (!m_ready)
    {
        return;
    }
    if (UserConfigParams::m_rich_presence_debug)
        Log::debug("RichPresence", "Updating status!");

    std::string playerName;
    PlayerProfile *player = PlayerManager::getCurrentPlayer();
    if (player)
    {
        if (PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_GUEST ||
            PlayerManager::getCurrentOnlineState() == PlayerProfile::OS_SIGNED_IN)
        {
            playerName = StringUtils::wideToUtf8(player->getLastOnlineName()) + "@stk";
        }
        else
        {
            playerName = StringUtils::wideToUtf8(player->getName());
        }
    }
    else
        playerName = "Guest";
    World* world = World::getWorld();
    RaceManager *raceManager = RaceManager::get();
    std::string trackId = raceManager->getTrackName();
    std::string difficulty = StringUtils::wideToUtf8(raceManager->getDifficultyName(
        raceManager->getDifficulty()));
    std::string minorModeName = StringUtils::wideToUtf8(raceManager->getNameOf(
        raceManager->getMinorMode()));
    // Discord takes the time when we started as unix timestamp
    uint64_t since = (now * 1000) - StkTime::getMonoTimeMs();
    if (world)
    {
        since += world->getStart();
    }

    // {cmd:SET_ACTIVITY,args:{activity:{},pid:0},nonce:0}

    HardwareStats::Json base;
    base.add("cmd", "SET_ACTIVITY");

    HardwareStats::Json args;
    HardwareStats::Json activity;

    std::string trackName = _("Getting ready to race");
    Track* track = nullptr;
    if (world)
    {
        track = track_manager->getTrack(trackId);
        if (track)
            trackName = StringUtils::wideToUtf8(track->getName());
    }

    auto protocol = LobbyProtocol::get<ClientLobby>();
    if (protocol != nullptr && protocol.get()->getJoinedServer() != nullptr)
    {
        trackName.append(" - ");
        trackName.append(StringUtils::wideToUtf8(
            protocol.get()->getJoinedServer().get()->getName()));
    }

    HardwareStats::Json assets;
    if (world && track)
    {
        bool useAddon = false;
        if (track->isInternal())
        {
            assets.add("large_image", "logo");
            trackName = _("Story Mode");
        }
        else
        {
            activity.add("details", minorModeName + " (" + difficulty + ")");
            if(track->isAddon())
            {
                std::string key = "\"track_";
                key.append(track->getIdent());
                key.append("\"");
                auto existing = m_asset_cache.find(key);
                if (existing == m_asset_cache.end())
                {
                    if (!m_assets.empty())
                    {
                        useAddon = m_assets.find(key) == std::string::npos;
                        m_asset_cache.insert({key, useAddon});
                    }
                    else
                        useAddon = true;
                }
                else
                {
                    useAddon = existing->second;
                }
                if (useAddon && UserConfigParams::m_rich_presence_debug)
                {
                    Log::info("RichPresence", "Couldn't find icon for track %s", key.c_str());
                }
            }
            assets.add("large_image", useAddon ?
                       "addons" : "track_" + trackId);
        }
        assets.add("large_text", trackName);
        AbstractKart *abstractKart = world->getLocalPlayerKart(0);
        if (abstractKart)
        {
            const KartProperties* kart = abstractKart->getKartModel()->getKartProperties();
            if (protocol && protocol->isSpectator())
            {
                assets.add("small_image", "spectate");
            }
            else
            {
                bool useAddon = false;
                if(kart->isAddon())
                {
                    std::string key = "\"kart_";
                    key.append(abstractKart->getIdent());
                    key.append("\"");
                    auto existing = m_asset_cache.find(key);
                    if (existing == m_asset_cache.end())
                    {
                        if (!m_assets.empty())
                        {
                            useAddon = m_assets.find(key) == std::string::npos;
                            m_asset_cache.insert({key, useAddon});
                        }
                        else
                            useAddon = true;
                    }
                    else
                    {
                        useAddon = existing->second;
                    }
                    if (useAddon && UserConfigParams::m_rich_presence_debug)
                    {
                        Log::info("RichPresence", "Couldn't find icon for kart %s", key.c_str());
                    }
                }
                assets.add("small_image", useAddon ? "addons" : "kart_" + abstractKart->getIdent());
            }
            if (!protocol || !protocol->isSpectator())
            {
                std::string kartName = StringUtils::wideToUtf8(kart->getName());
                assets.add("small_text", kartName + " (" + playerName + ")");
            }
        }
    }
    else
    {
        assets.add("large_text", "SuperTuxKart");
        assets.add("large_image", "logo");
        assets.add("small_text", playerName);
        // std::string filename = std::string(basename(player->getIconFilename().c_str()));
        // assets->add("small_image", "kart_" + filename);
    }
    activity.add("state", std::string(trackName.c_str()));
    assets.finish();
    activity.add<std::string>("assets", assets.toString());

    HardwareStats::Json timestamps;
    timestamps.add<std::string>("start", std::to_string(since));

    timestamps.finish();
    activity.add<std::string>("timestamps", timestamps.toString());

    activity.finish();
    args.add<std::string>("activity", activity.toString());
    int pid = 0;
#ifdef WIN32
    pid = _getpid();
#elif !defined(DISABLE_RPC)
    pid = getppid();
#endif
    args.add<int>("pid", pid);
    args.finish();
    base.add<int>("nonce", now);
    base.add<std::string>("args", args.toString());
    base.finish();

    sendData(OP_DATA, base.toString());
#endif // DISABLE_RPC
}

} // namespace
