#version 130
uniform sampler2D tex;

noperspective in vec3 nor;

void main() {
	// Calculate the spherical UV
	const vec3 forward = vec3(0.0, 0.0, 1.0);

	// get the angle between the forward vector and the horizontal portion of the normal
	vec3 normal_x = normalize(vec3(nor.x, 0.0, nor.z));
	float sin_theta_x = length(cross( forward, normal_x )) * nor.x / abs(nor.x);

	// get the angle between the forward vector and the vertical portion of the normal
	vec3 normal_y = normalize(vec3(0.0, nor.y, nor.z));
	float sin_theta_y = length(cross( forward, normal_y )) * nor.y / abs(nor.y);

	vec4 detail0 = texture(tex, 0.5 * vec2(sin_theta_x, sin_theta_y) + 0.5);

	gl_FragColor = vec4(detail0.xyz, 1.);
}
