#version 130
uniform sampler2D texture; //The texture
uniform sampler2D normalMap; //The bump-map

noperspective in vec3 tangent;
noperspective in vec3 bitangent;
noperspective in vec3 normal;

void main()
{
	// normal in Tangent Space
	vec3 TS_normal = 2.0 * texture2D (normalMap, gl_TexCoord[0].st).rgb - 1.0;
	// Because of interpolation, we need to renormalize
	vec3 Frag_tangent = normalize(tangent);
	vec3 Frag_bitangent = normalize(cross(normal, tangent));
	vec3 Frag_normal = cross(Frag_tangent, Frag_bitangent);


	vec3 FragmentNormal = TS_normal.x * Frag_tangent + TS_normal.y * Frag_bitangent + TS_normal.z * Frag_normal;
	FragmentNormal = normalize(FragmentNormal);
	

	gl_FragData[0] = texture2D (texture, gl_TexCoord[0].st);
	gl_FragData[1] = vec4(0.5 * FragmentNormal + 0.5, gl_FragCoord.z);
	gl_FragData[2] = vec4(0.);
}
