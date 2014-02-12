#version 330
uniform float time;
uniform vec3 campos;
uniform mat4 viewm;

in vec3 initialPosition;
out vec3 currentPosition;

void main()
{
	// This simulation will run accurately for a bit under five days.
	vec4 start = vec4(initialPosition, 1.0);
	start.y -= time;

	// How many times has it fell?
	float count = floor(start.y / 24.0);
	start.x += sin(count);
	start.z += cos(count);

	vec2 signs = sign(start.xz);
	start.xz = mod(start.xz, 17.5) * signs;

	start.y = mod(start.y, 24.0) - 3.0;

	start.xyz += campos;

	currentPosition = (viewm * start).xyz;
	gl_Position = vec4(0.);
}
