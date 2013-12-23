#version 130
uniform sampler2D tex;

noperspective in vec3 eyenor;
noperspective in vec3 viewpos;
noperspective in vec3 nor;

void main() {
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

	gl_FragData[1] = vec4(nor, gl_FragCoord.z);
}
