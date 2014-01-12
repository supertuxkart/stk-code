#version 130
uniform sampler2D texture;
in vec2 uv;

void main(void)
{
    gl_FragData[0] = texture2D(texture, uv);
    gl_FragData[1] = vec4(0., 0., 0., 1.);
    gl_FragData[2] = vec4(0., 0., 0., 1.);
}
