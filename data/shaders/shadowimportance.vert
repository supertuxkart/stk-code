#version 130
uniform sampler2D dtex;
uniform mat4 ipvmat;
uniform mat4 shadowmat;

varying vec3 wpos;
varying vec2 texc;

float decdepth(vec4 rgba) {
	return dot(rgba, vec4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/16581375.0));
}

void main()
{
	texc = gl_Vertex.xy / vec2(32767.0);
	float z = decdepth(vec4(texture2D(dtex, texc).xyz, 0.0));

	vec3 tmp = vec3(texc, z);
	tmp = tmp * 2.0 - 1.0;

	vec4 xpos = vec4(tmp, 1.0);
	xpos = ipvmat * xpos;
	xpos.xyz /= xpos.w;

	wpos = xpos.xyz;

	// Now we have this pixel's world-space position. Convert to shadow space.
	vec4 pos = shadowmat * vec4(xpos.xyz, 1.0);

	gl_Position = pos;
}
