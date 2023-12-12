//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 Marc Coll
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

#include "states_screens/grand_prix_editor_screen.hpp"

#include "audio/sfx_manager.hpp"
#include "graphics/irr_driver.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/text_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "io/file_manager.hpp"
#include "race/grand_prix_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/edit_gp_screen.hpp"
#include "states_screens/dialogs/general_text_field_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::core;

// -----------------------------------------------------------------------------
GrandPrixEditorScreen::GrandPrixEditorScreen() 
                     : Screen("grand_prix_editor.stkgui"), m_selection(NULL),
                       m_gpgroup(GrandPrixData::GP_NONE)
{
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::beforeAddingWidget()
{
    RibbonWidget* tabs = getWidget<RibbonWidget>("gpgroups");
    assert (tabs != NULL);

    tabs->clearAllChildren();
    tabs->setFlip(FLIP_DOWN_RIGHT);
    for (int i = 0; i < GrandPrixData::GP_GROUP_COUNT; i++)
    {
        core::stringw label = getGroupName((enum GrandPrixData::GPGroupType)i);
        tabs->addTextChild(label.c_str(), StringUtils::toString(i));
    }
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::loadedFromFile()
{
    static const int MAX_LABEL_LENGTH = 35;

    DynamicRibbonWidget* gplist_widget = getWidget<DynamicRibbonWidget>("gplist");
    assert (gplist_widget != NULL);
    gplist_widget->setMaxLabelLength(MAX_LABEL_LENGTH);

    DynamicRibbonWidget* tracks_widget = getWidget<DynamicRibbonWidget>("tracks");
    assert(tracks_widget != NULL);
    tracks_widget->setMaxLabelLength(MAX_LABEL_LENGTH);
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    if (name == "gplist")
    {
        DynamicRibbonWidget* gplist_widget = getWidget<DynamicRibbonWidget>("gplist");
        assert (gplist_widget != NULL);
        std::string selected = gplist_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        if (!selected.empty())
        {
            if (m_selection != NULL && selected == m_selection->getId() && m_selection->isEditable())
                showEditScreen(m_selection);
            else
                setSelection (grand_prix_manager->getGrandPrix(selected));
        }
    }
    else if (name == "menu")
    {
        RibbonWidget* menu = getWidget<RibbonWidget>("menu");
        assert(menu != NULL);
        m_action = menu->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (m_action == "new" || m_action == "copy")
        {
            new GeneralTextFieldDialog(_("Please enter the name of the grand prix"),
                std::bind(&GrandPrixEditorScreen::setNewGPWithName,
                          this, std::placeholders::_1), validateName);
        }
        else if (m_action == "edit" && m_selection != NULL)
        {
            showEditScreen(m_selection);
        }
        else if (m_action == "remove" && m_selection != NULL)
        {
            new MessageDialog(
                _("Are you sure you want to remove '%s'?", m_selection->getName().c_str()),
                MessageDialog::MESSAGE_DIALOG_CONFIRM,
                this, false);
        }
        else if (m_action == "rename" && m_selection != NULL)
        {
            new GeneralTextFieldDialog(_("Please enter the name of the grand prix"),
                std::bind(&GrandPrixEditorScreen::setNewGPWithName,
                          this, std::placeholders::_1), validateName);
        }
    }
    else if (name == "gpgroups")
    {
        RibbonWidget* tabs = getWidget<RibbonWidget>("gpgroups");
        assert(tabs != NULL);

        enum GrandPrixData::GPGroupType group = (enum GrandPrixData::GPGroupType)atoi(
            tabs->getSelectionIDString(PLAYER_ID_GAME_MASTER).c_str());
        if (m_gpgroup != group)
        {
            m_gpgroup = group;
            loadGPList();
            setSelection(NULL);
        }
    }
    else if (name == "back")
    {
        m_gpgroup = GrandPrixData::GP_NONE;
        setSelection(NULL);
        StateManager::get()->escapePressed();
    }
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::init()
{
    RibbonWidget* tabs = getWidget<RibbonWidget>("gpgroups");
    assert (tabs != NULL);

    tabs->select (StringUtils::toString(m_gpgroup), PLAYER_ID_GAME_MASTER);
    loadGPList();
    setSelection(m_selection);
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::setSelection (const GrandPrixData* gpdata)
{
    LabelWidget* gpname_widget = getWidget<LabelWidget>("gpname");
    assert(gpname_widget != NULL);
    DynamicRibbonWidget* gplist_widget = getWidget<DynamicRibbonWidget>("gplist");
    assert (gplist_widget != NULL);
    DynamicRibbonWidget* tracks_widget = getWidget<DynamicRibbonWidget>("tracks");
    assert(tracks_widget != NULL);

    if (gpdata == NULL)
    {
        m_selection = NULL;
        gpname_widget->setText (_("Please select a Grand Prix"), true);
        tracks_widget->clearItems();
        tracks_widget->updateItemDisplay();
    }
    else
    {
        m_selection = grand_prix_manager->editGrandPrix(gpdata->getId());
        gpname_widget->setText(gpdata->getName(), true);
        gplist_widget->setSelection(m_selection->getId(), PLAYER_ID_GAME_MASTER, true);
        loadTrackList (gpdata->getId());
    }

    enableButtons();
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::loadTrackList (const std::string& gpname)
{
    if (gpname.empty())
        return;

    DynamicRibbonWidget* tracks_widget = getWidget<DynamicRibbonWidget>("tracks");
    assert(tracks_widget != NULL);

    const GrandPrixData* gp = grand_prix_manager->getGrandPrix(gpname);
    const std::vector<std::string> tracks = gp->getTrackNames(true);

    tracks_widget->clearItems();

    // Avoid too many items shown at the same time
    tracks_widget->setItemCountHint(std::min((int)tracks.size(), 15));
    for (unsigned int t = 0; t < tracks.size(); t++)
    {
        Track* curr = track_manager->getTrack(tracks[t]);
        if (curr == NULL)
        {
            Log::warn("GrandPrixEditor",
                "Grand Prix '%s' refers to track '%s', which does not exist\n",
                gp->getId().c_str(), tracks[t].c_str());
        }
        else
        {
            tracks_widget->addItem(
                StringUtils::toWString(t + 1) + ". " + curr->getName(),
                curr->getIdent(), curr->getScreenshotFile(), 0,
                IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE );
        }
    }

    tracks_widget->updateItemDisplay();
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::loadGPList()
{
    DynamicRibbonWidget* gplist_widget = getWidget<DynamicRibbonWidget>("gplist");
    assert(gplist_widget != NULL);

    // Reset GP list everytime (accounts for locking changes, etc.)
    gplist_widget->clearItems();

    // ensures that no GP and no track is NULL
    grand_prix_manager->checkConsistency();

    // Build GP list
    for (unsigned int i = 0; i < grand_prix_manager->getNumberOfGrandPrix(); i++)
    {
        const GrandPrixData* gp = grand_prix_manager->getGrandPrix(i);
        const std::vector<std::string> tracks = gp->getTrackNames(true);

        std::vector<std::string> sshot_files;
        for (unsigned int t=0; t<tracks.size(); t++)
        {
            Track* track = track_manager->getTrack(tracks[t]);
            sshot_files.push_back(track->getScreenshotFile());
        }
        if (sshot_files.empty())
            sshot_files.push_back(file_manager->getAsset(FileManager::GUI_ICON,"main_help.png"));

        if (m_gpgroup == GrandPrixData::GP_NONE || m_gpgroup == gp->getGroup())
        {
            gplist_widget->addAnimatedItem(gp->getName(),
                gp->getId(), sshot_files, 2.0f, 0,
                IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE );
        }
    }

    gplist_widget->updateItemDisplay();
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::showEditScreen(GrandPrixData* gp)
{
    assert(gp != NULL);
    EditGPScreen* edit = EditGPScreen::getInstance();
    edit->setSelectedGP(gp);
    edit->push();
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::enableButtons()
{
    IconButtonWidget* copy_button = getWidget<IconButtonWidget>("copy");
    IconButtonWidget* edit_button = getWidget<IconButtonWidget>("edit");
    IconButtonWidget* remove_button = getWidget<IconButtonWidget>("remove");
    IconButtonWidget* rename_button = getWidget<IconButtonWidget>("rename");
    assert(copy_button != NULL);
    assert(edit_button != NULL);
    assert(remove_button != NULL);
    assert(rename_button != NULL);

    bool b = m_selection && m_selection->getNumberOfTracks() > 0;
    copy_button->setActive(b);

    b = m_selection && m_selection->isEditable();
    edit_button->setActive(b);
    remove_button->setActive(b);
    rename_button->setActive(b);
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::setNewGPWithName(const stringw& newName)
{
    if (m_action == "copy" && m_selection != NULL)
    {
        setSelection(grand_prix_manager->copy(m_selection->getId(), newName));
    }
    else if (m_action == "rename" && m_selection != NULL)
    {
        m_selection->setName(newName);
        m_selection->writeToFile();
        setSelection(grand_prix_manager->getGrandPrix(m_selection->getId()));
    }
    else if (m_action == "new")
    {
        setSelection(grand_prix_manager->createNewGP(newName));
    }

    loadGPList();
    if (m_action != "rename" && m_selection != NULL)
        showEditScreen(m_selection);
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::onConfirm()
{
    if (m_action == "remove" && m_selection != NULL)
    {
        grand_prix_manager->remove(m_selection->getId());
        loadGPList();
        
        if (grand_prix_manager->getNumberOfGrandPrix() > 0)
        {
            setSelection(grand_prix_manager->getGrandPrix(0));
        }
        else
        {
            setSelection(NULL);
        }
    }
    ModalDialog::dismiss();
}

// ----------------------------------------------------------------------------
const core::stringw GrandPrixEditorScreen::getGroupName(enum GrandPrixData::GPGroupType group)
{
    switch (group)
    {
        case GrandPrixData::GP_NONE:          return _("All");
        case GrandPrixData::GP_STANDARD:      return _("Standard");
        case GrandPrixData::GP_USER_DEFINED:  return _("User defined");
        case GrandPrixData::GP_ADDONS:        return _("Add-Ons");
        default:                              return L"???";
    }
}

// -----------------------------------------------------------------------------
bool GrandPrixEditorScreen::validateName(LabelWidget* label,
                                         TextBoxWidget* text)
{
    stringw name = text->getText().trim();
    if (name.size() == 0)
    {
        label->setText(_("Name is empty."), false);
        SFXManager::get()->quickSound("anvil");
        return false;
    }
    else if (grand_prix_manager->existsName(name) ||
        name == GrandPrixData::getRandomGPName())
    {
        // check for duplicate names
        label->setText(_("Another grand prix with this name already exists."), false);
        SFXManager::get()->quickSound("anvil");
        return false;
    }
    else if (name.size() > 30)
    {
        label->setText(_("Name is too long."), false);
        SFXManager::get()->quickSound("anvil");
        return false;
    }
    else
    {
        return true;
    }
}
