#version 130
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform float track_x;
uniform float track_z;
uniform float track_x_len;
uniform float track_z_len;
uniform samplerBuffer heightmap;
uniform bool hasHeightMap;

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
	vec3 newposition = position;
			
	float i_as_float = clamp(256. * (position.x - track_x) / track_x_len, 0., 255.);
	float j_as_float = clamp(256. * (position.z - track_z) / track_z_len, 0., 255.);
	int i = int(i_as_float);
	int j = int(j_as_float);

	if (hasHeightMap) {
		float h = position.y - texelFetch(heightmap, i * 256 + j).r;
		newposition.y = (h > 0.)? position.y : position.y - h;
	}

    vec4 viewpos = ViewMatrix * vec4(newposition, 1.0);
	viewpos += size * vec4(quadcorner, 0., 0.);
	gl_Position = ProjectionMatrix * viewpos;
}
