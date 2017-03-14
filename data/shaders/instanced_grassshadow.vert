uniform int layer;

uniform vec3 windDir;
#ifdef Explicit_Attrib_Location_Usable
layout(location = 0) in vec3 Position;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;

layout(location = 7) in vec3 Origin;
layout(location = 8) in vec3 Orientation;
layout(location = 9) in vec3 Scale;
#ifdef Use_Bindless_Texture
layout(location = 11) in uvec2 Handle;
#endif

#else
in vec3 Position;
in vec4 Color;
in vec2 Texcoord;

in vec3 Origin;
in vec3 Orientation;
in vec3 Scale;
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

void main(void)
{

    vec3 test = sin(windDir * (Position.y* 0.5)) * 0.5;
    test += cos(windDir) * 0.7;
    mat4 ModelMatrix = getWorldMatrix(Origin, Orientation, Scale);
#ifdef VSLayer
    gl_Layer = layer;
    gl_Position = ShadowViewProjMatrixes[gl_Layer] * ModelMatrix * vec4(Position + test * Color.r, 1.);
    uv = Texcoord;
#ifdef Use_Bindless_Texture
    handle = Handle;
#endif
#else
    layerId = layer;
    gl_Position = ShadowViewProjMatrixes[layerId] * ModelMatrix * vec4(Position + test * Color.r, 1.);
    tc = Texcoord;
#ifdef Use_Bindless_Texture
    hdle = Handle;
#endif
#endif
}
