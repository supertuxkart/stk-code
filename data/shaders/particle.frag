uniform sampler2D tex;
uniform sampler2D dtex;
uniform mat4 invproj;

layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};

in float lf;
in vec2 tc;
in vec3 pc;
out vec4 FragColor;


void main(void)
{
	vec2 xy = gl_FragCoord.xy / screen;
	float FragZ = gl_FragCoord.z;
	float EnvZ = texture(dtex, xy).x;
	vec4 FragmentPos = invproj * (2. * vec4(xy, FragZ, 1.0) - 1.);
	FragmentPos /= FragmentPos.w;
	vec4 EnvPos = invproj * (2. * vec4(xy, EnvZ, 1.0) - 1.);
	EnvPos /= EnvPos.w;
	float alpha = clamp((EnvPos.z - FragmentPos.z) * 0.3, 0., 1.);
    vec4 color = texture(tex, tc) * vec4(pc, 1.0);
    FragColor = color * alpha * smoothstep(1., 0.8, lf);
}
