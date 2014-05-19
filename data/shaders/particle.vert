layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};
uniform vec3 color_from;
uniform vec3 color_to;

in vec2 quadcorner;
in vec2 texcoord;
in vec3 position;
in float lifetime;
in float size;

out float lf;
out vec2 tc;
out vec3 pc;

void main(void)
{
	tc = texcoord;
	lf = lifetime;
    pc = color_from + (color_to - color_from) * lifetime;
	vec3 newposition = position;

    vec4 viewpos = ViewMatrix * vec4(newposition, 1.0);
	viewpos += size * vec4(quadcorner, 0., 0.);
	gl_Position = ProjectionMatrix * viewpos;
}
