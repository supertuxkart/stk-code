#version 130

uniform float far;
uniform float objectid;
uniform sampler2D tex;

noperspective in vec3 nor;

const float near = 1.0;

vec4 encdepth(float v) {
	vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * v;
	enc = fract(enc);
	enc -= enc.yzww * vec4(1.0/255.0, 1.0/255.0, 1.0/255.0, 0.0);
	return enc;
}

void main()
{
	vec4 color = texture2D(tex, gl_TexCoord[0].st);

	float linear_z = (2.0 * near) / (far + near - gl_FragCoord.z * (far - near));
	// Tune for better inside range without losing outdoors
	linear_z *= 2.0;


	gl_FragData[0] = color;	
	gl_FragData[1] = vec4(0.5 * normalize(nor) + 0.5, linear_z);
	gl_FragData[2] = vec4(encdepth(gl_FragCoord.z).xyz, objectid);
}
