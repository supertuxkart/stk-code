// Shader based on work by Fabien Sanglard
// Released under the terms of CC-BY 3.0

uniform sampler2D BumpTex; //The bump-map 
uniform sampler2D DecalTex; //The texture
 
// New bumpmapping
varying vec3 lightVec;
varying vec3 halfVec;
varying vec3 eyeVec;


void main()
{
	// lookup normal from normal map, move from [0,1] to  [-1, 1] range, normalize
	vec3 normal = 2.0 * texture2D (BumpTex, gl_TexCoord[0].st).rgb - 1.0;
	normal = normalize (normal);
	
	// compute diffuse lighting
	float lamberFactor = max (dot (lightVec, normal), 0.0) ;
	vec4 diffuseMaterial;
  
    diffuseMaterial = texture2D (DecalTex, gl_TexCoord[0].st);

    // 0.4 is the ambient light
    gl_FragColor =	diffuseMaterial * (0.5 + lamberFactor*0.5);
}			
