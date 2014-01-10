#version 130
uniform sampler2D tex;
uniform int hastex;
uniform float objectid;

noperspective in vec3 nor;
in vec2 uv0;
in vec2 uv1;

void main() {

	//if (hastex != 0) {
		vec4 col = texture2D(tex, uv0);

		if (col.a < 0.5)
			discard;

		gl_FragData[0] = vec4(col.xyz, 1.);
	//} else {
	//	gl_FragData[0] = gl_Color;
	//}

	gl_FragData[1] = vec4(0.5 * normalize(nor) + 0.5, gl_FragCoord.z);
	gl_FragData[2] = vec4(1. - col.a);
}

