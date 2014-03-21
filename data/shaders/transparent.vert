uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TextureMatrix;

#if __VERSION__ >= 130
in vec3 Position;
in vec2 Texcoord;
in vec4 Color;
out vec2 uv;
out vec4 color;
#else
attribute vec3 Position;
attribute vec2 Texcoord;
varying vec2 uv;
#endif


void main()
{
    uv = (TextureMatrix * vec4(Texcoord, 1., 1.)).xy;
    gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);
	color = Color;
}
