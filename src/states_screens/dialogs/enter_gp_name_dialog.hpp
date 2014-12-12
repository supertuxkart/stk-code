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


#ifndef HEADER_ENTER_GP_NAME_DIALOG_HPP
#define HEADER_ENTER_GP_NAME_DIALOG_HPP

#include "guiengine/modaldialog.hpp"

#include <irrString.h>


namespace GUIEngine
{
    class TextBoxWidget;
    class ButtonWidget;
    class LabelWidget;
}

/**
 * \brief Dialog that allows the player to enter the name for a new grand prix
 * \ingroup states_screens
 */
class EnterGPNameDialog : public GUIEngine::ModalDialog
{

public:

    class INewGPListener
    {
    public:
        virtual void onNewGPWithName(const irr::core::stringw& newName) = 0;
        virtual ~INewGPListener(){}
    };

private:

    INewGPListener* m_listener;
    bool m_self_destroy;

public:

    /**
     * Creates a modal dialog with given percentage of screen width and height
     */
    EnterGPNameDialog(INewGPListener* listener, const float percentWidth,
        const float percentHeight);
    ~EnterGPNameDialog();

    void onEnterPressedInternal();
    void validateName();
    GUIEngine::EventPropagation processEvent(const std::string& eventSource);
    virtual void onUpdate(float dt);
};

#endif
