//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010-2015 Marianne Gagnon
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


#ifndef HEADER_DEBUG_SLIDER_DIALOG_HPP
#define HEADER_DEBUG_SLIDER_DIALOG_HPP

#include "guiengine/modaldialog.hpp"
#include "utils/cpp2011.hpp"
#include "utils/leak_check.hpp"
#include <functional>

/**
 * \brief For internal value tweaking
 * \ingroup states_screens
 */
class DebugSliderDialog : public GUIEngine::ModalDialog
{
private:

    std::string m_id;
    std::map<std::string, std::function<void(int)> >Setters;

public:
    DebugSliderDialog();

    ~DebugSliderDialog() {};
    void setSliderHook(std::string id, unsigned min, unsigned max, std::function<int()> G, std::function<void(int)> S);
    void changeLabel(std::string id, std::string new_label);
    void toggleSlider(std::string id, bool option);

    virtual void onEnterPressedInternal() OVERRIDE;
    virtual void onUpdate(float dt) OVERRIDE;

    GUIEngine::EventPropagation processEvent(const std::string& eventSource) OVERRIDE;
};


#endif
