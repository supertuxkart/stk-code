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

#include "karts/cached_characteristics.hpp"

#include "utils/interpolation_array.hpp"

CachedCharacteristics::CachedCharacteristics(const AbstractCharacteristics *origin) :
    m_values(CHARACTERISTIC_COUNT),
    m_origin(origin)
{
    updateSource();
}

CachedCharacteristics::~CachedCharacteristics()
{
    // Delete all not-null values
    for (int i = 0; i < CHARACTERISTIC_COUNT; i++)
    {
        SaveValue &v = m_values[i];
        if (v.content)
        {
            switch (getType(static_cast<CharacteristicType>(i)))
            {
            case TYPE_FLOAT:
                delete static_cast<float*>(v.content);
                break;
            case TYPE_FLOAT_VECTOR:
                delete static_cast<std::vector<float>*>(v.content);
                break;
            case TYPE_INTERPOLATION_ARRAY:
                delete static_cast<InterpolationArray*>(v.content);
                break;
            }
            v.content = nullptr;
        }
    }
}

void CachedCharacteristics::updateSource()
{
    for (int i = 0; i < CHARACTERISTIC_COUNT; i++)
    {
        SaveValue &v = m_values[i];
        
        bool isSet = false;
        switch (getType(static_cast<CharacteristicType>(i)))
        {
        case TYPE_FLOAT:
        {
            float value;
            float *ptr = static_cast<float*>(v.content);
            m_origin->process(static_cast<CharacteristicType>(i), &value, isSet);
            if (isSet)
            {
                if (!ptr)
                {
                    float *newPtr = new float();
                    v.content = newPtr;
                    ptr = newPtr;
                }
                *ptr = value;
            }
            else
            {
                if (ptr)
                {
                    delete ptr;
                    v.content = nullptr;
                }
            }
            break;
        }
        case TYPE_FLOAT_VECTOR:
        {
            std::vector<float> value;
            std::vector<float> *ptr = static_cast<std::vector<float>*>(v.content);
            m_origin->process(static_cast<CharacteristicType>(i), &value, isSet);
            if (isSet)
            {
                if (!ptr)
                {
                    std::vector<float> *newPtr = new std::vector<float>();
                    v.content = newPtr;
                    ptr = newPtr;
                }
                *ptr = value;
            }
            else
            {
                if (ptr)
                {
                    delete ptr;
                    v.content = nullptr;
                }
            }
            break;
        }
            break;
        case TYPE_INTERPOLATION_ARRAY:
        {
            InterpolationArray value;
            InterpolationArray *ptr = static_cast<InterpolationArray*>(v.content);
            m_origin->process(static_cast<CharacteristicType>(i), &value, isSet);
            if (isSet)
            {
                if (!ptr)
                {
                    InterpolationArray *newPtr = new InterpolationArray();
                    v.content = newPtr;
                    ptr = newPtr;
                }
                *ptr = value;
            }
            else
            {
                if (ptr)
                {
                    delete ptr;
                    v.content = nullptr;
                }
            }
            break;
        }
            break;
        }
    }
}

const SkiddingProperties* CachedCharacteristics::getSkiddingProperties() const
{
    return m_origin->getSkiddingProperties();
}

void CachedCharacteristics::process(CharacteristicType type, Value value, bool &isSet) const
{
    void *v = m_values[type].content;
    if (v)
    {
        switch (getType(type))
        {
        case TYPE_FLOAT:
            *value.f = *static_cast<float*>(v);
            break;
        case TYPE_FLOAT_VECTOR:
            *value.fv = *static_cast<std::vector<float>*>(v);
            break;
        case TYPE_INTERPOLATION_ARRAY:
            *value.ia = *static_cast<InterpolationArray*>(v);
            break;
        }
        isSet = true;
    }
}

