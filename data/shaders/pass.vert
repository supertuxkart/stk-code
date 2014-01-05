#version 130

void main() {
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_TextureMatrix[1] * gl_MultiTexCoord1;
	gl_Position = ftransform();
	gl_FrontColor = gl_Color;
}
