uniform sampler2D tex;

uniform float fogmax;
uniform float startH;
uniform float endH;
uniform float start;
uniform float end;
uniform vec3 col;


layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};

#if __VERSION__ >= 130
in vec2 uv;
in vec4 color;
out vec4 FragColor;
#else
varying vec2 uv;
#define FragColor gl_FragColor
#endif


void main()
{
	vec4 diffusecolor = texture(tex, uv) * pow(color, vec4(2.2));
	vec3 tmp = vec3(gl_FragCoord.xy / screen, gl_FragCoord.z);
	tmp = 2. * tmp - 1.;

	vec4 xpos = vec4(tmp, 1.0);
	xpos = InverseProjectionMatrix * xpos;
	xpos.xyz /= xpos.w;

	float dist = length(xpos.xyz);
	float fog = smoothstep(start, end, dist);

	fog = min(fog, fogmax);

    vec4 color = vec4(vec4(col, 0.) * fog + diffusecolor *(1. - fog));
    FragColor = vec4(color.rgb * color.a, color.a);
}
