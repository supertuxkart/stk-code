#ifdef Explicit_Attrib_Location_Usable
layout(location = 0) in vec2 Position;
layout(location = 3) in vec2 Texcoord;
#else
in vec2 Position;
in vec2 Texcoord;
#endif

out vec2 uv;

void main()
{
    uv = Texcoord;
    gl_Position = vec4(Position, 0., 1.);
}
