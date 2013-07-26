#ifndef NETWORK_KART_SELECTION_HPP
#define NETWORK_KART_SELECTION_HPP

#include "states_screens/kart_selection.hpp"

class NetworkKartSelectionScreen : public KartSelectionScreen
{
protected:

    NetworkKartSelectionScreen();
    virtual ~NetworkKartSelectionScreen();

public:
    virtual void init() OVERRIDE;
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;
};

#endif // NETWORK_KART_SELECTION_HPP
