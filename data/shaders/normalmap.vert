#version 130
noperspective out vec3 tangent;
noperspective out vec3 bitangent;
noperspective out vec3 normal;

void main()
{
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	normal = gl_NormalMatrix * gl_Normal;
	tangent = gl_NormalMatrix * gl_MultiTexCoord1.xyz;
	bitangent = gl_NormalMatrix * gl_MultiTexCoord2.xyz;
	gl_Position = ftransform();

}
