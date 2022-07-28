#include "utils/spm_layout.h"

void main()
{
    mat4 model_matrix = u_object_buffer.m_objects[gl_InstanceIndex].m_model;
    gl_Position = u_camera.m_projection_view_matrix * model_matrix *
        vec4(v_position, 1.0);
    f_vertex_color = v_color.zyxw;
    f_uv = v_uv;
    f_uv_two = v_uv_two;
    f_material_id = u_object_buffer.m_objects[gl_InstanceIndex].m_material_id;
}
