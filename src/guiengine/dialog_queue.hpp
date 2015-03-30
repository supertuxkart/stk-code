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

#ifndef HEADER_DIALOG_QUEUE_HPP
#define HEADER_DIALOG_QUEUE_HPP

#include <queue>
#include "guiengine/modaldialog.hpp"

/**
 * \ingroup guiengine
 */
namespace GUIEngine
{

    class DialogQueue
    {
    private :

        std::queue<ModalDialog *> m_queue;
        ModalDialog * m_closer;
        DialogQueue();
    public :
        /**Singleton */
        static DialogQueue *                 get();
        static void                          deallocate();
        void                                 pushDialog(ModalDialog * dialog, bool closes_any_dialog = false);
        void                                 update();

    };

}
#endif
