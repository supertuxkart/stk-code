//  $Id: animation_manager.hpp 1681 2008-04-09 13:52:48Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2009  Joerg Henrichs
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

#ifndef HEADER_ANIMATION_MANAGER_HPP
#define HEADER_ANIMATION_MANAGER_HPP

#include <string>
#include <vector>

#include "utils/no_copy.hpp"

class AnimationBase;
class Track;
class XMLNode;

/**
  * \brief Controls all animations of a track.
  * \ingroup animations
  */
class AnimationManager : public NoCopy
{
private:
    std::vector<AnimationBase*> m_all_animations;

public:
         AnimationManager(const Track &track, const XMLNode &node);
        ~AnimationManager();
    void update(float dt);
    void reset();
};   // AnimationManager

#endif

