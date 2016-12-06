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
layout(location = 15) in uint skinning_offset;

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
    vec4 skinned_position = vec4(0.);
    vec4 skinned_normal = vec4(0.);
    // Note : For normal we assume no scale factor in bone (otherwise we'll have to compute inversematrix for each bones...)
    vec4 single_bone_influenced_position;
    vec4 single_bone_influenced_normal;
    if (Joint[0] >= 0)
    {
        single_bone_influenced_position = joint_matrices[Joint[0] + skinning_offset] * idle_position;
        single_bone_influenced_position /= single_bone_influenced_position.w;
        single_bone_influenced_normal = joint_matrices[Joint[0] + skinning_offset] * idle_normal;
    }
    else
    {
        single_bone_influenced_position = idle_position;
        single_bone_influenced_normal = idle_normal;
    }
    skinned_position += Weight[0] * single_bone_influenced_position;
    skinned_normal += Weight[0] * single_bone_influenced_normal;

    if (Joint[1] >= 0)
    {
        single_bone_influenced_position= joint_matrices[Joint[1] + skinning_offset] * idle_position;
        single_bone_influenced_position /= single_bone_influenced_position.w;
        single_bone_influenced_normal = joint_matrices[Joint[1] + skinning_offset] * idle_normal;
    }
    else
    {
        single_bone_influenced_position = idle_position;
        single_bone_influenced_normal = idle_normal;
    }
    skinned_position += Weight[1] * single_bone_influenced_position;
    skinned_normal += Weight[1] * single_bone_influenced_normal;

    if (Joint[2] >= 0)
    {
        single_bone_influenced_position = joint_matrices[Joint[2] + skinning_offset] * idle_position;
        single_bone_influenced_position /= single_bone_influenced_position.w;
        single_bone_influenced_normal = joint_matrices[Joint[2] + skinning_offset] * idle_normal;
    }
    else
    {
        single_bone_influenced_position = idle_position;
        single_bone_influenced_normal = idle_normal;
    }
    skinned_position += Weight[2] * single_bone_influenced_position;
    skinned_normal += Weight[2] * single_bone_influenced_normal;

    if (Joint[3] >= 0)
    {
        single_bone_influenced_position = joint_matrices[Joint[3] + skinning_offset] * idle_position;
        single_bone_influenced_position /= single_bone_influenced_position.w;
        single_bone_influenced_normal = joint_matrices[Joint[3] + skinning_offset] * idle_normal;
    }
    else
    {
        single_bone_influenced_position = idle_position;
        single_bone_influenced_normal = idle_normal;
    }
    skinned_position += Weight[3] * single_bone_influenced_position;
    skinned_normal += Weight[3] * single_bone_influenced_normal;

    gl_Position = ProjectionViewMatrix *  ModelMatrix * skinned_position;
    // Keep orthogonality
    nor = (TransposeInverseModelView * skinned_normal).xyz;
    // Keep direction
    tangent = (ViewMatrix * ModelMatrix * vec4(Data1.z, Data1.w, Data2.x, 0.)).xyz;
    bitangent = (ViewMatrix * ModelMatrix * vec4(Data2.y, Data2.z, Data2.w, 0.)).xyz;

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
