uniform mat4 ModelMatrix;
uniform mat4 InverseModelMatrix;

#if __VERSION__ >= 330
layout(location = 0) in vec3 Position;
layout(location = 3) in vec2 Texcoord;
layout(location = 5) in vec3 Tangent;
layout(location = 6) in vec3 Bitangent;
#else
in vec3 Position;
in vec2 Texcoord;
in vec3 Tangent;
in vec3 Bitangent;
#endif

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
