// motion_blur.vert

void main()
{
	gl_TexCoord[0].st = vec2(gl_MultiTexCoord0.s, 1.0-gl_MultiTexCoord0.t);
	gl_Position = gl_Vertex;
}
