//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Marianne Gagnon
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

#include "states_screens/dialogs/icon_selection_dialog.hpp"

#include "config/player_manager.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"
#include "states_screens/state_manager.hpp"

using namespace GUIEngine;

// ----------------------------------------------------------------------------
IconSelectionDialog::IconSelectionDialog(PlayerProfile* pp, BaseUserScreen* user_screen)
                     : ModalDialog(0.75f, 0.75f, MODAL_DIALOG_LOCATION_CENTER)
{
    loadFromFile("icon_selection_dialog.stkgui");
    m_player_profile = pp;
    m_parent_screen = user_screen;

    m_buttons_widget = getWidget<RibbonWidget>("buttons");
    assert(m_buttons_widget);

    m_kart_icons_widget = getWidget<DynamicRibbonWidget>("kart_icons");
    assert(m_kart_icons);

    m_kart_icons_widget->updateItemDisplay();
}   // IconSelectionDialog

// ----------------------------------------------------------------------------
IconSelectionDialog::~IconSelectionDialog()
{
    PlayerManager::get()->save();
}   // ~IconSelectionDialog

// ----------------------------------------------------------------------------
void IconSelectionDialog::init()
{
    ModalDialog::init();
}


// ----------------------------------------------------------------------------
void IconSelectionDialog::beforeAddingWidgets()
{

    m_kart_icons_widget = getWidget<DynamicRibbonWidget>("kart_icons");
    assert(m_kart_icons_widget);

    m_kart_icons_widget->clearItems();
    for(unsigned int i=0; i<kart_properties_manager->getNumberOfKarts(); i++)
    {
        const KartProperties* kp = kart_properties_manager->getKartById(i);
        if (!kp) continue;

        m_kart_icons_widget->addItem(
            kp->getName(),
            kp->getIdent(),
            kp->getAbsoluteIconFile(),
            0,
            IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE
        );

    }
}   // beforeAddingWidgets

// ----------------------------------------------------------------------------
GUIEngine::EventPropagation
            IconSelectionDialog::processEvent(const std::string& eventSource)
{

    if (eventSource == "kart_icons")
    {
        m_selected_kart = m_kart_icons_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (!m_selected_kart.empty())
        {
            m_kart_icons_widget->setBadge(m_selected_kart, GUIEngine::OK_BADGE);
            m_kart_icons_widget->updateItemDisplay();
        }
    }

    else if (eventSource == "buttons")
    {
        const std::string& selection = m_buttons_widget->
                                    getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection == "apply")
        {
            if (!m_selected_kart.empty())
            {
                m_player_profile->setIconFromKart(m_selected_kart);
                m_parent_screen->loadProfilesIcons();
            }

            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
        else if (selection == "cancel")
        {
            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
    }
    return GUIEngine::EVENT_LET;
}   // processEvent
