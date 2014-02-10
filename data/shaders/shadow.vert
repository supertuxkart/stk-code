#version 330 core

in vec3 Position;
in vec2 Texcoord;


out vec2 tc;


void main(void)
{
    tc = Texcoord;
    gl_Position = vec4(Position, 1.);
}
