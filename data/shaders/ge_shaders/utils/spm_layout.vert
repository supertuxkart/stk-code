layout(location = 0) in vec3 v_position;
layout(location = 1) in vec4 v_normal;
layout(location = 2) in vec4 v_color;
layout(location = 3) in vec2 v_uv;
layout(location = 4) in vec2 v_uv_two;
layout(location = 5) in vec4 v_tangent;
layout(location = 6) in ivec4 v_joint;
layout(location = 7) in vec4 v_weight;

layout(location = 0) out vec4 f_vertex_color;
layout(location = 1) out vec2 f_uv;
layout(location = 2) out vec2 f_uv_two;
layout(location = 3) flat out int f_material_id;
layout(location = 4) out float f_hue_change;
layout(location = 5) out vec3 f_normal;
layout(location = 6) out vec3 f_tangent;
layout(location = 7) out vec3 f_bitangent;
layout(location = 8) out vec4 f_world_position;
