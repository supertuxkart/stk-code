//  $Id: TrackManager.cxx,v 1.4 2004/08/24 18:17:50 grumbel Exp $
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
#include "TuxkartError.h"
#include "Loader.h"
#include "StringUtils.h"
#include "TrackManager.h"

TrackManager track_manager;

TrackManager::TrackManager()
{
}

int
TrackManager::getTrackId(const std::string& ident)
{
  int j = 0;
  for(Tracks::iterator i = tracks.begin(); i != tracks.end(); ++i)
    {
      if (i->ident == ident)
        return j;
      ++j;
    }

  throw TuxkartError("TrackManager: Couldn't find track: '" + ident + "'");
}

const TrackData&
TrackManager::getTrack(const std::string& ident)
{
  for(Tracks::iterator i = tracks.begin(); i != tracks.end(); ++i)
    {
      if (i->ident == ident)
        return (*i);
    }

  throw TuxkartError("TrackManager: Couldn't find track: '" + ident + "'");
}

const TrackData&
TrackManager::getTrackById(int id)
{
  if (id >= 0 && id < int(tracks.size()))
    return tracks[id];

  throw TuxkartError("TrackManager: Couldn't find track-id: '" + StringUtils::to_string(id) + "'");  
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

