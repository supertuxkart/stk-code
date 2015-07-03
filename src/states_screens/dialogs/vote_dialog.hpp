//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013-2015 Glenn De Jonghe
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


#ifndef HEADER_VOTE_DIALOG_HPP
#define HEADER_VOTE_DIALOG_HPP


#include "guiengine/modaldialog.hpp"
#include "guiengine/widgets.hpp"

#include <irrString.h>

namespace Online
{
    class XMLRequest;
}
/**
 * \brief Dialog that allows a user to sign in
 * \ingroup states_screens
 */
class VoteDialog : public GUIEngine::ModalDialog
{
private:
    /** Stores the id of the addon being voted on. */
    const std::string m_addon_id;

    /** True if the dialog should be removed (which needs to be done
     *  in the update call each frame). */
    bool m_self_destroy;

    /** The request to fetch the current vote, which is submitted
     *  immediately when this dialog is opened. */
    Online::XMLRequest * m_fetch_vote_request;

    /** The request to perform a vote. */
    Online::XMLRequest* m_perform_vote_request;

    /** Pointer to the info widget of this dialog. */
    GUIEngine::LabelWidget * m_info_widget;

    /** Pointer to the rating widget of this dialog */
    GUIEngine::RatingBarWidget * m_rating_widget;

    /** Pointer to the options widget, which contains the canel button. */
    GUIEngine::RibbonWidget * m_options_widget;

    /** Pointer to the cancel button. */
    GUIEngine::IconButtonWidget * m_cancel_widget;

    void updateFetchVote();
    void sendVote();
public:
    VoteDialog(const std::string & addon_id);
    ~VoteDialog();
    GUIEngine::EventPropagation processEvent(const std::string& eventSource);
    virtual void onUpdate(float dt);
    virtual bool onEscapePressed();
};   // VoteDialog

#endif
