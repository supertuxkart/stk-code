#version 130

out vec2 uv;
out vec2 edger_uv;
out float camdist;

void main() {
	gl_Position = ftransform();
	uv = gl_MultiTexCoord0.xy;
	edger_uv = gl_MultiTexCoord1.xy;

	camdist = length((gl_ModelViewMatrix * gl_Vertex).xyz);
}
