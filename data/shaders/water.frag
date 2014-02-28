// Shader based on work by Fabien Sanglard
// Released under the terms of CC-BY 3.0
#version 330 compatibility
uniform sampler2D BumpTex1; // Normal map 1
uniform sampler2D BumpTex2; // Normal map 2
uniform sampler2D DecalTex; //The texture

uniform vec2 delta1;
uniform vec2 delta2;

in vec3 lightVec;
in vec3 halfVec;
in vec3 eyeVec;
in vec2 uv;
out vec4 FragColor;

void main()
{
	// lookup normal from normal map, move from [0,1] to  [-1, 1] range, normalize
	vec3 normal  = 2.0 * texture (BumpTex1, uv + delta1).rgb - 1.0;
	vec3 normal2 = 2.0 * texture (BumpTex2, uv + delta2).rgb - 1.0;

	// scale normals
	normal.y = 4.0*normal.y;
	normal2.y = 4.0*normal2.y;

	normal = (normalize(normal) + normalize(normal2))/2.0;

	// compute diffuse lighting
	float lamberFactor = max (dot (lightVec, normal), 0.0);
	vec4 diffuseMaterial;
	vec4 diffuseLight;

	diffuseMaterial = texture (DecalTex, uv + vec2(delta1.x, 0.0));
	diffuseLight  = vec4(1.0, 1.0, 1.0, 1.0);

	vec3 col = diffuseMaterial.xyz * (0.3 + lamberFactor*0.7);

	// specular (phong)
	vec3 R = normalize(reflect(lightVec, normal));
	float specular = max(dot(R, eyeVec), 0.0);

	// weak specular
	specular = specular*specular;
	specular = specular*specular;
	float specular_weak = specular*0.05;
	col += vec3(specular_weak, specular_weak, specular_weak);

	// strong specular
	specular = specular*specular;
	float specular_strong = specular*0.3;
	col += vec3(specular_strong, specular_strong, specular_strong);

	float summed = dot(vec3(1.0), col) / 3.0;
	float alpha = 0.9 + 0.1 * smoothstep(0.0, 1.0, summed);

	FragColor = vec4(col, alpha);
}
