//  $Id: binding.hpp 6504 2010-11-11 20:59:49Z hikerstk $
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2010 Marianne Gagnon
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


#ifndef BINDING_HPP
#define BINDING_HPP

#include <fstream>

#include "irrlicht.h"
#include "input/input.hpp"
#include "utils/no_copy.hpp"

/**
  * \ingroup config
  */
class Binding
{
private:
    Input::InputType        m_type;
    int                     m_id;
    Input::AxisDirection    m_dir;
    wchar_t                 m_character;
public:
    /** Returns the type of device this binding is using. */
    Input::InputType     getType() const {return m_type; }
    // ------------------------------------------------------------------------
    /** Returns the id this binding is using. */
    int                  getId() const {return m_id;}
    // ------------------------------------------------------------------------
    /** Returns the direction this binding is using. */
    Input::AxisDirection getDirection() const {return m_dir;}
    // ------------------------------------------------------------------------
    /** Defines all values of this binding. */
    void                 set(Input::InputType type, int id, 
                             Input::AxisDirection dir, 
                             wchar_t character)
    {
        m_type = type; m_id=id; m_dir=dir; m_character=character;
    }   // set

    // ------------------------------------------------------------------------
    void               serialize  (std::ofstream& stream) const;
    bool               deserialize(irr::io::IrrXMLReader* xml);
    irr::core::stringw getAsString() const;
};
#endif
