#version 130
uniform float far;
uniform float objectid;
uniform sampler2D tex;

noperspective in vec3 nor;

void main()
{
	vec4 color = texture2D(tex, gl_TexCoord[0].st);
	gl_FragData[0] = color;	
	gl_FragData[1] = vec4(0.5 * normalize(nor) + 0.5, gl_FragCoord.z);
}
