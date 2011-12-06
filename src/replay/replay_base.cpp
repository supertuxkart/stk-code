//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2007 Maik Semder <ikework@gmx.de>
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

#ifdef HAVE_GHOST_REPLAY

#include "replay/replay_base.hpp"


const std::string ReplayBase::REPLAY_FOLDER = "replay";
const std::string ReplayBase::REPLAY_FILE_EXTENSION_HUMAN_READABLE = "rph";
const std::string ReplayBase::REPLAY_FILE_EXTENSION_BINARY = "rpb";


ReplayBase::ReplayBase()
: m_ReplayBuffers()
{
}

ReplayBase::~ReplayBase()
{
    destroy();
}

void ReplayBase::destroy()
{
    m_ReplayBuffers.destroy();
}




#endif // HAVE_GHOST_REPLAY
