#include "utils/camera.glsl"
#include "utils/get_vertex_color.glsl"
#include "utils/spm_data.glsl"
#include "utils/spm_layout.vert"
#include "../utils/get_world_location.vert"

void main()
{
    vec4 v_world_position = getWorldPosition(
        u_object_buffer.m_objects[gl_InstanceIndex].m_translation,
        u_object_buffer.m_objects[gl_InstanceIndex].m_rotation,
        u_object_buffer.m_objects[gl_InstanceIndex].m_scale, v_position);
    gl_Position = u_camera.m_projection_view_matrix * v_world_position;
    f_vertex_color = v_color.zyxw * getVertexColor(
        u_object_buffer.m_objects[gl_InstanceIndex].m_custom_vertex_color);
    f_uv = v_uv + u_object_buffer.m_objects[gl_InstanceIndex].m_texture_trans;
    f_uv_two = v_uv_two;
    f_material_id = u_object_buffer.m_objects[gl_InstanceIndex].m_material_id;
#ifdef BIND_MESH_TEXTURES_AT_ONCE
    if (f_material_id < 0)
        f_material_id = u_material_ids.m_material_id[gl_DrawIDARB];
#endif
    f_hue_change = u_object_buffer.m_objects[gl_InstanceIndex].m_hue_change;
#ifdef PBR_ENABLED
    vec3 world_normal = rotateVector(u_object_buffer.m_objects[gl_InstanceIndex].m_rotation, v_normal.xyz);
    vec3 world_tangent = rotateVector(u_object_buffer.m_objects[gl_InstanceIndex].m_rotation, v_tangent.xyz);

    f_tangent = (u_camera.m_view_matrix * vec4(world_tangent, 0.0)).xyz;
    f_bitangent = normalize((u_camera.m_view_matrix *
      vec4(cross(world_normal, world_tangent) * v_tangent.w, 0.0)).xyz);
    f_normal = (u_camera.m_view_matrix * vec4(world_normal, 0.0)).xyz;
#endif
}
