//  $Id: TrackManager.cxx,v 1.2 2004/08/10 19:55:47 grumbel Exp $
//
//  TuxKart - a fun racing game with go-kart
//  Copyright (C) 2004 Steve Baker <sjbaker1@airmail.net>
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

#include <set>
#include "Loader.h"
#include "StringUtils.h"
#include "TrackManager.h"

TrackManager track_manager;

TrackManager::TrackManager()
{
}
 
void
TrackManager::loadTrackList ()
{
  // Load up a list of tracks - and their names
  std::set<std::string> files;
  loader->listFiles(files, "data");
  for(std::set<std::string>::iterator i = files.begin(); i != files.end(); ++i)
    {
      if(StringUtils::has_suffix(*i, ".track"))
        {
          tracks.push_back(TrackData("data/" + *i));
        }
    }
}

/* EOF */

