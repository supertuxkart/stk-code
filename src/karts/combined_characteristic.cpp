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

#include "karts/combined_characteristic.hpp"

#include "io/file_manager.hpp"
#include "karts/xml_characteristic.hpp"

#include <assert.h>

void CombinedCharacteristic::addCharacteristic(
    const AbstractCharacteristic *characteristic)
{
    m_children.push_back(characteristic);
}   // addCharacteristic

// ----------------------------------------------------------------------------
/** Combines all contained source characteristics. */
void CombinedCharacteristic::process(CharacteristicType type, Value value,
                                     bool *is_set) const
{
    for (const AbstractCharacteristic *characteristic : m_children)
        characteristic->process(type, value, is_set);
}   // process

// ============================================================================

void CombinedCharacteristic::unitTesting()
{
    // We need to use really existing xml attributes, all characteristics
    // were designed to work with those names.
    std::string s1=
        "<?xml version=\"1.0\"?>"
        "  <characteristic name=\"base\">"
        "    <suspension stiffness=\"4.5\" rest=\"-0.3\" travel=\"1+2\"/>"
        "    <stability roll-influence=\"1+2*3\" chassis-linear-damping=\"2*3+1\""
        "        chassis-angular-damping=\"0\" downward-impulse-factor=\"5\""
        "        track-connection-accel=\"2\" smooth-flying-impulse=\"250\" />"
        "  </characteristic>"
        "</characteristics>";

    XMLNode *xml1= file_manager->createXMLTreeFromString(s1);
    XmlCharacteristic *c1 = new XmlCharacteristic(xml1);
    delete xml1;

    CombinedCharacteristic *cc = new CombinedCharacteristic();
    cc->addCharacteristic(c1);

    assert( cc->getSuspensionStiffness()           ==  4.5f );
    assert( cc->getSuspensionRest()                == -0.3f );
    assert( cc->getSuspensionTravel()              ==  3.0f );
    // Note: no operator precedence supported, so 1+2*3 = 9
    assert( cc->getStabilityRollInfluence()        ==  9.0f );
    assert( cc->getStabilityChassisLinearDamping() ==  7.0f );

    std::string s2=
        "<?xml version=\"1.0\"?>"
        "  <characteristic name=\"base\">"
        "    <suspension stiffness=\"+1\" rest=\"-1\" travel=\"*2\"/>"
        "    <stability roll-influence=\"/3\" chassis-linear-damping=\"*1\"/>"
        "  </characteristic>"
        "</characteristics>";

    XMLNode *xml2= file_manager->createXMLTreeFromString(s2);
    XmlCharacteristic *c2 = new XmlCharacteristic(xml2);
    delete xml2;

    cc->addCharacteristic(c2);

    assert( cc->getSuspensionStiffness()           ==  5.5f );
    assert( cc->getSuspensionRest()                == -1.3f );
    assert( cc->getSuspensionTravel()              ==  6.0f );
    // Note: no operator precedence supported, so (1+2*3) / 3 = 3
    assert( cc->getStabilityRollInfluence()        ==  3.0f );
    assert( cc->getStabilityChassisLinearDamping() ==  7.0f );
    delete cc;

}   // unitTesting
