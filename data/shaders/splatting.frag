#version 130
uniform float far;
uniform float objectid;

uniform sampler2D tex_layout;
uniform sampler2D tex_detail0;
uniform sampler2D tex_detail1;
uniform sampler2D tex_detail2;
uniform sampler2D tex_detail3;
//uniform sampler2D tex_detail4;

noperspective in vec3 nor;

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

	// Splatting part
	vec4 splatting = texture2D(tex_layout, gl_TexCoord[1].st);
	vec4 detail0 = texture2D(tex_detail0, gl_TexCoord[0].st);
	vec4 detail1 = texture2D(tex_detail1, gl_TexCoord[0].st);
	vec4 detail2 = texture2D(tex_detail2, gl_TexCoord[0].st);
	vec4 detail3 = texture2D(tex_detail3, gl_TexCoord[0].st);
//	vec4 detail4 = texture2D(tex_detail4, gl_TexCoord[0].st);
	vec4 detail4 = vec4(0.0);

	vec4 splatted = (splatting.r * detail0 +
			splatting.g * detail1 +
			splatting.b * detail2 +
			(1.0 - splatting.r - splatting.g - splatting.b) * detail3 +
			(1.0 - splatting.a) * detail4)
			* gl_Color;

	gl_FragData[0] = splatted;

	gl_FragData[1] = vec4(normalize(nor) * 0.5 + 0.5, linear_z);
	gl_FragData[2] = vec4(encdepth(gl_FragCoord.z).xyz, objectid);
}
