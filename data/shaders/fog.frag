uniform sampler2D tex;

uniform float fogmax;
uniform float startH;
uniform float endH;
uniform float start;
uniform float end;
uniform vec3 col;

#ifdef UBO_DISABLED
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 InverseViewMatrix;
uniform mat4 InverseProjectionMatrix;
#else
layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};
#endif

in vec2 uv;
out vec4 FragColor;


vec4 getPosFromUVDepth(vec3 uvDepth, mat4 InverseProjectionMatrix);

void main()
{
	float z = texture(tex, uv).x;
	vec4 xpos = getPosFromUVDepth(vec3(uv, z), InverseProjectionMatrix);

	float dist = length(xpos.xyz);
	float fog = smoothstep(start, end, dist);

	fog = min(fog, fogmax);

	FragColor = vec4(col, fog);
}
