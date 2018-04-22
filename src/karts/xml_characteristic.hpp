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

#ifndef HEADER_XML_CHARACTERISTICS_HPP
#define HEADER_XML_CHARACTERISTICS_HPP

#include "karts/abstract_characteristic.hpp"

#include <string>

class XMLNode;

class XmlCharacteristic : public AbstractCharacteristic
{
private:
    /** The computation that was read from an xml file */
    std::vector<std::string> m_values;

public:
    XmlCharacteristic(const XMLNode *node = nullptr);

    virtual void process(CharacteristicType type, Value value, bool *is_set) const;

    void load(const XMLNode *node);
    virtual void copyFrom(const AbstractCharacteristic *other);

private:
    static void processFloat(const std::string &processor, float *value, bool *is_set);
    static void processBool(const std::string &processor, bool *value, bool *is_set);
};

#endif

