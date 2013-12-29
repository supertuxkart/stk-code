#version 130
uniform sampler2D texture;
out vec4 color;

void main(void)
{
    color = texture2D(texture, gl_PointCoord.xy);
}
