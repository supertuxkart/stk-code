#version 130
uniform sampler2D tex;

noperspective in vec3 nor;
in vec2 uv;
out vec2 EncodedNormal;

// from Crytek "a bit more deferred CryEngine"
vec2 EncodeNormal(vec3 n)
{
	return normalize(n.xy) * sqrt(n.z * 0.5 + 0.5);
}

void main() {
	vec4 col = texture(tex, uv);
	if (col.a < 0.5)
		discard;
	EncodedNormal = 0.5 * EncodeNormal(normalize(nor)) + 0.5;
}

