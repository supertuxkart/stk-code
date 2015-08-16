//
//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2014-2015  SuperTuxKart Team
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

#ifndef HEADER_SCRIPTVEC3_HPP
#define HEADER_SCRIPTVEC3_HPP

#include <angelscript.h>

namespace Scripting
{
    void RegisterVec3(asIScriptEngine *engine);

    struct SimpleVec3
    {
        float x;
        float y;
        float z;

        float getX() const { return x; }
        float getY() const { return y; }
        float getZ() const { return z; }

        float getLength() const
        {
            return sqrt(x*x + y*y + z*z);
        }

        SimpleVec3() : x(0), y(0), z(0) { }
        SimpleVec3(float p_x, float p_y, float p_z) : x(p_x), y(p_y), z(p_z) { }
        SimpleVec3(const SimpleVec3& other) : x(other.x), y(other.y), z(other.z) { }
        ~SimpleVec3() { }
    };
}
#endif
