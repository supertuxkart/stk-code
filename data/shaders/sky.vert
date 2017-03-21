layout(location = 0) in vec3 Position;

void main()
{
    gl_Position = vec4(Position, 1.);
}
