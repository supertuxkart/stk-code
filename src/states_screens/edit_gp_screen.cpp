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

#include "states_screens/edit_gp_screen.hpp"

#include "graphics/irr_driver.hpp"
#include "guiengine/CGUISpriteBank.h"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/list_widget.hpp"
#include "race/grand_prix_data.hpp"
#include "states_screens/edit_track_screen.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"


using namespace GUIEngine;

DEFINE_SCREEN_SINGLETON( EditGPScreen );

// -----------------------------------------------------------------------------
EditGPScreen::EditGPScreen()
    : Screen("gpedit.stkgui"), m_gp(NULL), m_list(NULL), m_icon_bank(NULL),
    m_selected(-1), m_modified(false)
{

}

// -----------------------------------------------------------------------------
EditGPScreen::~EditGPScreen()
{
    delete m_icon_bank;
}

// -----------------------------------------------------------------------------
void EditGPScreen::setSelectedGP(GrandPrixData* gp)
{
    assert(gp != NULL);
    m_gp = gp;
}

// -----------------------------------------------------------------------------
void EditGPScreen::loadedFromFile()
{
    if (m_icon_bank == NULL)
        m_icon_bank = new irr::gui::STKModifiedSpriteBank(GUIEngine::getGUIEnv());

    m_list = getWidget<ListWidget>("tracks");
    assert(m_list != NULL);
    m_list->addColumn(_("Track"), 3);
    m_list->addColumn(_("Laps"), 1);
    m_list->addColumn(_("Reversed"), 1);
}

// -----------------------------------------------------------------------------
void EditGPScreen::eventCallback(GUIEngine::Widget* widget, const std::string& name,
    const int playerID)
{
    setSelected(m_list->getSelectionID());

    if (name == "tracks")
    {
        m_action = "edit";
        edit();
    }
    else if (name == "menu")
    {
        RibbonWidget* menu = getWidget<RibbonWidget>("menu");
        assert(menu != NULL);
        m_action = menu->getSelectionIDString(PLAYER_ID_GAME_MASTER);

        if (m_action == "up")
        {
            if (canMoveUp())
            {
                m_gp->moveUp(m_selected--);
                loadList(m_selected);
                setModified(true);
            }
        }
        else if (m_action == "down")
        {
            if (canMoveDown())
            {
                m_gp->moveDown(m_selected++);
                loadList(m_selected);
                setModified(true);
            }
        }
        else if (m_action == "edit")
        {
            edit();
        }
        else if (m_action == "add")
        {
            EditTrackScreen* edit = EditTrackScreen::getInstance();
            assert(edit != NULL);
            //By default, 3 laps and no reversing
            edit->setSelection(NULL, 3, false);
            edit->push();
        }
        else if (m_action == "remove")
        {
            if (m_selected >= 0 && m_selected < m_list->getItemCount())
            {
                new MessageDialog(
                    _("Are you sure you want to remove '%s'?",
                        m_gp->getTrackName(m_selected).c_str()),
                    MessageDialog::MESSAGE_DIALOG_CONFIRM,
                    this, false);
            }
        }
        else if (m_action == "save")
        {
            save();
        }
    }
    else if (name == "back")
    {
        if (m_modified)
        {
            m_action = "back";
            new MessageDialog(
                _("Do you want to save your changes?"),
                MessageDialog::MESSAGE_DIALOG_CONFIRM,
                this, false);
        }
        else
        {
            back();
        }
    }
}

// -----------------------------------------------------------------------------
void EditGPScreen::init()
{
    if (m_action.empty())
    {
        LabelWidget* header = getWidget<LabelWidget>("title");
        assert(header != NULL);
        header->setText(m_gp->getName(), true);

        IconButtonWidget* button = getWidget<IconButtonWidget>("save");
        assert(button != NULL);
        button->setDeactivated();

        loadList(0);
        setModified(false);
    }
    else
    {
        EditTrackScreen* edit = EditTrackScreen::getInstance();
        assert(edit != NULL);

        if (edit->getResult())
        {
            if (m_action == "add")
            {
                m_gp->addTrack(edit->getTrack(), edit->getLaps(), edit->getReverse(),
                    m_selected);
                setSelected(m_selected + 1);
            }
            else if (m_action == "edit")
            {
                m_gp->editTrack(m_selected, edit->getTrack(), edit->getLaps(),
                    edit->getReverse());
            }
            setModified(true);
        }
        loadList(m_selected);
        m_action.clear();
    }
    enableButtons();
}

// -----------------------------------------------------------------------------
void EditGPScreen::onConfirm()
{
    ModalDialog::dismiss();
    if (m_action == "remove")
    {
        m_gp->remove(m_selected);
        setSelected(m_selected >= (int)m_gp->getNumberOfTracks(true) ?
            m_gp->getNumberOfTracks(true) - 1 : m_selected);
        loadList(m_selected);
        setModified(true);
    }
    else if (m_action == "back")
    {
        save();
        back();
    }
}

// -----------------------------------------------------------------------------
void EditGPScreen::onCancel()
{
    ModalDialog::dismiss();
    if (m_action == "back")
    {
        m_gp->reload(); // Discard changes
        back();
    }
}

// -----------------------------------------------------------------------------
void EditGPScreen::loadList(const int selected)
{
    m_list->clear();
    m_icons.clear();
    m_icon_bank->clear();
    m_icon_bank->scaleToHeight (64);
    m_list->setIcons(m_icon_bank, 64);

    for (unsigned int i = 0; i < m_gp->getNumberOfTracks(true); i++)
    {
        std::vector<GUIEngine::ListWidget::ListCell> row;

        Track* t = track_manager->getTrack(m_gp->getTrackId(i));
        assert(t != NULL);

        video::ITexture* screenShot = irr_driver->getTexture(t->getScreenshotFile());
        if (screenShot == NULL)
        {
            screenShot = irr_driver->getTexture(
                file_manager->getAsset(FileManager::GUI, "main_help.png"));
        }
        assert (screenShot != NULL);
        m_icons.push_back(m_icon_bank->addTextureAsSprite(screenShot));

        row.push_back(GUIEngine::ListWidget::ListCell(
            _LTR(m_gp->getTrackName(i).c_str()), m_icons[i], 3, false));
        row.push_back(GUIEngine::ListWidget::ListCell(
            StringUtils::toWString<unsigned int>(m_gp->getLaps(i)), -1, 1, true));
        row.push_back(GUIEngine::ListWidget::ListCell(
            m_gp->getReverse(i) ? _("Yes") : _("No"), -1, 1, true));

        m_list->addItem(m_gp->getId(), row);
    }
    m_list->setIcons(m_icon_bank);

    if (selected < m_list->getItemCount())
    {
        m_list->setSelectionID(selected);
        setSelected(selected);
    }
    else
    {
        enableButtons();
    }
}

// -----------------------------------------------------------------------------
void EditGPScreen::setModified(const bool modified)
{
    m_modified = modified;

    IconButtonWidget* save_button = getWidget<IconButtonWidget>("save");
    assert(save_button != NULL);
    if (modified)
        save_button->setActivated();
    else
        save_button->setDeactivated();

    LabelWidget* header = getWidget<LabelWidget>("title");
    assert(header != NULL);
    header->setText(m_gp->getName() + (modified ? L" (+)" : L""), true);

    enableButtons();
}

// -----------------------------------------------------------------------------
void EditGPScreen::setSelected(const int selected)
{
    m_selected = selected;
    enableButtons();
}

// -----------------------------------------------------------------------------
void EditGPScreen::edit()
{
    EditTrackScreen* edit_screen = EditTrackScreen::getInstance();
    assert(edit_screen != NULL);

    if (m_selected >= 0 && m_selected < m_list->getItemCount())
    {
        edit_screen->setSelection(track_manager->getTrack(
            m_gp->getTrackId(m_selected)),
            m_gp->getLaps((unsigned int)m_selected),
            m_gp->getReverse((unsigned int)m_selected));
        edit_screen->push();
    }
}

// -----------------------------------------------------------------------------
bool EditGPScreen::save()
{
    if (m_gp->writeToFile())
    {
        setModified(false);
        return true;
    }
    else
    {
        new MessageDialog(
            _("An error occurred while trying to save your grand prix."),
            MessageDialog::MESSAGE_DIALOG_OK, NULL, false);
        return false;
    }
}

// -----------------------------------------------------------------------------
void EditGPScreen::back()
{
    m_action.clear();
    m_modified = false;
    StateManager::get()->popMenu();
}

// -----------------------------------------------------------------------------
bool EditGPScreen::canMoveUp() const
{
    return (0 < m_selected && m_selected < m_list->getItemCount());
}

// -----------------------------------------------------------------------------
bool EditGPScreen::canMoveDown() const
{
    return (0 <= m_selected && m_selected < m_list->getItemCount() - 1);
}

// -----------------------------------------------------------------------------
void EditGPScreen::enableButtons()
{
    IconButtonWidget* up_button = getWidget<IconButtonWidget>("up");
    IconButtonWidget* down_button = getWidget<IconButtonWidget>("down");
    IconButtonWidget* edit_button = getWidget<IconButtonWidget>("edit");
    IconButtonWidget* remove_button = getWidget<IconButtonWidget>("remove");
    assert(up_button != NULL);
    assert(down_button != NULL);
    assert(edit_button != NULL);
    assert(remove_button != NULL);

    if (m_selected >= 0 && m_list->getItemCount() > 1)
    {
        up_button->setActivated();
        down_button->setActivated();
    }
    else
    {
        up_button->setDeactivated();
        down_button->setDeactivated();
    }

    if (m_selected >= 0)
    {
        edit_button->setActivated();
        remove_button->setActivated();
    }
    else
    {
        edit_button->setDeactivated();
        remove_button->setDeactivated();
    }
}
