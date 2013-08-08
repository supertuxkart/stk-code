// Jean-manuel clemencon (c) supertuxkart 2013
// Creates a bubble gum shield effect
// ---
// TODO: The texture should reflect the strength of the shield, 
// such that the user gets to know whether the shield has several 
// "layers" or whether the shield is about to break. 

varying vec2 uv;
varying vec3 eyeVec;
varying vec3 normal;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	vec4 viewp = gl_ModelViewMatrix * gl_Vertex;

	eyeVec = normalize(-viewp).xyz;
	normal = gl_NormalMatrix * gl_Normal;

	gl_Position = ftransform();

	uv = gl_TexCoord[0].st;
}
