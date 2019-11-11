//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2019 dumaosen
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

#include "states_screens/dialogs/start_tips_dialog.hpp"

#include "config/user_config.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "states_screens/state_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;

// ----------------------------------------------------------------------------
/** Simple constructor for a tip.
 *  \param start_tip The tipset it wants to read from.
 */
StartTipsDialog::StartTipsDialog(TipSet* start_tip, bool from_queue)
             : ModalDialog(0.7f, 0.7f)
{
    m_start_tip = start_tip;
    if(!from_queue)
        loadFromFile("start_tips.stkgui");
}   // StartTipsDialog(TipSet)

// ----------------------------------------------------------------------------

StartTipsDialog::~StartTipsDialog()
{
}

// ----------------------------------------------------------------------------

void StartTipsDialog::load()
{
    loadFromFile("start_tips.stkgui");
}

// ----------------------------------------------------------------------------

void StartTipsDialog::beforeAddingWidgets()
{
    showATip();

    CheckBoxWidget* box = getWidget<CheckBoxWidget>("showtips");
    
    box->setState(UserConfigParams::m_show_start_tips);
    if(m_start_tip->isImportant())
    {
        box->setActive(false);
    }
    
    getWidget<RibbonWidget>("buttons")->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
}

// ----------------------------------------------------------------------------
/** Change the tip showing.
 */
void StartTipsDialog::showATip()
{
    m_tip = m_start_tip->getTip();

    LabelWidget* text = getWidget<LabelWidget>("text");
    text->setText(m_tip.getWText(), false);

    IconButtonWidget* icon = getWidget<IconButtonWidget>("icon");
    icon->setImage(m_tip.icon_path);

    icon->setFocusable(true);
    if(m_tip.goto_type == TipSet::GOTO_SCREEN)
        icon->setTooltip(_("Go to the screen"));
    else if(m_tip.goto_type == TipSet::GOTO_WEBSITE)
        icon->setTooltip(_("Go to the website"));
    else if(m_tip.goto_type == TipSet::GOTO_NO)
    {
        icon->setTooltip("");
        icon->setFocusable(false);
    }
}

// ----------------------------------------------------------------------------

GUIEngine::EventPropagation StartTipsDialog::processEvent(const std::string& event_source)
{
    if(event_source == "buttons")
    {
        RibbonWidget* ribbon = getWidget<RibbonWidget>(event_source.c_str());
        
        if (ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER) == "next")
        {
            showATip();
        }
        else if (ribbon->getSelectionIDString(PLAYER_ID_GAME_MASTER) == "close")
        {
            ModalDialog::dismiss();
            return GUIEngine::EVENT_BLOCK;
        }
    }
    else if(event_source == "icon")
    {
        m_tip.runGoto();
    }
    else if(event_source == "showtips")
    {
        if(!m_start_tip->isImportant())
        {
            CheckBoxWidget* box = getWidget<CheckBoxWidget>("showtips");
            UserConfigParams::m_show_start_tips = box->getState();
        }
    }

    return GUIEngine::EVENT_LET;
}
