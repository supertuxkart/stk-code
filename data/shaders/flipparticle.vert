layout (std140) uniform MatrixesData
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    mat4 InverseViewMatrix;
    mat4 InverseProjectionMatrix;
    mat4 ShadowViewProjMatrixes[4];
    vec2 screen;
};

in vec2 quadcorner;
in vec2 texcoord;
in vec3 position;
in float lifetime;
in float size;

in vec3 rotationvec;
in float anglespeed;

out float lf;
out vec2 tc;
out vec3 pc;

void main(void)
{
	tc = texcoord;
	lf = lifetime;
	vec3 newposition = position;

	// from http://jeux.developpez.com/faq/math
	float angle = lf * anglespeed;
	float sin_a = sin(angle / 2.);
	float cos_a = cos(angle / 2.);

	vec4 quaternion = normalize(vec4(rotationvec * sin_a, cos_a));
	float xx      = quaternion.x * quaternion.x;
	float xy      = quaternion.x * quaternion.y;
	float xz      = quaternion.x * quaternion.z;
	float xw      = quaternion.x * quaternion.w;
	float yy      = quaternion.y * quaternion.y;
	float yz      = quaternion.y * quaternion.z;
	float yw      = quaternion.y * quaternion.w;
	float zz      = quaternion.z * quaternion.z;
	float zw      = quaternion.z * quaternion.w;

	vec4 col1 = vec4(
		1. - 2. * ( yy + zz ),
		2. * ( xy + zw ),
		2. * ( xz - yw ),
		0.);
	vec4 col2 = vec4(
		2. * ( xy - zw ),
		1. - 2. * ( xx + zz ),
		2. * ( yz + xw ),
		0.);
	vec4 col3 = vec4(
		2. * ( xz + yw ),
		2. * ( yz - xw ),
		1. - 2. * ( xx + yy ),
		0.);
	vec4 col4 = vec4(0., 0., 0., 1.);
	mat4 rotationMatrix = mat4(col1, col2, col3, col4);
	vec3 newquadcorner = size * vec3(quadcorner, 0.);
	newquadcorner = (rotationMatrix * vec4(newquadcorner, 0.)).xyz;

	vec4 viewpos = ViewMatrix * vec4(newposition + newquadcorner, 1.0);
	gl_Position = ProjectionMatrix * viewpos;
	pc = vec3(1.);
}
