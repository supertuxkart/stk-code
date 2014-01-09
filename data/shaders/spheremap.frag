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

#version 130
uniform sampler2D texture;
uniform vec3 lightdir;

noperspective in vec3 normal;
in vec4 vertex_color;
noperspective in vec3 eyeVec;
noperspective in vec3 lightVec;

void main()
{
    vec3 forward = vec3(0.0, 0.0, 1.0);

    // get the angle between the forward vector and the horizontal portion of the normal
    vec3 normal_x = normalize(vec3(normal.x, 0.0, normal.z));
    float sin_theta_x = length(cross( forward, normal_x )) * normal.x/abs(normal.x);

    // get the angle between the forward vector and the vertical portion of the normal
    vec3 normal_y = normalize(vec3(0.0, normal.y, normal.z));
    float sin_theta_y = length(cross( forward, normal_y ))* normal.y/abs(normal.y);

    vec4 detail0 = texture2D(texture, vec2(0.5 + sin_theta_x*0.5, 0.5 + sin_theta_y*0.5));

    gl_FragColor = detail0 * (0.5 + dot(lightdir, normal)) * vertex_color; // 0.5 is the ambient light.
    //gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);

    // specular (phong)
    vec3 R = normalize(reflect(lightVec, normal));
    float specular = max(dot(R,eyeVec),0.0);

    //gl_FragColor = vec4(specular, specular, specular, 1.0);

    if (specular > 0.0)
    {
        // weak specular
        specular = specular*specular;
        specular = specular*specular;
        float specular_weak = specular*2.0; //max(specular*1.1, 1.0);
        gl_FragColor += vec4(specular_weak, specular_weak, specular_weak, 0.0);

        /*
        // strong specular
        specular = specular*specular;
        float specular_strong = specular;
        gl_FragColor += vec4(specular_strong, specular_strong, specular_strong, 0.0);
        */
    }
}
