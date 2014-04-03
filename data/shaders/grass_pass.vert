uniform vec3 windDir;
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;


#if __VERSION__ >= 130
in vec3 Position;
in vec3 Normal;
in vec2 Texcoord;
in vec4 Color;
out vec3 nor;
out vec2 uv;
#else
attribute vec3 Position;
attribute vec3 Normal;
attribute vec2 Texcoord;
attribute vec4 Color;
varying vec3 nor;
varying vec2 uv;
#endif


void main()
{
	uv = Texcoord;
	nor = (TransposeInverseModelView * vec4(Normal, 1.)).xyz;
	gl_Position = ModelViewProjectionMatrix * vec4(Position + windDir * Color.r, 1.);
}
