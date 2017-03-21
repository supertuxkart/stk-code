uniform vec2 center;
uniform vec2 size;
uniform vec2 texcenter;
uniform vec2 texsize;

layout(location=0) in vec2 Position;
layout(location=3) in vec2 Texcoord;
layout(location=2) in uvec4 Color;

out vec2 uv;
out vec4 col;

void main()
{
    col = vec4(Color) / 255.;
    uv = Texcoord * texsize + texcenter;
    gl_Position = vec4(Position * size + center, 0., 1.);
}
