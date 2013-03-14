// motion_blur.vert

void main()
{
	gl_TexCoord[0].st = vec2(gl_MultiTexCoord0.s, gl_MultiTexCoord0.t);
	gl_Position = gl_Vertex;
}
