uniform sampler2D colorMapG;

const float threshold = 0.1;

out vec4 FragColor;

void main() {
	vec3 weights = vec3(0.2126,0.7152, 0.0722); // ITU-R BT. 709

	vec2 uv = gl_FragCoord.xy / screen;
	vec2 uv_left = uv + vec2(-1., 0.) / screen;
	vec2 uv_top = uv + vec2(0., 1.) / screen;
	vec2 uv_right = uv + vec2(1., 0.) / screen;
	vec2 uv_bottom = uv + vec2(0., -1.) / screen;

	/**
	 * Luma calculation requires gamma-corrected colors:
	 */
	float L = dot(texture(colorMapG, uv).rgb, weights);
	float Lleft = dot(texture(colorMapG, uv_left).rgb, weights);
	float Ltop = dot(texture(colorMapG, uv_top).rgb, weights);
	float Lright = dot(texture(colorMapG, uv_right).rgb, weights);
	float Lbottom = dot(texture(colorMapG, uv_bottom).rgb, weights);

	vec4 delta = abs(vec4(L) - vec4(Lleft, Ltop, Lright, Lbottom));
	vec4 edges = step(vec4(threshold), delta);

	if (dot(edges, vec4(1.0)) == 0.0)
		discard;

	FragColor = edges;
}
