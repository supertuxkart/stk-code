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


// Jean-manuel clemencon (c) supertuxkart 2013
// bubble gum shield
// TODO: Add a nice texture and soft edges when intersect with geometry

uniform sampler2D tex;
uniform float transparency;

varying vec2 uv;
varying vec3 eyeVec;
varying vec3 normal;

void main()
{
	float inter = dot(normal, eyeVec);
	float m = texture2D(tex, vec2(0.5, uv.y)).r;
	inter = 1.0 - inter;
	float alpha = inter + 1.0;// * m;

	gl_FragColor = vec4(0.8, 0.16, 0.48, alpha);
}
