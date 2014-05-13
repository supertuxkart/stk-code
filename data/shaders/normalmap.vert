layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
};

uniform mat4 ModelMatrix;
uniform mat4 InverseModelMatrix;

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
	mat4 ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix;
	mat4 TransposeInverseModelView = transpose(InverseModelMatrix * InverseViewMatrix);
	uv = Texcoord;
	tangent = (TransposeInverseModelView * vec4(Tangent, 1.)).xyz;
	bitangent = (TransposeInverseModelView * vec4(Bitangent, 1.)).xyz;
	gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);

}
