//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006-2015 SuperTuxKart-Team
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

#include "karts/xml_characteristics.hpp"

#include "io/xml_node.hpp"

XmlCharacteristics::XmlCharacteristics(const std::string &filename) :
    m_values(CHARACTERISTIC_COUNT),
    m_skidding(nullptr)
{
    if (!filename.empty())
        load(filename);
}

const SkiddingProperties* XmlCharacteristics::getSkiddingProperties() const
{
    return m_skidding;
}

void XmlCharacteristics::process(CharacteristicType type, Value value, bool &isSet) const
{
    //TODO
}

void XmlCharacteristics::load(const std::string &filename)
{
    const XMLNode* root = new XMLNode(filename);
}

