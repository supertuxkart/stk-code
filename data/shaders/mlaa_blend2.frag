#version 330 compatibility
#define MAX_SEARCH_STEPS 8.0
#define MAX_DISTANCE 33.0

#extension GL_ARB_shader_texture_lod: enable

uniform sampler2D edgesMap;
uniform sampler2D areaMap;

uniform vec2 PIXEL_SIZE;

out vec4 FragColor;

/**
 * This one just returns the first level of a mip map chain, which allow us to
 * avoid the nasty ddx/ddy warnings, even improving the performance a little
 * bit.
 */
vec4 tex2Doffset(sampler2D map, vec2 texcoord, vec2 offset) {
	return texture2DLod(map, texcoord + PIXEL_SIZE * offset, 0.0);
}

float SearchXLeft(vec2 texcoord) {
	// We compare with 0.9 to prevent bilinear access precision problems.
	float i;
	float e = 0.0;
	for (i = -1.5; i > -2.0 * MAX_SEARCH_STEPS; i -= 2.0) {
		e = tex2Doffset(edgesMap, texcoord, vec2(i, 0.0)).g;
		if (e < 0.9) break;
	}
	return max(i + 1.5 - 2.0 * e, -2.0 * MAX_SEARCH_STEPS);
}

float SearchXRight(vec2 texcoord) {
	float i;
	float e = 0.0;
	for (i = 1.5; i < 2.0 * MAX_SEARCH_STEPS; i += 2.0) {
		e = tex2Doffset(edgesMap, texcoord, vec2(i, 0.0)).g;
		if (e < 0.9) break;
	}
	return min(i - 1.5 + 2.0 * e, 2.0 * MAX_SEARCH_STEPS);
}

float SearchYDown(vec2 texcoord) {
	float i;
	float e = 0.0;
	for (i = -1.5; i > -2.0 * MAX_SEARCH_STEPS; i -= 2.0) {
		e = tex2Doffset(edgesMap, texcoord, vec2(i, 0.0).yx).r;
		if (e < 0.9) break;
	}
	return max(i + 1.5 - 2.0 * e, -2.0 * MAX_SEARCH_STEPS);
}

float SearchYUp(vec2 texcoord) {
	float i;
	float e = 0.0;
	for (i = 1.5; i < 2.0 * MAX_SEARCH_STEPS; i += 2.0) {
		e = tex2Doffset(edgesMap, texcoord, vec2(i, 0.0).yx).r;
		if (e < 0.9) break;
	}
	return min(i - 1.5 + 2.0 * e, 2.0 * MAX_SEARCH_STEPS);
}


vec2 round(vec2 invec) {
	return vec2(floor(abs(invec) + vec2(0.5)) * sign(invec));
}

vec2 Area(vec2 distance, float e1, float e2) {
	// * By dividing by areaSize - 1.0 below we are implicitely offsetting to
	//   always fall inside of a pixel
	// * Rounding prevents bilinear access precision problems
	float areaSize = MAX_DISTANCE * 5.0;
	vec2 pixcoord = MAX_DISTANCE * round(4.0 * vec2(e1, e2)) + distance;
	vec2 texcoord = pixcoord / (areaSize - 1.0);
	return texture2DLod(areaMap, texcoord, 0.0).ra;
}

void main() {
	vec4 areas = vec4(0.0);

	vec2 e = texture(edgesMap, gl_TexCoord[0].xy).rg;

	if (e.g != 0.0) { // Edge at north

		// Search distances to the left and to the right:
		vec2 d = vec2(SearchXLeft(gl_TexCoord[0].xy), SearchXRight(gl_TexCoord[0].xy));

		// Now fetch the crossing edges. Instead of sampling between edgels, we
		// sample at 0.25, to be able to discern what value has each edgel:
		vec4 coords = vec4(d.x, 0.25, d.y + 1.0, 0.25) * PIXEL_SIZE.xyxy + gl_TexCoord[0].xyxy;
		float e1 = texture2DLod(edgesMap, coords.xy, 0.0).r;
		float e2 = texture2DLod(edgesMap, coords.zw, 0.0).r;

		// Ok, we know how this pattern looks like, now it is time for getting
		// the actual area:
		areas.rg = Area(abs(d), e1, e2);
	}

	if (e.r != 0.0) { // Edge at west

		// Search distances to the top and to the bottom:
		vec2 d = vec2(SearchYUp(gl_TexCoord[0].xy), SearchYDown(gl_TexCoord[0].xy));

		// Now fetch the crossing edges (yet again):
		vec4 coords = vec4(-0.25, d.x, -0.25, d.y - 1.0) * PIXEL_SIZE.xyxy + gl_TexCoord[0].xyxy;
		float e1 = texture2DLod(edgesMap, coords.xy, 0.0).g;
		float e2 = texture2DLod(edgesMap, coords.zw, 0.0).g;

		// Get the area for this direction:
		areas.ba = Area(abs(d), e1, e2);
	}

	FragColor = areas;
}
