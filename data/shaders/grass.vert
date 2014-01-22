#version 130
uniform vec3 windDir;
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;

noperspective out vec3 nor;
out vec2 uv;

void main()
{
	uv = gl_MultiTexCoord0.st;

	vec4 vertexPosition = gl_Vertex;
	vertexPosition.xyz += windDir * gl_Color.r;

	nor = (TransposeInverseModelView * vec4(gl_Normal, 1.)).xyz;
	gl_Position = ModelViewProjectionMatrix * vertexPosition;
}
