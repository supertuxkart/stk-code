#include "states_screens/network_kart_selection.hpp"

#include "audio/sfx_manager.hpp"
#include "challenges/unlock_manager.hpp"
#include "config/player_manager.hpp"
#include "config/user_config.hpp"
#include "items/item_manager.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "network/protocol_manager.hpp"
#include "network/protocols/client_lobby_room_protocol.hpp"
#include "network/network_manager.hpp"
#include "states_screens/state_manager.hpp"

static const char RANDOM_KART_ID[] = "randomkart";
static const char ID_LOCKED[] = "locked/";

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( NetworkKartSelectionScreen );

NetworkKartSelectionScreen::NetworkKartSelectionScreen() : KartSelectionScreen("karts_online.stkgui")
{
    KartSelectionScreen::m_instance_ptr = this;
}

NetworkKartSelectionScreen::~NetworkKartSelectionScreen()
{
}

void NetworkKartSelectionScreen::init()
{
    m_multiplayer = false;
    KartSelectionScreen::init();

    RibbonWidget* tabs = getWidget<RibbonWidget>("kartgroups");
    assert( tabs != NULL );
    tabs->select( "standard", PLAYER_ID_GAME_MASTER); // select standard kart group
    tabs->setDeactivated();
    tabs->setVisible(false);

    // change the back button image (because it makes the game quit)
    IconButtonWidget* back_button = getWidget<IconButtonWidget>("back");
    back_button->setImage("gui/main_quit.png");

    m_multiplayer = false;

    // add a widget for each player except self (already exists):
    GameSetup* setup = NetworkManager::getInstance()->getGameSetup();
    if (!setup)
    {
        Log::error("NetworkKartSelectionScreen", "No network game setup registered.");
        return;
    }
    std::vector<NetworkPlayerProfile*> players = setup->getPlayers();

    Log::info("NKSS", "There are %d players", players.size());
    // ---- Get available area for karts
    // make a copy of the area, ands move it to be outside the screen
    Widget* kartsAreaWidget = getWidget("playerskarts");
    // start at the rightmost of the screen
    const int shift = irr_driver->getFrameSize().Width;
    core::recti kartsArea(kartsAreaWidget->m_x + shift,
                            kartsAreaWidget->m_y,
                            kartsAreaWidget->m_x + shift + kartsAreaWidget->m_w,
                            kartsAreaWidget->m_y + kartsAreaWidget->m_h);

    for (unsigned int i = 0; i < players.size(); i++)
    {
        if (players[i]->user_profile == PlayerManager::getCurrentOnlineProfile())
        {
            m_id_mapping.insert(m_id_mapping.begin(),players[i]->race_id); //!< first kart widget always me
            Log::info("NKSS", "Insert %d at pos 0", players[i]->race_id);
            continue; // it is me, don't add again
        }

        Log::info("NKSS", "Adding %d at pos %d", players[i]->race_id, i);
        m_id_mapping.push_back(players[i]->race_id);

        StateManager::ActivePlayer* aplayer = NULL; // player is remote

        std::string selected_kart_group = "standard"; // standard group

        PlayerKartWidget* newPlayerWidget =
            new PlayerKartWidget(this, aplayer, players[i]->user_profile, kartsArea, m_kart_widgets.size(),
                                 selected_kart_group);

        manualAddWidget(newPlayerWidget);
        m_kart_widgets.push_back(newPlayerWidget);

        newPlayerWidget->add();
    }

    const int amount = m_kart_widgets.size();
    Widget* fullarea = getWidget("playerskarts");

    const int splitWidth = fullarea->m_w / amount;

    for (int n=0; n<amount; n++)
    {
        m_kart_widgets[n].move( fullarea->m_x + splitWidth*n,
                                fullarea->m_y, splitWidth, fullarea->m_h);
    }

}

void NetworkKartSelectionScreen::playerConfirm(const int playerID)
{
    DynamicRibbonWidget* w = getWidget<DynamicRibbonWidget>("karts");
    assert(w != NULL);
    const std::string selection = w->getSelectionIDString(playerID);
    if (StringUtils::startsWith(selection, ID_LOCKED))
    {
        unlock_manager->playLockSound();
        return;
    }

    if (playerID == PLAYER_ID_GAME_MASTER)
    {
        UserConfigParams::m_default_kart = selection;
    }

    if (m_kart_widgets[playerID].getKartInternalName().size() == 0)
    {
        SFXManager::get()->quickSound( "anvil" );
        return;
    }
    if(playerID == PLAYER_ID_GAME_MASTER) // self
    {
        ClientLobbyRoomProtocol* protocol = static_cast<ClientLobbyRoomProtocol*>(
                ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM));
        protocol->requestKartSelection(selection);
    }
}

void NetworkKartSelectionScreen::playerSelected(uint8_t race_id, std::string kart_name)
{
    uint8_t widget_id = -1;
    for (unsigned int i = 0; i < m_id_mapping.size(); i++)
    {
        Log::info("NKSS", "Checking race id %d : mapped of %d is %d", race_id, i, m_id_mapping[i]);
        if (m_id_mapping[i] == race_id)
            widget_id = i;
    }

    assert(widget_id>=0 && widget_id < m_kart_widgets.size());

    KartSelectionScreen::updateKartWidgetModel(widget_id, kart_name, irr::core::stringw(kart_name.c_str()));
    KartSelectionScreen::updateKartStats(widget_id, kart_name);
    m_kart_widgets[widget_id].setKartInternalName(kart_name);
    m_kart_widgets[widget_id].markAsReady(); // mark player ready
}


/**
 * Callback handling events from the kart selection menu
 */
void NetworkKartSelectionScreen::eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID)
{
    if (name == "karts")
    {
        KartSelectionScreen::eventCallback(widget, name, playerID);
    }
    else if (name == "back")
    {
        KartSelectionScreen::eventCallback(widget, name, playerID);
    }
    else // name != karts
    {
        KartSelectionScreen::eventCallback(widget, name, playerID);
    }
}   // eventCallback


bool NetworkKartSelectionScreen::onEscapePressed()
{
    // then remove the lobby screen (you left the server)
    StateManager::get()->popMenu();
    // notify the server that we left
    ClientLobbyRoomProtocol* protocol = static_cast<ClientLobbyRoomProtocol*>(
            ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM));
    if (protocol)
        protocol->leave();
    return true; // remove the screen
}
