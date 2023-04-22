//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009-2015 Marianne Gagnon
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 3
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "states_screens/dialogs/enter_address_dialog.hpp"
#include "config/user_config.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "network/server.hpp"
#include "network/socket_address.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <vector>
#include <utility>
#include <IGUIEnvironment.h>

// ------------------------------------------------------------------------
EnterAddressDialog::EnterAddressDialog(std::shared_ptr<Server>* entered_server)
    : ModalDialog(0.8f, 0.8f, GUIEngine::MODAL_DIALOG_LOCATION_CENTER),
    m_self_destroy(false)
{
    loadFromFile("enter_address_dialog.stkgui");
    m_text_field = getWidget<GUIEngine::TextBoxWidget>("textfield");
    m_title = getWidget<GUIEngine::LabelWidget>("title");
    m_title->setText(_("Enter the server address optionally followed by : and"
            " then port or select address from list."), false);
    m_entered_server = entered_server;
    m_list = getWidget<GUIEngine::ListWidget>("list_history");

    loadList();

    GUIEngine::RibbonWidget* buttons_ribbon =
        getWidget<GUIEngine::RibbonWidget>("buttons");
    buttons_ribbon->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    buttons_ribbon->select("cancel", PLAYER_ID_GAME_MASTER);
}   // EnterAddressDialog

// ------------------------------------------------------------------------
EnterAddressDialog::~EnterAddressDialog()
{
    m_text_field->getIrrlichtElement()->remove();
    m_text_field->clearListeners();
}   // ~EnterAddressDialog

// ------------------------------------------------------------------------
void EnterAddressDialog::onUpdate(float dt)
{
    if (m_self_destroy)
    {
        GUIEngine::getGUIEnv()
            ->removeFocus(m_text_field->getIrrlichtElement());
        GUIEngine::getGUIEnv()->removeFocus(m_irrlicht_window);
        ModalDialog::dismiss();
    }
}   // onUpdate
// ------------------------------------------------------------------------
void EnterAddressDialog::onEnterPressedInternal()
{
    if (!m_self_destroy && validate())
    {
        m_self_destroy = true;
        UserConfigParams::m_address_history[
            StringUtils::wideToUtf8(m_text_field->getText().trim())] = (uint32_t)StkTime::getTimeSinceEpoch();
    }
}   // onEnterPressedInternal

// ------------------------------------------------------------------------
GUIEngine::EventPropagation EnterAddressDialog::processEvent(const std::string& event_source)
{
    if (event_source == "buttons")
    {
        GUIEngine::RibbonWidget* buttons_ribbon =
            getWidget<GUIEngine::RibbonWidget>("buttons");
        const std::string& button =
            buttons_ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (button == "cancel")
        {
            dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (button == "ok")
        {
            if (!m_self_destroy && validate())
            {
                m_self_destroy = true;
                UserConfigParams::m_address_history[
                    StringUtils::wideToUtf8(m_text_field->getText().trim())] = (uint32_t)StkTime::getTimeSinceEpoch();
            }
            return GUIEngine::EVENT_BLOCK;
        }
    }
    if (event_source == "list_history")
    {
        m_text_field->setText(m_list->getSelectionLabel());
    }
    return GUIEngine::EVENT_LET;
}   // processEvent

// ------------------------------------------------------------------------
bool EnterAddressDialog::validate()
{
    core::stringw addr_w = m_text_field->getText();
    std::string addr_u = StringUtils::wideToUtf8(addr_w.trim());
    SocketAddress server_addr(addr_u);
    if (server_addr.getIP() == 0 && !server_addr.isIPv6())
    {
        core::stringw err = _("Invalid server address: %s.",
            addr_w);
        m_title->setText(err, true);
        return false;
    }
    SocketAddress ipv4_addr = server_addr;
    if (server_addr.isIPv6())
        ipv4_addr.setIP(0);
    auto server =
        std::make_shared<UserDefinedServer>(addr_w, ipv4_addr);
    if (server_addr.isIPv6())
    {
        server->setIPV6Address(server_addr);
        server->setIPV6Connection(true);
    }
    *m_entered_server = server;
    return true;
}   // validate

// ------------------------------------------------------------------------
void EnterAddressDialog::loadList()
{
    const unsigned MAX_ENTRIES = 5;
    std::map<std::string, uint32_t>& addr = UserConfigParams::m_address_history;
    std::vector<std::pair<std::string, uint32_t> > entries;
    for (std::map<std::string,uint32_t>::iterator i = addr.begin();
        i != addr.end(); i++)
        entries.emplace_back(i->first, i->second);
    std::sort(entries.begin(),entries.end(),
        [](const std::pair<std::string,uint32_t>& a,
           const std::pair<std::string,uint32_t>& b)->bool
        { return a.second > b.second; });

    addr.clear();
    for (unsigned i = 0; i < entries.size(); i++)
    {
        m_list->addItem("list_item",
            std::vector<GUIEngine::ListWidget::ListCell>
            { GUIEngine::ListWidget::ListCell(StringUtils::utf8ToWide(entries[i].first)),
            GUIEngine::ListWidget::ListCell(StringUtils::utf8ToWide(StkTime::toString(entries[i].second)))});
        addr[entries[i].first] = entries[i].second;
        if (i >= MAX_ENTRIES - 1)
            break;
    }
}   // loadList
