#ifndef NETWORK_KART_SELECTION_HPP
#define NETWORK_KART_SELECTION_HPP

#include "states_screens/kart_selection.hpp"
#include "guiengine/screen.hpp"

class NetworkKartSelectionScreen : public KartSelectionScreen, public GUIEngine::ScreenSingleton<NetworkKartSelectionScreen>
{
    friend class GUIEngine::ScreenSingleton<NetworkKartSelectionScreen>;
protected:
    //!< map the id of the kart widgets to race ids
    std::vector<uint8_t> m_id_mapping;

    NetworkKartSelectionScreen();
    virtual ~NetworkKartSelectionScreen();

    virtual void playerConfirm(const int playerID);
    void considerKartHovered(uint8_t widget_id, std::string selection);
public:
    virtual void init() OVERRIDE;
    virtual void eventCallback(GUIEngine::Widget* widget, const std::string& name,
                               const int playerID) OVERRIDE;

    virtual bool onEscapePressed() OVERRIDE;

    virtual void playerSelected(uint8_t race_id, std::string kart_name);
};

#endif // NETWORK_KART_SELECTION_HPP
