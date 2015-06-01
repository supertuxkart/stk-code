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

#include "guiengine/dialog_queue.hpp"
#include "assert.h"

using namespace GUIEngine;

namespace GUIEngine
{
    static DialogQueue* dialog_queue_singleton(NULL);

    DialogQueue* DialogQueue::get()
    {
        if (dialog_queue_singleton == NULL)
            dialog_queue_singleton = new DialogQueue();
        return dialog_queue_singleton;
    }

    void DialogQueue::deallocate()
    {
        delete dialog_queue_singleton;
        dialog_queue_singleton = NULL;
    }   // deallocate


    // ----------------------------------------------------------------------------

    DialogQueue::DialogQueue()
    {
        m_closer = NULL;
    }

    // ----------------------------------------------------------------------------

    void DialogQueue::pushDialog(ModalDialog * dialog, bool closes_any_dialog)
    {
        assert(!dialog->isInited());
        if(closes_any_dialog)
        {
            delete m_closer;
            m_closer = dialog;
        }
        else
            m_queue.push(dialog);
    }

    // ----------------------------------------------------------------------------

    void DialogQueue::update()
    {
        if(m_closer != NULL)
        {
            ModalDialog::dismiss();
            m_closer->load();
            m_closer = NULL;
        }
        else if(!m_queue.empty())
        {
            ModalDialog * entry = m_queue.front();
            if(!ModalDialog::isADialogActive())
            {
                entry->load();
                m_queue.pop();
            }
        }
    }
}
