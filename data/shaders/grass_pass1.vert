#version 330
uniform vec3 windDir;
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;

in vec3 Position;
in vec3 Normal;
in vec2 Texcoord;
in vec4 Color;
noperspective out vec3 nor;
out vec2 uv;

void main()
{
	uv = Texcoord;
	nor = (TransposeInverseModelView * vec4(Normal, 1.)).xyz;
	gl_Position = ModelViewProjectionMatrix * vec4(Position + windDir * Color.r, 1.);
}
