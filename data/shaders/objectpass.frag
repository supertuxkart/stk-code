#version 330
uniform sampler2D tex;
uniform sampler2D lighttex;
uniform int hastex;
uniform int haslightmap;

noperspective in vec3 nor;
in vec4 color;
in vec2 uv0;
in vec2 uv1;
out vec4 Albedo;
out vec4 NormalDepth;
out vec4 Specular;

void main() {
	vec4 light = vec4(1.0);
	vec4 col;

	if (haslightmap != 0) {
		light = texture(lighttex, uv1);
	}

	if (hastex != 0)
		col = texture(tex, uv0) * light;
	else
		col = color;

	Albedo = vec4(col.xyz, 1.);
	NormalDepth = vec4(0.5 * normalize(nor) + 0.5, gl_FragCoord.z);
	Specular = vec4(1. - col.a);
}

