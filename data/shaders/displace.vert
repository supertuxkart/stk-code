#version 130
uniform mat4 ModelViewMatrix;
uniform mat4 ProjectionMatrix;

out vec2 uv;
out vec2 edger_uv;
out float camdist;

void main() {
	vec4 position = ModelViewMatrix * gl_Vertex;
	gl_Position = ProjectionMatrix * position;
	uv = gl_MultiTexCoord0.xy;
	edger_uv = gl_MultiTexCoord1.xy;

	camdist = length(position.xyz);
}
