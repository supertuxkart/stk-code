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


// motion_blur.frag

// The actual boost amount (which linearly scales the blur to be shown).
// should be in the range [0.0, 1.0], though a larger value might make
// the blurring too string. Atm we are using [0, 0.5].
uniform float boost_amount;

// The color buffer to use.
uniform sampler2D color_buffer;
uniform sampler2D dtex;

// Center (in texture coordinates) at which the kart is. A small circle
// around this center is not blurred (see mask_radius below)
uniform vec2 center;

// Radius of mask around the character in which no blurring happens
// so that the kart doesn't get blurred.
uniform float mask_radius;

uniform mat4 previous_viewproj;

out vec4 FragColor;

// Number of samples used for blurring
#define NB_SAMPLES 8

#stk_include "utils/getPosFromUVDepth.frag"

void main()
{
    vec2 texcoords = gl_FragCoord.xy / u_screen;

    // Sample the color buffer
    vec3 color = texture(color_buffer, texcoords).rgb;

    float z = texture(dtex, texcoords).x;
    vec4 ViewPos = getPosFromUVDepth(vec3(texcoords, z), u_inverse_projection_matrix);
    vec4 OldScreenPos = previous_viewproj * u_inverse_view_matrix * ViewPos;
    OldScreenPos /= OldScreenPos.w;
    OldScreenPos = .5 * OldScreenPos + .5;

    // Compute the blur direction.
    // IMPORTANT: we don't normalize it so that it avoids a glitch around 'center',
    // plus it naturally scales the motion blur in a cool way :)
    vec2 blur_dir = texcoords - OldScreenPos.xy;

    // Compute the blurring factor:
    // - apply the mask, i.e. no blurring in a small circle around the kart
    float blur_factor = max(0.0, length(texcoords - center) - mask_radius);

    // - apply the boost amount
    blur_factor *= boost_amount;

    // Scale the blur direction
    blur_dir *= blur_factor;

    // Compute the blur
    vec2 inc_vec = blur_dir / vec2(NB_SAMPLES);
    vec2 blur_texcoords = texcoords - inc_vec * float(NB_SAMPLES) / 2.;
    for(int i=1 ; i < NB_SAMPLES ; i++)
    {
        color += texture(color_buffer, blur_texcoords).rgb;
        blur_texcoords += inc_vec;
    }
    color /= vec3(NB_SAMPLES);
    FragColor = vec4(color, 1.0);
}
