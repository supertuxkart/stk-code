uniform int layer;
uniform mat4 ModelMatrix;
uniform vec3 windDir;

#ifdef Explicit_Attrib_Location_Usable
layout(location = 0) in vec3 Position;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;
#else
in vec3 Position;
in vec4 Color;
in vec2 Texcoord;
#endif

#ifdef VSLayer
out vec2 uv;
#else
out vec2 tc;
out int layerId;
#endif

void main(void)
{

    vec3 test = sin(windDir * (Position.y* 0.5)) * 0.5;
    test += cos(windDir) * 0.7;

#ifdef VSLayer
    gl_Layer = layer;
    uv = Texcoord;
    gl_Position = ShadowViewProjMatrixes[gl_Layer] * ModelMatrix * vec4(Position + test * Color.r, 1.);
#else
    layerId = layer;
    tc = Texcoord;
    gl_Position = ShadowViewProjMatrixes[layerId] * ModelMatrix * vec4(Position + test * Color.r, 1.);
#endif
}
