#version 130

noperspective out vec3 nor;
noperspective out vec3 eyenor;
noperspective out vec3 viewpos;

void main() {
	nor = gl_NormalMatrix * gl_Normal;
	eyenor = gl_NormalMatrix * gl_Normal;
	viewpos = -normalize((gl_ModelViewMatrix * gl_Vertex).xyz);

	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_FrontColor = gl_Color;
}
