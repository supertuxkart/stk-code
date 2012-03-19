//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2012  Joerg Henrichs
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


#include "karts/abstract_kart.hpp"

#include "items/powerup.hpp"
#include "karts/kart_model.hpp"
#include "karts/kart_properties.hpp"
#include "karts/kart_properties_manager.hpp"

/** Creates a kart. 
 *  \param ident The identifier of the kart.
 *  \param world_kart_id  The world index of this kart.
 *  \param position The start position of the kart (1<=position<=n).
 *  \param init_transform The start position of the kart.
 */
AbstractKart::AbstractKart(const std::string& ident, 
                           int world_kart_id, int position, 
                           const btTransform& init_transform)
             : Moveable()
{
    m_world_kart_id   = world_kart_id;
    m_kart_properties = kart_properties_manager->getKart(ident);
    assert(m_kart_properties != NULL);
    
    // We have to take a copy of the kart model, since otherwise
    // the animations will be mixed up (i.e. different instances of
    // the same model will set different animation frames).
    // Technically the mesh in m_kart_model needs to be grab'ed and
    // released when the kart is deleted, but since the original 
    // kart_model is stored in the kart_properties all the time,
    // there is no risk of a mesh being deleted to early.
    m_kart_model  = m_kart_properties->getKartModelCopy();
    m_kart_width  = m_kart_model->getWidth();
    m_kart_height = m_kart_model->getHeight();
    m_kart_length = m_kart_model->getLength();
}   // AbstractKart

// ----------------------------------------------------------------------------
AbstractKart::~AbstractKart()
{
    delete m_kart_model;
}   // ~AbstractKart

// ----------------------------------------------------------------------------
void AbstractKart::reset()
{
}   // reset

// ----------------------------------------------------------------------------
/** Returns a name to be displayed for this kart. */
const wchar_t* AbstractKart::getName() const 
{ 
    return m_kart_properties->getName(); 
}   // getName;
// ----------------------------------------------------------------------------
/** Returns a unique identifier for this kart (name of the directory the
 *  kart was loaded from). */
const std::string& AbstractKart::getIdent() const
{
    return m_kart_properties->getIdent();
}   // getIdent
// ----------------------------------------------------------------------------
bool AbstractKart::isWheeless() const 
{
    return m_kart_model->getWheelModel(0)==NULL;
}   // isWheeless

