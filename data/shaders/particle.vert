#version 130
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

in vec2 quadcorner;
in vec2 texcoord;
in vec3 position;
in float lifetime;
in float size;

out float lf;
out vec2 tc;

void main(void)
{
	tc = texcoord;
	lf = lifetime;
    vec4 viewpos = ViewMatrix * vec4(position, 1.0);
	viewpos += size * vec4(quadcorner, 0., 0.);
	gl_Position = ProjectionMatrix * viewpos;
}
