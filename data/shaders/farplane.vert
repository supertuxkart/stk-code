void main() {
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	gl_Position = (gl_ModelViewProjectionMatrix * gl_Vertex).xyww;
	gl_FrontColor = gl_Color;
}
