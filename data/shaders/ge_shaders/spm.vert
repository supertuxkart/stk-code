#include "utils/spm_layout.h"
#include "utils/get_vertex_color.h"
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
}
