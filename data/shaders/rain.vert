#version 330
uniform float screenw;

void main()
{
	const float size = 0.5;

	vec4 eyepos = gl_Vertex;
	vec4 projCorner = gl_ProjectionMatrix * vec4(vec2(size), eyepos.z, eyepos.w);

	gl_PointSize = screenw * projCorner.x / projCorner.w;
	gl_Position = gl_ProjectionMatrix * eyepos;
}
