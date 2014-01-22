#version 130
uniform sampler2D tex;
uniform int hastex;
uniform float objectid;

noperspective in vec3 nor;
noperspective in vec3 eyenor;
noperspective in vec3 viewpos;
out vec4 Albedo;
out vec4 NormalDepth;
out vec4 Specular;

void main() {
	float rim = 1.0 - dot(eyenor, viewpos);
	rim = smoothstep(0.5, 1.5, rim) * 0.35;
	vec4 color;

	if (hastex != 0) {
		vec4 col = texture(tex, gl_TexCoord[0].xy);

		if (col.a < 0.1)
			discard;

		col.xyz += rim;

		color = col;
	} else {
		color = gl_Color + vec4(vec3(rim), 0.0);
	}

	Albedo = vec4(color.xyz, 1.);
	NormalDepth = vec4(0.5 * normalize(nor) + 0.5, gl_FragCoord.z);
	Specular = vec4(1. - color.a);
}

