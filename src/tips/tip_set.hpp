//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2019 dumaosen
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

#ifndef HEADER_TIP_SET_HPP
#define HEADER_TIP_SET_HPP

#include "utils/string_utils.hpp"
#include "utils/translation.hpp"

#include <irrString.h>
#include <string>
#include <vector>

class XMLNode;

/** This class stores a set of tip definition from the xml file, including
 *  texts, icons and goto button.
 * \ingroup tips
 */
class TipSet
{
public:
    // The tipset types
    enum tipSetType
    {
        TIPSET_NOTYPE,
        TIPSET_QUEUE,
        TIPSET_RANDOM
    };

    // The goto types supported for a tip
    enum gotoType
    {
        GOTO_NO,
        GOTO_SCREEN,
        GOTO_WEBSITE
    };

    // Essential things a tip has
    struct tip
    {
        std::string text;
        std::string icon_path;
        gotoType goto_type;
        std::string goto_address;
        void runGoto();
        irr::core::stringw getWText() const { return _(text.c_str()); }
    };

private:
    /** The id of this tipset. */
    std::string m_id;
    
    /** The type of this tipset. */
    tipSetType m_type;

    /** When it's a queued tip, store the progress. */
    int m_progress;

    /** An important tipset which should be always shown. */
    bool m_is_important;

    /** If the tipset uses hardcode */
    bool m_hardcode;

    void parseTips(const XMLNode * input, std::vector<tip> &parent);

    /** The vector storing all tips */
    std::vector<tip> m_tipset;

public:
             TipSet(const XMLNode * input);
    virtual ~TipSet() {};

    std::string getID() const { return m_id; }
    tipSetType getType() const { return m_type; }
    bool isImportant()  const { return m_is_important; }

    tip getTip();
    void resetTip();

    void addHardcodeTip(std::string text, std::string icon, gotoType goto_type,
                        std::string address, int position);

};   // class TipSet

#endif
