//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015 Joerg Henrichs
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

#ifndef HEADER_MESSAGE_QUEUE_HPP
#define HEADER_MESSAGE_QUEUE_HPP

#include "guiengine/widgets/label_widget.hpp"

#include "irrString.h"

#include <queue>
#include <vector>

using namespace irr;

namespace MessageQueue
{
    /** The various message type which can be shown (which might use a
     *  different look. This type is used to sort the messages, so it is
     *  important that messages that need to be shown as early as possible
     *  will be listed last (i.e. have highest priority). */
    enum MessageType
    {
        MT_FRIEND,
        MT_ACHIEVEMENT,
        MT_ERROR,
        MT_GENERIC,
        MT_PROGRESS
    };

    void add(MessageType mt, const core::stringw &message);
    void showProgressBar(int progress, const wchar_t* msg);
    void updatePosition();
    void update(float dt);

};   // namespace GUIEngine
#endif
