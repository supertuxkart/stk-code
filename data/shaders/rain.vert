uniform float screenw;
uniform float time;
uniform mat4 viewm;
uniform vec3 campos;

void main()
{
	const float size = 0.5;

	// This simulation will run accurately for a bit under five days.
	vec4 start = gl_Vertex;
	start.y -= time;

	// How many times has it fell?
	float count = floor(start.y / 24.0);
	start.x += sin(count);
	start.z += cos(count);

	vec2 signs = sign(start.xz);
	start.xz = mod(start.xz, 17.5) * signs;

	start.y = mod(start.y, 24.0) - 3.0;

	start.xyz += campos;

	vec4 eyepos = viewm * start;
	vec4 projCorner = gl_ProjectionMatrix * vec4(vec2(size), eyepos.z, eyepos.w);

	gl_PointSize = screenw * projCorner.x / projCorner.w;
	gl_Position = gl_ProjectionMatrix * eyepos;

	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
}
