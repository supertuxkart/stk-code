//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Marianne Gagnon
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


#ifndef HEADER_CONFIRM_DIALOG_HPP
#define HEADER_CONFIRM_DIALOG_HPP

#include "config/player.hpp"
#include "guiengine/modaldialog.hpp"


class ConfirmDialog : public GUIEngine::ModalDialog
{
public:
    class IConfirmDialogListener
    {
    public:
        IConfirmDialogListener() {}
        virtual ~IConfirmDialogListener() {}
        
        /** \brief Implement to be notified of dialog confirmed */
        virtual void onConfirm() = 0;
        
        /** \brief Implement to be notified of dialog cancelled */
        virtual void onCancel() {}
    };
    
private:
    
    IConfirmDialogListener* m_listener;
    
public:

    ConfirmDialog(irr::core::stringw msg, IConfirmDialogListener* listener);
    void onEnterPressedInternal();
    GUIEngine::EventPropagation processEvent(const std::string& eventSource);
};


#endif
