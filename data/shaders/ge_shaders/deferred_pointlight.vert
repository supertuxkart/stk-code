#include "utils/camera.glsl"
#include "utils/global_light_data.glsl"
#include "utils/spm_data.glsl"
#include "../utils/get_world_location.vert"

layout(push_constant) uniform Constants
{
    vec4 m_billboard_rotation;
    int m_fullscreen_light;
} u_push_constants;

layout(location = 0) flat out int light_idx;

const vec3 g_vertices[4] =
    vec3[]
    (
        vec3( 1.0,  1.0, 0.0),
        vec3( 1.0, -1.0, 0.0),
        vec3(-1.0,  1.0, 0.0),
        vec3(-1.0, -1.0, 0.0)
    );

void main()
{
    // Get the light index from the instance ID
    light_idx = gl_InstanceIndex + u_push_constants.m_fullscreen_light;
    LightData light = u_global_light.m_lights[light_idx];
    vec4 pos_radius = light.m_position_radius;

    // Get camera position from inverse view matrix
    vec3 camera_pos = vec3(u_camera.m_inverse_view_matrix[3]);

    // Calculate vector from light to camera
    vec3 light_to_camera = normalize(camera_pos - pos_radius.xyz);

    /* The lights which cover the whole screen have been rendered already
    // Calculate distance from light to camera
    float dist_to_camera = distance(camera_pos, pos_radius.xyz);

    // If camera is within light radius, move the billboard quad towards the
    // near plane
    if (dist_to_camera < pos_radius.w)
    {
        gl_Position = vec4(g_vertices[gl_VertexIndex], 1.0);
        return;
    }
    */

    // Move the billboard towards camera by one radius unit
    vec4 world_pos = getWorldPosition(
        pos_radius.xyz + light_to_camera * pos_radius.w,
        u_push_constants.m_billboard_rotation,
        vec3(pos_radius.w), g_vertices[gl_VertexIndex]);
    vec4 pv = u_camera.m_projection_view_matrix * world_pos;
    if (pv.z < 0.0)
        gl_Position = vec4(pv.xy, 0.0, 1.0);
    else
        gl_Position = pv;
}
