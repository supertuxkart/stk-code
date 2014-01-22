#version 130
uniform mat4 ModelViewProjectionMatrix;

in vec3 Position;
in vec4 Color;
out vec4 color;

void main(void)
{
    color = Color;
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
}
