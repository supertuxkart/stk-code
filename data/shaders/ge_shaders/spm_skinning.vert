#include "utils/camera.glsl"
#include "utils/get_vertex_color.glsl"
#include "utils/spm_data.glsl"
#include "utils/spm_layout.vert"
#include "../utils/get_world_location.vert"

void main()
{
    int offset = u_object_buffer.m_objects[gl_InstanceIndex].m_skinning_offset;
    mat4 joint_matrix =
        v_weight[0] * u_skinning_matrices.m_mat[max(v_joint[0] + offset, 0)] +
        v_weight[1] * u_skinning_matrices.m_mat[max(v_joint[1] + offset, 0)] +
        v_weight[2] * u_skinning_matrices.m_mat[max(v_joint[2] + offset, 0)] +
        v_weight[3] * u_skinning_matrices.m_mat[max(v_joint[3] + offset, 0)];
    vec4 v_skinning_position = joint_matrix * vec4(v_position, 1.0);
    vec4 v_world_position = getWorldPosition(
        u_object_buffer.m_objects[gl_InstanceIndex].m_translation,
        u_object_buffer.m_objects[gl_InstanceIndex].m_rotation,
        u_object_buffer.m_objects[gl_InstanceIndex].m_scale,
        v_skinning_position.xyz);
    f_world_position = v_world_position;
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
    vec4 skinned_normal = joint_matrix * v_normal;
    vec4 skinned_tangent = joint_matrix * vec4(v_tangent.xyz, 0.0);
    vec3 world_normal = rotateVector(u_object_buffer.m_objects[gl_InstanceIndex].m_rotation, skinned_normal.xyz);
    vec3 world_tangent = rotateVector(u_object_buffer.m_objects[gl_InstanceIndex].m_rotation, skinned_tangent.xyz);
    f_bitangent = cross(world_normal, world_tangent) * v_tangent.w;
    f_tangent = world_tangent;
    f_normal = world_normal;
#endif
}
