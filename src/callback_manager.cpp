//  $Id: callback_manager.cpp 796 2006-09-27 07:06:34Z hiker $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Joerg Henrichs
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

#include "callback_manager.hpp"
#include "physics/moving_physics.hpp"

CallbackManager *callback_manager=NULL;

CallbackManager::CallbackManager()
{
    for(int i=0; i<CB_MAX; i++)
    {
        m_allCallbacks[i].clear();
    }
}   // CallbackManager

//-----------------------------------------------------------------------------
CallbackManager::~CallbackManager()
{
    for(int i=0; i<CB_MAX; i++)
    {
        for(std::vector<Callback*>::const_iterator c = m_allCallbacks[i].begin();
            c != m_allCallbacks[i].end(); c++)
        {
            MovingPhysics *mp = dynamic_cast<MovingPhysics*>(*c);
            if(mp)
            {
                ssgDeRefDelete(mp);
            }
            else
            {
                delete *c;
            }
        }   // for c in m_allCallbacks[i]
        m_allCallbacks[i].clear();
    }   // for i <CB_MAX
}   // ~CallbackManager

//-----------------------------------------------------------------------------
void CallbackManager::addCallback(Callback *c, CallbackType t) 
{
    m_allCallbacks[t].push_back(c);
        MovingPhysics *mp = dynamic_cast<MovingPhysics*>(c);
        if(mp)
        {
            mp->ref();
        }
}   // addCallback

//-----------------------------------------------------------------------------


void CallbackManager::clear(CallbackType cbType)
{
    for(std::vector<Callback*>::const_iterator c = m_allCallbacks[cbType].begin();
        c != m_allCallbacks[cbType].end(); c++)
    {
        MovingPhysics *mp = dynamic_cast<MovingPhysics*>(*c);
        if(mp)
        {
            ssgDeRefDelete(mp);
        }
        else
        {
            delete *c;
        }
    }

    m_allCallbacks[cbType].clear();
}  // clear

//-----------------------------------------------------------------------------
void CallbackManager::update(float dt) const
{
    for(int i=0; i<CB_MAX; i++)
    {
        for(std::vector<Callback*>::const_iterator c = m_allCallbacks[i].begin();
            c != m_allCallbacks[i].end(); c++)
            (*c)->update(dt);
    }   // for i

}   // update
//-----------------------------------------------------------------------------
void CallbackManager::initAll() const
{
    for(int i=0; i<CB_MAX; i++)
    {
        for(std::vector<Callback*>::const_iterator c = m_allCallbacks[i].begin();
            c != m_allCallbacks[i].end(); c++)
            (*c)->init();
    }   // for i

}   // initAll

//-----------------------------------------------------------------------------
// Called when restarting a race
void CallbackManager::reset() const
{
    for(int i=0; i<CB_MAX; i++)
    {
        for(std::vector<Callback*>::const_iterator c = m_allCallbacks[i].begin();
            c != m_allCallbacks[i].end(); c++)
            (*c)->reset();
    }   // for i

}   // initAll

//-----------------------------------------------------------------------------
void CallbackManager::handleExplosion(const btVector3& pos, 
                                      const MovingPhysics* mp) const
{
    for(int i=0; i<CB_MAX; i++)
    {
        for(std::vector<Callback*>::const_iterator c = m_allCallbacks[i].begin();
            c != m_allCallbacks[i].end(); c++)
            (*c)->handleExplosion(pos, mp==(*c));
    }   // for i

}   // handleExplosion

