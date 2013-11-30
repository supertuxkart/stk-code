varying vec3 nor;
uniform mat4 invtworldm;

varying vec3 eyenor;
varying vec3 viewpos;

void main() {

	nor = (invtworldm * vec4(gl_Normal, 0.0)).xyz;
	nor = normalize(nor);
	nor = nor * 0.5 + 0.5;

	eyenor = gl_NormalMatrix * gl_Normal;
	viewpos = -normalize((gl_ModelViewMatrix * gl_Vertex).xyz);

	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = ftransform();
	gl_FrontColor = gl_Color;
}
