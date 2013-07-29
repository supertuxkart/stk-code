#include "states_screens/network_kart_selection.hpp"

#include "network/protocol_manager.hpp"
#include "network/protocols/client_lobby_room_protocol.hpp"
#include "states_screens/state_manager.hpp"

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( NetworkKartSelectionScreen );

NetworkKartSelectionScreen::NetworkKartSelectionScreen() : KartSelectionScreen()
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
    tabs->setVisible(false);
    tabs->select( "standard", PLAYER_ID_GAME_MASTER); // select standard kart group

    // change the back button image (because it makes the game quit)
    IconButtonWidget* back_button = getWidget<IconButtonWidget>("back");
    back_button->setImage("gui/main_quit.png");

    m_multiplayer = false;
}

/**
 * Callback handling events from the kart selection menu
 */
void NetworkKartSelectionScreen::eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID)
{
    if (name == "karts")
    {

    }
    else if (name == "back")
    {
        // first do the back action
        KartSelectionScreen::eventCallback(widget, name, playerID);
        // then remove the lobby screen (you left the server)
        StateManager::get()->popMenu();
        // and notify the server that you left
        ClientLobbyRoomProtocol* protocol = static_cast<ClientLobbyRoomProtocol*>(
                ProtocolManager::getInstance()->getProtocol(PROTOCOL_LOBBY_ROOM));
        if (protocol)
            protocol->leave();
    }
    else // name != karts
    {
        KartSelectionScreen::eventCallback(widget, name, playerID);
    }
}   // eventCallback
