//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2024 SuperTuxKart-Team
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

// speed_lines.frag
// Creates radial speed lines effect during acceleration/boost

// Speed intensity [0.0, 1.0] - controls line density and length
uniform float speed_intensity;

// Boost intensity [0.0, 1.0] - for nitro/zipper effects (adds purple tint)
uniform float boost_intensity;

// Time for animation
uniform float time;

// The color buffer
uniform sampler2D color_buffer;

// Center point (in UV coordinates) - where the kart/driver is
uniform vec2 center;

// Inner radius where lines start to fade in
uniform float inner_radius;

out vec4 FragColor;

// Pseudo-random function
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

// Generate speed line pattern
float speedLine(vec2 uv, vec2 center_point, float angle_offset)
{
    vec2 dir = uv - center_point;
    float dist = length(dir);

    // Calculate angle from center
    float angle = atan(dir.y, dir.x);

    // Number of lines around the circle
    float num_lines = 60.0 + speed_intensity * 40.0;

    // Create angular sections
    float section = floor((angle + 3.14159) / (6.28318 / num_lines));

    // Random offset for each line to make them non-uniform
    float rand_offset = hash(vec2(section, floor(time * 2.0)));

    // Line position within its section
    float line_pos = fract((angle + 3.14159) / (6.28318 / num_lines));

    // Create thin lines (using smoothstep for anti-aliasing)
    float line_width = 0.08 + rand_offset * 0.04;
    float line = smoothstep(0.5 - line_width, 0.5, line_pos) *
                 smoothstep(0.5 + line_width, 0.5, line_pos);

    // Radial falloff - lines are stronger at the edges, fade near center
    float radial = smoothstep(inner_radius, inner_radius + 0.2, dist);

    // Lines get longer/more visible at edges of screen
    float edge_intensity = smoothstep(0.3, 0.8, dist);

    // Animate lines moving outward
    float anim_offset = fract(time * (1.0 + speed_intensity) + rand_offset);
    float anim_fade = smoothstep(0.0, 0.3, anim_offset) * smoothstep(1.0, 0.7, anim_offset);

    // Random visibility per line (some lines appear, some don't)
    float visibility = step(0.3 - speed_intensity * 0.25, hash(vec2(section, floor(time * 3.0 + angle_offset))));

    return line * radial * edge_intensity * anim_fade * visibility;
}

void main()
{
    vec2 texcoords = gl_FragCoord.xy / u_screen;

    // Sample the original color
    vec4 original = texture(color_buffer, texcoords);

    // Early exit if no speed effect
    if (speed_intensity < 0.01)
    {
        FragColor = original;
        return;
    }

    // Generate multiple layers of speed lines for depth
    float lines = 0.0;
    lines += speedLine(texcoords, center, 0.0) * 0.6;
    lines += speedLine(texcoords, center, 1.0) * 0.3;
    lines += speedLine(texcoords, center, 2.0) * 0.2;

    // Scale by speed intensity
    lines *= speed_intensity * 0.8;

    // Clamp lines intensity
    lines = clamp(lines, 0.0, 0.6);

    // Base line color (white)
    vec3 line_color = vec3(1.0, 1.0, 1.0);

    // Add purple tint for boost (nitro/zipper)
    // Purple: RGB(0.7, 0.4, 1.0)
    vec3 boost_color = vec3(0.75, 0.5, 1.0);
    line_color = mix(line_color, boost_color, boost_intensity * 0.7);

    // Slight glow effect - brighten the line area
    vec3 glow = line_color * lines * 1.5;

    // Blend lines with original image (additive blend)
    vec3 final_color = original.rgb + glow;

    // Add subtle chromatic aberration near lines for extra punch
    if (boost_intensity > 0.3)
    {
        float aberration = lines * boost_intensity * 0.003;
        vec2 dir_to_center = normalize(center - texcoords);
        float r = texture(color_buffer, texcoords + dir_to_center * aberration).r;
        float b = texture(color_buffer, texcoords - dir_to_center * aberration).b;
        final_color.r = mix(final_color.r, r, boost_intensity * 0.3);
        final_color.b = mix(final_color.b, b, boost_intensity * 0.3);
    }

    FragColor = vec4(final_color, original.a);
}
