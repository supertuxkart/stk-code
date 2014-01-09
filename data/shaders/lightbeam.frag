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


// Jean-manuel clemencon supertuxkart
// Creates a cone lightbeam effect by smoothing edges
// Original idea: http://udn.epicgames.com/Three/VolumetricLightbeamTutorial.html
// TODO: Soft edges when it intesects geometry
// Some artefacts are still visible
#version 130
uniform sampler2D tex;
uniform float transparency;

in vec2 uv;
noperspective in vec3 eyeVec;
noperspective in vec3 normal;

void main()
{
	float inter = dot(normal, eyeVec);
	float m = texture2D(tex, vec2(0.5, uv.y)).r;
	float alpha = inter * inter * inter * inter * m;

	gl_FragColor = vec4(1.0, 1.0, 0.8, alpha);
}
