#include "states_screens/network_kart_selection.hpp"

using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( NetworkKartSelectionScreen );

NetworkKartSelectionScreen::NetworkKartSelectionScreen() : KartSelectionScreen()
{
}

NetworkKartSelectionScreen::~NetworkKartSelectionScreen()
{
}


void NetworkKartSelectionScreen::init()
{
}

/**
 * Callback handling events from the kart selection menu
 */
void NetworkKartSelectionScreen::eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID)
{
    Log::info("NetworkKartSelectionScreen", "Fuck events !");
}   // eventCallback
