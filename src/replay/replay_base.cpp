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
#include "utils/file_utils.hpp"

// -----------------------------------------------------------------------------
ReplayBase::ReplayBase()
{
}   // ReplayBaese
// -----------------------------------------------------------------------------
/** Opens a replay file which is determined by sub classes.
 *  \param writeable True if the file should be opened for writing.
 *  \param full_path True if the file is full path.
 *  \return A FILE *, or NULL if the file could not be opened.
 */
FILE* ReplayBase::openReplayFile(bool writeable, bool full_path, int replay_file_number)
{
    FILE* fd = FileUtils::fopenU8Path(full_path ? getReplayFilename(replay_file_number) :
        file_manager->getReplayDir() + getReplayFilename(replay_file_number),
        writeable ? "w" : "r");
    if (!fd)
    {
        return NULL;
    }
    return fd;

}   // openReplayFile
