

uniform sampler2D main_texture;
varying vec2 uv;

void main()
{
    gl_FragColor = texture2D(main_texture, uv);
}
