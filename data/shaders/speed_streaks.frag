//  SuperTuxKart - a fun racing game with go-kart
//  Copyright (C) 2026 the SuperTuxKart team
//  Copyright (C) 2026 sbilliet
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


// speed_streaks.frag

uniform float u_intensity;            // 0.0 to 1.0, how strong the effect is (derived from the kart's speed excess)
uniform float u_radial_scroll_offset; // Integrated uv offset: encodes speed/time/direction.
uniform float u_time;                 // Raw world->getTime() specifically needed for color change over time.

uniform sampler2D noise_texture;

out vec4 FragColor;

const float WISP_DENSITY            = 0.60;     // Density <=> how many wisps should survive the density smoothstep.
const float WISP_SOFTNESS           = 0.30;     // Cutoff for noise range, determines how soft the wisps appear.
const float WISP_LENGTH             = 0.10;     // How much should we stretch the UV space radially.
const float WISP_AMOUNT             = 6.0;      // Amount of discrete sectors at the reference aspect ratio below.
const float WISP_REFERENCE_ASPECT   = 1.77;     // Aspect ratio the wisp amount is tuned for.
const float WISP_RIPPLE             = 0.003;    // Strength of the distortion applied to the noise_texture.y coordinates.
const float WISP_ACTIVATION         = 0.05;     // Threshold boost level at which the effect starts being fully visible.

const float RADIAL_MASK_SOFTNESS    = 1.0;
const float RADIAL_MASK_SIZE_MIN    = 1.0;
const float RADIAL_MASK_SIZE_MAX    = 0.5;

const float COLOR_CHANGE_SPEED      = 8.0;      // At what speed should the color change clockwise over time.
const float COLOR_SATURATION_MIN    = 0.1;
const float COLOR_SATURATION_MAX    = 1.0;

const float PI  = 3.14159;

void main() {
    // -- COORDS SETUP -- //
    // Normalize UVs [0, 1] 
    vec2 uv = gl_FragCoord.xy / u_screen;

    // Centre UVs [-0.5f, 0.5f]
    uv = uv - 0.5f;

    // Scale X coords to convert rectangular screen UVs into a square 1:1 space.
    float aspect_ratio = u_screen.x / u_screen.y;
    uv.x *= aspect_ratio;

    // Convert Cartesian coords to Polar coords (angle, distance & unit direction)
    // Calculate the angle (-PI to PI), distance around/from the center point and construct direction vector.
	float polar_angle_radians = atan(uv.y, uv.x);
	float polar_distance      = length(uv);
    vec2  polar_unit_vector   = vec2(cos(polar_angle_radians), sin(polar_angle_radians));

    // -- ASPECT RATIO CORRECTION -- //
    // Do a ray (unit_vector from before) box (screen extents) intersection to find the normalized polar_distance
    // to ensure UVs are mapped correctly to the whole screen.

    // Define the half-extents of the screen boundaries.
    // And the normalized ray vector (i.e. guard against division by zero)
    vec2  box_extents   = vec2(0.5 * aspect_ratio, 0.5);     
    vec2  ray_direction = max(abs(polar_unit_vector), vec2(0.0001));

    // t = distance / direction
    vec2  distances       = box_extents / ray_direction;       
    float border_distance = min(distances.x, distances.y);

    // Normalized polar coords.
    float polar_distance_normalized = polar_distance / border_distance;

    // Dynamically increase sector count on wider screens to keep streak density even
    float aspect_scale = sqrt(max(1.0, aspect_ratio / WISP_REFERENCE_ASPECT));
    float sectors = max(0.5 * WISP_AMOUNT, round(0.5 * WISP_AMOUNT * aspect_scale));
    float sector_index = polar_angle_radians / PI * sectors;

    // Radial coordinate (y portion of noise_texture UVs) used for scrolling the texture outwards.
	float base_radial_scroll = (polar_distance_normalized * WISP_LENGTH) - u_radial_scroll_offset;

    // -- NOISE SAMPLE(s) -- //
    // Wisp noise distortion, one ripple per wisp sector.
	float radial_ripple = sin(2.0 * PI * sector_index) * WISP_RIPPLE;
	float distorted_radial_scroll = base_radial_scroll + radial_ripple;

    // First layer of simplex noise (base noise layer)
	vec2 noise_uv_primary = vec2(sector_index, distorted_radial_scroll);
	float noise_sample_primary = texture(noise_texture, noise_uv_primary).r;

    // Second layer of simplex noise
    // Scale and shift coords at different frequencies to create parallax between noise layers.
	vec2 secondary_noise_modifiers = vec2(1.5, 1.350);
	vec2 noise_uv_secondary = noise_uv_primary * secondary_noise_modifiers;

	float noise_sample_secondary = texture(noise_texture, noise_uv_secondary).r;

    // Intersection of both noise layers, final noise data.
	float wisp_noise_mask = min(noise_sample_primary, noise_sample_secondary);

    // Threshold wisp mask using density parameters (e.g. discard range of noise frequencies)
	wisp_noise_mask = smoothstep(WISP_DENSITY, WISP_DENSITY + WISP_SOFTNESS, wisp_noise_mask);

    // -- RADIAL MASK -- //
    float radial_t = polar_distance_normalized;
	float radial_mask_size = mix(RADIAL_MASK_SIZE_MIN, RADIAL_MASK_SIZE_MAX, u_intensity);
	float radial_mask = smoothstep(radial_mask_size, radial_mask_size + RADIAL_MASK_SOFTNESS, radial_t);
	wisp_noise_mask = wisp_noise_mask * radial_mask;

    // -- ACTIVATION FADE -- //
    // Threshold the visibility of the effect at low boost levels and smoothly fade in at target edge.
	float wisp_activation_fade = smoothstep(0.0, WISP_ACTIVATION, u_intensity);
	wisp_noise_mask = wisp_noise_mask * wisp_activation_fade;

    //-------------//
    // -- COLOR -- //
    //-------------//
    // Inigo Quilez cosine palette technique to generate smooth procedural gradient based on input factor.
	float color_change_offset = u_time * COLOR_CHANGE_SPEED; // Rotate/Offset through color clockwise over time.
	float base_hue_shift = sector_index / (2.0 * sectors) + color_change_offset;
	float color_saturation = mix(COLOR_SATURATION_MIN, COLOR_SATURATION_MAX, u_intensity);

    vec3 spectral_hue = 0.5 + 0.5 * cos(6.28318 * (base_hue_shift + vec3(0.0, 0.333, 0.667)));
    vec3 palette_output_color = mix(vec3(1.0), spectral_hue, color_saturation);

    // -- FINAL OUTPUT -- /
    FragColor = vec4(palette_output_color * wisp_noise_mask, wisp_noise_mask);
}
