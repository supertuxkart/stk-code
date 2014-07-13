layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
};

uniform mat4 ModelMatrix;
uniform vec3 windDir;

layout(location = 0) in vec3 Position;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;

#ifdef VSLayer
out vec2 uv;
#else
out vec2 tc;
out int layerId;
#endif

void main(void)
{
#ifdef VSLayer
    gl_Layer = gl_InstanceID & 3;
    uv = Texcoord;
    gl_Position = ShadowViewProjMatrixes[gl_Layer] * ModelMatrix * vec4(Position + windDir * Color.r, 1.);
#else
    layerId = gl_InstanceID & 3;
    tc = Texcoord;
    gl_Position = ShadowViewProjMatrixes[layerId] * ModelMatrix * vec4(Position + windDir * Color.r, 1.);
#endif
}
