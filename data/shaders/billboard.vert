uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;
uniform vec3 Position;
uniform vec2 Size;

#if __VERSION__ >= 130
in vec2 Corner;
in vec2 Texcoord;
out vec2 uv;
#else
attribute vec2 Corner;
attribute vec2 Texcoord;
varying vec2 uv;
#endif


void main(void)
{
    uv = Texcoord;
    vec4 Center = ModelViewMatrix * vec4(Position, 1.);
    gl_Position = ProjectionMatrix * (Center + vec4(Size * Corner, 0., 0.));
}
