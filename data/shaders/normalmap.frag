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
	float lamberFactor= max (dot (lightVec, normal), 0.0) ;
	vec4 diffuseMaterial;
	vec4 diffuseLight;
	
	// compute specular lighting
	vec4 specularMaterial ;
	vec4 specularLight ;
	float shininess ;
  
	// compute ambient
	vec4 ambientLight = vec4(0.5, 0.5, 0.5, 1.0);	
	
	if (lamberFactor > 0.0)
	{
		diffuseMaterial = texture2D (DecalTex, gl_TexCoord[0].st);
		diffuseLight  = vec4(0.5, 0.5, 0.5, 1.0);

		gl_FragColor =	diffuseMaterial * diffuseLight * lamberFactor ;	
	}
	
	gl_FragColor +=	ambientLight;
	
}			
