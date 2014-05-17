uniform sampler2D displacement_tex;
uniform sampler2D mask_tex;
uniform sampler2D color_tex;
uniform vec2 dir;
uniform vec2 dir2;

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
in vec2 uv_bis;
in float camdist;

out vec4 FragColor;
#else
varying vec2 uv;
varying vec2 uv_bis;
varying float camdist;
#define FragColor gl_FragColor
#endif

const float maxlen = 0.02;

void main()
{
	float horiz = texture(displacement_tex, uv + dir).x;
	float vert = texture(displacement_tex, (uv.yx + dir2) * vec2(0.9)).x;

	vec2 offset = vec2(horiz, vert);
	offset *= 2.0;
	offset -= 1.0;

	// Fade according to distance to cam
	float fade = 1.0 - smoothstep(1.0, 100.0, camdist);

	// Fade according to distance from the edges
	const float mindist = 0.1;
	fade *= smoothstep(0.0, mindist, uv_bis.x) * smoothstep(0.0, mindist, uv_bis.y) *
		(1.0 - smoothstep(1.0 - mindist, 1.0, uv_bis.x)) *
		(1.0 - smoothstep(1.0 - mindist, 1.0, uv_bis.y));

	offset *= 50.0 * fade * maxlen;

	vec4 shiftval;
	shiftval.r = step(offset.x, 0.0) * -offset.x;
	shiftval.g = step(0.0, offset.x) * offset.x;
	shiftval.b = step(offset.y, 0.0) * -offset.y;
	shiftval.a = step(0.0, offset.y) * offset.y;

	vec2 shift;
	shift.x = -shiftval.x + shiftval.y;
	shift.y = -shiftval.z + shiftval.w;
	shift /= 50.;

	vec2 tc = gl_FragCoord.xy / screen;
	float mask = texture(mask_tex, tc + shift).x;
	tc += (mask < 1.) ? vec2(0.) : shift;

	FragColor = texture(color_tex, tc);
}
