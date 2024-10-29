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

#include "states_screens/edit_track_screen.hpp"

#include "config/player_manager.hpp"
#include "graphics/stk_tex_manager.hpp"
#include "guiengine/widgets/button_widget.hpp"
#include "guiengine/widgets/check_box_widget.hpp"
#include "guiengine/widgets/dynamic_ribbon_widget.hpp"
#include "guiengine/widgets/icon_button_widget.hpp"
#include "guiengine/widgets/label_widget.hpp"
#include "guiengine/widgets/ribbon_widget.hpp"
#include "guiengine/widgets/spinner_widget.hpp"
#include "states_screens/state_manager.hpp"
#include "tracks/track.hpp"
#include "tracks/track_manager.hpp"
#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

using namespace GUIEngine;
using namespace irr::core;

const char* EditTrackScreen::ALL_TRACKS_GROUP_ID = "all";

// -----------------------------------------------------------------------------
EditTrackScreen::EditTrackScreen()
    : Screen("edit_track.stkgui"), m_track_group("standard"),
    m_track(NULL), m_laps(0), m_reverse(false), m_result(false)
{

}

// -----------------------------------------------------------------------------
EditTrackScreen::~EditTrackScreen()
{

}

// -----------------------------------------------------------------------------
void EditTrackScreen::setSelection(Track* track, unsigned int laps, bool reverse)
{
    assert(laps > 0);
    m_track = track;
    m_laps = laps;
    m_reverse = reverse;
}

// -----------------------------------------------------------------------------
Track* EditTrackScreen::getTrack() const
{
    return m_track;
}

// -----------------------------------------------------------------------------
unsigned int EditTrackScreen::getLaps() const
{
    return m_laps;
}

// -----------------------------------------------------------------------------
bool EditTrackScreen::getReverse() const
{
    return m_reverse;
}

// -----------------------------------------------------------------------------
bool EditTrackScreen::getResult() const
{
    return m_result;
}

// -----------------------------------------------------------------------------
void EditTrackScreen::loadedFromFile()
{
    static const int MAX_LABEL_LENGTH = 35;

    DynamicRibbonWidget* tracks_widget = getWidget<DynamicRibbonWidget>("tracks");
    assert(tracks_widget != NULL);
    tracks_widget->setMaxLabelLength(MAX_LABEL_LENGTH);
    // Avoid too many items shown at the same time
    tracks_widget->setItemCountHint(std::min((int)track_manager->getNumberOfTracks()+1, 30));

    m_screenshot = getWidget<IconButtonWidget>("screenshot");
    m_screenshot->setFocusable(false);
    m_screenshot->m_tab_stop = false;
}

// -----------------------------------------------------------------------------
void EditTrackScreen::beforeAddingWidget()
{
    track_manager->setFavoriteTrackStatus(
        PlayerManager::getCurrentPlayer()->getFavoriteTrackStatus());
    
    RibbonWidget* tabs = getWidget<RibbonWidget>("trackgroups");
    assert (tabs != NULL);

    tabs->clearAllChildren();

    const std::vector<std::string>& groups = track_manager->getAllTrackGroups();
    if (groups.size() > 1)
    {
        tabs->addTextChild(_("All"), ALL_TRACKS_GROUP_ID);
        for (unsigned int i = 0; i < groups.size(); i++)
            tabs->addTextChild(_(groups[i].c_str()), groups[i]);
    }
}

// -----------------------------------------------------------------------------
void EditTrackScreen::init()
{
    m_search_box = getWidget<TextBoxWidget>("search");
    assert(m_search_box != NULL);
    m_search_box->clearListeners();
    m_search_box->addListener(this);

    RibbonWidget* tabs = getWidget<RibbonWidget>("trackgroups");
    assert (tabs != NULL);
    SpinnerWidget* laps = getWidget<SpinnerWidget>("laps");
    assert(laps != NULL);
    CheckBoxWidget* reverse = getWidget<CheckBoxWidget>("reverse");
    assert(reverse != NULL);

    if (m_track_group.empty())
        tabs->select (ALL_TRACKS_GROUP_ID, PLAYER_ID_GAME_MASTER);
    else
        tabs->select (m_track_group, PLAYER_ID_GAME_MASTER);
    laps->setValue(m_laps);
    reverse->setState(m_reverse);

    loadTrackList();
    if (m_track == NULL)
        selectTrack("");
    else
        selectTrack(m_track->getIdent());
}

// -----------------------------------------------------------------------------
void EditTrackScreen::eventCallback(GUIEngine::Widget* widget, const std::string& name,
    const int playerID)
{
    if (name == "ok")
    {
        m_result = true;
        StateManager::get()->popMenu();
    }
    else if (name == "cancel")
    {
        m_result = false;
        StateManager::get()->popMenu();
    }
    else if (name == "tracks")
    {
        DynamicRibbonWidget* tracks = getWidget<DynamicRibbonWidget>("tracks");
        assert(tracks != NULL);
        selectTrack(tracks->getSelectionIDString(PLAYER_ID_GAME_MASTER));
    }
    else if (name == "trackgroups")
    {
        RibbonWidget* tabs = getWidget<RibbonWidget>("trackgroups");
        assert(tabs != NULL);
        m_track_group = tabs->getSelectionIDString(PLAYER_ID_GAME_MASTER);
        loadTrackList();
    }
    else if (name == "laps")
    {
        SpinnerWidget* laps = getWidget<SpinnerWidget>("laps");
        assert(laps != NULL);
        m_laps = laps->getValue();
    }
    else if (name == "reverse")
    {
        CheckBoxWidget* reverse = getWidget<CheckBoxWidget>("reverse");
        assert(reverse != NULL);
        m_reverse = reverse->getState();
    }
}

// -----------------------------------------------------------------------------
void EditTrackScreen::loadTrackList()
{
    track_manager->setFavoriteTrackStatus(
        PlayerManager::getCurrentPlayer()->getFavoriteTrackStatus());

    DynamicRibbonWidget* tracks_widget = getWidget<DynamicRibbonWidget>("tracks");
    assert(tracks_widget != NULL);

    tracks_widget->clearItems();

    // First build a list of all tracks to be displayed
    // (e.g. exclude arenas, ...)
    PtrVector<Track, REF> tracks;

    if (m_track_group == ALL_TRACKS_GROUP_ID)
    {
        const int track_amount = (int)track_manager->getNumberOfTracks();
        for (int n = 0; n < track_amount; n++)
        {
            Track* curr = track_manager->getTrack(n);
            if (curr->isArena() || curr->isSoccer() || curr->isInternal()) continue;
            if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_EASTER_EGG
                && !curr->hasEasterEggs())
                continue;
            core::stringw search_text = m_search_box->getText();
            search_text.make_lower();
            if (!search_text.empty() &&
                curr->getName().make_lower().find(search_text.c_str()) == -1)
                continue;

            tracks.push_back(curr);
        }   // for n<track_amount
    }
    else
    {
        const std::vector<int>& curr_tracks = track_manager->getTracksInGroup(m_track_group);
        const int track_amount = (int)curr_tracks.size();

        for (int n = 0; n < track_amount; n++)
        {
            Track* curr = track_manager->getTrack(curr_tracks[n]);
            if (curr->isArena() || curr->isSoccer() || curr->isInternal()) continue;
            if (RaceManager::get()->getMinorMode() == RaceManager::MINOR_MODE_EASTER_EGG
                && !curr->hasEasterEggs())
                continue;
            core::stringw search_text = m_search_box->getText();
            search_text.make_lower();
            if (!search_text.empty() &&
                curr->getName().make_lower().find(search_text.c_str()) == -1)
                continue;

            tracks.push_back(curr);
        }   // for n<track_amount
    }

    tracks.insertionSort();

    for (unsigned int i = 0; i < tracks.size(); i++)
    {
        Track *curr = tracks.get(i);
        if (PlayerManager::getCurrentPlayer()->isFavoriteTrack(curr->getIdent()))
        {
            tracks_widget->addItem(curr->getName(), curr->getIdent(),
                curr->getScreenshotFile(), HEART_BADGE,
                IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
        }
        else
        {
            tracks_widget->addItem(curr->getName(), curr->getIdent(),
                curr->getScreenshotFile(), 0,
                IconButtonWidget::ICON_PATH_TYPE_ABSOLUTE);
        }
    }

    tracks_widget->updateItemDisplay();
}

// -----------------------------------------------------------------------------
void EditTrackScreen::selectTrack(const std::string& id)
{
    DynamicRibbonWidget* tracks = getWidget<DynamicRibbonWidget>("tracks");
    assert(tracks != NULL);
    LabelWidget* selected_track = getWidget<LabelWidget>("selected_track");
    assert(selected_track != NULL);
    SpinnerWidget* laps = getWidget<SpinnerWidget>("laps");
    assert(laps != NULL);
    LabelWidget* label_reverse = getWidget<LabelWidget>("reverse_label");
    assert(label_reverse != NULL);
    CheckBoxWidget* reverse = getWidget<CheckBoxWidget>("reverse");
    assert(reverse != NULL);
    ButtonWidget* ok_button = getWidget<ButtonWidget>("ok");
    assert(ok_button != NULL);

    m_track = track_manager->getTrack(id);
    ok_button->setActive(m_track!=NULL);
    if (m_track)
    {
        tracks->setSelection(m_track->getIdent(), PLAYER_ID_GAME_MASTER, true);
        selected_track->setText(m_track->getName(), true);

        m_laps = m_track->getDefaultNumberOfLaps();
        laps->setValue(m_laps);

        reverse->setVisible(m_track->reverseAvailable());
        label_reverse->setVisible(m_track->reverseAvailable());

        // Display the track's preview picture in a box,
        // so that the current selection remains obvious even
        // if the player doesn't notice the track name in title
        irr::video::ITexture* image = STKTexManager::getInstance()
            ->getTexture(m_track->getScreenshotFile(),
            "While loading screenshot for track '%s':", m_track->getFilename());
        if(!image)
        {
            image = STKTexManager::getInstance()->getTexture(GUIEngine::getSkin()->getThemedIcon("gui/icons/track_unknown.png"),
                "While loading screenshot for track '%s':", m_track->getFilename());
        }
        if (image != NULL)
            m_screenshot->setImage(image);
    }
    else
    {
        tracks->setSelection("", PLAYER_ID_GAME_MASTER, true);
        selected_track->setText(_("Select a track"), true);

        // We can't set a better default for number of laps. On the other
        // hand, if a track is selected, the number of laps will be updated.
        laps->setValue(3);

        reverse->setVisible(true);
        reverse->setState(false);

    }
}
