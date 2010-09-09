//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009 Marianne Gagnon
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


#ifndef HEADER_TRACKINFO_DIALOG_HPP
#define HEADER_TRACKINFO_DIALOG_HPP

#include "guiengine/modaldialog.hpp"
#include "guiengine/widgets/spinner_widget.hpp"

static const int HIGHSCORE_COUNT = 3;

namespace irr { namespace gui { class IGUIImage; class IGUIStaticText; } }

/**
 * \brief Dialog that shows the information about a given track
 * \ingroup states_screens
 */
class TrackInfoDialog : public GUIEngine::ModalDialog
{
    std::string m_track_ident;
    
    // When there is no need to tab through / click on images/labels, we can add directly
    // irrlicht labels (more complicated uses require the use of our widget set)
    GUIEngine::SpinnerWidget* m_spinner;
    irr::gui::IGUIImage* m_kart_icons[HIGHSCORE_COUNT];
    irr::gui::IGUIStaticText* m_highscore_entries[HIGHSCORE_COUNT];
    
    void addHighScoreWidgets(const int hscores_y_from, const int hscores_y_to);
    void updateHighScores();
    
public:
    /**
     * Creates a modal dialog with given percentage of screen width and height
     */
    TrackInfoDialog(const std::string& trackIdent, const irr::core::stringw& trackName,
                    irr::video::ITexture* screenshot, const float percentWidth, const float percentHeight);
    virtual ~TrackInfoDialog();
    
    void onEnterPressedInternal();
    GUIEngine::EventPropagation processEvent(const std::string& eventSource);
};

#endif
