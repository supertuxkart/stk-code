#version 130
uniform sampler2D normalMap;

noperspective in vec3 tangent;
noperspective in vec3 bitangent;
in vec2 uv;
out vec4 FragColor;

void main()
{
	// normal in Tangent Space
	vec3 TS_normal = 2.0 * texture (normalMap, uv).rgb - 1.0;
	// Because of interpolation, we need to renormalize
	vec3 Frag_tangent = normalize(tangent);
	vec3 Frag_normal = normalize(cross(Frag_tangent, bitangent));
	vec3 Frag_bitangent = cross(Frag_normal, Frag_tangent);

	vec3 FragmentNormal = TS_normal.x * Frag_tangent + TS_normal.y * Frag_bitangent - TS_normal.z * Frag_normal;
	FragmentNormal = normalize(FragmentNormal);
	
	FragColor = vec4(0.5 * FragmentNormal + 0.5, gl_FragCoord.z);
}
