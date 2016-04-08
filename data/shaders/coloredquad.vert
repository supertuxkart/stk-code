uniform vec2 center;
uniform vec2 size;

#if __VERSION__ >= 330
layout(location = 0) in vec2 Position;
#else
in vec2 Position;
#endif


void main()
{
    gl_Position = vec4(Position * size + center, 0., 1.);
}