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

#include "states_screens/options/options_screen_device.hpp"

#include "config/user_config.hpp"
#include "guiengine/CGUISpriteBank.hpp"
#include "guiengine/message_queue.hpp"
#include "guiengine/scalable_font.hpp"
#include "guiengine/screen.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "input/input_manager.hpp"
#include "input/device_manager.hpp"
#include "input/gamepad_config.hpp"
#include "input/gamepad_device.hpp"
#include "io/file_manager.hpp"
#include "states_screens/dialogs/press_a_key_dialog.hpp"
#include "states_screens/dialogs/general_text_field_dialog.hpp"
#include "states_screens/options/options_screen_audio.hpp"
#include "states_screens/options/options_screen_general.hpp"
#include "states_screens/options/options_screen_input.hpp"
#include "states_screens/options/options_screen_language.hpp"
#include "states_screens/options/options_screen_video.hpp"
#include "states_screens/options/options_screen_ui.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/options/user_screen.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <iostream>
#include <sstream>
#include <set>

using namespace GUIEngine;

// ----------------------------------------------------------------------------

OptionsScreenDevice::OptionsScreenDevice() : Screen("options_device.stkgui")
{
    m_config = NULL;
}   // OptionsScreenDevice

// ----------------------------------------------------------------------------

void OptionsScreenDevice::loadedFromFile()
{
}   // loadedFromFile

// ----------------------------------------------------------------------------

void OptionsScreenDevice::beforeAddingWidget()
{
    ListWidget* w_list = getWidget<GUIEngine::ListWidget>("actions");
    assert(w_list != NULL);
    w_list->clearColumns();
    w_list->addColumn(_("Action"), 1);
    w_list->addColumn(_("Key binding"), 1);
    w_list->setSortable(false);
}

// ----------------------------------------------------------------------------

void OptionsScreenDevice::init()
{
    Screen::init();
    RibbonWidget* tabBar = getWidget<RibbonWidget>("options_choice");
    assert(tabBar != NULL);
    // Focus is set to the actions list later in the init
    tabBar->select( "tab_controls", PLAYER_ID_GAME_MASTER );

    ButtonWidget* delete_button = getWidget<ButtonWidget>("delete");
    ButtonWidget* disable_toggle = getWidget<ButtonWidget>("disable_toggle");

    core::stringw label;

    CheckBoxWidget* ff = getWidget<CheckBoxWidget>("force_feedback");
    ff->setVisible(!m_config->isKeyboard());
    getWidget("force_feedback_text")->setVisible(!m_config->isKeyboard());
    if (!m_config->isKeyboard())
    {
        // Only allow to enable or disable a gamepad,
        // as it is only in the list when connected
        delete_button->setActive(false);
        disable_toggle->setActive(true);

        label = (m_config->isEnabled()
                ? //I18N: button to disable a gamepad configuration
                    _("Disable Device")
                : //I18N: button to enable a gamepad configuration
                    _("Enable Device"));
        ff->setState(
            static_cast<GamepadConfig*>(m_config)->useForceFeedback());
    }
    else
    {
        // Don't allow deleting or disabling the last enabled config
        bool enable = (input_manager->getDeviceManager()
                            ->getActiveKeyboardAmount() > 1 ||
                        !m_config->isEnabled());
        delete_button->setActive(enable);
        disable_toggle->setActive(enable);

        label = (m_config->isEnabled()
                ? //I18N: button to disable a keyboard configuration
                    _("Disable Configuration")
                : //I18N: button to enable a keyboard configuration
                    _("Enable Configuration"));
    }

    // Make sure button is wide enough as the text is being changed away
    // from the original value
    core::dimension2d<u32> size =
        GUIEngine::getFont()->getDimension(label.c_str());
    const int needed = size.Width + disable_toggle->getWidthNeededAroundLabel();
    if (disable_toggle->m_w < needed) disable_toggle->m_w = needed;

    disable_toggle->setLabel(label);

    // Make the three buttons the same length, not strictly needed but will
    // look nicer...
    ButtonWidget* backBtn = getWidget<ButtonWidget>("back_to_device_list");
    if (disable_toggle->m_w < delete_button->m_w)
    {
        disable_toggle->m_w = delete_button->m_w;
    }
    else
    {
        delete_button->m_w = disable_toggle->m_w;
    }
    // At this point, the delete button has the same width as the disable button.
    // One comparison is enough.
    if (backBtn->m_w < delete_button->m_w)
    {
        backBtn->m_w   = delete_button->m_w;
    }
    else
    {
        disable_toggle->m_w = backBtn->m_w;
        delete_button->m_w  = backBtn->m_w;
    }

    backBtn->moveIrrlichtElement();
    delete_button->moveIrrlichtElement();
    disable_toggle->moveIrrlichtElement();

    LabelWidget* label_widget = getWidget<LabelWidget>("title");
    label_widget->setText( m_config->getName().c_str(), false );

    // ---- create list skeleton (right number of items, right internal names)
    //      their actualy contents will be adapted as needed after

    ListWidget* actions = getWidget<GUIEngine::ListWidget>("actions");
    assert( actions != NULL );

    //I18N: Key binding section
    addListItemSubheader(actions, "game_keys_section", _("Game Keys"));
    addListItem(actions, PA_STEER_LEFT);
    addListItem(actions, PA_STEER_RIGHT);
    addListItem(actions, PA_ACCEL);
    addListItem(actions, PA_BRAKE);
    addListItem(actions, PA_FIRE);
    addListItem(actions, PA_NITRO);
    addListItem(actions, PA_DRIFT);
    addListItem(actions, PA_LOOK_BACK);
    addListItem(actions, PA_RESCUE);
    addListItem(actions, PA_PAUSE_RACE);


    //I18N: Key binding section
    addListItemSubheader(actions, "menu_keys_section", _("Menu Keys"));
    addListItem(actions, PA_MENU_UP);
    addListItem(actions, PA_MENU_DOWN);
    addListItem(actions, PA_MENU_LEFT);
    addListItem(actions, PA_MENU_RIGHT);
    addListItem(actions, PA_MENU_SELECT);
    addListItem(actions, PA_MENU_CANCEL);

    updateInputButtons();

    // Focus the list and select its first item
    actions->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    actions->setSelectionID(0);

    // Disable deleting or disabling configuration mid-race
    bool in_game = StateManager::get()->getGameState() == GUIEngine::INGAME_MENU;
    
    if (in_game)
    {
        delete_button->setActive(false);
        disable_toggle->setActive(false);
    }
}   // init

// -----------------------------------------------------------------------------

void OptionsScreenDevice::addListItemSubheader(GUIEngine::ListWidget* actions,
                                              const char* id,
                                              const core::stringw& text)
{
    std::vector<GUIEngine::ListWidget::ListCell> row;
    row.push_back(GUIEngine::ListWidget::ListCell(text, -1, 1, false));
    row.push_back(GUIEngine::ListWidget::ListCell(L"", -1, 1, false));
    actions->addItem(id, row);
}   // addListItemSubheader

// -----------------------------------------------------------------------------

void OptionsScreenDevice::addListItem(GUIEngine::ListWidget* actions,
                                     PlayerAction pa)
{
    std::vector<GUIEngine::ListWidget::ListCell> row;
    core::stringw s(KartActionStrings[pa].c_str());
    row.push_back(GUIEngine::ListWidget::ListCell(s, -1, 1, false));
    row.push_back(GUIEngine::ListWidget::ListCell(L"", -1, 1, false));
    actions->addItem(KartActionStrings[pa], row);
}   // addListItem

// -----------------------------------------------------------------------------

void OptionsScreenDevice::renameRow(GUIEngine::ListWidget* actions,
                                   int idRow,
                                   const irr::core::stringw &translatedName,
                                   PlayerAction action) const
{
    actions->renameCell(idRow, 0, core::stringw("    ") + translatedName);
    actions->renameCell(idRow, 1, m_config->getBindingAsString(action));

}   // makeLabel

// -----------------------------------------------------------------------------

void OptionsScreenDevice::updateInputButtons()
{
    assert(m_config != NULL);

    //TODO: detect duplicates

    GUIEngine::ListWidget* actions =
        getWidget<GUIEngine::ListWidget>("actions");
    assert( actions != NULL );

    int i = 0;
    i++; // section header

    //I18N: Key binding name
    renameRow(actions, i++, _("Steer Left"), PA_STEER_LEFT);

    //I18N: Key binding name
    renameRow(actions, i++, _("Steer Right"), PA_STEER_RIGHT);

    //I18N: Key binding name
    renameRow(actions, i++, _("Accelerate"), PA_ACCEL);

    //I18N: Key binding name
    renameRow(actions, i++, _("Brake / Reverse"), PA_BRAKE);

    //I18N: Key binding name
    renameRow(actions, i++, _("Fire"), PA_FIRE);

    //I18N: Key binding name
    renameRow(actions, i++, _("Nitro"), PA_NITRO);

    //I18N: Key binding name
    renameRow(actions, i++, _("Skidding"), PA_DRIFT);

    //I18N: Key binding name
    renameRow(actions, i++, _("Look Back"), PA_LOOK_BACK);

    //I18N: Key binding name
    renameRow(actions, i++, _("Rescue"), PA_RESCUE);

    //I18N: Key binding name
    renameRow(actions, i++, _("Pause Game"), PA_PAUSE_RACE);

    i++; // section header

    //I18N: Key binding name
    renameRow(actions, i++, _("Up"), PA_MENU_UP);

    //I18N: Key binding name
    renameRow(actions, i++, _("Down"), PA_MENU_DOWN);

    //I18N: Key binding name
    renameRow(actions, i++, _("Left"), PA_MENU_LEFT);

    //I18N: Key binding name
    renameRow(actions, i++, _("Right"), PA_MENU_RIGHT);

    //I18N: Key binding name
    renameRow(actions, i++, _("Select"), PA_MENU_SELECT);

    //I18N: Key binding name
    renameRow(actions, i++, _("Cancel/Back"), PA_MENU_CANCEL);



    bool conflicts_between = false;
    bool conflicts_inside  = false;
    // ---- make sure there are no binding conflicts
    // (same key used for two actions)
    std::set<irr::core::stringw> currently_used_keys;
    for (PlayerAction action = PA_FIRST_GAME_ACTION;
         action <= PA_LAST_GAME_ACTION;
         action=PlayerAction(action+1))
    {
        const irr::core::stringw item = m_config->getMappingIdString(action);
        if (currently_used_keys.find(item) == currently_used_keys.end())
        {
            currently_used_keys.insert( item );
            if (m_config->isKeyboard()
                && conflictsBetweenKbdConfig(action, PA_FIRST_GAME_ACTION,
                                             PA_LAST_GAME_ACTION))
            {
                conflicts_between = true;
                actions->markItemBlue (KartActionStrings[action]);
            }
        }
        else
        {
            // binding conflict!
            actions->markItemRed( KartActionStrings[action] );

            // also mark others
            for (PlayerAction others = PA_FIRST_GAME_ACTION;
                 others < action; others=PlayerAction(others+1))
            {
                const irr::core::stringw others_item =
                    m_config->getMappingIdString(others);
                if (others_item == item)
                {
                    conflicts_inside = true;
                    actions->markItemRed( KartActionStrings[others] );
                }
            }

            //actions->renameItem( KartActionStrings[action],
            //                    _("Binding Conflict!") );
        }
    }

    // menu keys and game keys can overlap, no problem, so forget game keys
    // before checking menu keys
    currently_used_keys.clear();
    for (PlayerAction action = PA_FIRST_MENU_ACTION;
         action <= PA_LAST_MENU_ACTION;
         action=PlayerAction(action+1))
    {
        const irr::core::stringw item = m_config->getBindingAsString(action);
        if (currently_used_keys.find(item) == currently_used_keys.end())
        {
            currently_used_keys.insert( item );
            if (m_config->isKeyboard()
                && conflictsBetweenKbdConfig(action, PA_FIRST_MENU_ACTION,
                                             PA_LAST_MENU_ACTION))
            {
                conflicts_between = true;
                actions->markItemBlue (KartActionStrings[action]);
            }
        }
        else   // existing key
        {
            // binding conflict!
            actions->markItemRed( KartActionStrings[action] );

            // also mark others
            for (PlayerAction others = PA_FIRST_MENU_ACTION;
                 others < action; others=PlayerAction(others+1))
            {
                const irr::core::stringw others_item =
                    m_config->getBindingAsString(others);
                if (others_item == item)
                {
                    conflicts_inside = true;
                    actions->markItemRed( KartActionStrings[others] );
                }
            }   // for others < action

        }   // if existing key
    }   // for action <= PA_LAST_MENU_ACTION;

    core::stringw warning;
    if (conflicts_between)
    {
        warning = _("* A blue item means a conflict with another configuration");
        if (conflicts_inside)
            warning += "\n";
    }
    if (conflicts_inside)
        warning += _("* A red item means a conflict in the current configuration");
    if (!warning.empty())
        MessageQueue::add(MessageQueue::MT_ERROR, warning);

}   // updateInputButtons

// -----------------------------------------------------------------------------

static PlayerAction binding_to_set;
static std::string binding_to_set_button;

void OptionsScreenDevice::gotSensedInput(const Input& sensed_input)
{
    const bool keyboard = (m_config->isKeyboard() &&
                           sensed_input.m_type == Input::IT_KEYBOARD);
    const bool gamepad =  (sensed_input.m_type == Input::IT_STICKMOTION ||
                           sensed_input.m_type == Input::IT_STICKBUTTON) &&
                           m_config->isGamePad();

    if (keyboard)
    {
        if (UserConfigParams::logMisc())
        {
            Log::info("OptionsScreenDevice", "Binding %s: setting to keyboard key %d",
                KartActionStrings[binding_to_set].c_str(), sensed_input.m_button_id);
        }

        KeyboardConfig* keyboard = (KeyboardConfig*)m_config;
        keyboard->setBinding(binding_to_set, Input::IT_KEYBOARD,
                             sensed_input.m_button_id, Input::AD_NEUTRAL,
                             Input::AR_HALF);

        // refresh display
        updateInputButtons();
    }
    else if (gamepad)
    {
        if (UserConfigParams::logMisc())
        {
            Log::info("OptionsScreenDevice", "Binding %s: setting to gamepad #%d",
                KartActionStrings[binding_to_set].c_str(), sensed_input.m_device_id);

            if (sensed_input.m_type == Input::IT_STICKMOTION)
            {
                Log::info("OptionsScreenDevice", "Axis %d; direction %s", sensed_input.m_button_id,
                    sensed_input.m_axis_direction == Input::AD_NEGATIVE ? "-" : "+");
            }
            else if (sensed_input.m_type == Input::IT_STICKBUTTON)
            {
                Log::info("OptionsScreenDevice", "Button %d", sensed_input.m_button_id);
            }
            else
            {
                Log::info("OptionsScreenDevice", "Sensed unknown gamepad event type??");
            }
        }

        GamePadDevice *gpad = input_manager->getDeviceManager()
                            ->getGamePadFromIrrID(sensed_input.m_device_id);

        std::string gamepad_name = gpad ? gpad->getName() : "UNKNOWN DEVICE";
        if (m_config->getName() == gamepad_name)
        {
            GamepadConfig* config =  (GamepadConfig*)m_config;
            config->setBinding(binding_to_set, sensed_input.m_type,
                               sensed_input.m_button_id,
                              (Input::AxisDirection)sensed_input.m_axis_direction,
                              (Input::AxisRange)sensed_input.m_axis_range);

            // refresh display
            updateInputButtons();
        }
        else
        {
            return;
        }
    }
    else if (sensed_input.m_type == Input::IT_NONE)
    {
        if (UserConfigParams::logMisc())
        {
            Log::info("OptionsScreenDevice", "Binding %s: setting to keyboard key NONE",
                KartActionStrings[binding_to_set].c_str());
        }

        KeyboardConfig* keyboard = (KeyboardConfig*)m_config;
        keyboard->setBinding(binding_to_set, Input::IT_NONE,
                             sensed_input.m_button_id, Input::AD_NEUTRAL,
                             Input::AR_HALF);

        // refresh display
        updateInputButtons();
    }
    else
    {
        return;
    }

    ModalDialog::dismiss();
    input_manager->setMode(InputManager::MENU);

    if (keyboard && (sensed_input.m_button_id == irr::IRR_KEY_SHIFT ||
                     sensed_input.m_button_id == irr::IRR_KEY_LSHIFT ||
                     sensed_input.m_button_id == irr::IRR_KEY_RSHIFT))
    {
        new MessageDialog(_("Warning: The 'Shift' is not a recommended key. When "
                            "'Shift' is pressed down, all keys that contain a "
                            "character that is different in upper-case will "
                            "stop working."));
    }

    // re-select the previous button (TODO!)
    //ButtonWidget* btn =
    //    getWidget<ButtonWidget>(binding_to_set_button.c_str());
    //if(btn != NULL) btn->setFocusForPlayer(PLAYER_ID_GAME_MASTER);

    // save new binding to file
    input_manager->getDeviceManager()->save();
}   // gotSensedInput


// ----------------------------------------------------------------------------
void OptionsScreenDevice::eventCallback(Widget* widget,
                                       const std::string& name,
                                       const int playerID)
{
    //const std::string& screen_name = getName();

    StateManager *sm = StateManager::get();
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        Screen *screen = NULL;
        if (selection == "tab_audio")
            screen = OptionsScreenAudio::getInstance();
        else if (selection == "tab_video")
            screen = OptionsScreenVideo::getInstance();
        else if (selection == "tab_players")
            screen = TabbedUserScreen::getInstance();
        //else if (selection == "tab_controls")
        //    screen = OptionsScreenInput::getInstance();
        else if (selection == "tab_ui")
            screen = OptionsScreenUI::getInstance();
        else if (selection == "tab_general")
            screen = OptionsScreenGeneral::getInstance();
        else if (selection == "tab_language")
            screen = OptionsScreenLanguage::getInstance();
        if(screen)
            StateManager::get()->replaceTopMostScreen(screen);
    }
    else if (name == "back_to_device_list")
    {
        sm->replaceTopMostScreen(OptionsScreenInput::getInstance());
    }
    else if (name == "back")
    {
        sm->replaceTopMostScreen(OptionsScreenInput::getInstance());
    }
    else if (name == "actions")
    {
        GUIEngine::ListWidget* actions =
            getWidget<GUIEngine::ListWidget>("actions");
        assert( actions != NULL );

        // a player action in the list was clicked. find which one
        const std::string& clicked = actions->getSelectionInternalName();
        for (int n=PA_BEFORE_FIRST+1; n<PA_COUNT; n++)
        {
            if (KartActionStrings[n] == clicked)
            {
                // we found which one. show the "press a key" dialog.
                if (UserConfigParams::logMisc())
                {
                    Log::info("OptionsScreenDevice", "Entering sensing mode for %s",
                         m_config->getName().c_str());
                }

                binding_to_set = (PlayerAction)n;

                new PressAKeyDialog(0.6f, 0.6f, m_config->isKeyboard());

                if (m_config->isKeyboard())
                {
                    input_manager->setMode(InputManager::INPUT_SENSE_KEYBOARD);
                }
                else if (m_config->isGamePad())
                {
                    input_manager->setMode(InputManager::INPUT_SENSE_GAMEPAD);
                }
                else
                {
                    Log::error("OptionsScreenDevice", "Unknown selection device in options: %s",
                        m_config->getName().c_str());
                }
                break;
            }
        }
    }
    else if (name == "delete")
    {
       // keyboard configs may be deleted
       // They should be the only one to have the button enabled
       //I18N: shown before deleting an input configuration
        new MessageDialog( _("Are you sure you want to permanently delete "
                             "this configuration?"),
            MessageDialog::MESSAGE_DIALOG_CONFIRM, this, false );
    }
    else if (name == "disable_toggle")
    {
        // gamepad and keyboard configs may be disabled
        if (m_config->isEnabled())  m_config->setEnabled(false);
        else                        m_config->setEnabled(true);

        // update widget label
        ButtonWidget* disable_toggle = getWidget<ButtonWidget>("disable_toggle");
        if (!m_config->isKeyboard())
        {
            disable_toggle->setLabel(m_config->isEnabled() ? _("Disable Device")
                                                          : _("Enable Device")  );
        }
        else
        {
            disable_toggle->setLabel(m_config->isEnabled() ? _("Disable Configuration")
                                                          : _("Enable Configuration")  );
        }

        input_manager->getDeviceManager()->save();
    }
    else if (name == "rename_config")
    {
        core::stringw instruction =
            _("Enter new configuration name, leave empty to revert default value.");
        DeviceConfig *the_config = m_config; //Can't give variable m_config directly

        new GeneralTextFieldDialog(instruction, [] (const irr::core::stringw& text) {},
            [the_config] (GUIEngine::LabelWidget* lw,
                GUIEngine::TextBoxWidget* tb)->bool
            {
                core::stringw info = tb->getText();
                the_config->setConfigName(info);
                input_manager->getDeviceManager()->save();
                return true;
            });
    }
    else if (name == "force_feedback")
    {
        GamepadConfig* gc = dynamic_cast<GamepadConfig*>(m_config);
        if (gc)
        {
            gc->setForceFeedback(
                getWidget<CheckBoxWidget>("force_feedback")->getState());
            input_manager->getDeviceManager()->save();
        }
    }
}   // eventCallback

// -----------------------------------------------------------------------------

void OptionsScreenDevice::unloaded()
{
}   // unloaded

// -----------------------------------------------------------------------------

bool OptionsScreenDevice::onEscapePressed()
{
    StateManager::get()
        ->replaceTopMostScreen(OptionsScreenInput::getInstance());
    return false; // don't use standard escape key handler,
                  // we handled it differently
}   // onEscapePressed

// -----------------------------------------------------------------------------

void OptionsScreenDevice::onConfirm()
{
    const bool success =
        input_manager->getDeviceManager()->deleteConfig(m_config);
    assert(success);
    if (!success)
        Log::error("OptionsScreenDevice", "Failed to delete config!");

    m_config = NULL;
    input_manager->getDeviceManager()->save();
    ModalDialog::dismiss();
    StateManager::get()
        ->replaceTopMostScreen(OptionsScreenInput::getInstance());
}   // onConfirm

// -----------------------------------------------------------------------------


bool OptionsScreenDevice::conflictsBetweenKbdConfig(PlayerAction action,
                                                   PlayerAction from,
                                                   PlayerAction to)
{
    int id = m_config->getBinding(action).getId();
    for (int i=0; i < input_manager->getDeviceManager()->getKeyboardAmount(); i++)
    {
        KeyboardConfig* other_kbd_config =
            input_manager->getDeviceManager()->getKeyboardConfig(i);

        if (m_config != other_kbd_config  &&
            other_kbd_config->hasBindingFor(id, from, to)
            && (other_kbd_config->getBinding(action).getId() != id ||
                 action == PA_FIRE)                                  )
        {
            return true;
        }
    }
    return false;
}   // conflictsBetweenKbdConfig
