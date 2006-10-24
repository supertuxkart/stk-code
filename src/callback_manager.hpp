//  $Id: callback_manager.hpp 796 2006-09-27 07:06:34Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#ifndef CALLBACK_MANAGER_H
#define CALLBACK_MANAGER_H

#include <vector>
#include "callback.hpp"

// It might actually be enough to have only two different values: track (which
// get deleted and loaded more than one), and everything else, which only
// gets loaded once, and deleted at the end (and when switching windows /
// fullscreen). Sinace it's not much overhead, a separate class is provided
// for every model that might contain a callback.
enum CallbackType { CB_COLLECTABLE, CB_ATTACHMENT, CB_EXPLOSION, CB_HERRING, 
                    CB_KART,        CB_TRACK,      CB_MAX                    };

class CallbackManager  
{
  std::vector<Callback*> m_allCallbacks[CB_MAX];

public:
       CallbackManager();
      ~CallbackManager();

  void update     (float dt) const;
  void clear      (CallbackType cbType);
  void addCallback(Callback *c, CallbackType t) {m_allCallbacks[t].push_back(c);}
};   // CallbackManager

extern CallbackManager *callback_manager;

#endif
/* EOF */
  
