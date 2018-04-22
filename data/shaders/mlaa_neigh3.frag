uniform sampler2D blendMap;
uniform sampler2D colorMap;

out vec4 FragColor;

void main() {
	vec2 uv = gl_FragCoord.xy / u_screen;
	vec2 uv_left = uv + vec2(-1., 0.) / u_screen;
	vec2 uv_top = uv + vec2(0., 1.) / u_screen;
	vec2 uv_right = uv + vec2(1., 0.) / u_screen;
	vec2 uv_bottom = uv + vec2(0., -1.) / u_screen;

	// Fetch the blending weights for current pixel:
	vec4 topLeft = texture(blendMap, uv);
	float bottom = texture(blendMap, uv_bottom).g;
	float right = texture(blendMap, uv_right).a;
	vec4 a = vec4(topLeft.r, bottom, topLeft.b, right);

	// Up to 4 lines can be crossing a pixel (one in each edge). So, we perform
	// a weighted average, where the weight of each line is 'a' cubed, which
	// favors blending and works well in practice.
	vec4 w = a * a * a;

	// There is some blending weight with a value greater than 0.0?
	float sum = dot(w, vec4(1.0));
	if (sum < 1e-5)
		discard;

	vec4 color = vec4(0.0);

	// Add the contributions of the possible 4 lines that can cross this pixel:
	vec4 C = texture(colorMap, uv);
	vec4 Cleft = texture(colorMap, uv_left);
	vec4 Ctop = texture(colorMap, uv_top);
	vec4 Cright = texture(colorMap, uv_right);
	vec4 Cbottom = texture(colorMap, uv_bottom);
	color = mix(C, Ctop, a.r) * w.r + color;
	color = mix(C, Cbottom, a.g) * w.g + color;
	color = mix(C, Cleft, a.b) * w.b + color;
	color = mix(C, Cright, a.a) * w.a + color;

	// Normalize the resulting color and we are finished!
	FragColor = vec4(color / sum);
}
