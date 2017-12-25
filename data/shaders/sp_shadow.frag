out vec4 o_frag_color;

void main()
{
    o_frag_color = vec4(exp(32.0 * (2.0 * gl_FragCoord.z - 1.) /
        gl_FragCoord.w));
}
