#include "utils/spm_layout.h"

void main()
{
    int offset = u_object_buffer.m_objects[gl_InstanceIndex].m_skinning_offest;
    mat4 joint_matrix =
        v_weight[0] * u_skinning_matrices.m_mat[max(v_joint[0] + offset, 0)] +
        v_weight[1] * u_skinning_matrices.m_mat[max(v_joint[1] + offset, 0)] +
        v_weight[2] * u_skinning_matrices.m_mat[max(v_joint[2] + offset, 0)] +
        v_weight[3] * u_skinning_matrices.m_mat[max(v_joint[3] + offset, 0)];
    mat4 model_matrix = u_object_buffer.m_objects[gl_InstanceIndex].m_model;
    gl_Position = u_camera.m_projection_view_matrix * model_matrix *
        joint_matrix * vec4(v_position, 1.0);
    f_vertex_color = v_color.zyxw;
    f_uv = v_uv;
    f_material_id = u_object_buffer.m_objects[gl_InstanceIndex].m_material_id;
}
