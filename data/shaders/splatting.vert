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


varying vec3 normal;
varying vec4 vertex_color;
varying vec3 lightdir2;
uniform vec3 lightdir;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_MultiTexCoord1;
    gl_Position = ftransform();
    vertex_color = gl_Color;

    //normal = normalize(gl_NormalMatrix * gl_Normal);
    normal = normalize(gl_Normal);
    lightdir2 = normalize(lightdir);
}
