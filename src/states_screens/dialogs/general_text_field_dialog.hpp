//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#ifndef HEADER_GENERAL_TEXT_FIELD_DIALOG_HPP
#define HEADER_GENERAL_TEXT_FIELD_DIALOG_HPP

#include "guiengine/modaldialog.hpp"
#include "utils/cpp2011.hpp"

#include "irrString.h"

#include <functional>

namespace GUIEngine
{
    class TextBoxWidget;
    class ButtonWidget;
    class LabelWidget;
}

/**
 * \brief A general textfield dialog to do anything based on captured text
 *  using callbacks.
 * \ingroup states_screens
 */
class GeneralTextFieldDialog : public GUIEngine::ModalDialog
{
private:
    typedef std::function<void(const irr::core::stringw&)> DismissCallback;

    typedef
        std::function<bool(GUIEngine::LabelWidget*, GUIEngine::TextBoxWidget*)>
        ValidationCallback;

    GUIEngine::LabelWidget* m_title;

    GUIEngine::TextBoxWidget* m_text_field;

    DismissCallback m_dm_cb;

    ValidationCallback m_val_cb;

    bool m_self_destroy;

public:
    GeneralTextFieldDialog(const core::stringw& title, DismissCallback dm_cb,
                           ValidationCallback val_cb = []
        (GUIEngine::LabelWidget* lw, GUIEngine::TextBoxWidget* tb)->bool
        {
            // No validation if not specify, always go to dismiss callback
            return true;
        });
    // ------------------------------------------------------------------------
    ~GeneralTextFieldDialog();
    // ------------------------------------------------------------------------
    virtual void onEnterPressedInternal() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void onUpdate(float dt) OVERRIDE;
    // ------------------------------------------------------------------------
    GUIEngine::EventPropagation processEvent(const std::string& eventSource)
        OVERRIDE;
    // ------------------------------------------------------------------------
    GUIEngine::TextBoxWidget* getTextField() const     { return m_text_field; }
    // ------------------------------------------------------------------------
    GUIEngine::LabelWidget* getTitle() const                { return m_title; }

};

#endif
