/*--- GENERIC HEADER ---*/

varying vec3 nor;
uniform mat4 invtworldm;


/*--- END OF GENERIC HEADER --*/

uniform vec3 windDir;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;

	vec4 vertexPosition = gl_Vertex;
	vertexPosition.xyz += windDir * gl_Color.r;

	nor = (invtworldm * vec4(gl_Normal, 0.0)).xyz;
	nor = normalize(nor);
	nor = nor * 0.5 + 0.5;

	gl_Position = gl_ModelViewProjectionMatrix * vertexPosition;
	
	
}
