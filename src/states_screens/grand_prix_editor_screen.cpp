//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014 Marc Coll
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

#include "graphics/irr_driver.hpp"
#include "guiengine/widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "io/file_manager.hpp"
#include "race/grand_prix_data.hpp"
#include "race/grand_prix_manager.hpp"
#include "states_screens/state_manager.hpp"
#include "states_screens/edit_gp_screen.hpp"
#include "states_screens/dialogs/enter_gp_name_dialog.hpp"
#include "states_screens/dialogs/gp_info_dialog.hpp"
#include "states_screens/dialogs/track_info_dialog.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/translation.hpp"


using namespace GUIEngine;
using namespace irr::core;

DEFINE_SCREEN_SINGLETON( GrandPrixEditorScreen );

// -----------------------------------------------------------------------------
GrandPrixEditorScreen::GrandPrixEditorScreen()
    : Screen("gpeditor.stkgui"), m_selection(NULL)
{
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::loadedFromFile()
{

}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::eventCallback(Widget* widget, const std::string& name, const int playerID)
{
    DynamicRibbonWidget* gplist_widget = getWidget<DynamicRibbonWidget>("gplist");
    assert (gplist_widget != NULL);
    std::string selected = gplist_widget->getSelectionIDString(PLAYER_ID_GAME_MASTER);
    if (!selected.empty())
        setSelection (grand_prix_manager->getGrandPrix(selected));

    if (name == "menu")
    {
        RibbonWidget* menu = getWidget<RibbonWidget>("menu");
        assert(menu != NULL);
        m_action = menu->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (m_action == "new" || m_action == "copy")
        {
            new EnterGPNameDialog(this, 0.5f, 0.4f);
        }
        else if (m_action == "edit")
        {
            if (m_selection->isEditable())
            {
                showEditScreen(m_selection);
            }
            else
            {
                new MessageDialog(
                    _("You can't edit the '%s' grand prix.\nYou might want to copy it first",
                        m_selection->getName().c_str()),
                    MessageDialog::MESSAGE_DIALOG_OK, NULL, false);
            }
        }
        else if (m_action == "remove")
        {
            if (m_selection->isEditable())
            {
                new MessageDialog(
                    _("Are you sure you want to remove '%s'?", m_selection->getName().c_str()),
                    MessageDialog::MESSAGE_DIALOG_CONFIRM,
                    this, false);
            }
            else
            {
                new MessageDialog(
                    _("You can't remove '%s'.", m_selection->getName().c_str()),
                    MessageDialog::MESSAGE_DIALOG_OK, NULL, false);
            }
        }
        else if (m_action == "rename")
        {
            if (m_selection->isEditable())
            {
                new EnterGPNameDialog(this, 0.5f, 0.4f);
            }
            else
            {
                new MessageDialog(
                    _("You can't rename '%s'.", m_selection->getName().c_str()),
                    MessageDialog::MESSAGE_DIALOG_OK, NULL, false);
            }
        }
    }
    else if (name == "back")
    {
        StateManager::get()->escapePressed();
    }
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::init()
{
    if (grand_prix_manager->getNumberOfGrandPrix() > 0)
    {
        if (m_selection == NULL)
        {
            loadGPList();
            setSelection (grand_prix_manager->getGrandPrix(0));
        }
        else
        {
            std::string id = m_selection->getId();
            grand_prix_manager->reload();
            loadGPList();
            m_selection = grand_prix_manager->editGrandPrix(id);
            m_selection->reload();
            setSelection (m_selection);
        }
    }
    else
    {
        loadGPList();
    }
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::setSelection (const GrandPrixData* gpdata)
{
    LabelWidget* gpname_widget = getWidget<LabelWidget>("gpname");
    assert(gpname_widget != NULL);
    DynamicRibbonWidget* gplist_widget = getWidget<DynamicRibbonWidget>("gplist");
    assert (gplist_widget != NULL);

    m_selection = grand_prix_manager->editGrandPrix(gpdata->getId());
    gpname_widget->setText (gpdata->getName(), true);
    gplist_widget->setSelection(m_selection->getId(), PLAYER_ID_GAME_MASTER, true);
    loadTrackList (gpdata->getId());
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
    tracks_widget->setItemCountHint(tracks.size());
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
                StringUtils::toWString(t + 1) + ". " + translations->fribidize(curr->getName()),
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

        gplist_widget->addAnimatedItem(translations->fribidize(gp->getName()), gp->getId(),
            sshot_files, 2.0f, 0, IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE );
    }

    gplist_widget->updateItemDisplay();
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::showEditScreen(GrandPrixData* gp)
{
    assert(gp != NULL);
    EditGPScreen* edit = EditGPScreen::getInstance();
    edit->setSelectedGP(gp);
    StateManager::get()->pushScreen(edit);
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::onNewGPWithName(const stringw& newName)
{
    if (m_action == "copy")
    {
        setSelection(grand_prix_manager->copy(m_selection->getId(), newName));
    }
    else if (m_action == "rename")
    {
        m_selection->setName(newName);
        m_selection->writeToFile();
    }
    else if (m_action == "new")
    {
        setSelection(grand_prix_manager->createNewGP(newName));
    }

    loadGPList();
    if (m_action != "rename")
        showEditScreen(m_selection);
}

// -----------------------------------------------------------------------------
void GrandPrixEditorScreen::onConfirm()
{
    if (m_action == "remove")
    {
        grand_prix_manager->remove(m_selection->getId());
        loadGPList();
        if (grand_prix_manager->getNumberOfGrandPrix() > 0)
            setSelection (grand_prix_manager->getGrandPrix(0));
    }
    ModalDialog::dismiss();
}
