#include "states_screens/network_kart_selection.hpp"

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
    KartSelectionScreen::init();

    RibbonWidget* tabs = getWidget<RibbonWidget>("kartgroups");
    assert( tabs != NULL );
    tabs->setVisible(false);
    tabs->select( "standard", PLAYER_ID_GAME_MASTER); // select standard kart group

    // change the back button image (because it makes the game quit)
    IconButtonWidget* back_button = getWidget<IconButtonWidget>("back");
    back_button->setImage("gui/main_quit.png");
}

/**
 * Callback handling events from the kart selection menu
 */
void NetworkKartSelectionScreen::eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID)
{
    if (name != "karts")
    {
        KartSelectionScreen::eventCallback(widget, name, playerID);
    }
    else // name = karts
    {
        // must quit the server here
    }
}   // eventCallback
