#version 330 compatibility
in vec4 offset[2];
in vec2 uv;

uniform sampler2D colorMapG;
const float threshold = 0.1;

out vec4 FragColor;

void main() {
	vec3 weights = vec3(0.2126,0.7152, 0.0722); // ITU-R BT. 709

	/**
	 * Luma calculation requires gamma-corrected colors:
	 */
	float L = dot(texture(colorMapG, uv).rgb, weights);
	float Lleft = dot(texture(colorMapG, offset[0].xy).rgb, weights);
	float Ltop = dot(texture(colorMapG, offset[0].zw).rgb, weights);
	float Lright = dot(texture(colorMapG, offset[1].xy).rgb, weights);
	float Lbottom = dot(texture(colorMapG, offset[1].zw).rgb, weights);

	vec4 delta = abs(vec4(L) - vec4(Lleft, Ltop, Lright, Lbottom));
	vec4 edges = step(vec4(threshold), delta);

	if (dot(edges, vec4(1.0)) == 0.0)
		discard;

	FragColor = edges;
}
