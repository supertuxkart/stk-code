uniform mat4 ModelMatrix;
uniform int skinning_offset;
uniform int layer;
#ifdef GL_ES
uniform sampler2D skinning_tex;
#else
uniform samplerBuffer skinning_tex;
#endif

#ifdef Explicit_Attrib_Location_Usable
layout(location = 0) in vec3 Position;
layout(location = 3) in vec4 Data1;
layout(location = 5) in ivec4 Joint;
layout(location = 6) in vec4 Weight;
#else
in vec3 Position;
in vec4 Data1;
in ivec4 Joint;
in vec4 Weight;
#endif

#ifdef VSLayer
out vec2 uv;
#else
out vec2 tc;
out int layerId;
#endif

void main(void)
{
    vec4 idle_position = vec4(Position, 1.);
    vec4 skinned_position = vec4(0.);

    for (int i = 0; i < 4; i++)
    {
#ifdef GL_ES
        mat4 joint_matrix = mat4(
            texelFetch(skinning_tex, ivec2(0, clamp(Joint[i] + skinning_offset, 0, MAX_BONES)), 0),
            texelFetch(skinning_tex, ivec2(1, clamp(Joint[i] + skinning_offset, 0, MAX_BONES)), 0),
            texelFetch(skinning_tex, ivec2(2, clamp(Joint[i] + skinning_offset, 0, MAX_BONES)), 0),
            texelFetch(skinning_tex, ivec2(3, clamp(Joint[i] + skinning_offset, 0, MAX_BONES)), 0));
#else
        mat4 joint_matrix = mat4(
            texelFetch(skinning_tex, clamp(Joint[i] + skinning_offset, 0, MAX_BONES) * 4),
            texelFetch(skinning_tex, clamp(Joint[i] + skinning_offset, 0, MAX_BONES) * 4 + 1),
            texelFetch(skinning_tex, clamp(Joint[i] + skinning_offset, 0, MAX_BONES) * 4 + 2),
            texelFetch(skinning_tex, clamp(Joint[i] + skinning_offset, 0, MAX_BONES) * 4 + 3));
#endif
            skinned_position += Weight[i] * joint_matrix * idle_position;
    }

#ifdef VSLayer
    gl_Layer = layer;
    uv = Data1.xy;
    gl_Position = ShadowViewProjMatrixes[gl_Layer] * ModelMatrix * skinned_position;
#else
    layerId = layer;
    tc = Data1.xy;
    gl_Position = ShadowViewProjMatrixes[layerId] * ModelMatrix * skinned_position;
#endif
}
