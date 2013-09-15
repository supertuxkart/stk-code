#include "states_screens/offline_kart_selection.hpp"

DEFINE_SCREEN_SINGLETON( OfflineKartSelectionScreen );

OfflineKartSelectionScreen::OfflineKartSelectionScreen() : KartSelectionScreen("karts.stkgui")
{
    KartSelectionScreen::m_instance_ptr = this;
}

OfflineKartSelectionScreen::~OfflineKartSelectionScreen()
{
    //dtor
}
