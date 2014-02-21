#version 330 compatibility
uniform sampler2D tex;
uniform sampler2D normals_and_depth;
uniform mat4 invproj;
uniform vec2 screen;

out vec4 FragColor;

void main()
{
	vec2 xy = gl_FragCoord.xy / screen;
	float FragZ = gl_FragCoord.z;
	float EnvZ = texture(normals_and_depth, xy).a;
	vec4 FragmentPos = invproj * (2. * vec4(xy, FragZ, 1.0) - 1.);
	FragmentPos /= FragmentPos.w;
	vec4 EnvPos = invproj * (2. * vec4(xy, EnvZ, 1.0) - 1.);
	EnvPos /= EnvPos.w;
	float len = dot(vec3(1.0), abs(texture(normals_and_depth, xy).xyz));
	float alpha = (len < 0.2) ? 1. : clamp((EnvPos.z - FragmentPos.z) * 0.3, 0., 1.);
	FragColor = texture(tex, gl_PointCoord.xy);
	FragColor.a *= alpha;
}
