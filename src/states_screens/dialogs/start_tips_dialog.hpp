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


#ifndef HEADER_START_TIPS_DIALOG_HPP
#define HEADER_START_TIPS_DIALOG_HPP

#include "guiengine/modaldialog.hpp"
#include "tips/tip_set.hpp"
#include "utils/cpp2011.hpp"

/**
 * \brief Dialog to show a tip when the game started
 * \ingroup states_screens
 */
class StartTipsDialog : public GUIEngine::ModalDialog
{

private:
    
    TipSet* m_start_tip;
    TipSet::tip m_tip;

    void showATip();

public:

    /**
      * \param start_tip The tipset it wants to read from.
      */
    StartTipsDialog(TipSet* start_tip);
    ~StartTipsDialog();
    
    virtual void beforeAddingWidgets();
    virtual void load();

    GUIEngine::EventPropagation processEvent(const std::string& event_source) OVERRIDE;

};

#endif
