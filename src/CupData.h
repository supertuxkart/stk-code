//  $Id$
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
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

#ifndef HEADER_CUPDATA_H
#define HEADER_CUPDATA_H

/** Simple class that hold the data relevant to a 'cup', aka. a number
    of races that has to be completed one after the other */
class CupData
{
public:
  /** The name of the cup, for example "Herring Cup" */
  std::string name;

  /** The ident of the tracks in this cup in their right order, ident
      means the filename of the .track file without .track extension
      (ie. 'volcano') */
  std::vector<std::string> tracks;

  /** Load the CupData from the given filename */
  CupData(const std::string& filename);
};

#endif

/* EOF */
