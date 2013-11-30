varying vec3 nor;
uniform sampler2D tex;
uniform float far;
uniform int hastex;
uniform float objectid;

varying vec3 eyenor;
varying vec3 viewpos;

const float near = 1.0;

vec4 encdepth(float v) {
	vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * v;
	enc = fract(enc);
	enc -= enc.yzww * vec4(1.0/255.0, 1.0/255.0, 1.0/255.0, 0.0);
	return enc;
}

void main() {

	float linear_z = (2.0 * near) / (far + near - gl_FragCoord.z * (far - near));

	// Tune for better inside range without losing outdoors
	linear_z *= 2.0;

	float rim = 1.0 - dot(eyenor, viewpos);
	rim = smoothstep(0.5, 1.5, rim) * 0.35;

	if (hastex != 0) {
		vec4 col = texture2D(tex, gl_TexCoord[0].xy);

		if (col.a < 0.5)
			discard;

		col.xyz += rim;

		gl_FragData[0] = col;
	} else {
		gl_FragData[0] = gl_Color + vec4(vec3(rim), 0.0);
	}

	gl_FragData[1] = vec4(nor, linear_z);
	gl_FragData[2] = vec4(encdepth(gl_FragCoord.z).xyz, objectid);
}

