#version 130
uniform sampler2D tex;
uniform sampler2D lighttex;
uniform int hastex;
uniform int haslightmap;

noperspective in vec3 nor;
in vec4 color;
in vec2 uv0;
in vec2 uv1;

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

	gl_FragData[0] = vec4(col.xyz, 1.);
	gl_FragData[1] = vec4(0.5 * normalize(nor) + 0.5, gl_FragCoord.z);
	gl_FragData[2] = vec4(1. - col.a);
}

