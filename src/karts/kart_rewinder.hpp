//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 Joerg Henrichs
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

#ifndef HEADER_KART_REWINDER_HPP
#define HEADER_KART_REWINDER_HPP

#include "graphics/render_info.hpp"
#include "karts/kart.hpp"
#include "network/rewinder.hpp"
#include "utils/cpp2011.hpp"

class AbstractKart;
class BareNetworkString;

class KartRewinder : public Rewinder, public Kart
{
private:
    
    // Flags to indicate the different event types
    enum { EVENT_CONTROL = 0x01,
           EVENT_ATTACH  = 0x02 };
public:
	             KartRewinder(const std::string& ident,
                              unsigned int world_kart_id,
                              int position, const btTransform& init_transform,
                              PerPlayerDifficulty difficulty,
                              KartRenderType krt = KRT_DEFAULT);
   virtual      ~KartRewinder() {};
   virtual BareNetworkString* saveState() const;
   void          reset();
   virtual void  rewindToState(BareNetworkString *p) OVERRIDE;
   virtual void  rewindToEvent(BareNetworkString *p) OVERRIDE;
   virtual void  update(float dt);

   // -------------------------------------------------------------------------
   virtual void  undoState(BareNetworkString *p) OVERRIDE
   {
   };   // undoState

   // -------------------------------------------------------------------------
   virtual void  undoEvent(BareNetworkString *p) OVERRIDE
   {
   };   // undoEvent

   // -------------------------------------------------------------------------

   

};   // Rewinder
#endif

