#include "camera.glsl"
#include "get_pos_from_frag_coord.glsl"
#include "pbr_light.glsl"
#include "sun_direction.glsl"

vec3 handlePBR(vec3 diffuse_color, vec3 pbr, vec3 normal)
{
    vec3 xpos = getPosFromFragCoord(gl_FragCoord, u_camera.m_viewport,
        u_camera.m_inverse_projection_matrix);
    vec3 eyedir = -normalize(xpos);

    vec3 sun = sunDirection(normal, eyedir, vec3(-642.22, 673.75, -219.26));
    vec3 lightdir = normalize(sun.xyz);

    vec3 mixed_color = PBRSunAmbientEmitLight(
        normal, eyedir, lightdir, diffuse_color,
        vec3(211./256., 235./256., 110./256.),
        vec3(120./256., 120./256., 120./256.),
        1.0 - pbr.x, pbr.y, pbr.z);

    float factor = (1.0 - exp(length(xpos) * -0.0001));
    mixed_color = mixed_color + vec3(0.5) * factor;

    mixed_color = (mixed_color * (6.9 * mixed_color + 0.5)) /
        (mixed_color * (5.2 * mixed_color + 1.7) + 0.06);
    return mixed_color;
}
