#version 130
uniform mat4 matrix;
in vec3 position;

void main(void)
{
    gl_Position = matrix * vec4(position, 1.0);
    gl_PointSize = 300. / gl_Position.w;
}
