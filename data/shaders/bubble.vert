//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2013 the SuperTuxKart team
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


// Creates a bubble (wave) effect by distorting the texture depending on time
#version 130
uniform mat4 ModelViewProjectionMatrix;
uniform float time;

in vec3 Position;
in vec2 Texcoord;
out vec2 uv;

void main()
{
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);

    float delta_x = cos(time*3.0) * sin( 4.0 * Texcoord.x * 6.28318531 );
    float delta_y = cos(time*2.0) * sin( 3.0 * Texcoord.y * 6.28318531 );

    uv = Texcoord + vec2(0.02*delta_x, 0.02*delta_y);
}
