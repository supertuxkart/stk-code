#version 130
in float lifetime;

out vec3 color;

void main(void)
{
    color = vec3(
		(lifetime < 33.) ? 1. : 0.,
		 (lifetime < 67. && lifetime >= 34.) ? 1. : 0.,
		 (lifetime > 68.) ? 1. : 0.);
}
