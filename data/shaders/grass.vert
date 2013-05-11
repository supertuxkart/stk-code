uniform float angle;
varying vec4 coord;


void main()
{
	gl_TexCoord[0] =  gl_MultiTexCoord0;
	vec4 vertexPosition = ftransform(); //gl_ModelViewMatrix *  gl_Vertex;
	vertexPosition += vec4(1,1,0,0) * 0.25 * gl_Color.r * sin(angle);
	gl_Position = vertexPosition;
	gl_FrontColor = vec4(1,1,1,1);
	gl_BackColor = vec4(1,1,1,1);
	coord = vertexPosition;
}