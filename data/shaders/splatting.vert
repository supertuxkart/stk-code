varying vec3 normal;
varying vec4 vertex_color;
varying vec3 lightdir2;
uniform vec3 lightdir;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_MultiTexCoord1;
    gl_Position = ftransform();
    vertex_color = gl_Color;
    
    //normal = normalize(gl_NormalMatrix * gl_Normal);
    normal = normalize(gl_Normal);
    lightdir2 = normalize(lightdir);
}
