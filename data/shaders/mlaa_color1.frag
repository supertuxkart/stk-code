uniform sampler2D colorMapG;

in vec4 offset[2];
in vec2 uv;

const float threshold = 0.1;

out vec4 FragColor;

void main() {
	vec3 weights = vec3(0.2126,0.7152, 0.0722); // ITU-R BT. 709

	/**
	 * Luma calculation requires gamma-corrected colors:
	 */
	float L = dot(pow(texture(colorMapG, uv).rgb, vec3(1./2.2)), weights);
	float Lleft = dot(pow(texture(colorMapG, offset[0].xy).rgb, vec3(1./2.2)), weights);
	float Ltop = dot(pow(texture(colorMapG, offset[0].zw).rgb, vec3(1./2.2)), weights);
	float Lright = dot(pow(texture(colorMapG, offset[1].xy).rgb, vec3(1./2.2)), weights);
	float Lbottom = dot(pow(texture(colorMapG, offset[1].zw).rgb, vec3(1./2.2)), weights);

	vec4 delta = abs(vec4(L) - vec4(Lleft, Ltop, Lright, Lbottom));
	vec4 edges = step(vec4(threshold), delta);

	if (dot(edges, vec4(1.0)) == 0.0)
		discard;

	FragColor = edges;
}
