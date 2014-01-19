#version 130
uniform sampler2D halft; // half is a reserved word
uniform sampler2D quarter;
uniform sampler2D eighth;
out vec4 FragColor;

void main()
{
	vec3 val[3];
	val[0] = texture(halft, gl_TexCoord[0].xy).xyz;
	val[1] = texture(quarter, gl_TexCoord[0].xy).xyz;
	val[2] = texture(eighth, gl_TexCoord[0].xy).xyz;

	// Find the first level with a penumbra value
	int i;
	float q = 0.0;
	float outval = 1.0;

	float hasshadow = dot(vec3(1.0), vec3(val[0].z, val[1].z, val[2].z));

	if (hasshadow > 0.9)
	{
		for (i = 0; i < 3; i++)
		{
			if (val[i].z > 0.9)
			{
				q = val[i].y;
				break;
			}
		}

		q *= 8.0;
		q = max(1.0, q);
		q = log2(q);
		q = min(1.9, q);

		// q is now between 0 and 1.9.
		int down = int(floor(q));
		int up = down + 1;
		float interp = q - float(down);

		outval = 1.0 - mix(val[down].x, val[up].x, interp);
	}

	FragColor = vec4(vec3(outval), 1.0);
}
