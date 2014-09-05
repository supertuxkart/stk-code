flat in vec4 glowColor;

out vec4 FragColor;

void main()
{
    FragColor = vec4(glowColor.rgb, 1.0);
}
