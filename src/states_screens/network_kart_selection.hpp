#ifndef NETWORK_KART_SELECTION_HPP
#define NETWORK_KART_SELECTION_HPP

#include "states_screens/kart_selection.hpp"
#include "guiengine/screen.hpp"

class NetworkKartSelectionScreen : public KartSelectionScreen, public GUIEngine::ScreenSingleton<NetworkKartSelectionScreen>
{
    friend class GUIEngine::ScreenSingleton<NetworkKartSelectionScreen>;
protected:
    NetworkKartSelectionScreen();
    virtual ~NetworkKartSelectionScreen();

    virtual void playerConfirm(const int playerID);
public:
    virtual void init() OVERRIDE;
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    virtual bool onEscapePressed() OVERRIDE;
};

#endif // NETWORK_KART_SELECTION_HPP
