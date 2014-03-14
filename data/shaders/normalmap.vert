uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;


#if __VERSION__ >= 130
in vec3 Position;
in vec2 Texcoord;
in vec3 Tangent;
in vec3 Bitangent;
out vec3 tangent;
out vec3 bitangent;
out vec2 uv;
#else
attribute vec3 Position;
attribute vec2 Texcoord;
attribute vec3 Tangent;
attribute vec3 Bitangent;
varying vec3 tangent;
varying vec3 bitangent;
varying vec2 uv;
#endif


void main()
{
	uv = Texcoord;
	tangent = (TransposeInverseModelView * vec4(Tangent, 1.)).xyz;
	bitangent = (TransposeInverseModelView * vec4(Bitangent, 1.)).xyz;
	gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);

}
