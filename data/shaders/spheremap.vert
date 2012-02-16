varying vec3 normal;
varying vec4 vertex_color;
varying vec3 eyeVec;
varying vec3 lightVec;
uniform vec3 lightdir;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = ftransform();
    vertex_color = gl_Color;
    
    //vec3 normal3 = normalize(gl_Normal);
    //vec4 normal4 = vec4(normal3.x, normal3.y, normal3.z, 0.0)*gl_ModelViewMatrix;
    //normal = normal4.xyz;
    
    eyeVec = normalize(-gl_Position).xyz; // we are in Eye Coordinates, so EyePos is (0,0,0)  
    normal = normalize(gl_NormalMatrix*gl_Normal);
    
    // Building the matrix Eye Space -> Tangent Space
    // gl_MultiTexCoord1.xyz
	vec3 t = normalize (gl_NormalMatrix * vec3(0.0, 0.0, 1.0)); // tangent
	vec3 b = cross (normal, t);
	    
	// transform light and half angle vectors by tangent basis
	vec3 v;
	v.x = dot(lightdir, t);
	v.y = dot(lightdir, b);
	v.z = dot(lightdir, normal);
	lightVec = normalize (v);
}
