#version 130
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;

noperspective out vec3 tangent;
noperspective out vec3 bitangent;
noperspective out vec3 normal;
out vec2 uv;

void main()
{
	uv = gl_MultiTexCoord0.st;
	normal = (TransposeInverseModelView * vec4(gl_Normal, 1.)).xyz;
	tangent = (TransposeInverseModelView * gl_MultiTexCoord1).xyz;
	bitangent = (TransposeInverseModelView * gl_MultiTexCoord2).xyz;
	gl_Position = ModelViewProjectionMatrix * gl_Vertex;

}
