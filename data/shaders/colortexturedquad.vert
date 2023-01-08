uniform vec2 center;
uniform vec2 size;
uniform vec2 texcenter;
uniform vec2 texsize;
uniform float rotation;

#ifdef Explicit_Attrib_Location_Usable
layout(location=0) in vec2 Position;
layout(location=3) in vec2 Texcoord;
layout(location=2) in vec4 Color;
#else
in vec2 Position;
in vec2 Texcoord;
in vec4 Color;
#endif

out vec2 uv;
out vec4 color;

void main()
{
    color = Color.zyxw;
    uv = Texcoord * texsize + texcenter;
    gl_Position = vec4(Position * size + center, 0., 1.);
}
