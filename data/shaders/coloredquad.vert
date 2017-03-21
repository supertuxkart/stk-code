uniform vec2 center;
uniform vec2 size;

layout(location = 0) in vec2 Position;

void main()
{
    gl_Position = vec4(Position * size + center, 0., 1.);
}
