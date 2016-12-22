#if __VERSION__ >= 330
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec4 Data1;
layout(location = 4) in vec4 Data2;
layout(location = 5) in ivec4 Joint;
layout(location = 6) in vec4 Weight;

layout(location = 7) in vec3 Origin;
layout(location = 8) in vec3 Orientation;
layout(location = 9) in vec3 Scale;
layout(location = 10) in vec4 misc_data;
#ifdef Use_Bindless_Texture
layout(location = 11) in sampler2D Handle;
layout(location = 12) in sampler2D SecondHandle;
layout(location = 13) in sampler2D ThirdHandle;
layout(location = 14) in sampler2D FourthHandle;
#endif
layout(location = 15) in int skinning_offset;
#else
in vec3 Position;
in vec3 Normal;
in vec4 Color;
in vec4 Data1;
in vec4 Data2;
in ivec4 Joint;
in vec4 Weight;

in vec3 Origin;
in vec3 Orientation;
in vec3 Scale;
in vec4 misc_data;
in int skinning_offset;
#endif

out vec3 nor;
out vec3 tangent;
out vec3 bitangent;
out vec2 uv;
out vec4 color;
out vec2 color_change;
#ifdef Use_Bindless_Texture
flat out sampler2D handle;
flat out sampler2D secondhandle;
flat out sampler2D thirdhandle;
flat out sampler2D fourthhandle;
#endif

#stk_include "utils/getworldmatrix.vert"

void main(void)
{
    mat4 ModelMatrix = getWorldMatrix(Origin, Orientation, Scale);
    mat4 TransposeInverseModelView = transpose(getInverseWorldMatrix(Origin, Orientation, Scale) * InverseViewMatrix);
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

    gl_Position = ProjectionViewMatrix *  ModelMatrix * skinned_position;
    // Keep orthogonality
    nor = (TransposeInverseModelView * skinned_normal).xyz;
    // Keep direction
    tangent = (ViewMatrix * ModelMatrix * skinned_tangent).xyz;
    bitangent = (ViewMatrix * ModelMatrix * skinned_bitangent).xyz;
    uv = vec2(Data1.x + misc_data.x, Data1.y + misc_data.y);
    color = Color.zyxw;
    color_change = misc_data.zw;
#ifdef Use_Bindless_Texture
    handle = Handle;
    secondhandle = SecondHandle;
    thirdhandle = ThirdHandle;
    fourthhandle = FourthHandle;
#endif
}
