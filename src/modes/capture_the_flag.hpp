//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2018 SuperTuxKart-Team
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

#ifndef CAPTURE_THE_FLAG_HPP
#define CAPTURE_THE_FLAG_HPP

#include "modes/free_for_all.hpp"

#include <vector>
#include <string>

namespace irr
{
    namespace scene
    {
        class IAnimatedMeshSceneNode; class IAnimatedMesh;
    }
}

class CaptureTheFlag : public FreeForAll
{
private:
    scene::IAnimatedMeshSceneNode* m_red_flag_node;

    scene::IAnimatedMeshSceneNode* m_blue_flag_node;

    irr::scene::IAnimatedMesh* m_red_flag_mesh;

    irr::scene::IAnimatedMesh* m_blue_flag_mesh;

    int m_red_scores, m_blue_scores, m_red_holder, m_blue_holder;

    btTransform m_red_trans, m_blue_trans;

    // ------------------------------------------------------------------------
    virtual video::SColor getColor(unsigned int kart_id) const OVERRIDE;

public:
    // ------------------------------------------------------------------------
    CaptureTheFlag();
    // ------------------------------------------------------------------------
    virtual ~CaptureTheFlag();
    // ------------------------------------------------------------------------
    virtual void init() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void reset() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void update(int ticks) OVERRIDE;
    // ------------------------------------------------------------------------
    virtual bool hasTeam() const OVERRIDE                      { return true; }
    // ------------------------------------------------------------------------
    bool getKartCTFResult(unsigned int kart_id) const
    {
        if (m_red_scores == m_blue_scores)
            return true;

        bool red_win = m_red_scores > m_blue_scores;
        KartTeam team = getKartTeam(kart_id);

        if ((red_win && team == KART_TEAM_RED) ||
            (!red_win && team == KART_TEAM_BLUE))
            return true;
        else
            return false;
    }
    // ------------------------------------------------------------------------
    int getRedScore() const                            { return m_red_scores; }
    // ------------------------------------------------------------------------
    int getBlueScore() const                          { return m_blue_scores; }
    // ------------------------------------------------------------------------
    int getRedHolder() const                           { return m_red_holder; }
    // ------------------------------------------------------------------------
    int getBlueHolder() const                         { return m_blue_holder; }
    // ------------------------------------------------------------------------
    const Vec3& getRedFlag() const   { return (Vec3&)m_red_trans.getOrigin(); }
    // ------------------------------------------------------------------------
    const Vec3& getBlueFlag() const { return (Vec3&)m_blue_trans.getOrigin(); }
    // ------------------------------------------------------------------------
    virtual bool isRaceOver() OVERRIDE;

};   // CaptureTheFlag

#endif
