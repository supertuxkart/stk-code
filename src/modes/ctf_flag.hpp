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

#ifndef HEADER_CTF_FLAG_HPP
#define HEADER_CTF_FLAG_HPP

#include "network/rewinder.hpp"
#include "utils/types.hpp"
#include "utils/vec3.hpp"

#include "LinearMath/btTransform.h"

enum FlagColor : unsigned int
{
    FC_RED = 0,
    FC_BLUE = 1
};

namespace irr
{
    namespace scene
    {
        class IAnimatedMeshSceneNode;
    }
}
class CTFFlag : public Rewinder
{
public:
    static const int IN_BASE = -1;
    static const int OFF_BASE = -2;
private:
    /* Either save the above 2 status, or kart id holder this flag. */
    int8_t m_flag_status;

    /* Currnet flag transformation. */
    btTransform m_flag_trans;

    /* Transformation of IN_BASE. */
    const btTransform m_flag_base_trans;

    /* If OFF_BASE of m_flag_status, save the ticks since off base, use for
     * auto return base for a flag (see ServerConfig). */
    uint16_t m_ticks_since_off_base;

    FlagColor m_flag_color;

public:
    // ------------------------------------------------------------------------
    CTFFlag(FlagColor fc, const btTransform& base_trans)
        : Rewinder(fc == FC_RED ? "TR" : "TB"), m_flag_base_trans(base_trans)
    {
        // UID rewinder with "T" which is after "Kx" for kart so
        // updateFlagTrans is called after kart is rewound
        m_flag_status = IN_BASE;
        m_flag_trans.setOrigin(Vec3(0.0f));
        m_flag_trans.setRotation(btQuaternion(0.0f, 0.0f, 0.0f, 1.0f));
        m_flag_color = fc;
        m_ticks_since_off_base = 0;
    }
    // ------------------------------------------------------------------------
    virtual void saveTransform() {}
    // ------------------------------------------------------------------------
    virtual void computeError() {}
    // ------------------------------------------------------------------------
    virtual BareNetworkString* saveState(std::vector<std::string>* ru);
    // ------------------------------------------------------------------------
    virtual void undoEvent(BareNetworkString* buffer) {}
    // ------------------------------------------------------------------------
    virtual void rewindToEvent(BareNetworkString* buffer) {}
    // ------------------------------------------------------------------------
    virtual void restoreState(BareNetworkString* buffer, int count);
    // ------------------------------------------------------------------------
    virtual void undoState(BareNetworkString* buffer) {}
    // ------------------------------------------------------------------------
    int getHolder() const
    {
        if (m_flag_status >= 0)
            return m_flag_status;
        return -1;
    }
    // ------------------------------------------------------------------------
    int getStatus() const                             { return m_flag_status; }
    // ------------------------------------------------------------------------
    const Vec3& getOrigin() const   { return (Vec3&)m_flag_trans.getOrigin(); }
    // ------------------------------------------------------------------------
    const Vec3& getBaseOrigin() const
                               { return (Vec3&)m_flag_base_trans.getOrigin(); }
    // ------------------------------------------------------------------------
    const btTransform& getBaseTrans() const       { return m_flag_base_trans; }
    // ------------------------------------------------------------------------
    void resetToBase()
    {
        m_flag_status = IN_BASE;
        m_ticks_since_off_base = 0;
        updateFlagTrans();
    }
    // ------------------------------------------------------------------------
    void setCapturedByKart(int kart_id)
    {
        m_flag_status = (int8_t)kart_id;
        m_ticks_since_off_base = 0;
        updateFlagTrans();
    }
    // ------------------------------------------------------------------------
    void dropFlagAt(const btTransform& t)
    {
        m_flag_status = OFF_BASE;
        m_ticks_since_off_base = 0;
        m_flag_trans = t;
    }
    // ------------------------------------------------------------------------
    bool isInBase() const                  { return m_flag_status == IN_BASE; }
    // ------------------------------------------------------------------------
    bool canBeCaptured() const                { return !(m_flag_status >= 0); }
    // ------------------------------------------------------------------------
    void update(int ticks);
    // ------------------------------------------------------------------------
    void updateFlagTrans(const btTransform& off_base_trans = btTransform());
    // ------------------------------------------------------------------------
    void updateFlagGraphics(irr::scene::IAnimatedMeshSceneNode* flag_node);
};   // CTFFlag
#endif

