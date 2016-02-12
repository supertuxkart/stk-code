//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012-2015 Joerg Henrichs
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

#include "replay/replay_base.hpp"

#include "io/file_manager.hpp"
#include "race/race_manager.hpp"

// -----------------------------------------------------------------------------
ReplayBase::ReplayBase()
{
    m_filename = "";
}   // ReplayBaese
// -----------------------------------------------------------------------------
/** Opens a replay file (depending on the track name, which is taken from
 *  the race manager).
 *  \param writeable True if the file should be opened for writing.
 *  \return A FILE *, or NULL if the file could not be opened.
 */
FILE* ReplayBase::openReplayFile(bool writeable)
{
    m_filename = file_manager->getReplayDir() +
        race_manager->getTrackName() + ".replay";
    FILE *fd = fopen(m_filename.c_str(), writeable ? "w" : "r");
    if (!fd)
    {
        return NULL;
    }
    return fd;

}   // openReplayFile
