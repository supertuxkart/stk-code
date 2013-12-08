// Shader based on work by Fabien Sanglard
// Released under the terms of CC-BY 3.0

uniform float speed;
uniform float height;
uniform float waveLength;

uniform vec3 lightdir;

varying vec3 lightVec;
varying vec3 halfVec;
varying vec3 eyeVec;

void main()
{
	vec4 pos = gl_Vertex;

	pos.y += (sin(pos.x/waveLength + speed) + cos(pos.z/waveLength + speed)) * height;

	vec3 vertexPosition = vec3(gl_ModelViewMatrix * pos);

	// Building the matrix Eye Space -> Tangent Space
	vec3 n = normalize (gl_NormalMatrix * gl_Normal);
	// gl_MultiTexCoord1.xyz
	vec3 t = normalize (gl_NormalMatrix * vec3(1.0, 0.0, 0.0)); // tangent
	vec3 b = cross (n, t);

	// transform light and half angle vectors by tangent basis
	vec3 v;
	v.x = dot (lightdir, t);
	v.y = dot (lightdir, b);
	v.z = dot (lightdir, n);
	lightVec = normalize (v);

	vertexPosition = normalize(vertexPosition);

	eyeVec = normalize(-vertexPosition); // we are in Eye Coordinates, so EyePos is (0,0,0)

	// Normalize the halfVector to pass it to the fragment shader

	// No need to divide by two, the result is normalized anyway.
	// vec3 halfVector = normalize((vertexPosition + lightDir) / 2.0);
	vec3 halfVector = normalize(vertexPosition + lightdir);
	v.x = dot (halfVector, t);
	v.y = dot (halfVector, b);
	v.z = dot (halfVector, n);

	// No need to normalize, t,b,n and halfVector are normal vectors.
	//normalize (v);
	halfVec = v ;

	gl_Position = gl_ModelViewProjectionMatrix * pos;
	gl_TexCoord[0] =  gl_MultiTexCoord0;
	gl_TexCoord[1] =  gl_MultiTexCoord1;
}
