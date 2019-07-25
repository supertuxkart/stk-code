//  SuperTuxKart - a fun racing game with go-kart
//
//  Copyright (C) 2019 SuperTuxKart-Team
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

#ifndef HEADER_DOWNLOAD_ASSETS_SIZE_HPP
#define HEADER_DOWNLOAD_ASSETS_SIZE_HPP
/* Return .zip file size in bytes (as in ls -l)
 */
inline unsigned getDownloadAssetsSize(bool is_full, bool is_hd)
{
    // Todo: generated from some sed script
    unsigned full_hd = 188442091;
    unsigned full_nonhd = 83487825;
    unsigned nonfull_hd = 166310030;
    unsigned nonfull_nonhd = 69282428;
    if (is_full && is_hd)
        return full_hd;
    if (is_full && !is_hd)
        return full_nonhd;
    if (!is_full && is_hd)
        return nonfull_hd;
    return nonfull_nonhd;
}
#endif
