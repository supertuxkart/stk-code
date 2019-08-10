//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2004-2005 Steve Baker <sjbaker1@airmail.net>
//  Copyright (C) 2006-2007 Eduardo Hernandez Munoz
//  Copyright (C) 2010-2015 Joerg Henrichs
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

#ifndef HEADER_BATTLE_AI_HPP
#define HEADER_BATTLE_AI_HPP

#include "karts/controller/arena_ai.hpp"

class ThreeStrikesBattle;
class WorldWithRank;

/** The actual battle AI.
 * \ingroup controller
 */
class BattleAI : public ArenaAI
{
protected:
    /** Keep a pointer to world. */
    WorldWithRank *m_world;
    ThreeStrikesBattle* m_tsb_world;

    // ------------------------------------------------------------------------
    virtual void  findClosestKart(bool consider_difficulty,
                                  bool find_sta) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual int   getCurrentNode() const OVERRIDE;

private:
    // ------------------------------------------------------------------------
    virtual void  findTarget() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual float getKartDistance(const AbstractKart* kart) const OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool  isKartOnRoad() const OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool  isWaiting() const OVERRIDE;

public:
                  BattleAI(AbstractKart *kart);
    // ------------------------------------------------------------------------
                 ~BattleAI();

};

#endif
