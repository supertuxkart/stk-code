uniform int layer;

#ifdef Explicit_Attrib_Location_Usable
layout(location = 0) in vec3 Position;
layout(location = 3) in vec4 Data1;
layout(location = 5) in ivec4 Joint;
layout(location = 6) in vec4 Weight;
layout(location = 7) in vec3 Origin;
layout(location = 8) in vec3 Orientation;
layout(location = 9) in vec3 Scale;
#ifdef Use_Bindless_Texture
layout(location = 11) in uvec2 Handle;
#endif
layout(location = 15) in int skinning_offset;
#else
in vec3 Position;
in vec4 Data1;
in ivec4 Joint;
in vec4 Weight;
in vec3 Origin;
in vec3 Orientation;
in vec3 Scale;
in int skinning_offset;
#endif

#ifdef VSLayer
out vec2 uv;
#ifdef Use_Bindless_Texture
flat out uvec2 handle;
#endif
#else
out vec2 tc;
out int layerId;
#ifdef Use_Bindless_Texture
flat out uvec2 hdle;
#endif
#endif

#stk_include "utils/getworldmatrix.vert"

uniform samplerBuffer skinning_tex;

void main(void)
{
    mat4 ModelMatrix = getWorldMatrix(Origin, Orientation, Scale);
    vec4 idle_position = vec4(Position, 1.);
    vec4 skinned_position = vec4(0.);
    for (int i = 0; i < 4; i++)
    {
        mat4 joint_matrix = mat4(
            texelFetch(skinning_tex, clamp(Joint[i] + skinning_offset, 0, MAX_BONES) * 4),
            texelFetch(skinning_tex, clamp(Joint[i] + skinning_offset, 0, MAX_BONES) * 4 + 1),
            texelFetch(skinning_tex, clamp(Joint[i] + skinning_offset, 0, MAX_BONES) * 4 + 2),
            texelFetch(skinning_tex, clamp(Joint[i] + skinning_offset, 0, MAX_BONES) * 4 + 3));
        skinned_position += Weight[i] * joint_matrix * idle_position;
    }

#ifdef VSLayer
    gl_Layer = layer;
    gl_Position = ShadowViewProjMatrixes[gl_Layer] * ModelMatrix * skinned_position;
    uv = Data1.xy;
#ifdef Use_Bindless_Texture
    handle = Handle;
#endif
#else
    layerId = layer;
    gl_Position = ShadowViewProjMatrixes[layerId] * ModelMatrix * skinned_position;
    tc = Data1.xy;
#ifdef Use_Bindless_Texture
    hdle = Handle;
#endif
#endif
}
