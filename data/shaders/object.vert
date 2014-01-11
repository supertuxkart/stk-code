#version 130
uniform mat4 ModelViewProjectionMatrix;

in vec3 Position;

void main(void)
{
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
}
