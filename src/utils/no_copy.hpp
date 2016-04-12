//  SuperTuxKart - a fun racing game with go-kart
//
// Copyright (C) 2003-2015 Matthias-2013 Braun <matze@braunis.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef __NOCOPY_H__
#define __NOCOPY_H__

/** Utility class, you can inherit from this class to disallow the assignment
 * operator and copy construction
 */
class NoCopy
{
public:
    NoCopy()
    { }

private:
    NoCopy(const NoCopy& )
    { }
    void operator=(const NoCopy& )
    { }
}   // NoCopy
;

#endif
