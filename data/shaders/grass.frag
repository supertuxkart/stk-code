#version 130
uniform float far;
uniform float objectid;
uniform sampler2D tex;

noperspective in vec3 nor;

void main()
{
	gl_FragData[0] = texture2D(tex, gl_TexCoord[0].st);
	gl_FragData[1] = vec4(0.5 * normalize(nor) + 0.5, gl_FragCoord.z);
	gl_FragData[2] = vec4(1.);
}
