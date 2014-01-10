#version 130
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;
uniform mat4 TextureMatrix0;
uniform mat4 TextureMatrix1;

noperspective out vec3 nor;
out vec4 color;
out vec2 uv0;
out vec2 uv1;

void main() {

	nor = (TransposeInverseModelView * vec4(gl_Normal, 1.)).xyz;
	uv0 = (gl_TextureMatrix[0] * gl_MultiTexCoord0).st;
	uv1 = (gl_TextureMatrix[1] * gl_MultiTexCoord1).st;
	gl_Position = ModelViewProjectionMatrix * gl_Vertex;
	color = gl_Color;
}
