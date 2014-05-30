uniform sampler3D SHR;
uniform sampler3D SHG;
uniform sampler3D SHB;

in vec3 uvw;
out vec4 FragColor;

void main()
{
    float r = texture(SHR, uvw).w;
    float g = texture(SHG, uvw).w;
    float b = texture(SHB, uvw).w;
    FragColor = max(vec4(r, g, b, 1.0), vec4(0.));
}
