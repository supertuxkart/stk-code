#version 130
uniform float far;
uniform float objectid;
uniform sampler2D tex;

noperspective in vec3 nor;
in vec2 uv;
out vec4 Albedo;
out vec4 NormalDepth;
out vec4 Specular;

void main()
{
	Albedo = texture(tex, uv);
	NormalDepth = vec4(0.5 * normalize(nor) + 0.5, gl_FragCoord.z);
	Specular = vec4(0.);
}
