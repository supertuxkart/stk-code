#version 330
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;

in vec3 Position;
in vec2 Texcoord;
in vec3 Tangent;
in vec3 Bitangent;
noperspective out vec3 tangent;
noperspective out vec3 bitangent;
out vec2 uv;

void main()
{
	uv = Texcoord;
	tangent = (TransposeInverseModelView * vec4(Tangent, 1.)).xyz;
	bitangent = (TransposeInverseModelView * vec4(Bitangent, 1.)).xyz;
	gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.);

}
