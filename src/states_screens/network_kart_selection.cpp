#include "states_screens/network_kart_selection.hpp"

#include "network/protocol_manager.hpp"
#include "network/protocols/client_lobby_room_protocol.hpp"
#include "network/network_manager.hpp"
#include "online/current_user.hpp"
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

    // add a widget for each player except self (already exists):
    GameSetup* setup = NetworkManager::getInstance()->getGameSetup();
    if (!setup)
    {
        Log::error("NetworkKartSelectionScreen", "No network game setup registered.");
        return;
    }
    std::vector<NetworkPlayerProfile*> players = setup->getPlayers();


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
        if (players[i]->user_profile == Online::CurrentUser::get())
            return; // it is me, don't add again


        StateManager::ActivePlayer* aplayer =
            StateManager::get()->getActivePlayer(players[i]->race_id);

        std::string selected_kart_group = "standard"; // standard group

        PlayerKartWidget* newPlayerWidget =
            new PlayerKartWidget(this, aplayer, kartsArea, m_kart_widgets.size(),
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
