#ifdef GL_ES
uniform mat4 ModelMatrix;
uniform mat4 InverseModelMatrix;
uniform vec2 texture_trans;
#else
uniform mat4 ModelMatrix =
    mat4(1., 0., 0., 0.,
         0., 1., 0., 0.,
         0., 0., 1., 0.,
         0., 0., 0., 1.);
uniform mat4 InverseModelMatrix =
    mat4(1., 0., 0., 0.,
         0., 1., 0., 0.,
         0., 0., 1., 0.,
         0., 0., 0., 1.);

uniform vec2 texture_trans = vec2(0., 0.);
#endif
uniform int skinning_offset;

#ifdef Explicit_Attrib_Location_Usable
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec4 Data1;
layout(location = 4) in vec4 Data2;
layout(location = 5) in ivec4 Joint;
layout(location = 6) in vec4 Weight;
#else
in vec3 Position;
in vec3 Normal;
in vec4 Color;
in vec4 Data1;
in vec4 Data2;
in ivec4 Joint;
in vec4 Weight;
#endif

out vec3 nor;
out vec3 tangent;
out vec3 bitangent;
out vec2 uv;
out vec4 color;

#stk_include "utils/getworldmatrix.vert"

void main(void)
{
    mat4 TransposeInverseModelView =  transpose(InverseModelMatrix * InverseViewMatrix);
    vec4 idle_position = vec4(Position, 1.);
    vec4 idle_normal = vec4(Normal, 0.);
    vec4 idle_tangent = vec4(Data1.z, Data1.w, Data2.x, 0.);
    vec4 idle_bitangent = vec4(Data2.y, Data2.z, Data2.w, 0.);
    vec4 skinned_position = vec4(0.);
    vec4 skinned_normal = vec4(0.);
    vec4 skinned_tangent = vec4(0.);
    vec4 skinned_bitangent = vec4(0.);
    // Note : For normal we assume no scale factor in bone (otherwise we'll have to compute inversematrix for each bones...)
    for (int i = 0; i < 4; i++)
    {
        vec4 single_bone_influenced_position = joint_matrices[clamp(Joint[i] + skinning_offset, 0, MAX_BONES)] * idle_position;
        single_bone_influenced_position /= single_bone_influenced_position.w;
        vec4 single_bone_influenced_normal = joint_matrices[clamp(Joint[i] + skinning_offset, 0, MAX_BONES)] * idle_normal;
        vec4 single_bone_influenced_tangent = joint_matrices[clamp(Joint[i] + skinning_offset, 0, MAX_BONES)] * idle_tangent;
        vec4 single_bone_influenced_bitangent = joint_matrices[clamp(Joint[i] + skinning_offset, 0, MAX_BONES)] * idle_bitangent;
        skinned_position += Weight[i] * single_bone_influenced_position;
        skinned_normal += Weight[i] * single_bone_influenced_normal;
        skinned_tangent += Weight[i] * single_bone_influenced_tangent;
        skinned_bitangent += Weight[i] * single_bone_influenced_bitangent;
    }

    gl_Position = ProjectionViewMatrix * ModelMatrix * skinned_position;
    // Keep orthogonality
    nor = (TransposeInverseModelView * skinned_normal).xyz;
    // Keep direction
    tangent = (ViewMatrix * ModelMatrix * skinned_tangent).xyz;
    bitangent = (ViewMatrix * ModelMatrix * skinned_bitangent).xyz;
    uv = vec2(Data1.x + texture_trans.x, Data1.y + texture_trans.y);
    color = Color.zyxw;
}
