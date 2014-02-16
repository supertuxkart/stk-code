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
#version 330 compatibility
uniform sampler2D tex;
uniform sampler2D glow_tex;
uniform float transparency;
uniform vec3 sun_pos;

in vec2 uv_anim;
in vec2 uv;
in vec2 uv_cl;
in vec3 vertex;
in vec2 uv_fast;
out vec4 FragColor;

void main()
{
  vec2 uv_temp = uv;
  vec3 V = normalize(vertex);
  vec3 L = normalize(vec3(sun_pos));
  
  vec3 col = texture(tex, vec2((L.y + 1.0) / 2.0, V.y)).xyz;
  
  float vl = clamp(dot(V, L), 0., 1.);

	float paint = texture(tex, uv_temp * 3).a;
	
	uv_temp += 20;
	//float paint2 = texture(tex, uv_temp * 5).a;
	
	float paint2 = texture(tex, uv * 5.).a;
	
	// Get the general cloud mask
	
	
	float hello = texture(glow_tex, (uv_cl + paint2 * 0.07) *2.).g;
	
	float cld_mask = texture(glow_tex, (uv_anim + hello * 0.007 )).r;
	
	vec2 fast = vec2(-uv_fast.x, uv_fast.y);// + (hello * 0.007);
	float cld_fast = texture(glow_tex, fast ).r;



	
	cld_mask = (cld_mask * hello * 0.5);
  cld_fast = (cld_fast * hello );

	col = cld_mask + col*(1. - cld_mask);
	col = cld_fast + col*(1. - cld_fast);
	
  FragColor = vec4( vec3(col * paint * paint2), 1.0);
  

  //FragColor = vec4(vec3(ou), 1.0);
}
