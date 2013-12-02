varying vec3 nor;
uniform sampler2D tex;
uniform float far;
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

	// Calculate the spherical UV
	const vec3 forward = vec3(0.0, 0.0, 1.0);

	// get the angle between the forward vector and the horizontal portion of the normal
	vec3 normal_x = normalize(vec3(eyenor.x, 0.0, eyenor.z));
	float sin_theta_x = length(cross( forward, normal_x )) * eyenor.x/abs(eyenor.x);

	// get the angle between the forward vector and the vertical portion of the normal
	vec3 normal_y = normalize(vec3(0.0, eyenor.y, eyenor.z));
	float sin_theta_y = length(cross( forward, normal_y )) * eyenor.y/abs(eyenor.y);

	vec4 detail0 = texture2D(tex, vec2(0.5 + sin_theta_x*0.5, 0.5 + sin_theta_y*0.5));

	gl_FragData[0] = detail0 * gl_Color;

	gl_FragData[1] = vec4(nor, linear_z);
	gl_FragData[2] = vec4(encdepth(gl_FragCoord.z).xyz, objectid);
}
