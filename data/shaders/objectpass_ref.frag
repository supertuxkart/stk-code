#version 330
uniform sampler2D tex;
uniform int hastex;
uniform float objectid;

noperspective in vec3 nor;
in vec2 uv0;
in vec2 uv1;
out vec4 Albedo;
out vec4 NormalDepth;
out vec4 Specular;

void main() {

	//if (hastex != 0) {
		vec4 col = texture(tex, uv0);

		if (col.a < 0.5)
			discard;

		Albedo = vec4(col.xyz, 1.);
	//} else {
	//	Albedo = gl_Color;
	//}

	NormalDepth = vec4(0.5 * normalize(nor) + 0.5, gl_FragCoord.z);
	Specular = vec4(1. - col.a);
}

