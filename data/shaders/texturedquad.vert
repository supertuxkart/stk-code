uniform vec2 center;
uniform vec2 size;
uniform vec2 texcenter;
uniform vec2 texsize;

layout(location=0) in vec2 Position;
layout(location=3) in vec2 Texcoord;

out vec2 uv;

void main()
{
    uv = Texcoord * texsize + texcenter;
    gl_Position = vec4(Position * size + center, 0., 1.);
}
