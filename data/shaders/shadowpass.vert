#version 330 compatibility
uniform sampler2D warpx;
uniform sampler2D warpy;

out vec2 uv;

float decdepth(vec4 rgba) {
	return dot(rgba, vec4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/16581375.0));
}

void main()
{
	vec4 pos = gl_ModelViewProjectionMatrix * gl_Vertex;
	uv = gl_MultiTexCoord0.xy;

	vec2 tc = pos.xy * vec2(0.5) + vec2(0.5);

	float movex = decdepth(texture(warpx, tc));
	float movey = decdepth(texture(warpy, tc));

	float dx = movex * 2.0 - 1.0;
	float dy = movey * 2.0 - 1.0;

	dx *= 2.0;
	dy *= 2.0;

	gl_Position = pos + vec4(dx, dy, vec2(0.0));
}
