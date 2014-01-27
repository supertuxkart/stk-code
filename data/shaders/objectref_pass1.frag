#version 130
uniform sampler2D tex;

noperspective in vec3 nor;
in vec2 uv;
out vec3 Normal;

void main() {
	vec4 col = texture(tex, uv);
	if (col.a < 0.5)
		discard;
	Normal = 0.5 * normalize(nor) + 0.5;
}

