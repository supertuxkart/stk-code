uniform int layer;
uniform mat4 ModelMatrix;

layout(location = 0) in vec3 Position;
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
    gl_Layer = layer;
    uv = Texcoord;
    gl_Position = ShadowViewProjMatrixes[gl_Layer] * ModelMatrix * vec4(Position, 1.);
#else
    layerId = layer;
    tc = Texcoord;
    gl_Position = ShadowViewProjMatrixes[layerId] * ModelMatrix * vec4(Position, 1.);
#endif
}
