#ifndef OFFLINE_KART_SELECTION_HPP
#define OFFLINE_KART_SELECTION_HPP

#include "states_screens/kart_selection.hpp"
#include "guiengine/screen.hpp"

class OfflineKartSelectionScreen : public KartSelectionScreen, public GUIEngine::ScreenSingleton<OfflineKartSelectionScreen>
{
    friend class GUIEngine::ScreenSingleton<OfflineKartSelectionScreen>;
    protected:
        OfflineKartSelectionScreen();
        virtual ~OfflineKartSelectionScreen();

    public:
        static bool isRunning();

        // we do not override anything, this class is just there to have a singleton
};

#endif // OFFLINE_KART_SELECTION_HPP
