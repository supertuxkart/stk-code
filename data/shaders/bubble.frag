

uniform sampler2D main_texture;
uniform float transparency;
varying vec2 uv;

void main()
{
    gl_FragColor = texture2D(main_texture, uv);
    gl_FragColor.a *= transparency;
}
