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

#include "karts/abstract_characteristics.hpp"

#include "utils/log.hpp"

#include <cmath>

AbstractCharacteristics::AbstractCharacteristics()
{
}

float AbstractCharacteristics::processFloat(CharacteristicType type, float value) const
{
    return getFloat(type);
}

std::vector<float> AbstractCharacteristics::processFloatVector(CharacteristicType type, const std::vector<float> &value) const
{
    return getFloatVector(type);
}

InterpolationArray AbstractCharacteristics::processInterpolationArray(CharacteristicType type, const InterpolationArray &value) const
{
    return getInterpolationArray(type);
}

float AbstractCharacteristics::getFloat(CharacteristicType type) const
{
    Log::fatal("AbstractCharacteristics", "This type does not support getFloat");
    return NAN;
}

std::vector<float> AbstractCharacteristics::getFloatVector(CharacteristicType type) const
{
    Log::fatal("AbstractCharacteristics", "This type does not support getFloatVector");
    return std::vector<float>();
}

InterpolationArray AbstractCharacteristics::getInterpolationArray(CharacteristicType type) const
{
    Log::fatal("AbstractCharacteristics", "This type does not support getInterpolationArray");
    return InterpolationArray();
}

