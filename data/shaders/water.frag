// Shader based on work by Fabien Sanglard
// Released under the terms of CC-BY 3.0

uniform sampler2D BumpTex1; // Normal map 1
uniform sampler2D BumpTex2; // Normal map 2
uniform sampler2D DecalTex; //The texture

uniform vec2 delta1;
uniform vec2 delta2;

varying vec3 lightVec;
varying vec3 halfVec;
varying vec3 eyeVec;

void main()
{
	// lookup normal from normal map, move from [0,1] to  [-1, 1] range, normalize
	vec3 normal  = 2.0 * texture2D (BumpTex1, gl_TexCoord[0].st + delta1).rgb - 1.0;
    vec3 normal2 = 2.0 * texture2D (BumpTex2, gl_TexCoord[0].st + delta2).rgb - 1.0;
    
    // scale normals
    //normal.y = 4.0*normal.y;
    //normal2.y = 4.0*normal2.y;
    
	normal = (normalize(normal) + normalize(normal2))/2.0;
	    
	// compute diffuse lighting
	float lamberFactor = max (dot (lightVec, normal), 0.0);
	vec4 diffuseMaterial;
	vec4 diffuseLight;

    diffuseMaterial = texture2D (DecalTex, gl_TexCoord[0].st + vec2(delta1.x, 0.0));
    diffuseLight  = vec4(1.0, 1.0, 1.0, 1.0);

    //if (lamberFactor < 0.7)
    //{
    //    lamberFactor = 0.0;
    //}

    gl_FragColor =	diffuseMaterial * (lamberFactor);
    
    // specular (phong)
    vec3 R = normalize(reflect(lightVec, normal));
    float specular = max(dot(R,eyeVec),0.0);
    
    if (specular > 0.0)
    {
        // weak specular
        specular = specular*specular;
        float specular03 = specular*0.2;
        gl_FragColor += vec4(specular03, specular03, specular03, 0.0);
        
        // strong specular
        specular = specular*specular*specular;
        float specular06 = specular*0.5;
        gl_FragColor += vec4(specular06, specular06, specular06, 0.0);
    }
}
