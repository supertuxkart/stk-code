out vec4 FragColor;

void main()
{
    FragColor = vec4(exp(32. * (2. * gl_FragCoord.z - 1.) / gl_FragCoord.w));
}
