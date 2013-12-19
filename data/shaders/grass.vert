#version 130
uniform vec3 windDir;

noperspective out vec3 nor;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;

	vec4 vertexPosition = gl_Vertex;
	vertexPosition.xyz += windDir * gl_Color.r;

	nor = gl_NormalMatrix * gl_Normal;
	gl_Position = gl_ModelViewProjectionMatrix * vertexPosition;
}
