uniform vec3 windDir;
uniform mat4 ModelViewProjectionMatrix;
uniform mat4 TransposeInverseModelView;


#if __VERSION__ >= 330
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec4 Color;
layout(location = 3) in vec2 Texcoord;
#else
in vec3 Position;
in vec3 Normal;
in vec4 Color;
in vec2 Texcoord;
#endif

out vec3 nor;
out vec2 uv;

void main()
{
	uv = Texcoord;
	nor = (TransposeInverseModelView * vec4(Normal, 1.)).xyz;
	gl_Position = ModelViewProjectionMatrix * vec4(Position + windDir * Color.r, 1.);
}
