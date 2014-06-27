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


layout(location = 0) in vec3 Position;
layout(location = 3) in vec2 Texcoord;
layout(location = 5) in vec3 Tangent;
layout(location = 6) in vec3 Bitangent;

out vec3 tangent;
out vec3 bitangent;
out vec2 uv;

void main()
{
	mat4 ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix;
	mat4 TransposeInverseModelView = transpose(InverseModelMatrix * InverseViewMatrix);
	uv = Texcoord;
	tangent = (TransposeInverseModelView * vec4(Tangent, 1.)).xyz;
	bitangent = (TransposeInverseModelView * vec4(Bitangent, 1.)).xyz;
	gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);

}
