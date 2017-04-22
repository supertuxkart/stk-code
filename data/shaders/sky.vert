#ifdef Explicit_Attrib_Location_Usable
layout(location = 0) in vec3 Position;
#else
in vec3 Position;
#endif

void main()
{
    gl_Position = vec4(Position, 1.);
}
