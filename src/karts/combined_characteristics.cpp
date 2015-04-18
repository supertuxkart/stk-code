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

#include "karts/combined_characteristics.hpp"

void CombinedCharacteristics::addCharacteristic(const AbstractCharacteristics *characteristic)
{
    m_childs.push_back(characteristic);
}

const SkiddingProperties* CombinedCharacteristics::getSkiddingProperties() const
{
    for (const AbstractCharacteristics *characteristic : m_childs)
    {
        const SkiddingProperties *skid = characteristic->getSkiddingProperties();
        if (skid)
            return skid;
    }
    return nullptr;
}

void CombinedCharacteristics::process(CharacteristicType type, Value value, bool *isSet) const
{
    for (const AbstractCharacteristics *characteristic : m_childs)
        characteristic->process(type, value, isSet);
}

