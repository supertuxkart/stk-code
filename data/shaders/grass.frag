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


uniform vec4 fogColor;
uniform float fogFrom;
uniform float fogTo;
uniform int fog;
uniform sampler2D tex;
varying vec4 coord;

void main()
{
    vec4 color = texture2D(tex, gl_TexCoord[0].st);
    vec4 solidColor = vec4(color.r, color.g, color.b, 1);

    if (fog == 1)
    {
        if (coord.z > fogTo)
        {
            gl_FragColor = fogColor;
            gl_FragColor.a = color.a;
        }
        else if (coord.z > fogFrom)
        {
            float fogIntensity = (coord.z - fogFrom) / (fogTo - fogFrom);
            vec4 color2 = fogIntensity*fogColor + (1.0 - fogIntensity)*solidColor;
            color2.a = color.a;
            gl_FragColor = color2;
        }
        else
        {
            gl_FragColor = color;
        }
    }
    else
    {
        gl_FragColor = color;
    }
}
