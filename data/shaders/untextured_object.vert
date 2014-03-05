uniform mat4 ModelViewProjectionMatrix;

#if __VERSION__ >= 130
in vec3 Position;
in vec4 Color;
out vec4 color;
#else
attribute vec3 Position;
attribute vec4 Color;
varying vec4 color;
#endif


void main(void)
{
    color = Color.zyxw;
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
}
