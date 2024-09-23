layout(location = 0) in vec4 f_vertex_color;
layout(location = 1) in vec2 f_uv;
layout(location = 3) flat in int f_material_id;
layout(location = 4) in float f_hue_change;
#ifdef PBR_ENABLED
layout(location = 5) in vec3 f_normal;
#endif

layout(location = 0) out vec4 o_color;

#include "utils/camera.glsl"
#include "utils/get_pos_from_frag_coord.glsl"
#include "utils/pbr_light.glsl"
#include "utils/sample_mesh_texture.glsl"
#include "../utils/rgb_conversion.frag"

vec3 SunMRP(vec3 normal, vec3 eyedir, vec3 sundirection)
{
    vec3 local_sundir = normalize((transpose(u_camera.m_inverse_view_matrix) * vec4(sundirection, 0.)).xyz);
    vec3 R = reflect(-eyedir, normal);
    float angularRadius = 3.14 * 5. / 180.;
    vec3 D = local_sundir;
    float d = cos(angularRadius);
    float r = sin(angularRadius);
    float DdotR = dot(D, R);
    vec3 S = R - DdotR * D;
    return (DdotR < d) ? normalize(d * D + normalize (S) * r) : R;
}

void main()
{
    vec4 tex_color = sampleMeshTexture0(f_material_id, f_uv);
    if (tex_color.a * f_vertex_color.a < 0.5)
        discard;

    if (f_hue_change > 0.0)
    {
        vec3 old_hsv = rgbToHsv(tex_color.rgb);
        vec2 new_xy = vec2(f_hue_change, old_hsv.y);
        vec3 new_color = hsvToRgb(vec3(new_xy.x, new_xy.y, old_hsv.z));
        tex_color = vec4(new_color.r, new_color.g, new_color.b, tex_color.a);
    }
#ifndef PBR_ENABLED
    vec3 mixed_color = tex_color.xyz * f_vertex_color.xyz;
    o_color = vec4(mixed_color, 1.0);
#else
    vec3 diffuse_color = tex_color.xyz * f_vertex_color.xyz;
    vec3 normal = normalize(f_normal.xyz);

    vec4 pbr = sampleMeshTexture2(f_material_id, f_uv);
    vec3 xpos = getPosFromFragCoord(gl_FragCoord, u_camera.m_viewport, u_camera.m_inverse_projection_matrix);
    vec3 eyedir = -normalize(xpos);

    vec3 sun = SunMRP(normal, eyedir, vec3(-642.22, 673.75, -219.26));
    vec3 lightdir = normalize(sun.xyz);

    vec3 mixed_color = PBRSunAmbientEmitLight(
        normal, eyedir, lightdir, diffuse_color,
        vec3(211./256., 235./256., 110./256.),
        vec3(120./256., 120./256., 120./256.),
        1.0 - pbr.x, pbr.y, pbr.z);

    float factor = (1.0 - exp(length(xpos) * -0.0001));
    mixed_color = mixed_color + vec3(0.5) * factor;

    mixed_color = (mixed_color * (6.9 * mixed_color + 0.5)) / (mixed_color * (5.2 * mixed_color + 1.7) + 0.06);

    o_color = vec4(mixed_color, 1.0);
#endif
}
