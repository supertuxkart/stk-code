varying vec3 normal;
varying vec4 vertex_color;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = ftransform();
    vertex_color = gl_Color;
    
    //vec3 normal3 = normalize(gl_Normal);
    //vec4 normal4 = vec4(normal3.x, normal3.y, normal3.z, 0.0)*gl_ModelViewMatrix;
    //normal = normal4.xyz;
    
    normal = normalize(gl_NormalMatrix*gl_Normal);
}
