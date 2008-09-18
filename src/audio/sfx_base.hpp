//  $Id$
//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2006 Patrick Ammann <pammann@aro.ch>
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

#ifndef HEADER_SFX_HPP
#define HEADER_SFX_HPP

class Vec3;

class SFXBase
{
public:

    virtual void play() = 0;
    virtual void loop() = 0;
    virtual void stop() = 0;
    virtual void speed(float factor) = 0;
    void position(const Vec3 &position) {};
    virtual int getStatus() = 0;
};   // SfxBase


#endif // HEADER_SFX_HPP

