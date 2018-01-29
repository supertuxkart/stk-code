uniform sampler2D tex;
uniform float zn;
uniform float zf;

out float Depth;

void main()
{
    vec2 uv = gl_FragCoord.xy / u_screen;
    float d = texture(tex, uv).x;
    float c0 = zn * zf, c1 = zn - zf, c2 = zf;
    Depth = c0 / (d * c1 + c2);
}
