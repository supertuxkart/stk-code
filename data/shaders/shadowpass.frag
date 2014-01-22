#version 130
uniform sampler2D tex;
uniform int hastex;
uniform int viz;
uniform int wireframe;
uniform float objectid;

in vec2 uv;
out vec4 FragColor;

vec4 encdepth(float v) {
	vec4 enc = vec4(1.0, 255.0, 65025.0, 16581375.0) * v;
	enc = fract(enc);
	enc -= enc.yzww * vec4(1.0/255.0, 1.0/255.0, 1.0/255.0, 0.0);
	return enc;
}

void main() {

	if (hastex != 0) {
		float alpha = texture(tex, uv).a;

		if (alpha < 0.5)
			discard;
	}

	if (viz < 1)
	{
		FragColor = vec4(encdepth(gl_FragCoord.z).xyz, objectid);
	}
	else {
		if (wireframe > 0)
			FragColor = vec4(1.0);
		else
			FragColor = texture(tex, uv);
	}
}

