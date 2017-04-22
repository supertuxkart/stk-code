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

#ifndef HEADER_CACHED_CHARACTERISTICS_HPP
#define HEADER_CACHED_CHARACTERISTICS_HPP

#include "karts/abstract_characteristic.hpp"

#include <assert.h>

class CachedCharacteristic : public AbstractCharacteristic
{
private:
    /** Used to store a value. */
    struct SaveValue
    {
        void *content;

        SaveValue() : content(nullptr) {}
        SaveValue(void *content) : content(content) {}
    };

    /** All values for a characteristic. A nullptr means it is not set. */
    std::vector<SaveValue> m_values;

    /** The characteristics that hold the original values. */
    const AbstractCharacteristic *m_origin;

public:
    CachedCharacteristic(const AbstractCharacteristic *origin);
    CachedCharacteristic(const CachedCharacteristic &characteristics) = delete;
    virtual ~CachedCharacteristic();

    /** Fetches all cached values from the original source. */
    void updateSource();
    virtual void copyFrom(const AbstractCharacteristic *other) { assert(false); }
    virtual void process(CharacteristicType type, Value value, bool *is_set) const;
};

#endif

