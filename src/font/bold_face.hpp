//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2016 SuperTuxKart-Team
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

#ifndef HEADER_BOLD_FACE_HPP
#define HEADER_BOLD_FACE_HPP

#include "font/font_with_face.hpp"

class FaceTTF;

class BoldFace : public FontWithFace
{
private:
    virtual bool supportLazyLoadChar() const OVERRIDE          { return true; }
    // ------------------------------------------------------------------------
    virtual unsigned int getGlyphPageSize() const OVERRIDE     { return 1024; }
    // ------------------------------------------------------------------------
    virtual float getScalingFactorOne() const OVERRIDE         { return 0.2f; }
    // ------------------------------------------------------------------------
    virtual unsigned int getScalingFactorTwo() const OVERRIDE   { return 120; }

public:
    LEAK_CHECK()
    // ------------------------------------------------------------------------
    BoldFace(FaceTTF* ttf);
    // ------------------------------------------------------------------------
    virtual ~BoldFace() {}
    // ------------------------------------------------------------------------
    virtual void init() OVERRIDE;
    // ------------------------------------------------------------------------
    virtual void reset() OVERRIDE;

};   // BoldFace

#endif
/* EOF */
