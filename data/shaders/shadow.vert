uniform mat4 ModelMatrix;

in vec3 Position;
in vec2 Texcoord;

out vec2 tc;

void main(void)
{
    tc = Texcoord;
    gl_Position = ModelMatrix * vec4(Position, 1.);
}
