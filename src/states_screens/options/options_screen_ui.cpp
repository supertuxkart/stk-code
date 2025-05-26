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

// Manages includes common to all options screens
#include "states_screens/options/options_common.hpp"

#include "challenges/story_mode_timer.hpp"
#include "config/player_manager.hpp"
#include "font/font_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "items/attachment_manager.hpp"
#include "items/powerup_manager.hpp"
#include "modes/world.hpp"
#include "states_screens/dialogs/message_dialog.hpp"
#include "states_screens/main_menu_screen.hpp"

#include <IrrlichtDevice.h>

using namespace GUIEngine;
using namespace Online;

// -----------------------------------------------------------------------------

OptionsScreenUI::OptionsScreenUI() : Screen("options/options_ui.stkgui")
{
    m_inited = false;
}   // OptionsScreenVideo

// -----------------------------------------------------------------------------

void OptionsScreenUI::loadedFromFile()
{
    m_inited = false;

    m_base_skin_selector = getWidget<GUIEngine::SpinnerWidget>("base_skinchoice");
    m_variant_skin_selector = getWidget<GUIEngine::SpinnerWidget>("variant_skinchoice");
    assert( m_base_skin_selector != NULL );
    assert( m_variant_skin_selector != NULL );

    m_base_skin_selector->m_properties[PROP_WRAP_AROUND] = "true";
    m_variant_skin_selector->m_properties[PROP_WRAP_AROUND] = "true";

    // Setup the minimap options spinner
    GUIEngine::SpinnerWidget* minimap_options = getWidget<GUIEngine::SpinnerWidget>("minimap");
    assert( minimap_options != NULL );

    minimap_options->m_properties[PROP_WRAP_AROUND] = "true";
    minimap_options->clearLabels();
    //I18N: In the UI options, minimap position in the race UI 
    minimap_options->addLabel( core::stringw(_("In the bottom-left")));
    //I18N: In the UI options, minimap position in the race UI 
    minimap_options->addLabel( core::stringw(_("On the right side")));
    //I18N: In the UI options, minimap position in the race UI 
    minimap_options->addLabel( core::stringw(_("Hidden")));
    //I18N: In the UI options, minimap position in the race UI
    minimap_options->addLabel( core::stringw(_("Centered")));
    minimap_options->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";

    bool multitouch_enabled = (UserConfigParams::m_multitouch_active == 1 && 
                               irr_driver->getDevice()->supportsTouchDevice()) ||
                               UserConfigParams::m_multitouch_active > 1;

    if (multitouch_enabled && UserConfigParams::m_multitouch_draw_gui)
        minimap_options->m_properties[GUIEngine::PROP_MIN_VALUE] = "1";

    minimap_options->m_properties[GUIEngine::PROP_MAX_VALUE] = "3";

    // Setup the fontsize spinner
    GUIEngine::SpinnerWidget* font_size = getWidget<GUIEngine::SpinnerWidget>("font_size");
    assert( font_size != NULL );

    font_size->clearLabels();
    font_size->addLabel(L"Extremely small");
    //I18N: In the UI options, Very small font size
    font_size->addLabel(_("Very small"));
    //I18N: In the UI options, Small font size
    font_size->addLabel(_("Small"));
    //I18N: In the UI options, Medium font size
    font_size->addLabel(_("Medium"));
    //I18N: In the UI options, Large font size
    font_size->addLabel(_("Large"));
    //I18N: In the UI options, Very large font size
    font_size->addLabel(_("Very large"));
    font_size->addLabel(L"Extremely large");

    if (UserConfigParams::m_artist_debug_mode)
    {
        // Only show extreme size in artist debug mode
        font_size->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";
        font_size->m_properties[GUIEngine::PROP_MAX_VALUE] = "6";
    }
    else
    {
        font_size->m_properties[GUIEngine::PROP_MIN_VALUE] = "1";
        font_size->m_properties[GUIEngine::PROP_MAX_VALUE] = "5";
    }

    font_size->setValueUpdatedCallback([this](SpinnerWidget* spinner)
    {
        // Add a special value updated callback so font size is updated when
        // it's pressed instead of release to prevent multiple event
        bool right = spinner->isButtonSelected(true/*right*/);
        UserConfigParams::m_font_size = spinner->getValue();
        m_reload_option = std::unique_ptr<ReloadOption>(new ReloadOption);
        m_reload_option->m_reload_font = true;
        m_reload_option->m_reload_skin = false;
        m_reload_option->m_focus_name = "font_size";
        m_reload_option->m_focus_right = right;
    });

}   // loadedFromFile

// -----------------------------------------------------------------------------

void OptionsScreenUI::init()
{
    Screen::init();
    OptionsCommon::setTabStatus();

    bool in_game = StateManager::get()->getGameState() == GUIEngine::INGAME_MENU;

    RibbonWidget* ribbon = getWidget<RibbonWidget>("options_choice");
    assert(ribbon != NULL);
    ribbon->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
    ribbon->select( "tab_ui", PLAYER_ID_GAME_MASTER );

    m_skins.clear();
    m_base_skins.clear();
    m_current_skin_variants.clear();
    m_base_skin_selector ->clearLabels();
    m_variant_skin_selector->clearLabels();

    std::set<std::string> skin_files;
    file_manager->listFiles(skin_files /* out */, file_manager->getAsset(FileManager::SKIN,""),
                            true /* make full path */ );
    std::set<std::string> addon_skin_files;
    file_manager->listFiles(addon_skin_files /* out */, file_manager->getAddonsFile("skins/"),
                            true /* make full path */ );

    loadSkins(skin_files, false);
    loadSkins(addon_skin_files, true);

    if (m_skins.size() == 0)
    {
        Log::warn("OptionsScreenUI", "Could not find a single skin, make sure that "
                                     "the data files are correctly installed");
        m_base_skin_selector ->setActive(false);
        m_variant_skin_selector->setActive(false);
        return;
    }

    const int base_skin_count = (int)m_base_skins.size();
    for (auto& p : m_base_skins)
        m_base_skin_selector ->addLabel(p);
    m_base_skin_selector ->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";
    m_base_skin_selector ->m_properties[GUIEngine::PROP_MAX_VALUE] = StringUtils::toString(base_skin_count-1);

    // --- select the right skin in the spinner
    bool currSkinFound = false;
    const std::string& user_skin = UserConfigParams::m_skin_file;
    m_base_skin_selector ->setActive(!in_game);
    m_variant_skin_selector->setActive(!in_game);

    for (unsigned int i = 0; i < m_skins.size(); i++)
    {
        if (m_skins[i].m_folder_name == user_skin)
        {
            m_active_base_skin = m_skins[i].m_base_theme_name;
            m_base_skin_selector ->setValue(getBaseID(m_skins[i]));
            loadCurrentSkinVariants();
            m_variant_skin_selector->setValue(getVariantID(m_skins[i]));
            currSkinFound = true;
            break;
        } 
    }
    if (!currSkinFound)
    {
        Log::warn("OptionsScreenUI", "Couldn't find the current skin in the list of skins!");
        m_base_skin_selector ->setValue(0);
        m_variant_skin_selector->setValue(0);
        irr_driver->unsetMaxTextureSize();
        GUIEngine::reloadSkin();
        irr_driver->setMaxTextureSize();
    }

    // --- Setup other spinners and checkboxes

    GUIEngine::SpinnerWidget* minimap_options = getWidget<GUIEngine::SpinnerWidget>("minimap");
    assert( minimap_options != NULL );

    bool multitouch_enabled = (UserConfigParams::m_multitouch_active == 1 && 
                               irr_driver->getDevice()->supportsTouchDevice()) ||
                               UserConfigParams::m_multitouch_active > 1;

    if (multitouch_enabled && UserConfigParams::m_multitouch_draw_gui &&
        UserConfigParams::m_minimap_display == 0)
    {
        UserConfigParams::m_minimap_display = 1;
    }
    minimap_options->setValue(UserConfigParams::m_minimap_display);
    
    GUIEngine::SpinnerWidget* font_size = getWidget<GUIEngine::SpinnerWidget>("font_size");
    assert( font_size != NULL );

    int size_int = (int)roundf(UserConfigParams::m_font_size);
    if (size_int < 0 || size_int > 6)
        size_int = 3;

    if (!UserConfigParams::m_artist_debug_mode &&
        (size_int < 1 || size_int > 5))
        size_int = 3;

    font_size->setValue(size_int);
    UserConfigParams::m_font_size = font_size->getValue();
    font_size->setActive(!in_game);

    CheckBoxWidget* karts_powerup_gui = getWidget<CheckBoxWidget>("karts_powerup_gui");
    assert(karts_powerup_gui != NULL);
    karts_powerup_gui->setState(UserConfigParams::m_karts_powerup_gui);

    CheckBoxWidget* fps = getWidget<CheckBoxWidget>("showfps");
    assert( fps != NULL );
    fps->setState( UserConfigParams::m_display_fps );

    CheckBoxWidget* story_timer = getWidget<CheckBoxWidget>("story-mode-timer");
    assert( story_timer != NULL );
    story_timer->setState( UserConfigParams::m_display_story_mode_timer );
    CheckBoxWidget* speedrun_timer = getWidget<CheckBoxWidget>("speedrun-timer");
    assert( speedrun_timer != NULL );
    if (story_mode_timer->getStoryModeTime() < 0)
    {
        story_timer->setActive(false);
        speedrun_timer->setActive(false);
    }
    else
    {
        story_timer->setActive(true);

        speedrun_timer->setActive(UserConfigParams::m_display_story_mode_timer);
        getWidget<LabelWidget>("speedrun-timer-text")
            ->setActive(UserConfigParams::m_display_story_mode_timer);
    }
    if (UserConfigParams::m_speedrun_mode)
    {
        if (!story_mode_timer->playerCanRun())
        {
            UserConfigParams::m_speedrun_mode = false;
            new MessageDialog(_("Speedrun mode disabled. It can only be enabled if the game"
                                " has not been closed since the launch of the story mode.\n\n"
                                "Closing the game before the story mode's"
                                " completion invalidates the timer.\n\n"
                                "To use the speedrun mode, please use a new profile."),
                                MessageDialog::MESSAGE_DIALOG_OK,
                                NULL, false, false, 0.6f, 0.7f);
        }
    }
    speedrun_timer->setState( UserConfigParams::m_speedrun_mode );
}   // init

// -----------------------------------------------------------------------------
void OptionsScreenUI::loadSkins(const std::set<std::string>& files, bool addon)
{
    for (auto& f : files)
    {
        std::string stkskin = f + "/stkskin.xml";
        if (file_manager->fileExists(stkskin))
        {
            XMLNode* root = file_manager->createXMLTree(stkskin);
            if (!root)
                continue;

            SkinID skin;
            std::string folder_name = StringUtils::getBasename(f);
            if (addon)
                folder_name = std::string("addon_") + folder_name;

            if (root->get("base_theme_name", &skin.m_base_theme_name))
            {
                if (!root->get("variant_name", &skin.m_variant_name))
                    skin.m_variant_name = core::stringw(" ");
            }
            else if (root->get("name", &skin.m_base_theme_name))
            {
                skin.m_variant_name = " ";
            }
            else
            {
                delete root;
                return;
            }

            skin.m_folder_name = folder_name;
            m_skins.push_back(skin);

            // Add the base theme to the list of base Themes
            bool new_base_theme = true;
            for (int i = 0; i<(int)m_base_skins.size(); i++)
            {
                if (m_base_skins[i] == skin.m_base_theme_name)
                    new_base_theme = false;
            }
            if (new_base_theme)
                m_base_skins.push_back(skin.m_base_theme_name);

            delete root;
        }
    }
} // loadSkins

// -----------------------------------------------------------------------------
/** Set up the variant spinner with the appropriate values based on the current base skin. */
void OptionsScreenUI::loadCurrentSkinVariants()
{
    core::stringw old_label = m_variant_skin_selector->getStringValue();
    m_variant_skin_selector->clearLabels();
    m_current_skin_variants.clear();

    for (int i=0; i<(int)m_skins.size();i++)
    {
        if (m_skins[i].m_base_theme_name == m_active_base_skin)
        {
            m_current_skin_variants.push_back(m_skins[i].m_variant_name);
            m_variant_skin_selector->addLabel(m_skins[i].m_variant_name);
        }
    }
    
    m_variant_skin_selector->m_properties[GUIEngine::PROP_MIN_VALUE] = "0";
    m_variant_skin_selector->m_properties[GUIEngine::PROP_MAX_VALUE] =
        StringUtils::toString(m_current_skin_variants.size()-1);

    // When switching base theme, don't reset the variant spinner
    // if the variant exists for both the previous and the new base theme.
    for (int i=0; i<(int)m_current_skin_variants.size();i++)
    {
        if(m_current_skin_variants[i] == old_label)
            m_variant_skin_selector->setValue(i);
    }

    bool in_game = StateManager::get()->getGameState() == GUIEngine::INGAME_MENU;

    if (m_current_skin_variants.size() == 1)
        m_variant_skin_selector->setActive(false);
    else
        m_variant_skin_selector->setActive(!in_game);
} // loadCurrentSkinVariants

// -----------------------------------------------------------------------------
/** Returns the spinner value matching the given name */
int OptionsScreenUI::getBaseID(SkinID skin)
{
    for (int i=0; i<(int)m_base_skins.size(); i++)
    {
        if (m_base_skins[i] == skin.m_base_theme_name)
            return i;
    }
    return 0;
} // getBaseID

// -----------------------------------------------------------------------------
/** Returns the spinner value matching the given name */
int OptionsScreenUI::getVariantID(SkinID skin)
{
    for (int i=0; i<(int)m_current_skin_variants.size(); i++)
    {
        if (m_current_skin_variants[i] == skin.m_variant_name)
            return i;
    }
    return 0;
} // getVariantID

// -----------------------------------------------------------------------------
/** Returns the folder name of the current skin based on the spinners */
std::string OptionsScreenUI::getCurrentSpinnerSkin()
{
    for (int i=0; i<(int)m_skins.size();i++)
    {
        if (m_skins[i].m_base_theme_name == m_base_skin_selector ->getStringValue() &&
            m_skins[i].m_variant_name    == m_variant_skin_selector->getStringValue())
            return m_skins[i].m_folder_name;
    }
    return "classic"; // Default if nothing is found
} // getCurrentSpinnerSkin

// -----------------------------------------------------------------------------
void OptionsScreenUI::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
#ifndef SERVER_ONLY
    if (name == "options_choice")
    {
        std::string selection = ((RibbonWidget*)widget)->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (selection != "tab_ui")
            OptionsCommon::switchTab(selection);
    }
    else if(name == "back")
    {
        StateManager::get()->escapePressed();
    }
    else if (name == "base_skinchoice")
    {
        m_active_base_skin = m_base_skin_selector->getStringValue();
        loadCurrentSkinVariants();
        UserConfigParams::m_skin_file = getCurrentSpinnerSkin();
        onSkinChange(false);
        m_reload_option->m_focus_name = "base_skinchoice";
        m_reload_option->m_focus_right = m_base_skin_selector->isButtonSelected(true/*right*/);
    }
    else if (name == "variant_skinchoice")
    {
        UserConfigParams::m_skin_file = getCurrentSpinnerSkin();
        onSkinChange(true);
        m_reload_option->m_focus_name = "variant_skinchoice";
        m_reload_option->m_focus_right = m_variant_skin_selector->isButtonSelected(true/*right*/);
    }
    else if (name == "minimap")
    {
        GUIEngine::SpinnerWidget* minimap_options = getWidget<GUIEngine::SpinnerWidget>("minimap");
        assert( minimap_options != NULL );
        UserConfigParams::m_minimap_display = minimap_options->getValue();
        if (World::getWorld())
            World::getWorld()->getRaceGUI()->recreateGUI();
    }
    else if (name == "font_size")
    {
        GUIEngine::SpinnerWidget* font_size = getWidget<GUIEngine::SpinnerWidget>("font_size");
        assert( font_size != NULL );
        bool right = font_size->isButtonSelected(true/*right*/);
        UserConfigParams::m_font_size = font_size->getValue();
        // Reload GUIEngine will clear widgets so we don't do that in eventCallback
        m_reload_option = std::unique_ptr<ReloadOption>(new ReloadOption);
        m_reload_option->m_reload_font = true;
        m_reload_option->m_reload_skin = false;
        m_reload_option->m_focus_name = "font_size";
        m_reload_option->m_focus_right = right;
    }
    else if (name == "karts_powerup_gui")
    {
        CheckBoxWidget* karts_powerup_gui = getWidget<CheckBoxWidget>("karts_powerup_gui");
        assert(karts_powerup_gui != NULL);
        UserConfigParams::m_karts_powerup_gui = karts_powerup_gui->getState();
    }
    else if (name == "showfps")
    {
        CheckBoxWidget* fps = getWidget<CheckBoxWidget>("showfps");
        assert( fps != NULL );
        UserConfigParams::m_display_fps = fps->getState();
    }
    else if (name == "story-mode-timer")
    {
        CheckBoxWidget* story_timer = getWidget<CheckBoxWidget>("story-mode-timer");
        assert( story_timer != NULL );
        UserConfigParams::m_display_story_mode_timer = story_timer->getState();

        CheckBoxWidget* speedrun_timer = getWidget<CheckBoxWidget>("speedrun-timer");
        assert( speedrun_timer != NULL );
        speedrun_timer->setActive( UserConfigParams::m_display_story_mode_timer );
        getWidget<LabelWidget>("speedrun-timer-text")
            ->setActive(UserConfigParams::m_display_story_mode_timer);

        // Disable speedrun mode if the story mode timer is disabled
        if (!UserConfigParams::m_display_story_mode_timer)
        {
            UserConfigParams::m_speedrun_mode = false;
            speedrun_timer->setState(false);
        }

    }
    else if (name == "speedrun-timer")
    {
        CheckBoxWidget* speedrun_timer = getWidget<CheckBoxWidget>("speedrun-timer");
        assert( speedrun_timer != NULL );
        if (speedrun_timer->getState())
        {
            if (!story_mode_timer->playerCanRun())
            {
                speedrun_timer->setState(false);
                new MessageDialog(_("Speedrun mode can only be enabled if the game has not"
                                    " been closed since the launch of the story mode.\n\n"
                                    "Closing the game before the story mode's"
                                    " completion invalidates the timer.\n\n"
                                    "To use the speedrun mode, please use a new profile."),
                                    MessageDialog::MESSAGE_DIALOG_OK,
                                    NULL, false, false, 0.6f, 0.7f);
            }
        }
        UserConfigParams::m_speedrun_mode = speedrun_timer->getState();
    }
#endif
}   // eventCallback

// -----------------------------------------------------------------------------

void OptionsScreenUI::onUpdate(float delta)
{
    if (m_reload_option)
        reloadGUIEngine();
}   // onUpdate

// -----------------------------------------------------------------------------

void OptionsScreenUI::reloadGUIEngine()
{
    bool reload_font = m_reload_option->m_reload_font;
    bool reload_skin = m_reload_option->m_reload_skin;
    std::string focus_name = m_reload_option->m_focus_name;
    bool focus_right = m_reload_option->m_focus_right;

    if (reload_skin || reload_font)
    {
        if (reload_font)
        {
            GUIEngine::clear();
            GUIEngine::cleanUp();
        }

        GUIEngine::clearScreenCache();

        if (reload_font)
        {
            delete font_manager;
            font_manager = new FontManager(); // Fonts are loaded in GUIEngine::init
            GUIEngine::init(irr_driver->getDevice(), irr_driver->getVideoDriver(),
                StateManager::get(), false/*loading*/);
        }

        Screen* screen_list[] =
            {
                MainMenuScreen::getInstance(),
                OptionsScreenUI::getInstance(),
                nullptr
            };
        GUIEngine::switchToScreen(MainMenuScreen::getInstance());
        StateManager::get()->resetAndSetStack(screen_list);
        GUIEngine::SpinnerWidget* spinner = OptionsScreenUI::getInstance()
            ->getWidget<GUIEngine::SpinnerWidget>(focus_name.c_str());
        spinner->setFocusForPlayer(PLAYER_ID_GAME_MASTER);
        spinner->setSelectedButton(focus_right);
    }
    if (reload_skin)
    {
        irr_driver->setMaxTextureSize();

        delete powerup_manager;
        powerup_manager = new PowerupManager();
        powerup_manager->loadPowerupsModels();

        delete attachment_manager;
        attachment_manager = new AttachmentManager();
        attachment_manager->loadModels();
    }
    OptionsScreenUI::getInstance()->m_reload_option = nullptr;
}   // reloadGUIEngine

// -----------------------------------------------------------------------------
void OptionsScreenUI::onSkinChange(bool is_variant)
{
    bool change_font = GUIEngine::getSkin()->hasFont();
    irr_driver->unsetMaxTextureSize();
    GUIEngine::reloadSkin();
    // Reload GUIEngine will clear widgets and set max texture Size so we don't do that here
    m_reload_option = std::unique_ptr<ReloadOption>(new ReloadOption);
    // Check either old or new skin use custom_font
    change_font |= GUIEngine::getSkin()->hasFont();
    // Assume skin variants use the same font set
    m_reload_option->m_reload_font = change_font && !is_variant;
    m_reload_option->m_reload_skin = true;
} // onSkinChange

// -----------------------------------------------------------------------------

void OptionsScreenUI::tearDown()
{
    Screen::tearDown();
    // save changes when leaving screen
    user_config->saveConfig();
}   // tearDown

// -----------------------------------------------------------------------------

void OptionsScreenUI::unloaded()
{
    m_inited = false;
}   // unloaded

// -----------------------------------------------------------------------------
