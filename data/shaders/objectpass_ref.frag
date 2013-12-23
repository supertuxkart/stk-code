#version 130
uniform sampler2D tex;
uniform int hastex;
uniform float objectid;

noperspective in vec3 nor;

void main() {

	//if (hastex != 0) {
		vec4 col = texture2D(tex, gl_TexCoord[0].xy);

		if (col.a < 0.5)
			discard;

		gl_FragData[0] = col;
	//} else {
	//	gl_FragData[0] = gl_Color;
	//}

	gl_FragData[1] = vec4(0.5 * normalize(nor) + 0.5, gl_FragCoord.z);
}

