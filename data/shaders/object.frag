#version 130
uniform sampler2D texture;
in vec2 uv;
noperspective in vec3 nor;

void main(void)
{
    vec4 tex = texture2D(texture, uv);
    gl_FragData[0] = vec4(tex.xyz, 1.);
    gl_FragData[1] = vec4(0.5 * normalize(nor) + 0.5, gl_FragCoord.z);
    gl_FragData[2] = vec4(1. - tex.a);
}
