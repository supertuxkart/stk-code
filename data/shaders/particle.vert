#version 140
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform float track_x;
uniform float track_z;
uniform float track_x_len;
uniform float track_z_len;
uniform samplerBuffer heightmap;
uniform bool hasHeightMap;
uniform bool flips;

in vec2 quadcorner;
in vec2 texcoord;
in vec3 position;
in float lifetime;
in float size;

in vec3 rotationvec;
in float anglespeed;

out float lf;
out vec2 tc;

void main(void)
{
	tc = texcoord;
	lf = lifetime;
	vec3 newposition = position;
			
	float i_as_float = clamp(256. * (position.x - track_x) / track_x_len, 0., 255.);
	float j_as_float = clamp(256. * (position.z - track_z) / track_z_len, 0., 255.);
	int i = int(i_as_float);
	int j = int(j_as_float);

	if (hasHeightMap) {
		float h = position.y - texelFetch(heightmap, i * 256 + j).r;
		newposition.y = (h > 0.)? position.y : position.y - h;
	}

	if (flips)
	{
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
	} else {
	    vec4 viewpos = ViewMatrix * vec4(newposition, 1.0);
		viewpos += size * vec4(quadcorner, 0., 0.);
		gl_Position = ProjectionMatrix * viewpos;
	}


}
