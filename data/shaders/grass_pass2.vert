#version 330
uniform vec3 windDir;
uniform mat4 ModelViewProjectionMatrix;

in vec3 Position;
in vec2 Texcoord;
in vec4 Color;
out vec2 uv;

void main()
{
	uv = Texcoord;
	gl_Position = ModelViewProjectionMatrix * vec4(Position + windDir * Color.r, 1.);
}
